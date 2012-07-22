//Stefan Seritan, July 13th 2012
//Header file for fakeController.h

#ifndef VIRCHEM_FAKE_H
#define VIRCHEM_FAKE_H

#include <stdint.h>

//OpenGL functions
void initGraphics(int* argc, char** argv);
void ResizeGLScene(int w, int h);
void keyPressed(unsigned char key, int x, int y);
void DrawGLScene();
void updateDepthTexture(uint8_t* depth);

#endif