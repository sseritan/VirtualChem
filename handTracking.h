//Stefan Seritan, June 27, 2012
//Header file for handTracking.c

#ifndef VIRCHEM_HANDTRACKING_H
#define VIRCHEM_HANDTRACKING_H

#include "libfreenect.h"

//Struct for image segmentation
//Size = 8
typedef struct {
	int ul;		//Upper left corner
	int br;		//Bottom right corner
	int empty; 	//if empty, there is nothing of interest in this region
} Region;

//Kinect functions
void* kinectThreadFunc(void* arg);
void depthCB(freenect_device* dev, void* tmpDepth, uint32_t timestamp);

//OpenGL functions
void initGraphics(int* argc, char** argv);
void ResizeGLScene(int w, int h);
void keyPressed(unsigned char key, int x, int y);
void DrawGLScene();

//Image processing functions


#endif