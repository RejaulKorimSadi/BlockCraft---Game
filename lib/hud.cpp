// BlockCraft by Md Rejaul Korim Sadi (231-115-349), CSE 402 CGIP Lab, Metropolitan University
// everything drawn flat on the screen: HUD, title screen, developer credit, and the screenshot saver
#include "game.h"
#include <cmath>
#include <cstdio>
#include <ctime>
#include <vector>
#include <algorithm>

static void ortho() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, game.winW, game.winH, 0);   // y grows downwards, like screen pixels
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
}

static void text(float x, float y, const std::string& s, void* font = GLUT_BITMAP_HELVETICA_12,
                 float r = 1, float g = 1, float b = 1, float a = 1) {
    glColor4f(0.1f, 0.1f, 0.1f, a * 0.9f);
    glRasterPos2f(x + 1, y + 1);
    glutBitmapString(font, (const unsigned char*)s.c_str());
    glColor4f(r, g, b, a);
    glRasterPos2f(x, y);
    glutBitmapString(font, (const unsigned char*)s.c_str());
}

static float textWidth(const std::string& s, void* font) {
    return (float)glutBitmapLength(font, (const unsigned char*)s.c_str());
}

static void textCentered(float cx, float y, const std::string& s, void* font, float r = 1, float g = 1,
                         float b = 1, float a = 1) {
    text(cx - textWidth(s, font) / 2, y, s, font, r, g, b, a);
}

// letters spread out, used for small headings like "DEVELOPED BY"
static void spacedText(float cx, float y, const std::string& s, float gap, float r, float g, float b, float a) {
    float w = 0;
    for (char ch : s) w += glutBitmapWidth(GLUT_BITMAP_HELVETICA_12, ch) + gap;
    float x = cx - (w - gap) / 2;
    for (char ch : s) {
        text(x, y, std::string(1, ch), GLUT_BITMAP_HELVETICA_12, r, g, b, a);
        x += glutBitmapWidth(GLUT_BITMAP_HELVETICA_12, ch) + gap;
    }
}

// scalable vector text from freeglut, good for big titles
static float strokeWidth(const std::string& s, float height) {
    return glutStrokeLength(GLUT_STROKE_ROMAN, (const unsigned char*)s.c_str()) * height / 100.0f;
}

static void strokeText(float x, float y, const std::string& s, float height, float lineW, float r, float g,
                       float b, float a) {
    float k = height / 100.0f;
    glEnable(GL_LINE_SMOOTH);
    for (int pass = 0; pass < 2; pass++) {
        float off = pass == 0 ? height * 0.06f : 0.0f;
        if (pass == 0) glColor4f(0.05f, 0.05f, 0.08f, a * 0.7f);
        else glColor4f(r, g, b, a);
        glLineWidth(lineW);
        glPushMatrix();
        glTranslatef(x + off, y + off, 0);
        glScalef(k, -k, 1);
        glutStrokeString(GLUT_STROKE_ROMAN, (const unsigned char*)s.c_str());
        glPopMatrix();
    }
    glDisable(GL_LINE_SMOOTH);
}

static void rect(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x + w, y); glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}

static void gradientRect(float x, float y, float w, float h, const float top[4], const float bottom[4]) {
    glBegin(GL_QUADS);
    glColor4fv(top); glVertex2f(x, y); glVertex2f(x + w, y);
    glColor4fv(bottom); glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}

