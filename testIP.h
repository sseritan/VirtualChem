//Stefan Seritan, July 11th 2012
//Header file for testIP.c

#ifndef VIRCHEM_TESTIP_H
#define VIRCHEM_TESTIP_H

#include "main.h"

/**
Function Prototypes
**/

void testConversions();
int comparePixelPoint(Point expected, int pixel);
int comparePointPixel(int expected, Point point);

#endif