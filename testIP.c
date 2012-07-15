//Stefan Seritan, July 11th 2012
//testIP.c, test suite for imageProcessing.c

//Basic includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "main.h"
#include "testIP.h"
#include "imageProcessing.h"

//Main testing function
int main(int argc, char** argv) {
	printf("Running test suite for imageProcessing.c.\n");
	
	//Run subtests
	int totalErrors = 0;
	totalErrors += testConversions();
	totalErrors += testLinkedListFuncs();
	totalErrors += testIPFuncs();
	
	printf("\n");
	
	//Notify of test success/failure
	if (!totalErrors) {
		printf("Every test passed!\n");
	} else if (totalErrors == 1) {
		printf("There was 1 test that failed.\n");
	} else {
		printf("There were %i tests that failed.\n", totalErrors);
	}
	
	printf("Exiting test suite.\n");
	
	return 0;
}

/**
Testing IP Functions

Tests the segment and filter functions
I'm fairly confident in the performance of the segment function
**/

int testIPFuncs() {
	printf("\nTesting image processing functions.\n");
	int errorCount = 0;
	
	printf("Testing region filtering.\n");
	Point p1 = createPoint(0, 0); Point p2 = createPoint(9,9);
	Point p3 = createPoint(12, 12); Point p4 = createPoint(13, 13);
	Region small = createRegion(p1, p2); Region large1 = createRegion(p1, p3);
	Region large2 = createRegion(p1, p4);
	
	Node* n1 = createNode(large1); Node* n2 = createNode(small); Node* n3 = createNode(small);
	Node* n4 = createNode(large2); Node* n5 = createNode(small); Node* n6 = createNode(large1);
	
	Node* e1 = createNode(large1); Node* e2 = createNode(large2); Node* e3 = createNode(large1);
	
	//Initialize the two lists
	n1->next = n2; n2->next = n3; n3->next = n4; n4->next = n5; n5->next = n6;
	e1->next = e2; e2->next = e3;
	
	Node* result1 = filterRegions(n1);
	
	errorCount += compareLists(result1, e1);
	
	//Initialize a "harder" list
	n2 = createNode(small); n3 = createNode(small); n5 = createNode(small);
	n2->next = n3; n3->next = n1; n1->next = n4; n4->next = n6; n6->next = n5;
	
	Node* result2 = filterRegions(n2);
	
	errorCount += compareLists(result2, e1);
	
	printf("Testing segmentation and filtering.\n");
	
	//Empty test
	uint8_t* depth = (uint8_t*)malloc(640*480);
	for (int i = 0; i < 640*480; i++) {
		depth[i] = 0;
	}
	
	Node* full = createNode(createRegion(createPoint(0, 0), createPoint(639, 479)));
	Node* result3 = segmentRegions(full, depth, PREV_H);
	result3 = filterRegions(result3);
	
	errorCount += compareLists(result3, NULL);

	//Test with one region to be hit
	for (int i = 0; i < 480; i++) {
		for (int j = 0; j < 640; j++) {
			if (i >= 5  && i <= 15 && j >= 5 && j <= 15) {
				depth[640*i + j] = 1;
			}
		}
	}
	
	Node* result4 = segmentRegions(full, depth, PREV_H);
	result4 = filterRegions(result4);
	
	if (result4 != NULL && result4->next == NULL) {
		printf("Test passed.\n");
	} else {
		printf("Test failed.\n");
		errorCount++;
	}
	
	//Test with two regions, not perfectly horizontal
	for (int i = 0; i < 480; i++) {
		for (int j = 0; j < 640; j++) {
			if (i >= 22 && i <= 32 && j >= 7 && j <= 17) {
				depth[640*i + j] = 1;
			}
		}
	}
	
	Node* result5 = segmentRegions(full, depth, PREV_H);
	result5 = filterRegions(result5);
	
	if (result5 != NULL && result5->next != NULL
			&& result5->next->next == NULL) {
		printf("Test passed.\n");
	} else {
		errorCount++;
		printf("Test failed.\n");
	}
	
	if (!errorCount) {
		printf("All image processing tests passed!\n");
	} else {
		printf("%i tests failed.\n", errorCount);
	}
	
	//Memory Management
	free(n1); free(n4); free(n6); free(e1); free(e2); free(e3);
	
	return errorCount;
}

/**
Testing Utility Functions

Tests conversion and linked list functions
**/

