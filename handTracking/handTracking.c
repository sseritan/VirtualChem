//Stefan Seritan, June 27, 2012
//handTracking.c, takes depth data from the depth stream and turns it into events

//Basic includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "handTracking.h"
#include "imageProcessing.h"
#include "main.h"

//Get the hand regions
Node* getHandRegions(uint8_t* depth) {
	Point p1 = createPoint(0,0); Point p2 = createPoint(639, 479);
	Region full = createRegion(p1, p2);
	Node* fullRegion = createNode(full);
	
	//Segment the image
	Node* head = segmentRegions(fullRegion, depth, PREV_H);
	
	//Throw out small segments
	head = filterRegions(head);
	Node* current = head;
	
	int handCount = 0;
	while (current != NULL) {
		handCount++;
		current = current->next;
	}
	
	if (!handCount) {
		printf("No hands detected.\n");
	} else if (handCount == 1) {
		printf("One hand detected.\n");
	} else if (handCount == 2) {
		printf("Two hands detected.\n");
	} else {
		printf("More than two hands detected.\n");
	}
	
	return head;
}