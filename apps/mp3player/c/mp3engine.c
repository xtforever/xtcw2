/*
 * mp3engine.c — in-process DSP engine for the XTCW mp3player.
 *
 * libmpg123 decode -> 5-band biquad EQ -> libpulse-simple output.
 * A worker thread keeps audio out of the LUI event loop; Lua reads a
 * mutex-protected status snapshot at ~25 fps.
 */

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#if defined(__has_include)
# if __has_include(<mpg123.h>)
#  include <mpg123.h>
# else
#  include "mpg123_min.h"
# endif
#else
# include "mpg123_min.h"
#endif

#include <pulse/simple.h>
#include <pulse/error.h>

#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define ENGINE_MT "xtcw.mp3engine"
#define EQ_BANDS 5
#define FFT_SIZE 1024
#define MAX_BANDS 64
#define PCM_FRAMES 2048

enum {
    ST_IDLE = 0,
    ST_PLAYING,
    ST_PAUSED,
    ST_STOPPED,
    ST_ERROR
};

typedef struct {
    double b0, b1, b2, a1, a2;
    double z1[2], z2[2];
} Biquad;

typedef struct {
    pthread_t thread;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int thread_started;
    int quit;
    int paused;
    int state;
    char error[256];
    char path[1024];
    char sink[256];
    int rate;
    int channels;
    int bands;
    int volume;
    double eq_freq[EQ_BANDS];
    double eq_db[EQ_BANDS];
    int eq_dirty;
    int seek_req;
    double seek_pos;
    int flush_req;
    double duration;
    double position;
    double rms[2];
    double peak[2];
    double levels[MAX_BANDS];
} Engine;

static const double eq_freq_default[EQ_BANDS] = { 60.0, 250.0, 1000.0, 4000.0, 12000.0 };

static void set_error(Engine *e, const char *msg)
{
    pthread_mutex_lock(&e->lock);
    snprintf(e->error, sizeof e->error, "%s", msg ? msg : "engine error");
    e->state = ST_ERROR;
    pthread_mutex_unlock(&e->lock);
}

static void biquad_peaking(Biquad *b, double fs, double f, double gain_db, double q)
{
    double A = pow(10.0, gain_db / 40.0);
    double w0 = 2.0 * M_PI * f / fs;
    double alpha = sin(w0) / (2.0 * q);
    double a0 = 1.0 + alpha / A;
    double cw = cos(w0);
    b->b0 = (1.0 + alpha * A) / a0;
    b->b1 = (-2.0 * cw) / a0;
    b->b2 = (1.0 - alpha * A) / a0;
    b->a1 = (-2.0 * cw) / a0;
    b->a2 = (1.0 - alpha / A) / a0;
    b->z1[0] = b->z1[1] = 0.0;
    b->z2[0] = b->z2[1] = 0.0;
}

static double biquad_run(Biquad *b, int ch, double x)
{
    double y = b->b0 * x + b->z1[ch];
    b->z1[ch] = b->b1 * x - b->a1 * y + b->z2[ch];
    b->z2[ch] = b->b2 * x - b->a2 * y;
    return y;
}

static void fft_radix2(double *re, double *im, int n)
{
    int *rev = malloc((size_t)n * sizeof(int));
    int i, len;
    if (!rev) return;
    rev[0] = 0;
    {
        int j = 0;
        for (i = 1; i < n; i++) {
            int bit = n >> 1;
            while (j & bit) { j ^= bit; bit >>= 1; }
            j ^= bit;
            rev[i] = j;
        }
    }
    for (i = 1; i < n; i++) {
        int j = rev[i];
        if (i < j) {
            double tr = re[i], ti = im[i];
            re[i] = re[j]; im[i] = im[j];
            re[j] = tr; im[j] = ti;
        }
    }
    for (len = 2; len <= n; len <<= 1) {
        double ang = -2.0 * M_PI / (double)len;
        double wr = cos(ang), wi = sin(ang);
        int half = len >> 1;
        for (i = 0; i < n; i += len) {
            double cr = 1.0, ci = 0.0;
            int j;
            for (j = 0; j < half; j++) {
                int p = i + j, q = p + half;
                double vr = re[q] * cr - im[q] * ci;
                double vi = re[q] * ci + im[q] * cr;
                double ur = re[p], ui = im[p];
                re[p] = ur + vr; im[p] = ui + vi;
                re[q] = ur - vr; im[q] = ui - vi;
                {
                    double ncr = cr * wr - ci * wi;
                    double nci = cr * wi + ci * wr;
                    cr = ncr; ci = nci;
                }
            }
        }
    }
    free(rev);
}

