//Stefan Seritan, June 27, 2012
//Header file for handTracking.c

#ifndef VIRCHEM_HANDTRACKING_H
#define VIRCHEM_HANDTRACKING_H

#include "main.h"
#include "depthStream.h"

//Hand Tracking functions
Node* getHandRegions(uint8_t* depth);
Node* refineHandRegions(Node* crudeHands);

#endif