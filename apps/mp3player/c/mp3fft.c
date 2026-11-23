/*
 * mp3fft.c — small self-contained FFT helper for the XTCW mp3player demo.
 *
 * Lua 5.3 module:
 *   local fft = require("mp3fft")
 *   local a = fft.new{ rate=22050, size=1024, bands=24 }
 *   local r = a:process_pcm_file("/tmp/x.pcm", offset, 2)
 *
 * The module intentionally has no FFTW dependency.  It reads little-endian
 * signed 16-bit PCM, applies a Hann window, runs a radix-2 Cooley-Tukey FFT
 * and returns log-spaced band levels plus per-channel RMS/peak values.
 */

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MP3FFT_MT "xtcw.mp3fft"
#define MP3FFT_MAX_SIZE 8192
#define MP3FFT_MAX_BANDS 128

typedef struct {
    int rate;
    int size;
    int bands;
    double min_db;
    double max_db;

    double *window;
    int *bitrev;
    double *re;
    double *im;
    int *band_lo;
    int *band_hi;
    double *levels;
    int16_t *pcm;
    size_t pcm_cap;
} Analyzer;

static void analyzer_free(Analyzer *a)
{
    if (!a) return;
    free(a->window);
    free(a->bitrev);
    free(a->re);
    free(a->im);
    free(a->band_lo);
    free(a->band_hi);
    free(a->levels);
    free(a->pcm);
    a->window = NULL;
    a->bitrev = NULL;
    a->re = NULL;
    a->im = NULL;
    a->band_lo = NULL;
    a->band_hi = NULL;
    a->levels = NULL;
    a->pcm = NULL;
}

static int is_power_of_two(int n)
{
    return n > 0 && (n & (n - 1)) == 0;
}

static double clamp_db(const Analyzer *a, double db)
{
    if (db < a->min_db) return a->min_db;
    if (db > a->max_db) return a->max_db;
    return db;
}

static double to_db(double linear)
{
    if (!(linear > 0.0)) return -120.0;
    return 20.0 * log10(linear);
}

static int analyzer_init(Analyzer *a)
{
    int i;

    a->window = calloc((size_t)a->size, sizeof(double));
    a->bitrev = calloc((size_t)a->size, sizeof(int));
    a->re = calloc((size_t)a->size, sizeof(double));
    a->im = calloc((size_t)a->size, sizeof(double));
    a->band_lo = calloc((size_t)a->bands, sizeof(int));
    a->band_hi = calloc((size_t)a->bands, sizeof(int));
    a->levels = calloc((size_t)a->bands, sizeof(double));
    if (!a->window || !a->bitrev || !a->re || !a->im ||
        !a->band_lo || !a->band_hi || !a->levels) {
        analyzer_free(a);
        return 0;
    }

    for (i = 0; i < a->size; i++)
        a->window[i] = 0.5 * (1.0 - cos(2.0 * M_PI * (double)i /
                                        (double)(a->size - 1)));

    {
        int j = 0;
        a->bitrev[0] = 0;
        for (i = 1; i < a->size; i++) {
            int bit = a->size >> 1;
            while (j & bit) {
                j ^= bit;
                bit >>= 1;
            }
            j ^= bit;
            a->bitrev[i] = j;
        }
    }

    {
        double fmin = 40.0;
        double fmax = (double)a->rate * 0.5 * 0.95;
        if (fmax > 16000.0) fmax = 16000.0;
        if (fmax <= fmin) fmax = (double)a->rate * 0.5;
        if (fmax <= fmin) fmax = fmin + 1.0;

        for (i = 0; i < a->bands; i++) {
            double t0 = (double)i / (double)a->bands;
            double t1 = (double)(i + 1) / (double)a->bands;
            double f0 = fmin * pow(fmax / fmin, t0);
            double f1 = fmin * pow(fmax / fmin, t1);
            int lo = (int)floor(f0 * (double)a->size / (double)a->rate);
            int hi = (int)ceil(f1 * (double)a->size / (double)a->rate);
            if (lo < 1) lo = 1;
            if (hi <= lo) hi = lo + 1;
            if (hi > a->size / 2) hi = a->size / 2;
            if (lo > a->size / 2) lo = a->size / 2;
            if (hi <= lo) hi = lo + 1;
            a->band_lo[i] = lo;
            a->band_hi[i] = hi;
        }
    }

    return 1;
}

