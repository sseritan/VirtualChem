//Stefan Seritan, June 25, 2012
//Header file for depthStream.c, which is my main file to get depth information
//from the Kinect

#ifndef VIRCHEM_DEPTHSTREAM_H
#define VIRCHEM_DEPTHSTREAM_H

#include "libfreenect.h"

typedef struct{
	int x;
	int y;
} Point;

//Function Prototypes
//Thread functions
void * fThreadFunc(void * arg);
void * glThreadFunc(void * arg);

//OpenGL functions
void InitGl(int w, int h);
void ResizeGlScene(int w, int h);
void keyPressed(unsigned char key, int x, int y);
void DrawGlScene();

//Freenect callbacks
void depthCallback(freenect_device * dev, void * depth, uint32_t  timestamp);

//Hand tracking functions
void getEdgePixels();

#endif