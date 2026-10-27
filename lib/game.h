// BlockCraft by Md Rejaul Korim Sadi (231-115-349), CSE 402 CGIP Lab, Metropolitan University
#ifndef GAME_H
#define GAME_H

#include <GL/freeglut.h>
#include <string>
#include <cstdlib>
#include "world.h"
#include "player.h"
#include "textures.h"
#include "sky.h"
#include "effects.h"
#include "hud.h"

extern const char* DEVELOPER;
extern const char* DEV_INFO;
extern const char* VERSION;

enum Screen { TITLE, PLAYING };

// where the camera is this frame: the player's eyes, or a slow orbit on the title screen
struct View {
    float x = 0, y = 0, z = 0, yaw = 0, pitch = 0, fov = 70;
};

struct Game {
    World* world = nullptr;
    Player player;
    Input input;
    View view;
    GLuint atlas = 0;
    Screen screen = TITLE;

    int winW = 1280, winH = 720;
    bool mouseCaptured = false, showHelp = true, showDebug = true, showAbout = false, shotPending = false;
    int timeOfDay = 0;   // 0 day, 1 sunset, 2 night

    Block hotbar[9] = {GRASS, DIRT, STONE, COBBLE, PLANKS, LOG, GLASS, BRICK, ROSE};
    int selected = 0;

    float clock = 0, titleTime = 0, introTimer = 0, swingTimer = 0, messageTimer = 0;
    float walkPhase = 0, bobAmount = 0, fov = 70;
    float signX = 96, signY = 40, signZ = 60;   // centre of the name sign, the title camera looks at it
    int fps = 0, frames = 0;
    std::string message;
};

extern Game game;

void say(const std::string& text);
void newWorld(uint32_t seed);
void startGame();

float frameTime();
void updateGame(float dt);

bool readyToPlay(int button);
bool lookingAround();
RayHit lookAt();
void breakBlock(const RayHit& hit);
void placeBlock(const RayHit& hit);
void pickBlock(const RayHit& hit);
bool scrollHotbar(int button);

void keyDown(unsigned char key, int, int);
void keyUp(unsigned char key, int, int);
void specialDown(int key, int, int);
void specialUp(int key, int, int);

#endif
