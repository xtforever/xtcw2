/*
 * mp3engine_test.c — standalone decoder/DSP/output proof for the mp3player.
 *
 * Decoding : libmpg123
 * DSP      : 5-band peaking EQ + 1024-point FFT/RMS/peak
 * Output   : libpulse-simple (or --null for headless tests)
 *
 * Build (requires libmpg123-dev + libpulse-dev):
 *   make -C apps/mp3player/c mp3engine_test
 *
 * Run:
 *   ./apps/mp3player/c/mp3engine_test track.mp3
 *   ./apps/mp3player/c/mp3engine_test --null --seconds 5 track.mp3
 *   ./apps/mp3player/c/mp3engine_test --eq 0,2,0,-1,3 track.mp3
 */

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
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define FFT_SIZE 1024
#define BANDS 24
#define EQ_BANDS 5
#define PCM_FRAMES 4096

typedef struct {
    double b0, b1, b2, a1, a2;
    double z1[2], z2[2];
} Biquad;

static volatile sig_atomic_t stop_requested = 0;

static void on_sigint(int sig)
{
    (void)sig;
    stop_requested = 1;
}

static const double eq_freq[EQ_BANDS] = { 60.0, 250.0, 1000.0, 4000.0, 12000.0 };

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
    int i, len;
    int *rev = malloc((size_t)n * sizeof(int));
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

static void print_spectrum(const float *mono, int n, double rate)
{
    double re[FFT_SIZE], im[FFT_SIZE];
    int i, b;
    for (i = 0; i < FFT_SIZE; i++) {
        double w = 0.5 * (1.0 - cos(2.0 * M_PI * i / (FFT_SIZE - 1)));
        re[i] = (i < n ? mono[i] : 0.0) * w;
        im[i] = 0.0;
    }
    fft_radix2(re, im, FFT_SIZE);

    double fmin = 40.0, fmax = 16000.0;
    if (fmax > rate * 0.45) fmax = rate * 0.45;
    if (fmax <= fmin) fmax = fmin + 1.0;

    printf("spec");
    for (b = 0; b < BANDS; b++) {
        double t0 = (double)b / BANDS, t1 = (double)(b + 1) / BANDS;
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
            printf(" %.0f", db);
        }
    }
    printf("\n");
}

static void usage(const char *argv0)
{
    printf("usage: %s [--null] [--sink NAME] [--seconds N] [--eq g0,g1,g2,g3,g4] file.mp3\n", argv0);
}

