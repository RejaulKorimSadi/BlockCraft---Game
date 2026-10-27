// BlockCraft by Md Rejaul Korim Sadi (231-115-349), CSE 402 CGIP Lab, Metropolitan University
#include "textures.h"
#include "blocks.h"
#include "noise.h"
#include <vector>
#include <algorithm>
#include <cmath>

// every texture is painted pixel by pixel here, so the project needs no image files
namespace {

const int W = TILE_PX * ATLAS_COLS;
const int H = TILE_PX * ATLAS_ROWS;
std::vector<unsigned char> pixels(W * H * 4);
Noise rnd(2026);

struct RGBA { int r, g, b, a; };

int clampi(int v) { return std::max(0, std::min(255, v)); }

void put(int tile, int x, int y, RGBA c) {
    int px = (tile % ATLAS_COLS) * TILE_PX + x;
    int py = (tile / ATLAS_COLS) * TILE_PX + y;
    unsigned char* p = &pixels[(py * W + px) * 4];
    p[0] = clampi(c.r); p[1] = clampi(c.g); p[2] = clampi(c.b); p[3] = clampi(c.a);
}

// base colour plus a little random brightness per pixel, which gives the pixel-art grain
RGBA grain(RGBA base, float n, int spread) {
    int d = (int)((n - 0.5f) * 2.0f * spread);
    return {base.r + d, base.g + d, base.b + d, base.a};
}

RGBA stoneAt(float n, float m) {
    RGBA c = grain({125, 125, 125, 255}, n, 20);
    if (m > 0.86f) c = {100, 100, 100, 255};
    return c;
}

// a few small lumps of ore colour pressed into stone
RGBA ore(int x, int y, float n, float m, RGBA colour, int seed) {
    for (int i = 0; i < 5; i++) {
        float cx = 2.5f + rnd.hash(i, seed) * 11.0f, cy = 2.5f + rnd.hash(seed, i) * 11.0f;
        float d = std::sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
        if (d < 1.1f) return grain(colour, n, 18);
        if (d < 1.9f) return grain({colour.r * 2 / 3, colour.g * 2 / 3, colour.b * 2 / 3, 255}, n, 10);
    }
    return stoneAt(n, m);
}

void paint(int tile) {
    for (int y = 0; y < TILE_PX; y++) {
        for (int x = 0; x < TILE_PX; x++) {
            float n = rnd.hash(x + tile * 31, y + tile * 17);
            float m = rnd.hash(x * 7 + tile, y * 5 - tile);
            RGBA c{255, 0, 255, 255};
            switch (tile) {
            case T_GRASS_TOP:
                c = grain({95, 159, 53, 255}, n, 26);
                break;
            case T_DIRT:
                c = grain({134, 96, 67, 255}, n, 22);
                if (m > 0.9f) c = {105, 75, 52, 255};
                break;
            case T_GRASS_SIDE: {
                int edge = 3 + (int)(rnd.hash(x, 99) * 2.5f);   // jagged grass edge
                if (y < edge) c = grain({95, 159, 53, 255}, n, 24);
                else { c = grain({134, 96, 67, 255}, n, 22); if (m > 0.9f) c = {105, 75, 52, 255}; }
                break;
            }
            case T_STONE:
                c = stoneAt(n, m);
                break;
            case T_SAND:
                c = grain({219, 207, 163, 255}, n, 14);
                break;
            case T_LOG_SIDE: {
                int stripe = (x * 3 + (int)(n * 2)) % 5;
                c = grain(stripe == 0 ? RGBA{78, 58, 34, 255} : RGBA{104, 80, 48, 255}, m, 12);
                break;
            }
            case T_LOG_TOP: {
                float dx = x - 7.5f, dy = y - 7.5f;
                int ring = (int)(std::sqrt(dx * dx + dy * dy) * 0.9f);
                if (x == 0 || y == 0 || x == 15 || y == 15) c = grain({104, 80, 48, 255}, n, 10);
                else c = grain(ring % 2 ? RGBA{172, 139, 86, 255} : RGBA{150, 117, 70, 255}, n, 8);
                break;
            }
            case T_LEAVES:
                c = grain({58, 122, 38, 255}, n, 30);
                if (m > 0.78f) c.a = 0;    // holes, cut out with alpha testing
                break;
            case T_PLANKS: {
                bool seam = (y % 4 == 3) || ((y / 4) % 2 == 0 ? x == 11 : x == 4);
                c = seam ? RGBA{110, 84, 50, 255} : grain({162, 130, 78, 255}, n, 12);
                break;
            }
            case T_COBBLE: {
                int cx = (x + (y / 4 % 2) * 3) / 5, cy = y / 4;
                float cell = rnd.hash(cx, cy);
                bool gap = (x + (y / 4 % 2) * 3) % 5 == 0 || y % 4 == 0;
                c = gap ? RGBA{80, 80, 80, 255} : grain({(int)(110 + cell * 40), (int)(110 + cell * 40),
                                                         (int)(110 + cell * 40), 255}, n, 10);
                break;
            }
            case T_GLASS: {
                bool frame = x == 0 || y == 0 || x == 15 || y == 15;
                bool shine = (x - y == 3 || x - y == 4) && x > 4 && x < 12;
                if (frame) c = {200, 230, 240, 255};
                else if (shine) c = {235, 248, 255, 255};
                else c = {220, 240, 250, 0};
                break;
            }
            case T_WATER:
                c = grain({47, 92, 207, 170}, n, 12);
                break;
            case T_BEDROCK:
                c = grain(m > 0.5f ? RGBA{60, 60, 60, 255} : RGBA{95, 95, 95, 255}, n, 20);
                break;
            case T_BRICK: {
                int row = y / 4;
                bool mortar = y % 4 == 0 || (x + (row % 2) * 4) % 8 == 0;
                c = mortar ? RGBA{190, 180, 170, 255} : grain({150, 74, 58, 255}, n, 16);
                break;
            }
            case T_TALL_GRASS: {
                int blade = 6 + (int)(rnd.hash(x, 7) * 9.0f);
                bool on = (x % 3 != 2) && y >= 16 - blade && x > 0 && x < 15;
                c = on ? grain({82, 150, 46, 255}, n, 26) : RGBA{0, 0, 0, 0};
                break;
            }
            case T_ROSE:
            case T_DANDELION: {
                bool rose = tile == T_ROSE;
                float dx = x - 7.5f, dy = y - (rose ? 5.0f : 6.0f);
                float r = std::sqrt(dx * dx + dy * dy);
                c = {0, 0, 0, 0};
                if ((x == 7 || x == 8) && y >= 7) c = {54, 120, 36, 255};
                if (y == 11 && (x == 5 || x == 6 || x == 9 || x == 10)) c = {64, 140, 44, 255};
                if (r < (rose ? 3.2f : 2.4f)) {
                    c = rose ? grain({196, 30, 38, 255}, n, 24) : grain({240, 210, 40, 255}, n, 18);
                    if (r < 1.0f) c = rose ? RGBA{120, 16, 22, 255} : RGBA{200, 140, 20, 255};
                }
                break;
            }
            case T_COAL:    c = ore(x, y, n, m, {44, 44, 44, 255}, 1); break;
            case T_IRON:    c = ore(x, y, n, m, {216, 175, 147, 255}, 2); break;
            case T_GOLD:    c = ore(x, y, n, m, {250, 230, 70, 255}, 3); break;
            case T_DIAMOND: c = ore(x, y, n, m, {96, 232, 240, 255}, 4); break;
            }
            put(tile, x, y, c);
        }
    }
}

}

