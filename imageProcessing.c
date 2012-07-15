//Stefan Seritan, July 11th 2012
//imageProcessing.c, segments image and throws out the really small regions

//Basic includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "imageProcessing.h"

/**
Image Processing Functions

The core of this will be the recursive segmentRegions function, which will recursively
segment the image into smaller and smaller regions (but not if there is something there).
Once the size of the regions is small enough (20 by 20 pixels), they are discarded.
**/

//Segment the image into regions, either containing white or smaller than 10 by 10
Node* segmentRegions(Node* fullReg, uint8_t* depth, segStatus status) {
	Node* n1, * n2;
	Point ul = fullReg->reg.ul;
	Point br = fullReg->reg.br;
	
	//Try to cut the opposite way from the previous
	if (status == PREV_H) {
		//Pass to cutting function
		int cutX = findVerticalCut(depth, ul, br);
		
		if (cutX >= 0) {
			//Cut was found
			n1 = createNode(createRegion(ul, createPoint(cutX, br.y)));
			n2 = createNode(createRegion(createPoint(cutX, ul.y), br));
		} else {
			int cutY = findHorizontalCut(depth, ul, br);
			
			if (cutY >= 0) {
				n1 = createNode(createRegion(ul, createPoint(br.x, cutY)));
				n2 = createNode(createRegion(createPoint(ul.x, cutY), br));
			} else {
				return fullReg;
			}
		}
	} else if (status == PREV_V) {
		int cutY = findHorizontalCut(depth, ul, br);
		
		if (cutY >= 0) {
			n1 = createNode(createRegion(ul, createPoint(br.x, cutY)));
			n2 = createNode(createRegion(createPoint(ul.x, cutY), br));
		} else {
			int cutX = findVerticalCut(depth, ul, br);
			
			if (cutX >= 0) {
				n1 = createNode(createRegion(ul, createPoint(cutX, br.y)));
				n2 = createNode(createRegion(createPoint(cutX, ul.y), br));
			} else {
				return fullReg;
			}
		}
	}
	
	segStatus newStatus;
	if (status == PREV_H) {
		newStatus = PREV_V;
	} else {
		newStatus = PREV_H;
	}
	
	//MAGIC... make the recursive calls
	//Unless these segments are smaller than 10 pixels in any dimension
	if ((n1->reg.br.x - n1->reg.ul.x) > 10 && (n1->reg.br.y - n1->reg.ul.y) > 10) {
		n1 = segmentRegions(n1, depth, newStatus);
	}
	if ((n2->reg.br.x - n2->reg.ul.x) > 10 && (n2->reg.br.y - n2->reg.ul.y) > 10) {
		n2 = segmentRegions(n2, depth, newStatus);
	}
	
	//Find the tail to the first node, to link it all together
	Node* tail = n1;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	
	tail->next = n2;
	
	return n1;
}
//Find a vertical cut position
int findVerticalCut(uint8_t* depth, Point ul, Point br) {
	int avg = ((ul.x + br.x)/2);
	int x = avg;
	int directionFlag = 1;
	
	//Try to cut to the right, then the left
	while (x < br.x && x > ul.x) {
		Point start = createPoint(x, ul.y);
		Point end = createPoint(x, br.y);
		
		if (!testVertical(depth, start, end)) {
			return x;
		}
		
		if (directionFlag) {
			x += 5;
			if (x > br.x) {
				//Reset to left of middle, switch direction
				x = avg - 5;
				directionFlag = 0;
			}
		} else {
			x -= 5;
		}
	}
	
	//Haven't found cut, return negative
	return -1;
}

//Find a horizontal cut position
int findHorizontalCut(uint8_t* depth, Point ul, Point br) {
	int avg = ((ul.y + br.x)/2);
	int y = avg;
	int directionFlag = 1;
	
	//Try to find cut upwards first, then down
	//Remember my coordinate system
	while (y < br.y && y > ul.y) {
		Point start = createPoint(ul.x, y);
		Point end = createPoint(br.x, y);
		
		if (!testHorizontal(depth, start, end)) {
			return y;
		}
		
		if (directionFlag) {
			y -= 5;
			if (y < ul.y) {
				//Reset to below average, switch direction
				y = avg + 5;
				directionFlag = 0;
			}
		} else {
			y += 5;
		}
	}
	
	//Haven't found a cut, return negative
	return -1;
}

