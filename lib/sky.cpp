// BlockCraft by Md Rejaul Korim Sadi (231-115-349), CSE 402 CGIP Lab, Metropolitan University
#include "game.h"
#include "noise.h"
#include <cmath>

static const float PI = 3.14159265f;

bool eyeInWater() {
    const View& v = game.view;
    return game.world->get((int)std::floor(v.x), (int)std::floor(v.y), (int)std::floor(v.z)) == WATER;
}

static void skyColor(float& r, float& g, float& b) {
    if (game.screen == PLAYING && eyeInWater()) { r = 0.08f; g = 0.2f; b = 0.5f; return; }
    if (game.timeOfDay == 0) { r = 0.53f; g = 0.78f; b = 1.0f; }
    else if (game.timeOfDay == 1) { r = 0.95f; g = 0.62f; b = 0.42f; }
    else { r = 0.07f; g = 0.1f; b = 0.22f; }
}

// clears the frame to the sky colour and makes the fog the same colour, so far land fades into the sky
void clearSky() {
    float r, g, b;
    skyColor(r, g, b);
    glClearColor(r, g, b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    GLfloat fogColor[4] = {r, g, b, 1};
    glFogfv(GL_FOG_COLOR, fogColor);
    if (game.screen == TITLE) { glFogf(GL_FOG_START, 110.0f); glFogf(GL_FOG_END, 190.0f); }
    else if (eyeInWater()) { glFogf(GL_FOG_START, 0.0f); glFogf(GL_FOG_END, 16.0f); }
    else { glFogf(GL_FOG_START, 40.0f); glFogf(GL_FOG_END, 95.0f); }
}

static void drawStars() {
    if (game.timeOfDay != 2) return;
    static Noise starNoise(99);
    const View& v = game.view;
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 600; i++) {
        float a = starNoise.hash3(i, 1, 0) * 2.0f * PI;
        float h = starNoise.hash3(i, 2, 0);
        if (h < 0.08f) continue;
        float r = std::sqrt(1.0f - h * h);
        float twinkle = 0.6f + 0.4f * std::sin(game.clock * 2.0f + i);
        glColor3f(twinkle, twinkle, twinkle * 1.05f);
        glVertex3f(v.x + std::cos(a) * r * 150.0f, v.y + h * 150.0f, v.z + std::sin(a) * r * 150.0f);
    }
    glEnd();
}

void drawSky() {
    const View& v = game.view;
    int t = game.timeOfDay;
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_FOG);
    glDepthMask(GL_FALSE);
    drawStars();
    glPushMatrix();
    glTranslatef(v.x, v.y, v.z);   // the sun moves with us, so it looks infinitely far away
    glRotatef(t == 1 ? 15.0f : 55.0f, 1, 0, 0);
    if (t == 2) glColor3f(0.85f, 0.88f, 0.95f);
    else if (t == 1) glColor3f(1.0f, 0.55f, 0.25f);
    else glColor3f(1.0f, 0.95f, 0.6f);
    float s = t == 2 ? 6.0f : 9.0f;
    glBegin(GL_QUADS);
    glVertex3f(-s, -s, -120); glVertex3f(s, -s, -120); glVertex3f(s, s, -120); glVertex3f(-s, s, -120);
    glEnd();
    glPopMatrix();
    glDepthMask(GL_TRUE);
    glEnable(GL_FOG);
}

void drawClouds() {
    static Noise cloudNoise(77);
    const View& v = game.view;
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    float shade = game.timeOfDay == 2 ? 0.35f : 1.0f;
    glColor4f(shade, shade, shade, 0.85f);
    float drift = game.clock * 1.2f, y = WORLD_Y + 14.0f;
    int base = (int)std::floor((v.x + drift) / 8.0f), baseZ = (int)std::floor(v.z / 8.0f);
    glBegin(GL_QUADS);
    for (int i = -20; i <= 20; i++)
        for (int j = -20; j <= 20; j++) {
            int cx = base + i, cz = baseZ + j;
            if (cloudNoise.fractal(cx / 6.0f, cz / 6.0f, 2) < 0.58f) continue;
            float x0 = cx * 8.0f - drift, z0 = cz * 8.0f;
            glVertex3f(x0, y, z0); glVertex3f(x0 + 8, y, z0); glVertex3f(x0 + 8, y, z0 + 8); glVertex3f(x0, y, z0 + 8);
        }
    glEnd();
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}