static void analysis_update(Engine *e, const float *mono, double rate)
{
    double re[FFT_SIZE], im[FFT_SIZE];
    double fmin = 40.0, fmax = 16000.0;
    int b, i;

    if (fmax > rate * 0.45) fmax = rate * 0.45;
    if (fmax <= fmin) fmax = fmin + 1.0;

    for (i = 0; i < FFT_SIZE; i++) {
        double w = 0.5 * (1.0 - cos(2.0 * M_PI * i / (FFT_SIZE - 1)));
        re[i] = mono[i] * w;
        im[i] = 0.0;
    }
    fft_radix2(re, im, FFT_SIZE);

    pthread_mutex_lock(&e->lock);
    for (b = 0; b < e->bands; b++) {
        double t0 = (double)b / e->bands;
        double t1 = (double)(b + 1) / e->bands;
        double f0 = fmin * pow(fmax / fmin, t0);
        double f1 = fmin * pow(fmax / fmin, t1);
        int bin0 = (int)floor(f0 * FFT_SIZE / rate);
        int bin1 = (int)ceil(f1 * FFT_SIZE / rate);
        double mag = 0.0;
        int k;
        if (bin0 < 1) bin0 = 1;
        if (bin1 <= bin0) bin1 = bin0 + 1;
        if (bin1 > FFT_SIZE / 2) bin1 = FFT_SIZE / 2;
        for (k = bin0; k < bin1 && k <= FFT_SIZE / 2; k++) {
            double m = sqrt(re[k] * re[k] + im[k] * im[k]);
            if (m > mag) mag = m;
        }
        {
            double amp = 4.0 * mag / (FFT_SIZE * 1.0);
            double db = 20.0 * log10(amp + 1e-12);
            if (db < -50.0) db = -50.0;
            if (db > 0.0) db = 0.0;
            e->levels[b] = db;
        }
    }
    pthread_mutex_unlock(&e->lock);
}