// a small 3D-looking cube for the game.hotbar, drawn from three flat faces
static void blockIcon(Block b, float cx, float cy, float s, float alpha = 1.0f) {
    float u0, v0, u1, v1;
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_ALPHA_TEST);
    if (isPlant(b)) {
        tileUV(tileFor(b, F_SIDE), u0, v0, u1, v1);
        glColor4f(1, 1, 1, alpha);
        glBegin(GL_QUADS);
        glTexCoord2f(u0, v0); glVertex2f(cx - s, cy - s);
        glTexCoord2f(u1, v0); glVertex2f(cx + s, cy - s);
        glTexCoord2f(u1, v1); glVertex2f(cx + s, cy + s);
        glTexCoord2f(u0, v1); glVertex2f(cx - s, cy + s);
        glEnd();
        glDisable(GL_ALPHA_TEST);
        glDisable(GL_TEXTURE_2D);
        return;
    }
    float hx = s * 0.87f, hy = s * 0.5f;

    tileUV(tileFor(b, F_TOP), u0, v0, u1, v1);
    glColor4f(1, 1, 1, alpha);
    glBegin(GL_QUADS);
    glTexCoord2f(u0, v0); glVertex2f(cx, cy - s);
    glTexCoord2f(u1, v0); glVertex2f(cx + hx, cy - hy);
    glTexCoord2f(u1, v1); glVertex2f(cx, cy);
    glTexCoord2f(u0, v1); glVertex2f(cx - hx, cy - hy);
    glEnd();

    tileUV(tileFor(b, F_SIDE), u0, v0, u1, v1);
    glColor4f(0.78f, 0.78f, 0.78f, alpha);
    glBegin(GL_QUADS);
    glTexCoord2f(u0, v0); glVertex2f(cx - hx, cy - hy);
    glTexCoord2f(u1, v0); glVertex2f(cx, cy);
    glTexCoord2f(u1, v1); glVertex2f(cx, cy + s);
    glTexCoord2f(u0, v1); glVertex2f(cx - hx, cy + hy);
    glEnd();
    glColor4f(0.6f, 0.6f, 0.6f, alpha);
    glBegin(GL_QUADS);
    glTexCoord2f(u0, v0); glVertex2f(cx, cy);
    glTexCoord2f(u1, v0); glVertex2f(cx + hx, cy - hy);
    glTexCoord2f(u1, v1); glVertex2f(cx + hx, cy + hy);
    glTexCoord2f(u0, v1); glVertex2f(cx, cy + s);
    glEnd();
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_TEXTURE_2D);
}

static const char* GLYPHS[][7] = {
    {"1111.", "1...1", "1...1", "1111.", "1...1", "1...1", "1111."},   // B
    {"1....", "1....", "1....", "1....", "1....", "1....", "11111"},   // L
    {".111.", "1...1", "1...1", "1...1", "1...1", "1...1", ".111."},   // O
    {".1111", "1....", "1....", "1....", "1....", "1....", ".1111"},   // C
    {"1...1", "1..1.", "1.1..", "11...", "1.1..", "1..1.", "1...1"},   // K
    {"1111.", "1...1", "1...1", "1111.", "1.1..", "1..1.", "1...1"},   // R
    {".111.", "1...1", "1...1", "11111", "1...1", "1...1", "1...1"},   // A
    {"11111", "1....", "1....", "1111.", "1....", "1....", "1...."},   // F
    {"11111", "..1..", "..1..", "..1..", "..1..", "..1..", "..1.."},   // T
};

static int glyphIndex(char c) {
    const char* order = "BLOCKRAFT";
    for (int i = 0; order[i]; i++)
        if (order[i] == c) return i;
    return -1;
}

// "BLOCKCRAFT" built out of little stone blocks with grass on top
static void drawLogo(float cx, float top, float px, float alpha) {
    const char* word = "BLOCKCRAFT";
    int n = 10;
    float width = n * 6 * px - px;
    float x0 = cx - width / 2;
    float u0, v0, u1, v1;
    for (int pass = 0; pass < 2; pass++) {
        if (pass == 0) { glDisable(GL_TEXTURE_2D); glColor4f(0, 0, 0, 0.55f * alpha); }
        else glEnable(GL_TEXTURE_2D);
        for (int i = 0; i < n; i++) {
            int g = glyphIndex(word[i]);
            for (int row = 0; row < 7; row++)
                for (int col = 0; col < 5; col++) {
                    if (GLYPHS[g][row][col] != '1') continue;
                    float x = x0 + (i * 6 + col) * px, y = top + row * px;
                    if (pass == 0) { rect(x + px * 0.4f, y + px * 0.4f, px, px); continue; }
                    bool grassy = row == 0 || GLYPHS[g][row - 1][col] != '1';
                    tileUV(grassy ? T_GRASS_SIDE : T_STONE, u0, v0, u1, v1);
                    float l = 1.0f - row * 0.05f;
                    glColor4f(l, l, l, alpha);
                    glBegin(GL_QUADS);
                    glTexCoord2f(u0, v0); glVertex2f(x, y);
                    glTexCoord2f(u1, v0); glVertex2f(x + px, y);
                    glTexCoord2f(u1, v1); glVertex2f(x + px, y + px);
                    glTexCoord2f(u0, v1); glVertex2f(x, y + px);
                    glEnd();
                }
        }
    }
    glDisable(GL_TEXTURE_2D);
}

