// BlockCraft by Md Rejaul Korim Sadi (231-115-349), CSE 402 CGIP Lab, Metropolitan University
#ifndef PLAYER_H
#define PLAYER_H

#include "world.h"

struct Input {
    bool forward = false, back = false, left = false, right = false;
    bool jump = false, sneak = false, sprint = false;
};

class Player {
public:
    float x = 64, y = 40, z = 64;   // feet position
    float vx = 0, vy = 0, vz = 0;
    float yaw = 0, pitch = 0;       // degrees
    bool onGround = false;
    bool flying = false;
    bool inWater = false;

    static constexpr float WIDTH = 0.6f;
    static constexpr float HEIGHT = 1.8f;
    static constexpr float EYE = 1.62f;

    void update(const World& w, const Input& in, float dt);
    void lookDir(float& dx, float& dy, float& dz) const;
    bool overlapsBlock(int bx, int by, int bz) const;

private:
    bool collides(const World& w, float px, float py, float pz) const;
    void moveAxis(const World& w, float& pos, float delta, int axis);
};

#endif
