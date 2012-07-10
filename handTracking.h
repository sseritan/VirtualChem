//Stefan Seritan, June 27, 2012
//Header file for handTracking.c

#ifndef VIRCHEM_HANDTRACKING_H
#define VIRCHEM_HANDTRACKING_H

#include "libfreenect.h"

/**
Image Segmentation structs and enums
-Region
	Describes a rectangle of pixels
-Node
	Basic linked list structure
-SegStatus
	Tells which way the previous cut was
-Point
	Simple (x,y) point
**/

typedef struct {
	int x;	//X coordinate (from top left corner)
	int y;	//Y coordinate (from top left corner)
} Point;

typedef struct {
	Point ul;		//Upper left corner
	Point br;		//Bottom right corner
} Region;

typedef struct Node {
	Region* reg;
	struct Node* next;
} Node;

typedef enum {
	PREV_H = 0,		//The previous region was segmented horizontally
	PREV_V = 1		//The previous region was segmented vertically
} segStatus;

//Kinect functions
void* kinectThreadFunc(void* arg);
void depthCB(freenect_device* dev, void* tmpDepth, uint32_t timestamp);

//OpenGL functions
void initGraphics(int* argc, char** argv);
void ResizeGLScene(int w, int h);
void keyPressed(unsigned char key, int x, int y);
void DrawGLScene();

//Image processing functions
Node* getHandRegions(uint8_t* depth);
Node* segmentRegions(Node* fullReg, uint8_t* depth, segStatus status);
Node* analyzeRegions(Node* head);
int testVertical(uint8_t* depth, Point start, Point end);
int testHorizontal(uint8_t* depth, Point start, Point end);

//Utility functions
Node* createNode(Point pUl, Point pBr);
Point getCartesian(int pixel);
int getPixel(Point p);

#endif