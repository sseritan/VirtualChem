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
Node* segmentRegions(Node* fullReg, uint8_t* depth, segStatus status, int attemptFlag) {
	Node* n1, * n2;
	Point ul = fullReg->reg.ul;
	Point br = fullReg->reg.br;
	Region reg1, reg2;
	int foundCut = 0;
	
	//Try to cut the opposite way from the previous
	if (status == PREV_H) {
		//Find average
		int avgX = ((ul.x + br.x)/2);
		int x = avgX;
		
		//Try going right first
		while (x <= br.x && !foundCut) {
			Point start = createPoint(x, ul.y);
			Point end = createPoint(x, br.y);
			
			if (!testVertical(depth, start, end)) {
				foundCut = 1;
				reg1 = createRegion(ul, end);
				reg2 = createRegion(start, br);
				
				n1 = createNode(reg1); n2 = createNode(reg2);
			}
			
			//Increment over 5 pixels
			x += 5;
		}
		
		//Cut was not found to the right, go left
		//Reset to the left of average
		x = avgX - 5;
		while (x >= ul.x && !foundCut) {
			Point start = createPoint(x, ul.y);
			Point end = createPoint(x, br.y);
		
			if (!testVertical(depth, start, end)) {
				foundCut = 1;
				reg1 = createRegion(ul, end);
				reg2 = createRegion(start, br);
				
				n1 = createNode(reg1); n2 = createNode(reg2);
			}
			
			//Increment over 5 pixels
			x -= 5;
		}
		
		//Cut was not found on either side
		if (attemptFlag && !foundCut) {
			//I already tried horizontal cuts, no way to segment
			printf("No way to cut region. Returning full region.\n");
			return fullReg;
		} else if (!foundCut) {
			//Try horizontal split
			printf("Vertical cut not found.\n");
			segmentRegions(fullReg, depth, PREV_H, 1);
		}
	} else if (status == PREV_V) {
		//Find average
		int avgY = ((ul.y + br.y)/2);
		int y = avgY;
		
		//Try to go up first
		while (y >= ul.y && !foundCut) {
			Point start = createPoint(ul.x, y);
			Point end = createPoint(br.x, y);
			
			if(!testHorizontal(depth, start, end)) {
				foundCut = 1;
				reg1 = createRegion(ul, end);
				reg2 = createRegion(start, br);
				
				n1 = createNode(reg1); n2 = createNode(reg2);
			}
			
			//Increment up 5 pixels (remember coord system!!!)
			y -= 5;
		}
		
		y = avgY + 5;
		//Try to go down if cut not found up
		while (y <= br.y && !foundCut) {
			Point start = createPoint(ul.x, y);
			Point end = createPoint(br.x, y);
			
			if (!testHorizontal(depth, start, end)) {
				foundCut = 1;
				reg1 = createRegion(ul, end);
				reg2 = createRegion(start, br);
				
				n1 = createNode(reg1); n2 = createNode(reg2);
			}
			
			//Increment
			y += 5;
		}
		
		//Cut was not found either way
		if (attemptFlag && !foundCut) {
			printf("No way to cut region. Returning full region.\n");
			return fullReg;
		} else if (!foundCut) {
			printf("No horizontal cut possible.\n");
			segmentRegions(fullReg, depth, PREV_V, 1);
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
	if ((reg1.br.x - reg1.ul.x) > 10 && (reg1.br.y - reg1.ul.y) > 10) {
		n1 = segmentRegions(n1, depth, newStatus, 0);
	}
	if ((reg2.br.x - reg2.ul.x) > 10 && (reg2.br.y - reg2.ul.y) > 10) {
		n2 = segmentRegions(n2, depth, newStatus, 0);
	}
	
	//Find the tail to the first node, to link it all together
	Node* tail = n1;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	
	//Link the two lists
	tail->next = n2;
	
	return n1;
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
