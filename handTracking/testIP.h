//Stefan Seritan, July 11th 2012
//Header file for testIP.c

#ifndef VIRCHEM_TESTIP_H
#define VIRCHEM_TESTIP_H

#include "main.h"

/**
Function Prototypes
**/
int testIPFuncs();
int testConversions();
int comparePixelPoint(Point expected, int pixel);
int comparePointPixel(int expected, Point point);
int testLinkedListFuncs();
int compareLists(Node* head1, Node* head2);
int compareNodes(Node* n1, Node* n2);

#endif