static void *engine_thread(void *arg)
{
    Engine *e = (Engine *)arg;
    mpg123_handle *mh = NULL;
    pa_simple *pa = NULL;
    Biquad eq[EQ_BANDS];
    float pcm[PCM_FRAMES * 2];
    float mono[FFT_SIZE];
    int mono_pos = 0;
    int analysis_countdown = 0;
    long long frames_done = 0;
    int rc, err = 0;
    int use_null = (strcmp(e->sink, "null") == 0);
    double rate = (double)e->rate;
    int channels = e->channels;
    int i;

    for (i = 0; i < EQ_BANDS; i++)
        biquad_peaking(&eq[i], rate, e->eq_freq[i], e->eq_db[i], 1.41);

    if (mpg123_init() != MPG123_OK) {
        set_error(e, "mpg123_init failed");
        return NULL;
    }
    mh = mpg123_new(NULL, &err);
    if (!mh) {
        set_error(e, mpg123_plain_strerror(err));
        mpg123_exit();
        return NULL;
    }
    if (mpg123_open(mh, e->path) != MPG123_OK) {
        set_error(e, mpg123_strerror(mh));
        goto done;
    }
    mpg123_format_none(mh);
    if (mpg123_format(mh, (long)rate, MPG123_STEREO, MPG123_ENC_FLOAT_32) != MPG123_OK) {
        set_error(e, mpg123_strerror(mh));
        goto done;
    }
    {
        long orate = 0;
        int och = 0, enc = 0;
        if (mpg123_getformat(mh, &orate, &och, &enc) != MPG123_OK) {
            set_error(e, mpg123_strerror(mh));
            goto done;
        }
        rate = (double)orate;
        channels = och;
        pthread_mutex_lock(&e->lock);
        e->rate = (int)orate;
        e->channels = och;
        pthread_mutex_unlock(&e->lock);
    }

    if (mpg123_scan(mh) == MPG123_OK) {
        off_t len = mpg123_length(mh);
        if (len > 0) {
            pthread_mutex_lock(&e->lock);
            e->duration = (double)len / rate;
            pthread_mutex_unlock(&e->lock);
        }
        mpg123_seek(mh, 0, SEEK_SET);
    }

    if (!use_null) {
        pa_sample_spec ss;
        ss.format = PA_SAMPLE_FLOAT32LE;
        ss.rate = (uint32_t)rate;
        ss.channels = (uint8_t)channels;
        pa = pa_simple_new(NULL, "mp3engine", PA_STREAM_PLAYBACK,
                           e->sink[0] ? e->sink : NULL, "mp3player", &ss,
                           NULL, NULL, &err);
        if (!pa) {
            set_error(e, pa_strerror(err));
            goto done;
        }
    }

    pthread_mutex_lock(&e->lock);
    e->state = ST_PLAYING;
    pthread_mutex_unlock(&e->lock);

    for (;;) {
        size_t done = 0;
        int do_seek = 0;
        double seek_pos = 0.0;
        int do_flush = 0;
        int volume = 100;

        pthread_mutex_lock(&e->lock);
        if (e->quit) { pthread_mutex_unlock(&e->lock); break; }
        if (e->flush_req) {
            do_flush = 1;
            e->flush_req = 0;
        }
        if (e->paused) {
            e->state = ST_PAUSED;
            pthread_mutex_unlock(&e->lock);
            if (do_flush && pa) pa_simple_flush(pa, NULL);
            pthread_mutex_lock(&e->lock);
            pthread_cond_wait(&e->cond, &e->lock);
            pthread_mutex_unlock(&e->lock);
            continue;
        }
        e->state = ST_PLAYING;
        if (e->seek_req) {
            do_seek = 1;
            seek_pos = e->seek_pos;
            e->seek_req = 0;
        }
        if (e->eq_dirty) {
            for (i = 0; i < EQ_BANDS; i++)
                biquad_peaking(&eq[i], rate, e->eq_freq[i], e->eq_db[i], 1.41);
            e->eq_dirty = 0;
        }
        volume = e->volume;
        pthread_mutex_unlock(&e->lock);

        if (do_flush && pa) pa_simple_flush(pa, NULL);
        if (do_seek) {
            if (mpg123_seek(mh, (off_t)(seek_pos * rate), SEEK_SET) < 0) {
                set_error(e, mpg123_strerror(mh));
                break;
            }
            if (pa) pa_simple_flush(pa, NULL);
            for (i = 0; i < EQ_BANDS; i++) {
                eq[i].z1[0] = eq[i].z1[1] = 0.0;
                eq[i].z2[0] = eq[i].z2[1] = 0.0;
            }
            frames_done = (long long)(seek_pos * rate);
            pthread_mutex_lock(&e->lock);
            e->position = seek_pos;
            pthread_mutex_unlock(&e->lock);
        }

        rc = mpg123_read(mh, (unsigned char *)pcm, sizeof pcm, &done);
        if (rc == MPG123_NEW_FORMAT) {
            long orate = 0;
            int och = 0, enc = 0;
            mpg123_getformat(mh, &orate, &och, &enc);
            rate = (double)orate;
            channels = och;
            continue;
        }
        if (rc != MPG123_OK && rc != MPG123_DONE) {
            set_error(e, mpg123_strerror(mh));
            break;
        }
        if (done == 0) {
            if (rc == MPG123_DONE) break;
            continue;
        }

        size_t frames = done / (sizeof(float) * (size_t)channels);
        size_t f, c;
        double sum[2] = { 0.0, 0.0 };
        double peak[2] = { 0.0, 0.0 };

        for (f = 0; f < frames; f++) {
            for (c = 0; c < (size_t)channels && c < 2; c++) {
                double x = pcm[f * (size_t)channels + c];
                for (i = 0; i < EQ_BANDS; i++)
                    x = biquad_run(&eq[i], (int)c, x);
                x *= (double)volume / 100.0;
                pcm[f * (size_t)channels + c] = (float)x;
                sum[c] += x * x;
                if (fabs(x) > peak[c]) peak[c] = fabs(x);
            }
            {
                double m = 0.0;
                for (c = 0; c < (size_t)channels && c < 2; c++)
                    m += pcm[f * (size_t)channels + c];
                m /= (channels > 1 ? 2.0 : 1.0);
                mono[mono_pos++] = (float)m;
                if (mono_pos == FFT_SIZE) {
                    mono_pos = 0;
                    if (analysis_countdown-- <= 0) {
                        analysis_countdown = 4;
                        analysis_update(e, mono, rate);
                    }
                }
            }
        }

        if (pa) {
            if (pa_simple_write(pa, pcm,
                    frames * sizeof(float) * (size_t)channels, &err) < 0) {
                set_error(e, pa_strerror(err));
                break;
            }
        } else {
            /* Null output should still run in real time. */
            double secs = (double)frames / rate;
            struct timespec ts;
            ts.tv_sec = (time_t)secs;
            ts.tv_nsec = (long)((secs - (double)ts.tv_sec) * 1000000000.0);
            nanosleep(&ts, NULL);
        }

        frames_done += (long long)frames;
        pthread_mutex_lock(&e->lock);
        e->position = (double)frames_done / rate;
        if (channels > 0 && frames > 0) {
            e->rms[0] = 20.0 * log10(sqrt(sum[0] / (double)frames) + 1e-12);
            e->rms[1] = 20.0 * log10(sqrt(sum[1] / (double)frames) + 1e-12);
            e->peak[0] = 20.0 * log10(peak[0] + 1e-12);
            e->peak[1] = 20.0 * log10(peak[1] + 1e-12);
        }
        pthread_mutex_unlock(&e->lock);
    }

done:
    if (pa) {
        pa_simple_drain(pa, NULL);
        pa_simple_free(pa);
    }
    if (mh) {
        mpg123_close(mh);
        mpg123_delete(mh);
    }
    mpg123_exit();

    pthread_mutex_lock(&e->lock);
    if (e->state != ST_ERROR) e->state = ST_STOPPED;
    pthread_mutex_unlock(&e->lock);
    return NULL;
}

