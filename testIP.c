//Stefan Seritan, July 11th 2012
//testIP.c, test suite for imageProcessing.c

//Basic includes
#include <stdio.h>
#include <stdlib.h>
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
Testing Utility Functions

Tests conversion and linked list functions
**/

int testConversions() {
	int errorCount = 0;
	printf("Testing pixel to Point conversions.\n");
	
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
	printf("Testing the linked list functions.\n");
	int errorCount = 0;
	
	//Initialize some nodes
	Point p1 = createPoint(0, 0); Point p2 = createPoint(1, 1); Point p3 = createPoint(2, 2);
	Region r1 = createRegion(p1, p2); Region r2 = createRegion(p1, p3);
	Region r3 = createRegion(p2, p3);
	Node* n1 = createNode(r1); Node* n2 = createNode(r2); Node* n3 = createNode(r3);
	Node* e1 = createNode(r1); Node* e2 = createNode(r2); Node* e3 = createNode(r3);
	
	//First, try to remove the head of a list, with 2 and 3 nodes
	Node* head = n1;
	n2->next = n3; n1->next = n2;
	e2->next = e3;
	freeNode(NULL, head);
	
	errorCount += compareLists(n2, e2);
	//errorCount += compareLists(head, e2);
	
	if (!errorCount) {
		printf("All linked list tests passed.\n");
	} else {
		printf("%i linked list tests failed.\n", errorCount);
	}
	
	return errorCount;
}

//Helper function to testLinkedListFuncs()
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
	//Compare every element. I will not pass weird cases (empty or 1);
	if (n1->reg.ul.x != n2->reg.ul.x || n1->reg.ul.y != n2->reg.ul.y
			|| n1->reg.br.x != n2->reg.br.x || n1->reg.br.y != n2->reg.br.y
			|| n1->next != n2->next) {
		return 1;
	} else {
		return 0;
	}
}