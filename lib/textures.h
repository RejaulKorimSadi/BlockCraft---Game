// BlockCraft by Md Rejaul Korim Sadi (231-115-349), CSE 402 CGIP Lab, Metropolitan University
#ifndef TEXTURES_H
#define TEXTURES_H

#include <GL/freeglut.h>

const int TILE_PX = 16;
const int ATLAS_COLS = 8;
const int ATLAS_ROWS = 4;

GLuint buildAtlas();
void animateWater(GLuint atlas, float time);
void tileUV(int tile, float& u0, float& v0, float& u1, float& v1);

#endif
