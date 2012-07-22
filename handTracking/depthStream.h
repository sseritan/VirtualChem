//Stefan Seritan, July 13th, 2012
//Header file to depthStream.c

#ifndef VIRCHEM_DEPTHSTREAM_H
#define VIRCHEM_DEPTHSTREAM_H

#include "libfreenect.h"

//Kinect functions
int initKinect();
void stopKinect();
void* kinectThreadFunc(void* arg);
void depthCB(freenect_device* dev, void* tmpDepth, uint32_t timestamp);
uint8_t* getDepthData();

#endif