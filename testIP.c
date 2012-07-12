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
		printf("Test failed. :(\n");
		return 1;
	} else {
		printf("Test passed.\n");
		return 0;
	}
}

//Helper function to testConversions()
int comparePointPixel(int expected, Point point) {
	if (expected != getPixel(point)) {
		printf("Test failed. :(\n");
		return 1;
	} else {
		printf("Test passed.\n");
		return 0;
	}
}

