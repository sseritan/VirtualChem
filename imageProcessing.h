//Stefan Seritan, July 11th 2012
//Header file for imageProcessing.c

#ifndef VIRCHEM_IMAGEPROCESSING_H
#define VIRCHEM_IMAGEPROCESSING_H

#include <stdint.h>

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
	Region reg;			//The region
	struct Node* next;	//Pointer to the next element in the linked list
} Node;

typedef enum {
	PREV_H = 0,		//The previous region was segmented horizontally
	PREV_V = 1		//The previous region was segmented vertically
} segStatus;


/**
Function Prototypes
**/

//Image processing functions
Node* segmentRegions(Node* fullReg, uint8_t* depth, segStatus status, int attemptFlag);
void analyzeRegions(Node* head);
int testVertical(uint8_t* depth, Point start, Point end);
int testHorizontal(uint8_t* depth, Point start, Point end);
int testRegionSize(Node* node);

//Utility functions
Point createPoint(int X, int Y);
Region createRegion(Point p1, Point p2);
Node* createNode(Region r);
void freeNode(Node* prev, Node* current);
Point getCartesian(int pixel);
int getPixel(Point p);

#endif