static float ease(float t) {
    if (t < 0) return 0;
    if (t > 1) return 1;
    return 1 - (1 - t) * (1 - t) * (1 - t);
}

// the "developed by" card, shared by the title screen and the I key
static void drawCreditCard(float cx, float y, float appear) {
    float a = ease(appear);
    float w = 620, h = 168;
    float x = cx - w / 2;
    y += (1 - a) * 24;
    glColor4f(0.03f, 0.05f, 0.08f, 0.72f * a);
    rect(x, y, w, h);
    glColor4f(0.37f, 0.69f, 0.21f, a);
    rect(x, y, w, 4);

    spacedText(cx, y + 34, "DEVELOPED BY", 5, 0.62f, 0.85f, 0.45f, a);

    std::string name = DEVELOPER;
    float nh = 36, nw = strokeWidth(name, nh);
    strokeText(cx - nw / 2, y + 88, name, nh, 3.0f, 1.0f, 0.85f, 0.32f, a);

    float line = nw * ease(appear * 1.4f - 0.4f);
    glColor4f(1.0f, 0.85f, 0.32f, 0.9f * a);
    rect(cx - line / 2, y + 102, line, 2);

    textCentered(cx, y + 130, DEV_INFO, GLUT_BITMAP_HELVETICA_12, 0.85f, 0.88f, 0.92f, a);
    textCentered(cx, y + 152, "CSE 402  Computer Graphics & Image Processing Lab", GLUT_BITMAP_HELVETICA_12,
                 0.6f, 0.65f, 0.7f, a);
}