static void fft_radix2(Analyzer *a)
{
    int n = a->size;
    int i, len;

    for (i = 1; i < n; i++) {
        int j = a->bitrev[i];
        if (i < j) {
            double tr = a->re[i], ti = a->im[i];
            a->re[i] = a->re[j];
            a->im[i] = a->im[j];
            a->re[j] = tr;
            a->im[j] = ti;
        }
    }

    for (len = 2; len <= n; len <<= 1) {
        double ang = -2.0 * M_PI / (double)len;
        double wr = cos(ang);
        double wi = sin(ang);
        int half = len >> 1;

        for (i = 0; i < n; i += len) {
            double cr = 1.0, ci = 0.0;
            int j;
            for (j = 0; j < half; j++) {
                int p = i + j;
                int q = p + half;
                double vr = a->re[q] * cr - a->im[q] * ci;
                double vi = a->re[q] * ci + a->im[q] * cr;
                double ur = a->re[p];
                double ui = a->im[p];
                double ncr, nci;

                a->re[p] = ur + vr;
                a->im[p] = ui + vi;
                a->re[q] = ur - vr;
                a->im[q] = ui - vi;

                ncr = cr * wr - ci * wi;
                nci = cr * wi + ci * wr;
                cr = ncr;
                ci = nci;
            }
        }
    }
}

static int ensure_pcm(Analyzer *a, size_t samples)
{
    int16_t *p;
    if (samples <= a->pcm_cap) return 1;
    p = realloc(a->pcm, samples * sizeof(int16_t));
    if (!p) return 0;
    a->pcm = p;
    a->pcm_cap = samples;
    return 1;
}

static int analyze_samples(Analyzer *a, lua_State *L,
                           const int16_t *pcm, size_t frames, int channels)
{
    size_t i;
    double sum_l = 0.0, sum_r = 0.0;
    double peak_l = 0.0, peak_r = 0.0;

    if (frames < (size_t)a->size) {
        lua_pushnil(L);
        lua_pushfstring(L, "short PCM window: need %d frames, got %d",
                        a->size, (int)frames);
        return 2;
    }

    for (i = 0; i < (size_t)a->size; i++) {
        double l = (double)pcm[i * (size_t)channels];
        double r = (channels == 2) ? (double)pcm[i * 2 + 1] : l;
        double mono = (l + r) * 0.5;

        sum_l += l * l;
        sum_r += r * r;
        if (fabs(l) > peak_l) peak_l = fabs(l);
        if (fabs(r) > peak_r) peak_r = fabs(r);

        a->re[i] = mono * a->window[i];
        a->im[i] = 0.0;
    }

    fft_radix2(a);

    for (i = 0; i < (size_t)a->bands; i++) {
        int lo = a->band_lo[i];
        int hi = a->band_hi[i];
        int k;
        double mag = 0.0;
        for (k = lo; k < hi && k <= a->size / 2; k++) {
            double m = sqrt(a->re[k] * a->re[k] + a->im[k] * a->im[k]);
            if (m > mag) mag = m;
        }
        /* Hann coherent gain is 0.5; a real sine of amplitude A gives
         * |X[k]| = A*N/4 after windowing, hence 4*mag/(N*32768). */
        a->levels[i] = clamp_db(a,
            20.0 * log10((4.0 * mag) / ((double)a->size * 32768.0) + 1e-12));
    }

    lua_createtable(L, 0, 4);       /* result */

    lua_createtable(L, a->bands, 0); /* result.levels */
    for (i = 0; i < (size_t)a->bands; i++) {
        lua_pushnumber(L, a->levels[i]);
        lua_rawseti(L, -2, (lua_Integer)i + 1);
    }
    lua_setfield(L, -2, "levels");

    lua_createtable(L, 2, 0);        /* result.rms */
    lua_pushnumber(L, clamp_db(a, to_db(sqrt(sum_l / (double)a->size) / 32768.0)));
    lua_rawseti(L, -2, 1);
    lua_pushnumber(L, clamp_db(a, to_db(sqrt(sum_r / (double)a->size) / 32768.0)));
    lua_rawseti(L, -2, 2);
    lua_setfield(L, -2, "rms");

    lua_createtable(L, 2, 0);        /* result.peak */
    lua_pushnumber(L, clamp_db(a, to_db(peak_l / 32768.0)));
    lua_rawseti(L, -2, 1);
    lua_pushnumber(L, clamp_db(a, to_db(peak_r / 32768.0)));
    lua_rawseti(L, -2, 2);
    lua_setfield(L, -2, "peak");

    lua_pushinteger(L, (lua_Integer)(a->size * channels * 2));
    lua_setfield(L, -2, "bytes");

    return 1;
}

static Analyzer *check_analyzer(lua_State *L, int idx)
{
    return (Analyzer *)luaL_checkudata(L, idx, MP3FFT_MT);
}

