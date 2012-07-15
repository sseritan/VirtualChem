//Stefan Seritan, July 11th 2012
//Header file that contains the structs, constants, and enums

#ifndef VIRCHEM_MAIN_H
#define VIRCHEM_MAIN_H

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

#endif