//Throw out all of the small nodes
Node* filterRegions(Node* head) {
	//Case of 0
	if (head == NULL) {
		printf("Error occurred in segmentRegions, no Nodes passed to analyzeRegions.\n");
		return NULL;
	}
	
	//Case of one
	if (head->next == NULL) {
		if (testRegionSize(head)) {
			return NULL;
		} else {
			return head;
		}
	}
	
	//Test all Nodes between head and tail
	Node* current = head->next;
	Node* prev = head;
	while (current->next != NULL) {
		if (testRegionSize(current)) {
			freeNode(prev, current);
			current = prev->next;
		} else {
			prev = current;
			current = current->next;
		}
	}
	
	//Base cases
	//Tail
	if (testRegionSize(current)) {
		freeNode(prev, current);
	}
	
	//Head
	if (testRegionSize(head)) {
		head = freeHeadNode(head);
	}
	
	if (head == NULL) {
		return head;
	}
	
	return head;
}

//Test that the node is smaller than 10 in at least one dimension
int testRegionSize(Node* node) {
	return (node->reg.br.x - node->reg.ul.x <= 10 || node->reg.br.y - node->reg.ul.y <= 10);
}

//Go between the two points vertically and test for white
//Returns number of times it hits white (if 0, can segment there)
int testVertical(uint8_t* depth, Point start, Point end) {
	//Get the pixel values for the points
	int s = getPixel(start); int e = getPixel(end);
	
	int whiteCount = 0;
	
	//Test each pixel in the line
	while (s <= e) {
		if (depth[s]) {
			whiteCount++;
		}
		
		//Increment down a full row
		//If performance not high enough, I can skip more rows here
		s += 640;
	}
	
	return whiteCount;
}

//Go between the two points horizontally and test for white
//Returns number of times it hits white (if 0, can segment there)
int testHorizontal(uint8_t* depth, Point start, Point end) {
	int s = getPixel(start); int e = getPixel(end);
	
	int whiteCount = 0;
	
	while (s <= e) {
		if (depth[s]) {
			whiteCount++;
		}
		
		s++;
	}
	
	return whiteCount;
}

/**
General Utility Functions

Basic functions that I need, but aren't really pure image processing
Mostly constructors or conversion functions
**/

//Simple constructor for the Node struct
Node* createNode(Region r) {
	Node* node = (Node*)malloc(sizeof(Node));
	node->reg = r;
	node->next = NULL;
	return node;
}

//Destructor for the Node struct, also keeping the the list together, and freeing memory
void freeNode(Node* prev, Node* current) {
	//Make the previous point to the next, then free
	prev->next = current->next;
	free(current);
}

Node* freeHeadNode(Node* head) {
	Node* newHead = head->next;
	free(head);
	return newHead;
}

//Simple constructor for the Point struct
Point createPoint(int X, int Y) {
	Point p;
	p.x = X;
	p.y = Y;
	return p;
}

Region createRegion(Point p1, Point p2) {
	Region reg;
	reg.ul = p1;
	reg.br = p2;
	return reg;
}

//Simple function to convert from a pixel to a Point value
Point getCartesian(int pixel) {
	Point p;
	
	//Y coord is easy, the division of ints will always round down
	p.y = pixel/640;
	
	//X coord is easy if on the first line, so we translate the pixel vertically (no change in X)
	while (pixel >= 640) {
		pixel -= 640;
	}
	
	p.x = pixel;
	
	return p;
}

//Simple function to convert from a Point to a pixel value
int getPixel(Point p) {
	//First get to the right row, then move over the right amount
	return (p.y*640) + p.x;
}