static int l_new(lua_State *L)
{
    Analyzer *a;
    int rate = 22050;
    int size = 1024;
    int bands = 24;
    double min_db = -50.0;
    double max_db = 0.0;

    if (lua_istable(L, 1)) {
        lua_getfield(L, 1, "rate");
        if (lua_isnumber(L, -1)) rate = (int)lua_tointeger(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, 1, "size");
        if (lua_isnumber(L, -1)) size = (int)lua_tointeger(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, 1, "bands");
        if (lua_isnumber(L, -1)) bands = (int)lua_tointeger(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, 1, "min_db");
        if (lua_isnumber(L, -1)) min_db = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, 1, "max_db");
        if (lua_isnumber(L, -1)) max_db = lua_tonumber(L, -1);
        lua_pop(L, 1);
    }

    if (rate < 1000 || rate > 192000)
        return luaL_error(L, "mp3fft: rate out of range");
    if (!is_power_of_two(size) || size < 16 || size > MP3FFT_MAX_SIZE)
        return luaL_error(L, "mp3fft: size must be a power of two in 16..%d",
                          MP3FFT_MAX_SIZE);
    if (bands < 1 || bands > MP3FFT_MAX_BANDS)
        return luaL_error(L, "mp3fft: bands must be in 1..%d", MP3FFT_MAX_BANDS);
    if (!(min_db < max_db))
        return luaL_error(L, "mp3fft: min_db must be below max_db");

    a = (Analyzer *)lua_newuserdata(L, sizeof(*a));
    memset(a, 0, sizeof(*a));
    a->rate = rate;
    a->size = size;
    a->bands = bands;
    a->min_db = min_db;
    a->max_db = max_db;

    luaL_getmetatable(L, MP3FFT_MT);
    lua_setmetatable(L, -2);

    if (!analyzer_init(a))
        return luaL_error(L, "mp3fft: out of memory");

    return 1;
}

static int l_version(lua_State *L)
{
    lua_pushliteral(L, "1.0");
    return 1;
}

static int l_gc(lua_State *L)
{
    Analyzer *a = check_analyzer(L, 1);
    analyzer_free(a);
    return 0;
}

static int l_reset(lua_State *L)
{
    Analyzer *a = check_analyzer(L, 1);
    memset(a->re, 0, (size_t)a->size * sizeof(double));
    memset(a->im, 0, (size_t)a->size * sizeof(double));
    memset(a->levels, 0, (size_t)a->bands * sizeof(double));
    lua_pushboolean(L, 1);
    return 1;
}

static int l_process_pcm_file(lua_State *L)
{
    Analyzer *a = check_analyzer(L, 1);
    const char *path = luaL_checkstring(L, 2);
    lua_Integer offset = luaL_optinteger(L, 3, 0);
    int channels = (int)luaL_optinteger(L, 4, 2);
    FILE *f;
    size_t samples, want, got;

    if (channels != 1 && channels != 2)
        return luaL_error(L, "mp3fft: channels must be 1 or 2");

    samples = (size_t)a->size * (size_t)channels;
    if (!ensure_pcm(a, samples))
        return luaL_error(L, "mp3fft: out of memory");

    f = fopen(path, "rb");
    if (!f) {
        lua_pushnil(L);
        lua_pushfstring(L, "cannot open %s", path);
        return 2;
    }
    if (offset > 0 && fseek(f, (long)offset, SEEK_SET) != 0) {
        fclose(f);
        lua_pushnil(L);
        lua_pushfstring(L, "cannot seek to %d", (int)offset);
        return 2;
    }

    want = samples * sizeof(int16_t);
    got = fread(a->pcm, 1, want, f);
    fclose(f);
    if (got < want) {
        lua_pushnil(L);
        lua_pushfstring(L, "short read: wanted %d bytes, got %d",
                        (int)want, (int)got);
        return 2;
    }

    return analyze_samples(a, L, a->pcm, (size_t)a->size, channels);
}

static int l_process_pcm_data(lua_State *L)
{
    Analyzer *a = check_analyzer(L, 1);
    size_t len = 0;
    const char *data = luaL_checklstring(L, 2, &len);
    int channels = (int)luaL_optinteger(L, 3, 2);
    size_t samples, want;

    if (channels != 1 && channels != 2)
        return luaL_error(L, "mp3fft: channels must be 1 or 2");

    samples = (size_t)a->size * (size_t)channels;
    want = samples * sizeof(int16_t);
    if (len < want) {
        lua_pushnil(L);
        lua_pushfstring(L, "short PCM data: wanted %d bytes, got %d",
                        (int)want, (int)len);
        return 2;
    }
    if (!ensure_pcm(a, samples))
        return luaL_error(L, "mp3fft: out of memory");
    memcpy(a->pcm, data, want);

    return analyze_samples(a, L, a->pcm, (size_t)a->size, channels);
}

static const luaL_Reg analyzer_methods[] = {
    { "process_pcm_file", l_process_pcm_file },
    { "process_pcm_data", l_process_pcm_data },
    { "reset", l_reset },
    { "__gc", l_gc },
    { NULL, NULL }
};

static const luaL_Reg module_functions[] = {
    { "new", l_new },
    { "version", l_version },
    { NULL, NULL }
};

int luaopen_mp3fft(lua_State *L)
{
    luaL_newmetatable(L, MP3FFT_MT);
    luaL_setfuncs(L, analyzer_methods, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    luaL_newlib(L, module_functions);
    return 1;
}