GLuint buildAtlas() {
    for (int t = 0; t < TILE_COUNT; t++) paint(t);
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    // nearest filtering keeps the blocky pixel look instead of blurring it
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    return id;
}

// repaints only the water tile with moving ripples, then sends those 16x16 pixels to the card
void animateWater(GLuint atlas, float t) {
    unsigned char buf[TILE_PX * TILE_PX * 4];
    for (int y = 0; y < TILE_PX; y++)
        for (int x = 0; x < TILE_PX; x++) {
            float wave = std::sin((x + t * 3.0f) * 0.8f) * std::cos(y * 0.7f - t * 2.2f);
            float n = rnd.hash(x + 341, (y + (int)(t * 4.0f)) % TILE_PX);
            int d = (int)(wave * 16.0f + (n - 0.5f) * 14.0f);
            unsigned char* p = &buf[(y * TILE_PX + x) * 4];
            p[0] = clampi(47 + d); p[1] = clampi(92 + d); p[2] = clampi(207 + d); p[3] = 170;
        }
    glBindTexture(GL_TEXTURE_2D, atlas);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, (T_WATER % ATLAS_COLS) * TILE_PX, (T_WATER / ATLAS_COLS) * TILE_PX,
                    TILE_PX, TILE_PX, GL_RGBA, GL_UNSIGNED_BYTE, buf);
}

void tileUV(int tile, float& u0, float& v0, float& u1, float& v1) {
    const float e = 0.0005f;
    u0 = (float)(tile % ATLAS_COLS) / ATLAS_COLS + e;
    v0 = (float)(tile / ATLAS_COLS) / ATLAS_ROWS + e;
    u1 = u0 + 1.0f / ATLAS_COLS - 2 * e;
    v1 = v0 + 1.0f / ATLAS_ROWS - 2 * e;
}