static Engine *check_engine(lua_State *L, int idx)
{
    return (Engine *)luaL_checkudata(L, idx, ENGINE_MT);
}

static int l_new(lua_State *L)
{
    const char *path = NULL;
    const char *sink = NULL;
    int volume = 80;
    int bands = 24;
    int rate = 44100;
    int channels = 2;
    int i;

    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "file");
    path = lua_tostring(L, -1);
    lua_pop(L, 1);
    if (!path || !*path) return luaL_error(L, "mp3engine: file is required");

    lua_getfield(L, 1, "sink");
    if (lua_isstring(L, -1)) sink = lua_tostring(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, 1, "volume");
    if (lua_isnumber(L, -1)) volume = (int)lua_tointeger(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, 1, "bands");
    if (lua_isnumber(L, -1)) bands = (int)lua_tointeger(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, 1, "rate");
    if (lua_isnumber(L, -1)) rate = (int)lua_tointeger(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, 1, "channels");
    if (lua_isnumber(L, -1)) channels = (int)lua_tointeger(L, -1);
    lua_pop(L, 1);

    if (bands < 1) bands = 1;
    if (bands > MAX_BANDS) bands = MAX_BANDS;
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;

    Engine *e = (Engine *)lua_newuserdata(L, sizeof(*e));
    memset(e, 0, sizeof(*e));
    pthread_mutex_init(&e->lock, NULL);
    pthread_cond_init(&e->cond, NULL);
    snprintf(e->path, sizeof e->path, "%s", path);
    snprintf(e->sink, sizeof e->sink, "%s", sink ? sink : "");
    e->volume = volume;
    e->bands = bands;
    e->rate = rate;
    e->channels = channels;
    e->state = ST_IDLE;
    for (i = 0; i < EQ_BANDS; i++)
        e->eq_freq[i] = eq_freq_default[i];

    lua_getfield(L, 1, "freqs");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_getfield(L, 1, "eq_bands");
    }
    if (lua_istable(L, -1)) {
        for (i = 0; i < EQ_BANDS; i++) {
            lua_rawgeti(L, -1, i + 1);
            if (lua_isnumber(L, -1)) {
                double f = lua_tonumber(L, -1);
                if (f > 10.0 && f < 24000.0) e->eq_freq[i] = f;
            }
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "eq");
    if (lua_istable(L, -1)) {
        for (i = 0; i < EQ_BANDS; i++) {
            lua_rawgeti(L, -1, i + 1);
            if (lua_isnumber(L, -1)) e->eq_db[i] = lua_tonumber(L, -1);
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);

    luaL_getmetatable(L, ENGINE_MT);
    lua_setmetatable(L, -2);
    return 1;
}

static int l_play(lua_State *L)
{
    Engine *e = check_engine(L, 1);
    pthread_mutex_lock(&e->lock);
    if (e->thread_started) {
        e->paused = 0;
        e->state = ST_PLAYING;
        pthread_cond_broadcast(&e->cond);
        pthread_mutex_unlock(&e->lock);
        lua_pushboolean(L, 1);
        return 1;
    }
    e->quit = 0;
    e->paused = 0;
    e->state = ST_PLAYING;
    if (pthread_create(&e->thread, NULL, engine_thread, e) != 0) {
        e->state = ST_ERROR;
        snprintf(e->error, sizeof e->error, "cannot start engine thread");
        pthread_mutex_unlock(&e->lock);
        lua_pushnil(L);
        lua_pushstring(L, "cannot start engine thread");
        return 2;
    }
    e->thread_started = 1;
    pthread_mutex_unlock(&e->lock);
    lua_pushboolean(L, 1);
    return 1;
}

static int l_pause(lua_State *L)
{
    Engine *e = check_engine(L, 1);
    pthread_mutex_lock(&e->lock);
    e->paused = 1;
    e->flush_req = 1;
    e->state = ST_PAUSED;
    pthread_cond_broadcast(&e->cond);
    pthread_mutex_unlock(&e->lock);
    lua_pushboolean(L, 1);
    return 1;
}

static int l_resume(lua_State *L)
{
    Engine *e = check_engine(L, 1);
    pthread_mutex_lock(&e->lock);
    e->paused = 0;
    e->state = ST_PLAYING;
    pthread_cond_broadcast(&e->cond);
    pthread_mutex_unlock(&e->lock);
    lua_pushboolean(L, 1);
    return 1;
}

static int l_stop(lua_State *L)
{
    Engine *e = check_engine(L, 1);
    pthread_mutex_lock(&e->lock);
    if (e->thread_started) {
        e->quit = 1;
        pthread_cond_broadcast(&e->cond);
    }
    pthread_mutex_unlock(&e->lock);
    if (e->thread_started) {
        pthread_join(e->thread, NULL);
        pthread_mutex_lock(&e->lock);
        e->thread_started = 0;
        e->state = ST_STOPPED;
        pthread_mutex_unlock(&e->lock);
    }
    lua_pushboolean(L, 1);
    return 1;
}

static int l_seek(lua_State *L)
{
    Engine *e = check_engine(L, 1);
    double pos = luaL_checknumber(L, 2);
    if (pos < 0) pos = 0;
    pthread_mutex_lock(&e->lock);
    e->seek_req = 1;
    e->seek_pos = pos;
    e->flush_req = 1;
    pthread_cond_broadcast(&e->cond);
    pthread_mutex_unlock(&e->lock);
    lua_pushboolean(L, 1);
    return 1;
}

static int l_set_eq(lua_State *L)
{
    Engine *e = check_engine(L, 1);
    int band = (int)luaL_checkinteger(L, 2);
    double db = luaL_checknumber(L, 3);
    if (band < 0 || band >= EQ_BANDS)
        return luaL_error(L, "mp3engine: EQ band must be 0..4");
    if (db < -24.0) db = -24.0;
    if (db > 24.0) db = 24.0;
    pthread_mutex_lock(&e->lock);
    e->eq_db[band] = db;
    e->eq_dirty = 1;
    pthread_mutex_unlock(&e->lock);
    lua_pushboolean(L, 1);
    return 1;
}

static int l_set_volume(lua_State *L)
{
    Engine *e = check_engine(L, 1);
    int v = (int)luaL_checkinteger(L, 2);
    if (v < 0) v = 0;
    if (v > 100) v = 100;
    pthread_mutex_lock(&e->lock);
    e->volume = v;
    pthread_mutex_unlock(&e->lock);
    lua_pushboolean(L, 1);
    return 1;
}

static int l_is_running(lua_State *L)
{
    Engine *e = check_engine(L, 1);
    pthread_mutex_lock(&e->lock);
    int running = e->thread_started && !e->quit &&
                  (e->state == ST_PLAYING || e->state == ST_PAUSED);
    pthread_mutex_unlock(&e->lock);
    lua_pushboolean(L, running);
    return 1;
}

static int l_status(lua_State *L)
{
    Engine *e = check_engine(L, 1);
    char state[16];
    double pos, dur, rms[2], peak[2], levels[MAX_BANDS];
    int bands, i;

    pthread_mutex_lock(&e->lock);
    pos = e->position;
    dur = e->duration;
    rms[0] = e->rms[0]; rms[1] = e->rms[1];
    peak[0] = e->peak[0]; peak[1] = e->peak[1];
    bands = e->bands;
    for (i = 0; i < bands; i++) levels[i] = e->levels[i];
    switch (e->state) {
    case ST_PLAYING: strcpy(state, "playing"); break;
    case ST_PAUSED: strcpy(state, "paused"); break;
    case ST_STOPPED: strcpy(state, "stopped"); break;
    case ST_ERROR: strcpy(state, "error"); break;
    default: strcpy(state, "idle"); break;
    }
    char error[256];
    snprintf(error, sizeof error, "%s", e->error);
    pthread_mutex_unlock(&e->lock);

    lua_createtable(L, 0, 8);
    lua_pushstring(L, state); lua_setfield(L, -2, "state");
    lua_pushnumber(L, pos); lua_setfield(L, -2, "position");
    lua_pushnumber(L, dur); lua_setfield(L, -2, "duration");
    lua_pushstring(L, error); lua_setfield(L, -2, "error");

    lua_createtable(L, 2, 0);
    lua_pushnumber(L, rms[0]); lua_rawseti(L, -2, 1);
    lua_pushnumber(L, rms[1]); lua_rawseti(L, -2, 2);
    lua_setfield(L, -2, "rms");

    lua_createtable(L, 2, 0);
    lua_pushnumber(L, peak[0]); lua_rawseti(L, -2, 1);
    lua_pushnumber(L, peak[1]); lua_rawseti(L, -2, 2);
    lua_setfield(L, -2, "peak");

    lua_createtable(L, bands, 0);
    for (i = 0; i < bands; i++) {
        lua_pushnumber(L, levels[i]);
        lua_rawseti(L, -2, i + 1);
    }
    lua_setfield(L, -2, "levels");
    return 1;
}

static int l_duration(lua_State *L)
{
    Engine *e = check_engine(L, 1);
    pthread_mutex_lock(&e->lock);
    double d = e->duration;
    pthread_mutex_unlock(&e->lock);
    lua_pushnumber(L, d);
    return 1;
}

static int l_gc(lua_State *L)
{
    Engine *e = check_engine(L, 1);
    pthread_mutex_lock(&e->lock);
    if (e->thread_started) {
        e->quit = 1;
        pthread_cond_broadcast(&e->cond);
    }
    pthread_mutex_unlock(&e->lock);
    if (e->thread_started) {
        pthread_join(e->thread, NULL);
        e->thread_started = 0;
    }
    pthread_mutex_destroy(&e->lock);
    pthread_cond_destroy(&e->cond);
    return 0;
}

static const luaL_Reg methods[] = {
    { "play", l_play },
    { "pause", l_pause },
    { "resume", l_resume },
    { "stop", l_stop },
    { "seek", l_seek },
    { "set_eq", l_set_eq },
    { "set_volume", l_set_volume },
    { "is_running", l_is_running },
    { "status", l_status },
    { "duration", l_duration },
    { "__gc", l_gc },
    { NULL, NULL }
};

static int l_available(lua_State *L)
{
    lua_pushboolean(L, 1);
    return 1;
}

static const luaL_Reg module_funcs[] = {
    { "new", l_new },
    { "available", l_available },
    { NULL, NULL }
};

int luaopen_mp3engine(lua_State *L)
{
    luaL_newmetatable(L, ENGINE_MT);
    luaL_setfuncs(L, methods, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    luaL_newlib(L, module_funcs);
    return 1;
}
