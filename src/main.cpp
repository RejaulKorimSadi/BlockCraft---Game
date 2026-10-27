// BlockCraft - developed by Md Rejaul Korim Sadi (231-115-349), CSE 402 CGIP Lab, Metropolitan University
#include "../lib/game.h"
// 1. OpenGL settings, done once
void initGL() {
    glEnable(GL_DEPTH_TEST);                             // depth buffer: near faces hide far ones
    glEnable(GL_CULL_FACE);                              // back-face culling
    glAlphaFunc(GL_GREATER, 0.5f);                       // alpha test, for holes in leaves and glass
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);   // alpha blending, for water
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);                      // far land fades into the sky
    game.atlas = buildAtlas();                           // one texture holding every block picture
}

// 2. Camera: projection (the lens), then the viewing transform (where we stand)
void setCamera() {
    View v = game.view;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(v.fov, (double)game.winW / game.winH, 0.05, 260.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(-v.pitch, 1, 0, 0);                        // look up and down
    glRotatef(v.yaw, 0, 1, 0);                           // look left and right
    glTranslatef(-v.x, -v.y, -v.z);                      // move the world opposite to the eye
}

// 3. Drawing one frame
void display() {
    clearSky();                                          // clear colour and depth buffers
    setCamera();
    drawSky();
    glEnable(GL_TEXTURE_2D);                             // solid blocks, with texture and alpha test
    glBindTexture(GL_TEXTURE_2D, game.atlas);
    glEnable(GL_ALPHA_TEST);
    game.world->drawOpaque(game.view.x, game.view.z);
    glDisable(GL_ALPHA_TEST);
    RayHit hit = lookAt();                               // the block under the crosshair
    if (game.screen == PLAYING) { drawSelection(hit); drawParticles(); }
    drawClouds();
    glEnable(GL_TEXTURE_2D);                             // water last, blended with what is behind it
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    game.world->drawWater(game.view.x, game.view.z);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    drawOverlay(hit);                                    // 2D layer: HUD, or the title screen
    glutSwapBuffers();                                   // double buffering: show the finished frame
}

// 4. Animation: update and redraw about 60 times a second
void timer(int) {
    updateGame(frameTime());
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    game.winW = w; game.winH = h;
    glViewport(0, 0, w, h);                              // draw into the whole window
}

// 5. Mouse: look around, and break or place the block under the crosshair
void mouseMove(int x, int y) {
    int dx = x - game.winW / 2, dy = y - game.winH / 2;
    if (!lookingAround() || (dx == 0 && dy == 0)) return;
    game.player.yaw += dx * 0.15f;                       // mouse left/right turns, up/down tilts
    game.player.pitch -= dy * 0.15f;
    glutWarpPointer(game.winW / 2, game.winH / 2);       // keep the pointer in the middle
}

void mouseButton(int button, int state, int, int) {
    if (state != GLUT_DOWN || !readyToPlay(button) || scrollHotbar(button)) return;
    RayHit hit = lookAt();
    if (button == GLUT_LEFT_BUTTON) breakBlock(hit);
    if (button == GLUT_RIGHT_BUTTON) placeBlock(hit);
    if (button == GLUT_MIDDLE_BUTTON) pickBlock(hit);
}

// 6. Window and callbacks
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("BlockCraft - developed by Md Rejaul Korim Sadi");
    initGL();
    newWorld(argc > 1 ? std::strtoul(argv[1], nullptr, 10) : 20261028);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(16, timer, 0);
    glutKeyboardFunc(keyDown);                           // keys are handled in lib/game.cpp
    glutKeyboardUpFunc(keyUp);
    glutSpecialFunc(specialDown);
    glutSpecialUpFunc(specialUp);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMove);
    glutPassiveMotionFunc(mouseMove);
    glutIgnoreKeyRepeat(1);
    glutMainLoop();
}