int main(int argc, char **argv)
{
    const char *path = NULL;
    const char *sink = NULL;
    int use_null = 0;
    double max_seconds = 0.0;
    double gains[EQ_BANDS] = { 0, 0, 0, 0, 0 };
    int i;

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--null")) use_null = 1;
        else if (!strcmp(argv[i], "--sink") && i + 1 < argc) sink = argv[++i];
        else if (!strcmp(argv[i], "--seconds") && i + 1 < argc) max_seconds = atof(argv[++i]);
        else if (!strcmp(argv[i], "--eq") && i + 1 < argc) {
            sscanf(argv[++i], "%lf,%lf,%lf,%lf,%lf",
                   &gains[0], &gains[1], &gains[2], &gains[3], &gains[4]);
        } else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            usage(argv[0]); return 0;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "unknown option: %s\n", argv[i]);
            usage(argv[0]); return 2;
        } else {
            path = argv[i];
        }
    }
    if (!path) { usage(argv[0]); return 2; }

    signal(SIGINT, on_sigint);
    signal(SIGTERM, on_sigint);

    if (mpg123_init() != MPG123_OK) {
        fprintf(stderr, "mpg123_init failed\n");
        return 1;
    }

    int err = MPG123_OK;
    mpg123_handle *mh = mpg123_new(NULL, &err);
    if (!mh) {
        fprintf(stderr, "mpg123_new: %s\n", mpg123_plain_strerror(err));
        mpg123_exit();
        return 1;
    }

    if (mpg123_open(mh, path) != MPG123_OK) {
        fprintf(stderr, "mpg123_open: %s\n", mpg123_strerror(mh));
        mpg123_delete(mh); mpg123_exit();
        return 1;
    }

    const double rate = 44100.0;
    mpg123_format_none(mh);
    if (mpg123_format(mh, (long)rate, MPG123_STEREO, MPG123_ENC_FLOAT_32) != MPG123_OK) {
        fprintf(stderr, "mpg123_format: %s\n", mpg123_strerror(mh));
        mpg123_close(mh); mpg123_delete(mh); mpg123_exit();
        return 1;
    }

    long out_rate = 0;
    int out_channels = 0, encoding = 0;
    if (mpg123_getformat(mh, &out_rate, &out_channels, &encoding) != MPG123_OK) {
        fprintf(stderr, "mpg123_getformat: %s\n", mpg123_strerror(mh));
        mpg123_close(mh); mpg123_delete(mh); mpg123_exit();
        return 1;
    }
    fprintf(stderr, "format: %ld Hz, %d ch, enc=0x%x\n", out_rate, out_channels, encoding);

    double duration = 0.0;
    if (mpg123_scan(mh) == MPG123_OK) {
        off_t len = mpg123_length(mh);
        if (len > 0) duration = (double)len / (double)out_rate;
        mpg123_seek(mh, 0, SEEK_SET);
    }
    fprintf(stderr, "duration: %.2f s\n", duration);

    Biquad eq[EQ_BANDS];
    for (i = 0; i < EQ_BANDS; i++)
        biquad_peaking(&eq[i], (double)out_rate, eq_freq[i], gains[i], 1.41);

    pa_simple *pa = NULL;
    if (!use_null) {
        pa_sample_spec ss;
        ss.format = PA_SAMPLE_FLOAT32LE;
        ss.rate = (uint32_t)out_rate;
        ss.channels = (uint8_t)out_channels;
        int perr = 0;
        pa = pa_simple_new(NULL, "mp3engine_test", PA_STREAM_PLAYBACK,
                           sink, "mp3engine_test", &ss, NULL, NULL, &perr);
        if (!pa) {
            fprintf(stderr, "pa_simple_new failed: %s\n", pa_strerror(perr));
            mpg123_close(mh); mpg123_delete(mh); mpg123_exit();
            return 1;
        }
    }

    float pcm[PCM_FRAMES * 2];
    float mono[FFT_SIZE];
    int mono_pos = 0;
    long long frames_done = 0;
    int analysis_countdown = 0;
    double next_report = 0.0;

    for (;;) {
        size_t done = 0;
        int rc;
        if (stop_requested) break;
        if (max_seconds > 0.0 &&
            (double)frames_done / (double)out_rate >= max_seconds) break;

        rc = mpg123_read(mh, (unsigned char *)pcm, sizeof(pcm), &done);
        if (rc == MPG123_NEW_FORMAT) {
            long r2 = 0; int c2 = 0, e2 = 0;
            mpg123_getformat(mh, &r2, &c2, &e2);
            continue;
        }
        if (rc != MPG123_OK && rc != MPG123_DONE) {
            fprintf(stderr, "mpg123_read: %s\n", mpg123_strerror(mh));
            break;
        }
        if (done == 0) {
            if (rc == MPG123_DONE) break;
            continue;
        }

        size_t frames = done / (sizeof(float) * (size_t)out_channels);
        size_t f, c;

        for (f = 0; f < frames; f++) {
            for (c = 0; c < (size_t)out_channels && c < 2; c++) {
                double x = pcm[f * (size_t)out_channels + c];
                for (i = 0; i < EQ_BANDS; i++)
                    x = biquad_run(&eq[i], (int)c, x);
                pcm[f * (size_t)out_channels + c] = (float)x;
            }
            double m = 0.0;
            for (c = 0; c < (size_t)out_channels && c < 2; c++)
                m += pcm[f * (size_t)out_channels + c];
            m /= (out_channels > 1 ? 2.0 : 1.0);
            mono[mono_pos++] = (float)m;
            if (mono_pos == FFT_SIZE) {
                mono_pos = 0;
                if (analysis_countdown-- <= 0) {
                    analysis_countdown = 10;
                    print_spectrum(mono, FFT_SIZE, (double)out_rate);
                    fflush(stdout);
                }
            }
        }

        if (pa) {
            int perr = 0;
            if (pa_simple_write(pa, pcm, frames * sizeof(float) * (size_t)out_channels, &perr) < 0) {
                fprintf(stderr, "pa_simple_write: %s\n", pa_strerror(perr));
                break;
            }
        }

        frames_done += (long long)frames;
        {
            double pos = (double)frames_done / (double)out_rate;
            if (pos >= next_report) {
                next_report = pos + 0.25;
                if (pa) {
                    int perr = 0;
                    pa_usec_t latency = pa_simple_get_latency(pa, &perr);
                    if (perr == 0) pos -= (double)latency / 1000000.0;
                }
                fprintf(stderr, "\rpos %.2f / %.2f s", pos > 0 ? pos : 0, duration);
                fflush(stderr);
            }
        }
    }

    fprintf(stderr, "\n");
    if (pa) {
        int perr = 0;
        pa_simple_drain(pa, &perr);
        pa_simple_free(pa);
    }
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    return 0;
}