static void drawTitle() {
    ortho();
    float t = game.titleTime;
    const float clearTop[4] = {0.02f, 0.03f, 0.06f, 0.65f}, clear0[4] = {0.02f, 0.03f, 0.06f, 0.0f};
    gradientRect(0, 0, (float)game.winW, game.winH * 0.45f, clearTop, clear0);
    gradientRect(0, game.winH * 0.55f, (float)game.winW, game.winH * 0.45f, clear0, clearTop);

    float cx = game.winW / 2.0f;
    float px = std::min(game.winW / 66.0f, 15.0f);
    float drop = (1 - ease(t / 0.9f)) * -60.0f;
    drawLogo(cx, game.winH * 0.14f + drop, px, ease(t / 0.6f));

    // yellow splash line that pulses, like the one on the Minecraft menu
    if (t > 0.8f) {
        std::string splash = "Made from scratch in C++!";
        float s = 1.0f + 0.05f * std::sin(game.clock * 7.0f);
        float sh = 17.0f * s;
        glPushMatrix();
        glTranslatef(cx + 59 * px * 0.38f, game.winH * 0.14f + 6.6f * px, 0);
        glRotatef(-12.0f, 0, 0, 1);
        strokeText(-strokeWidth(splash, sh) / 2, 0, splash, sh, 2.0f, 1.0f, 1.0f, 0.25f, ease((t - 0.8f) / 0.4f));
        glPopMatrix();
    }

    drawCreditCard(cx, game.winH * 0.47f, (t - 0.9f) / 0.8f);

    if (t > 1.8f) {
        float blink = 0.55f + 0.45f * std::sin(game.clock * 4.0f);
        glColor4f(0.03f, 0.05f, 0.08f, 0.6f);
        rect(cx - 170, game.winH * 0.47f + 198, 340, 58);
        textCentered(cx, game.winH * 0.47f + 220, "Click or press Enter to play", GLUT_BITMAP_HELVETICA_18, 1, 1, 1, blink);
        textCentered(cx, game.winH * 0.47f + 246, "N  new world      Esc  quit", GLUT_BITMAP_HELVETICA_12, 0.8f, 0.82f, 0.86f, 0.8f);
    }
    char seed[64];
    std::snprintf(seed, sizeof seed, "Seed %u", game.world->seed);
    text(14, game.winH - 14.0f, seed, GLUT_BITMAP_HELVETICA_12, 0.8f, 0.82f, 0.86f, 0.8f);
    text(game.winW - 14 - textWidth(std::string("BlockCraft ") + VERSION, GLUT_BITMAP_HELVETICA_12), game.winH - 14.0f,
         std::string("BlockCraft ") + VERSION, GLUT_BITMAP_HELVETICA_12, 0.8f, 0.82f, 0.86f, 0.8f);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

static const char* facing() {
    float a = std::fmod(game.player.yaw + 360.0f + 45.0f, 360.0f);
    static const char* names[] = {"North", "East", "South", "West"};
    return names[(int)(a / 90.0f) % 4];
}

// the name popup that drops in when the game starts
static void drawIntroToast() {
    if (game.introTimer <= 0) return;
    float shown = 6.0f - game.introTimer;
    float slide = std::min(ease(shown / 0.5f), ease(game.introTimer / 0.5f));
    float w = 430, h = 64, x = game.winW / 2.0f - w / 2, y = -h + slide * (h + 18);
    glColor4f(0.03f, 0.05f, 0.08f, 0.8f);
    rect(x, y, w, h);
    glColor4f(0.37f, 0.69f, 0.21f, 1);
    rect(x, y + h - 3, w, 3);
    blockIcon(GRASS, x + 36, y + 32, 16);
    glEnable(GL_BLEND);
    text(x + 68, y + 26, "Welcome to BlockCraft", GLUT_BITMAP_HELVETICA_18, 1, 1, 1, 1);
    text(x + 68, y + 46, std::string("developed by ") + DEVELOPER, GLUT_BITMAP_HELVETICA_12, 1.0f, 0.85f, 0.32f, 1);
}

// always on screen: who made this
static void drawDevBadge() {
    float w = 214, h = 42, x = 12, y = game.winH - h - 12;
    glColor4f(0.03f, 0.05f, 0.08f, 0.55f);
    rect(x, y, w, h);
    glColor4f(0.37f, 0.69f, 0.21f, 1);
    rect(x, y, 3, h);
    blockIcon(GRASS, x + 24, y + 21, 11);
    glEnable(GL_BLEND);
    text(x + 44, y + 17, "DEVELOPER", GLUT_BITMAP_HELVETICA_10, 0.62f, 0.85f, 0.45f, 1);
    text(x + 44, y + 33, DEVELOPER, GLUT_BITMAP_HELVETICA_12, 1.0f, 0.85f, 0.32f, 1);
}

static void drawHUD(const RayHit& hit) {
    ortho();

    if (game.timeOfDay == 2) {
        glColor4f(0.02f, 0.03f, 0.12f, 0.3f);
        rect(0, 0, (float)game.winW, (float)game.winH);
    }
    if (eyeInWater()) {
        glColor4f(0.1f, 0.25f, 0.7f, 0.35f);
        rect(0, 0, (float)game.winW, (float)game.winH);
    }

    float cx = game.winW / 2.0f, cy = game.winH / 2.0f;
    glColor4f(1, 1, 1, 0.9f);
    rect(cx - 10, cy - 1, 20, 2);
    rect(cx - 1, cy - 10, 2, 20);

    const float slot = 50.0f;
    float bx = cx - slot * 4.5f, by = game.winH - slot - 14.0f;
    glColor4f(0, 0, 0, 0.45f);
    rect(bx - 4, by - 4, slot * 9 + 8, slot + 8);
    for (int i = 0; i < 9; i++) {
        float sx = bx + i * slot;
        glColor4f(0.55f, 0.55f, 0.55f, 0.5f);
        rect(sx + 2, by + 2, slot - 4, slot - 4);
        if (i == game.selected) {
            glColor4f(1, 1, 1, 1);
            rect(sx, by, slot, 3); rect(sx, by + slot - 3, slot, 3);
            rect(sx, by, 3, slot); rect(sx + slot - 3, by, 3, slot);
        }
        blockIcon(game.hotbar[i], sx + slot / 2, by + slot / 2 + 1, 15.0f);
        glEnable(GL_BLEND);
        text(sx + 5, by + 14, std::to_string(i + 1));
    }
    textCentered(cx, by - 12, blockName(game.hotbar[game.selected]), GLUT_BITMAP_HELVETICA_18);

    if (game.showDebug) {
        char buf[160];
        text(10, 20, std::string("BlockCraft ") + VERSION, GLUT_BITMAP_HELVETICA_18);
        std::snprintf(buf, sizeof buf, "FPS: %d", game.fps);
        text(10, 42, buf);
        std::snprintf(buf, sizeof buf, "XYZ: %.1f / %.1f / %.1f   Facing: %s", game.player.x, game.player.y, game.player.z, facing());
        text(10, 60, buf);
        std::snprintf(buf, sizeof buf, "Mode: %s%s   Seed: %u", game.player.flying ? "Flying" : "Walking",
                      game.player.inWater ? " (swimming)" : "", game.world->seed);
        text(10, 78, buf);
        if (hit.hit) {
            std::snprintf(buf, sizeof buf, "Looking at: %s (%d, %d, %d)", blockName(game.world->get(hit.x, hit.y, hit.z)),
                          hit.x, hit.y, hit.z);
            text(10, 96, buf);
        }
    }

    if (game.showHelp && !game.showAbout) {
        const char* keys[][2] = {{"Mouse", "look around"}, {"W A S D", "move"},
                                 {"Space", "jump / swim / fly up"}, {"Ctrl", "sprint"}, {"Shift", "fly down"},
                                 {"Left click", "break block"}, {"Right click", "place block"},
                                 {"Middle click", "pick the block you look at"}, {"1-9 / wheel", "choose block"},
                                 {"F", "fly on / off"}, {"T", "day / sunset / night"},
                                 {"F2", "screenshot"}, {"F5 / F9", "save / load world"},
                                 {"N", "new random world"}, {"I", "about the developer"},
                                 {"Esc", "free mouse, again for menu"}};
        int rows = 16;
        float w = 330, h = 44.0f + rows * 15 + 8, x = game.winW - w - 14, y = 14;
        glColor4f(0, 0, 0, 0.55f);
        rect(x, y, w, h);
        text(x + 12, y + 22, "CONTROLS   (H to hide)");
        for (int i = 0; i < rows; i++) {
            text(x + 12, y + 44 + i * 15, keys[i][0]);
            text(x + 118, y + 44 + i * 15, keys[i][1]);
        }
    }

    if (!game.mouseCaptured) textCentered(cx, cy - 30, "Click inside the window to play", GLUT_BITMAP_HELVETICA_18);
    if (game.messageTimer > 0) textCentered(cx, by - 44, game.message, GLUT_BITMAP_HELVETICA_18);

    drawDevBadge();
    drawIntroToast();
    if (game.showAbout) drawCreditCard(cx, cy - 84, 1.0f);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

static void put32(unsigned char* p, uint32_t v) { p[0] = v; p[1] = v >> 8; p[2] = v >> 16; p[3] = v >> 24; }

// BMP rows go bottom to top, which is exactly the order glReadPixels gives us
void saveScreenshot() {
    int rowBytes = (game.winW * 3 + 3) & ~3;
    std::vector<unsigned char> pixels(rowBytes * game.winH);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, game.winW, game.winH, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels.data());

    unsigned char header[54] = {'B', 'M'};
    put32(header + 2, 54 + (uint32_t)pixels.size());
    put32(header + 10, 54);
    put32(header + 14, 40);
    put32(header + 18, game.winW);
    put32(header + 22, game.winH);
    header[26] = 1;
    header[28] = 24;
    put32(header + 34, (uint32_t)pixels.size());

    char name[64];
    std::time_t now = std::time(nullptr);
    std::strftime(name, sizeof name, "screenshot_%Y%m%d_%H%M%S.bmp", std::localtime(&now));
    FILE* f = std::fopen(name, "wb");
    if (!f) { say("Could not save the screenshot"); return; }
    std::fwrite(header, 1, 54, f);
    std::fwrite(pixels.data(), 1, pixels.size(), f);
    std::fclose(f);
    say(std::string("Saved ") + name);
}

void drawOverlay(const RayHit& hit) {
    game.frames++;
    if (game.screen == PLAYING) {
        drawHand();
        drawHUD(hit);
    } else {
        drawTitle();
    }
    if (game.shotPending) { game.shotPending = false; saveScreenshot(); }   // read the frame before it is swapped away
}
