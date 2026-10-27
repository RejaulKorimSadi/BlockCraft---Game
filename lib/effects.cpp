// BlockCraft by Md Rejaul Korim Sadi (231-115-349), CSE 402 CGIP Lab, Metropolitan University
#include "game.h"
#include <cmath>
#include <vector>

static const float PI = 3.14159265f;

void drawSelection(const RayHit& hit) {
    if (!hit.hit) return;
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.05f, 0.05f, 0.05f);
    glLineWidth(2.0f);
    float e = 0.003f;
    float x0 = hit.x - e, y0 = hit.y - e, z0 = hit.z - e, x1 = hit.x + 1 + e, y1 = hit.y + 1 + e, z1 = hit.z + 1 + e;
    float xs[2] = {x0, x1}, ys[2] = {y0, y1}, zs[2] = {z0, z1};
    glBegin(GL_LINES);
    for (int a = 0; a < 2; a++)
        for (int b = 0; b < 2; b++) {
            glVertex3f(x0, ys[a], zs[b]); glVertex3f(x1, ys[a], zs[b]);
            glVertex3f(xs[a], y0, zs[b]); glVertex3f(xs[a], y1, zs[b]);
            glVertex3f(xs[a], ys[b], z0); glVertex3f(xs[a], ys[b], z1);
        }
    glEnd();
}

struct Particle { float x, y, z, vx, vy, vz, life, u, v, du, dv; };
static std::vector<Particle> particles;
static uint32_t rng = 12345u;

static float frand() {
    rng = rng * 1664525u + 1013904223u;
    return (rng >> 8) / 16777216.0f;
}

void clearParticles() { particles.clear(); }

// little bits of the broken block, each showing a random 4x4 corner of its texture
void spawnBreakParticles(int bx, int by, int bz, Block b) {
    float u0, v0, u1, v1;
    tileUV(tileFor(b, F_SIDE), u0, v0, u1, v1);
    for (int i = 0; i < 28; i++) {
        Particle p;
        p.x = bx + 0.15f + frand() * 0.7f;
        p.y = by + 0.15f + frand() * 0.7f;
        p.z = bz + 0.15f + frand() * 0.7f;
        p.vx = (frand() - 0.5f) * 4.0f;
        p.vy = 1.5f + frand() * 3.0f;
        p.vz = (frand() - 0.5f) * 4.0f;
        p.life = 0.5f + frand() * 0.6f;
        p.du = (u1 - u0) / 4.0f;
        p.dv = (v1 - v0) / 4.0f;
        p.u = u0 + (int)(frand() * 4) * p.du;
        p.v = v0 + (int)(frand() * 4) * p.dv;
        particles.push_back(p);
    }
}

void updateParticles(float dt) {
    for (size_t i = 0; i < particles.size();) {
        Particle& p = particles[i];
        p.life -= dt;
        if (p.life <= 0) { particles[i] = particles.back(); particles.pop_back(); continue; }
        p.vy -= 18.0f * dt;
        float ny = p.y + p.vy * dt;
        if (isSolid(game.world->get((int)std::floor(p.x), (int)std::floor(ny), (int)std::floor(p.z)))) {
            p.vy = 0; p.vx *= 0.6f; p.vz *= 0.6f;
        } else p.y = ny;
        p.x += p.vx * dt;
        p.z += p.vz * dt;
        i++;
    }
}

void drawParticles() {
    if (particles.empty()) return;
    float m[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, m);
    // the first two rows of the view matrix are the camera's right and up directions
    float rx = m[0] * 0.09f, ry = m[4] * 0.09f, rz = m[8] * 0.09f;
    float ux = m[1] * 0.09f, uy = m[5] * 0.09f, uz = m[9] * 0.09f;
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_ALPHA_TEST);
    glDisable(GL_CULL_FACE);
    glColor3f(0.85f, 0.85f, 0.85f);
    glBegin(GL_QUADS);
    for (const Particle& p : particles) {
        glTexCoord2f(p.u, p.v + p.dv);        glVertex3f(p.x - rx - ux, p.y - ry - uy, p.z - rz - uz);
        glTexCoord2f(p.u + p.du, p.v + p.dv); glVertex3f(p.x + rx - ux, p.y + ry - uy, p.z + rz - uz);
        glTexCoord2f(p.u + p.du, p.v);        glVertex3f(p.x + rx + ux, p.y + ry + uy, p.z + rz + uz);
        glTexCoord2f(p.u, p.v);               glVertex3f(p.x - rx + ux, p.y - ry + uy, p.z - rz + uz);
    }
    glEnd();
    glEnable(GL_CULL_FACE);
    glDisable(GL_ALPHA_TEST);
}

static void cubeFace(Block b, Face kind, const float c[4][3], float light) {
    float u0, v0, u1, v1;
    tileUV(tileFor(b, kind), u0, v0, u1, v1);
    glColor3f(light, light, light);
    glTexCoord2f(u0, v1); glVertex3fv(c[0]);
    glTexCoord2f(u1, v1); glVertex3fv(c[1]);
    glTexCoord2f(u1, v0); glVertex3fv(c[2]);
    glTexCoord2f(u0, v0); glVertex3fv(c[3]);
}

// the block in your hand, drawn in its own small 3D view after the world
void drawHand() {
    const View& v = game.view;
    Block b = game.hotbar[game.selected];
    float light = game.world->skyLight((int)std::floor(v.x), (int)std::floor(v.y), (int)std::floor(v.z));
    float bob = game.bobAmount, phase = game.walkPhase;

    glClear(GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70.0, (double)game.winW / game.winH, 0.01, 10.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float swing = game.swingTimer > 0 ? std::sin((1.0f - game.swingTimer / 0.25f) * PI) : 0.0f;
    glTranslatef(0.66f + std::sin(phase) * 0.025f * bob,
                 -0.6f + std::fabs(std::cos(phase)) * 0.025f * bob - swing * 0.12f, -1.05f);
    glRotatef(-swing * 35.0f, 1, 0, 0);
    glRotatef(-12.0f, 0, 1, 0);
    glRotatef(8.0f, 1, 0, 0);
    glScalef(0.34f, 0.34f, 0.34f);

    glDisable(GL_FOG);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_ALPHA_TEST);
    if (isPlant(b)) {
        const float q[4][3] = {{-0.5f, -0.5f, 0}, {0.5f, -0.5f, 0}, {0.5f, 0.5f, 0}, {-0.5f, 0.5f, 0}};
        glDisable(GL_CULL_FACE);
        glBegin(GL_QUADS);
        cubeFace(b, F_SIDE, q, light);
        glEnd();
        glEnable(GL_CULL_FACE);
    } else {
        const float top[4][3] = {{-0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}};
        const float front[4][3] = {{-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}};
        const float left[4][3] = {{-0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, -0.5f}};
        glBegin(GL_QUADS);
        cubeFace(b, F_TOP, top, light);
        cubeFace(b, F_SIDE, front, 0.75f * light);
        cubeFace(b, F_SIDE, left, 0.6f * light);
        glEnd();
    }
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_FOG);
}