int testConversions() {
	int errorCount = 0;
	printf("\nTesting pixel to Point conversions.\n");
	
	//Initialize some test cases
	int p1 = 0; int p2 = 640; int p3 = 640*480-1;
	int p4 = 640*3 + 56; int p5 = 640*242 + 359;
	
	//Make the expected return values
	Point e1 = createPoint(0, 0); Point e2 = createPoint(0, 1);
	Point e3 = createPoint(639, 479); Point e4 = createPoint(56, 3);
	Point e5 = createPoint(359, 242);
	
	//Compare the results
	errorCount += comparePixelPoint(e1, p1);
	errorCount += comparePixelPoint(e2, p2);
	errorCount += comparePixelPoint(e3, p3);
	errorCount += comparePixelPoint(e4, p4);
	errorCount += comparePixelPoint(e5, p5);
	
	printf("Testing Point to pixel conversions.\n");
	
	errorCount += comparePointPixel(p1, e1);
	errorCount += comparePointPixel(p2, e2);
	errorCount += comparePointPixel(p3, e3);
	errorCount += comparePointPixel(p4, e4);
	errorCount += comparePointPixel(p5, e5);
	
	if (!errorCount) {
		printf("All conversion tests passed!\n");
	} else {
		printf("%i conversion tests failed.\n", errorCount);
	}
	
	return errorCount;
}

//Helper function to testConversions()
int comparePixelPoint(Point expected, int pixel) {
	if (expected.x != getCartesian(pixel).x || expected.y != getCartesian(pixel).y) {
		printf("Test failed.\n");
		return 1;
	} else {
		printf("Test passed.\n");
		return 0;
	}
}

//Helper function to testConversions()
int comparePointPixel(int expected, Point point) {
	if (expected != getPixel(point)) {
		printf("Test failed.\n");
		return 1;
	} else {
		printf("Test passed.\n");
		return 0;
	}
}

int testLinkedListFuncs() {
	printf("\nTesting the linked list functions.\n");
	int errorCount = 0;
	
	//Initialize some nodes
	Point p1 = createPoint(0, 0); Point p2 = createPoint(1, 1); Point p3 = createPoint(2, 2);
	Region r1 = createRegion(p1, p2); Region r2 = createRegion(p1, p3);
	Region r3 = createRegion(p2, p3);
	Node* n1 = createNode(r1); Node* n2 = createNode(r2); Node* n3 = createNode(r3);
	Node* e1 = createNode(r1); Node* e2 = createNode(r2); Node* e3 = createNode(r3);
	
	//First remove a node in the middle
	n1->next = n2; n2->next = n3;
	e1->next = e3;
	freeNode(n1, n2);
	
	errorCount += compareLists(n1, e1);
	
	//Next try to remove the tail
	//First in a list of 3, then 2
	n2 = createNode(r2); n1->next = n2; n2->next = n3;
	e1->next = e2;
	freeNode(n2, n3);
	
	errorCount += compareLists(n1, e1);
	
	e1->next = NULL;
	freeNode(n1, n2);
	
	errorCount += compareLists(n1, e1);
	
	//Now remove head, from lists of length 2 and 1
	n2 = createNode(r2);Node* head = n1; n1->next = n2;
	e2->next = NULL;
	head = freeHeadNode(head);
	
	errorCount+= compareLists(head, e2);
	
	head = n2;
	head = freeHeadNode(head);
	
	errorCount += compareLists(head, NULL);
	
	
	if (!errorCount) {
		printf("All linked list tests passed!\n");
	} else {
		printf("%i linked list tests failed.\n", errorCount);
	}
	
	//Memory Management
	free(e1); free(e2); free(e3);
	
	return errorCount;
}

//Helper function to testLinkedListFuncs() and testIPFuncs()
int compareLists(Node* head1, Node* head2) {
	int error = 0;
	Node* c1, * c2;
	c1 = head1; c2 = head2;
	
	if (c1 == NULL && c2 == NULL) {
		printf("Test passed.\n");
		return 0;
	}
	
	if (compareNodes(c1, c2)) {
		printf("Test failed.\n");
		return 1;
	}
	
	error = compareLists(c1->next, c2->next);
	
	return error;
}

//Helper function to compareLists()
int compareNodes(Node* n1, Node* n2) {
	if ((n1 == NULL && n2 != NULL) || (n1 != NULL && n2 == NULL)) {
		return 1;
	}

	if (n1->reg.ul.x != n2->reg.ul.x || n1->reg.ul.y != n2->reg.ul.y
			|| n1->reg.br.x != n2->reg.br.x || n1->reg.br.y != n2->reg.br.y) {
		return 1;
	} else {
		return 0;
	}
}