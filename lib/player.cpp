// BlockCraft by Md Rejaul Korim Sadi (231-115-349), CSE 402 CGIP Lab, Metropolitan University
#include "player.h"
#include <cmath>

static const float PI = 3.14159265f;

void Player::lookDir(float& dx, float& dy, float& dz) const {
    float ry = yaw * PI / 180.0f, rp = pitch * PI / 180.0f;
    dx = std::sin(ry) * std::cos(rp);
    dy = std::sin(rp);
    dz = -std::cos(ry) * std::cos(rp);
}

// does the player's box, standing at (px,py,pz), touch any solid block?
bool Player::collides(const World& w, float px, float py, float pz) const {
    float h = WIDTH / 2;
    int x0 = (int)std::floor(px - h), x1 = (int)std::floor(px + h);
    int y0 = (int)std::floor(py), y1 = (int)std::floor(py + HEIGHT);
    int z0 = (int)std::floor(pz - h), z1 = (int)std::floor(pz + h);
    for (int x = x0; x <= x1; x++)
        for (int y = y0; y <= y1; y++)
            for (int z = z0; z <= z1; z++) {
                if (x < 0 || z < 0 || x >= WORLD_X || z >= WORLD_Z) return true;   // invisible wall at the edge
                if (isSolid(w.get(x, y, z))) return true;
            }
    return false;
}

bool Player::overlapsBlock(int bx, int by, int bz) const {
    float h = WIDTH / 2;
    return x + h > bx && x - h < bx + 1 && y + HEIGHT > by && y < by + 1 && z + h > bz && z - h < bz + 1;
}

// move along one axis at a time; if we hit something, step back until we are just touching it
void Player::moveAxis(const World& w, float& pos, float delta, int axis) {
    const int steps = 4;
    for (int i = 0; i < steps; i++) {
        float old = pos;
        pos += delta / steps;
        if (collides(w, x, y, z)) {
            pos = old;
            if (axis == 1) {
                if (delta < 0) onGround = true;
                vy = 0;
            }
            return;
        }
    }
}

void Player::update(const World& w, const Input& in, float dt) {
    if (dt > 0.05f) dt = 0.05f;
    yaw = std::fmod(yaw + 360.0f, 360.0f);   // keep the heading between 0 and 360
    if (pitch > 89) pitch = 89;
    if (pitch < -89) pitch = -89;

    float ry = yaw * PI / 180.0f;
    float fx = std::sin(ry), fz = -std::cos(ry);
    float rx = std::cos(ry), rz = std::sin(ry);
    float mx = 0, mz = 0;
    if (in.forward) { mx += fx; mz += fz; }
    if (in.back) { mx -= fx; mz -= fz; }
    if (in.right) { mx += rx; mz += rz; }
    if (in.left) { mx -= rx; mz -= rz; }
    float len = std::sqrt(mx * mx + mz * mz);
    if (len > 0) { mx /= len; mz /= len; }

    int headY = (int)std::floor(y + 0.4f);
    inWater = w.get((int)std::floor(x), headY, (int)std::floor(z)) == WATER;

    float speed = flying ? 11.0f : (in.sprint ? 5.6f : 4.3f);
    if (inWater && !flying) speed *= 0.5f;
    vx = mx * speed;
    vz = mz * speed;

    if (flying) {
        vy = (in.jump ? 8.0f : 0.0f) - (in.sneak ? 8.0f : 0.0f);
    } else if (inWater) {
        vy -= 9.0f * dt;
        if (in.jump) vy = 3.0f;
        if (vy < -3.0f) vy = -3.0f;
    } else {
        vy -= 28.0f * dt;                 // gravity
        if (vy < -40.0f) vy = -40.0f;
        if (in.jump && onGround) vy = 8.6f;
    }

    onGround = false;
    moveAxis(w, x, vx * dt, 0);
    moveAxis(w, y, vy * dt, 1);
    moveAxis(w, z, vz * dt, 2);

    if (y < -10) { y = (float)w.surfaceHeight((int)x, (int)z) + 2; vy = 0; }
}
