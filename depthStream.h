//Stefan Seritan, July 13th, 2012
//Header file to depthStream.c

#ifndef VIRCHEM_DEPTHSTREAM_H
#define VIRCHEM_DEPTHSTREAM_H

#include "libfreenect.h"

//Kinect functions
void* kinectThreadFunc(void* arg);
void depthCB(freenect_device* dev, void* tmpDepth, uint32_t timestamp);

//OpenGL functions
void initGraphics(int* argc, char** argv);
void ResizeGLScene(int w, int h);
void keyPressed(unsigned char key, int x, int y);
void DrawGLScene();

#endif