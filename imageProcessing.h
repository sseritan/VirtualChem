//Stefan Seritan, July 11th 2012
//Header file for imageProcessing.c

#ifndef VIRCHEM_IMAGEPROCESSING_H
#define VIRCHEM_IMAGEPROCESSING_H

#include <stdint.h>
#include "main.h"

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