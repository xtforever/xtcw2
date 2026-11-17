// xtcw2 port: wbuild_widgets/snake-draw.c + demos/demo_snake.lua
// Snake for Bevel OS. Arrow keys or WASD steer, P pauses, Enter starts again, Esc leaves.
// Only the cells that change are redrawn, so it costs almost nothing.
#include "bevel.h"

#define CELL 12
#define GW 28
#define GH 19
#define TOP 22
#define W (GW * CELL)
#define H (TOP + GH * CELL)
#define MAXLEN (GW * GH)

#define C_BG     BV_RGB(0, 1, 0)
#define C_BG2    BV_RGB(0, 1, 1)
#define C_SNAKE  BV_RGB(1, 3, 0)
#define C_HEAD   BV_RGB(3, 3, 1)
#define C_DARK   BV_RGB(0, 2, 0)
#define C_FOOD   BV_RGB(3, 0, 0)
#define C_PANEL  BV_RGB(2, 2, 2)

static const bevel_api_t *os;
static uint8_t sx[MAXLEN], sy[MAXLEN];
static int head, len, dir, next_dir, fx, fy, score, best, alive, paused;

static void num(int v, char *out)
{
    char t[12];
    int n = 0;
    if (!v) t[n++] = '0';
    while (v) { t[n++] = '0' + v % 10; v /= 10; }
    while (n) *out++ = t[--n];
    *out = 0;
}

static void cell(int x, int y, int kind)
{
    int px = x * CELL, py = TOP + y * CELL;
    os->fill(px, py, CELL, CELL, ((x + y) & 1) ? C_BG : C_BG2);
    if (kind == 1 || kind == 2) {
        os->fill(px + 1, py + 1, CELL - 2, CELL - 2, kind == 2 ? C_HEAD : C_SNAKE);
        os->fill(px + 1, py + CELL - 3, CELL - 2, 2, C_DARK);
        os->fill(px + CELL - 3, py + 1, 2, CELL - 2, C_DARK);
        if (kind == 2) { os->fill(px + 3, py + 3, 2, 2, 0); os->fill(px + 7, py + 3, 2, 2, 0); }
    } else if (kind == 3) {
        os->fill(px + 3, py + 2, 6, 8, C_FOOD);
        os->fill(px + 2, py + 3, 8, 6, C_FOOD);
        os->fill(px + 4, py + 3, 2, 2, BV_RGB(3, 2, 2));
        os->fill(px + 6, py, 1, 3, BV_RGB(0, 2, 0));
    }
    os->flush_rect(px, py, CELL, CELL);
}

static void panel(void)
{
    char s[48] = "Score ", b[12];
    os->fill(0, 0, W, TOP, C_PANEL);
    os->bevel(0, 0, W, TOP, 1);
    num(score, b);
    int i = 6;
    for (char *p = b; *p;) s[i++] = *p++;
    s[i] = 0;
    os->text(8, 5, s, BV_FONT_BOLD, 0);
    char t[32] = "Best ";
    num(best, t + 5);
    os->text(100, 5, t, BV_FONT_UI, 0);
    const char *msg = !alive ? "Game over - Enter plays again" : paused ? "Paused - P continues" : "Arrows steer, P pauses";
    os->text(W - 8 - os->text_width(msg, BV_FONT_UI), 5, msg, BV_FONT_UI, alive ? BV_RGB(1, 1, 1) : BV_RGB(2, 0, 0));
    os->flush_rect(0, 0, W, TOP);
}

static int on_snake(int x, int y)
{
    for (int i = 0; i < len; i++) {
        int k = (head - i + MAXLEN) % MAXLEN;
        if (sx[k] == x && sy[k] == y) return 1;
    }
    return 0;
}

static void place_food(void)
{
    do { fx = os->random() % GW; fy = os->random() % GH; } while (on_snake(fx, fy));
    cell(fx, fy, 3);
}

static void reset(void)
{
    for (int y = 0; y < GH; y++) for (int x = 0; x < GW; x++) {
        os->fill(x * CELL, TOP + y * CELL, CELL, CELL, ((x + y) & 1) ? C_BG : C_BG2);
    }
    os->flush();
    len = 4; head = 3; dir = next_dir = 0; score = 0; alive = 1; paused = 0;
    for (int i = 0; i < 4; i++) { sx[i] = 6 + i; sy[i] = GH / 2; cell(sx[i], sy[i], i == 3 ? 2 : 1); }
    place_food();
    panel();
}

static void step(void)
{
    static const int ddx[4] = {1, 0, -1, 0}, ddy[4] = {0, 1, 0, -1};
    dir = next_dir;
    int nx = sx[head] + ddx[dir], ny = sy[head] + ddy[dir];
    int tail = (head - len + 1 + MAXLEN) % MAXLEN;
    int grow = nx == fx && ny == fy;
    // moving into the cell the tail is leaving is fine
    int hit = nx < 0 || ny < 0 || nx >= GW || ny >= GH;
    if (!hit && on_snake(nx, ny) && !(nx == sx[tail] && ny == sy[tail] && !grow)) hit = 1;
    if (hit) {
        alive = 0;
        if (score > best) { best = score; os->file_write("/System/snake.dat", &best, sizeof(best)); }
        panel();
        return;
    }
    cell(sx[head], sy[head], 1);
    if (!grow) cell(sx[tail], sy[tail], 0); else len++;
    head = (head + 1) % MAXLEN;
    sx[head] = nx; sy[head] = ny;
    cell(nx, ny, 2);
    if (grow) {
        score += 10;
        if (len < MAXLEN) place_food();
        panel();
    }
}

int app_main(const bevel_api_t *api, const char *arg)
{
    os = api;
    if (os->win_open("Snake", W, H) != 0) return 1;
    if (os->file_read("/System/snake.dat", &best, sizeof(best)) != sizeof(best) || best < 0 || best > 100000) best = 0;
    reset();
    uint32_t next = os->ticks_ms() + 150;
    for (;;) {
        bv_event_t e;
        int wait = (int)(next - os->ticks_ms());
        if (wait < 0) wait = 0;
        if (os->event(&e, wait)) {
            if (e.type == BV_CLOSE) return 0;
            if (e.type != BV_KEY_DOWN || e.repeat) continue;
            int want = -1;
            switch (e.key) {
            case BV_K_RIGHT: case BV_K_D: want = 0; break;
            case BV_K_DOWN: case BV_K_S: want = 1; break;
            case BV_K_LEFT: case BV_K_A: want = 2; break;
            case BV_K_UP: case BV_K_W: want = 3; break;
            case BV_K_P: if (alive) { paused = !paused; panel(); } break;
            case BV_K_ENTER: case BV_K_SPACE: case BV_K_N: if (!alive) { reset(); next = os->ticks_ms() + 150; } break;
            case BV_K_ESC: return 0;
            }
            if (want >= 0 && alive && !paused && ((want + 2) & 3) != dir) next_dir = want;
            continue;
        }
        // timer tick
        int period = 150 - score / 4;
        if (period < 60) period = 60;
        next = os->ticks_ms() + period;
        if (alive && !paused) step();
    }
}
