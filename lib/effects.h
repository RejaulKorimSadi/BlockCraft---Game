// BlockCraft by Md Rejaul Korim Sadi (231-115-349), CSE 402 CGIP Lab, Metropolitan University
#ifndef EFFECTS_H
#define EFFECTS_H

#include "world.h"

void drawSelection(const RayHit& hit);
void spawnBreakParticles(int x, int y, int z, Block b);
void updateParticles(float dt);
void drawParticles();
void clearParticles();
void drawHand();

#endif
