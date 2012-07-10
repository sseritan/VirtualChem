//Stefan Seritan, June 27, 2012
//handTracking.c, takes depth data from the Kinect and turns it into events
//I will attempt to make this synchronous, as opposed to the callbacks used in depthStream

//Basic includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "handTracking.h"

//Include libfreenect library
#include "libfreenect.h"

//Include threading library
#include <pthread.h>

//For testing and development purposes, the easiest way to test this is visually
//Include OpenGL and GLUT (must run on main thread, not thread safe)_
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>

//Threading globals, and flags
pthread_t kinectThread;
pthread_mutex_t bufferSwapMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t bufferCond = PTHREAD_COND_INITIALIZER;
static int gotDepth = 0;
static int kill = 0;
static int kinectReady = 0;

//I have to have one global array to pass the data through, because libfreenect sucks
//It doesn't actually suck, but it doesn't offer this functionality :(
uint8_t* depthData;

//OpenGL globals, will not be in final product
GLuint depthTex;
int window;

//Main function
int main(int argc, char** argv) {
	printf("Welcome!\n");
	
	//Initialize the depth data array
	depthData = (uint8_t*)malloc(640*480);
	
	//Initialize Kinect thread and device
	if (pthread_create(&kinectThread, NULL, kinectThreadFunc, NULL)) {
		printf("Setting up thread for Kinect failed. Shutting down.\n");
		return 1;
	}
	
	while (!kill && !kinectReady) {
		//Wait for the kinect to finish setting up
		//Otherwise we get in a weird state where the kinect thread dies
		//but OpenGL keeps going
	}
	
	if (!kill) {
		//Initialize OpenGL on the main thread
		initGraphics(&argc, argv);
		
		//Start the OpenGL loop
		glutMainLoop();
	}
	//Memory Management and Safe Shutdown
	printf("Shutting down.\n");
	free(depthData);
	
	return 0;
}

/**
Image Processing Functions

The core of this will be the recursive segmentRegions function, which will recursively
segment the image into smaller and smaller regions (but not if there is something there).
Once the size of the regions is small enough (20 by 20 pixels), they are discarded.
**/

//Segment the image into regions, either containing white or smaller than 20 by 20
Node* segmentRegions(Node* fullReg, uint8_t* depth, segStatus status, int attemptFlag) {
	Node* n1, * n2;
	Point ul = fullReg->reg.ul;
	Point br = fullReg->reg.br;
	int foundCut = 0;
	
	//Try to cut the opposite way from the previous
	if (PREV_H) {
		//Find average
		int avgX = ((ul.x + br.x)/2);
		int x = avgX;
		
		//Try going right first
		while (x <= br.x && !foundCut) {
			Point start = createPoint(x, ul.y);
			Point end = createPoint(x, br.y);
			
			if (!testVertical(depth, start, end)) {
				foundCut = 1;
				Region reg1 = createRegion(ul, end);
				Region reg2 = createRegion(start, br);
				
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
				Region reg1 = createRegion(ul, end);
				Region reg2 = createRegion(start, br);
				
				n1 = createNode(reg1); n2 = createNode(reg2);
			}
			
			//Increment over 5 pixels
			x -= 5;
		}
		
		//Cut was not found on either side
		if (attemptFlag) {
			//I already tried horizontal cuts, no way to segment
			return fullReg;
		} else {
			//Try horizontal split
			segmentRegions(fullReg, depth, PREV_H, 1);
		}
	} else if (PREV_V) {
		//Find average
		int avgY = ((ul.y + br.y)/2);
		int y = avgY;
		
		//Try to go up first
		while (y <= ul.y && !foundCut) {
			Point start = createPoint(ul.x, y);
			Point end = createPoint(br.x, y);
			
			if(!testHorizontal(depth, start, end)) {
				foundCut = 1;
				Region reg1 = createRegion(ul, end);
				Region reg2 = createRegion(start, br);
				
				n1 = createNode(reg1); n2 = createNode(reg2);
			}
			
			//Increment up 5 pixels
			y += 5;
		}
		
		y = avgY - 5;
		//Try to go down if cut not found up
		while (y >= br.y && !foundCut) {
			Point start = createPoint(ul.x, y);
			Point end = createPoint(br.x, y);
			
			if (!testHorizontal(depth, start, end)) {
				foundCut = 1;
				Region reg1 = createRegion(ul, end);
				Region reg2 = createRegion(start, br);
				
				n1 = createNode(reg1); n2 = createNode(reg2);
			}
			
			//Increment
			y -= 5;
		}
	} else {
		printf("Bad status code to segmentRegions.\n");
	}
	
	segStatus newStatus;
	if (status == PREV_H) {
		newStatus = PREV_V;
	} else {
		newStatus = PREV_H;
	}
	
	//MAGIC... make the recursive calls
	n1 = segmentRegions(n1, depth, newStatus, 0);
	n2 = segmentRegions(n2, depth, newStatus, 0);
	
	//Find the tail to the first node, to link it all together
	Node* tail;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	
	//Link the two lists
	tail->next = n2;
	
	return n1;
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
	if (prev == NULL) {
		//This is the head of the list
		free(current);
		return;
	}
	
	//Make the previous point to the next, then free
	prev->next = current->next;
	free(current);
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
	while (pixel > 640) {
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

/**
Kinect Functions

This whole section sets up a separate thread and completely handles all of the Kinect parts of the code.
**/

//Sets up and runs the thread that the Kinect data will be pulled in on
void* kinectThreadFunc(void* arg) {
	//Initialize Kinect variables
	freenect_context* ctx; freenect_device* dev;
	
	printf("Starting the Kinect.\n");
	
	//Initialize the Kinect context (usb info)
	if (freenect_init(&ctx, NULL) < 0) {
		printf("freenect_init() failed.\n");
		return NULL;
	}
	
	//Set the log level to only important message, and take control of the camera and motor
	freenect_set_log_level(ctx, FREENECT_LOG_NOTICE);
	freenect_select_subdevices(ctx, (freenect_device_flags)(FREENECT_DEVICE_MOTOR | FREENECT_DEVICE_CAMERA));
	
	//Get the number of connected Kinects (hopefully just 1)
	int numOfDevs = freenect_num_devices(ctx);
	printf("Found %d Kinect devices connected. \n",numOfDevs);
	
	//Stop if no Kinects
	if (numOfDevs == 0) {
		kill = 1;
		freenect_shutdown(ctx);
		return NULL;
	}
	
	
	//Assign the device to dev
	if (freenect_open_device(ctx, &dev, 0) < 0) {
		printf("Failed to open the device.\n");
		kill = 1;
		freenect_shutdown(ctx);
		return NULL;
	}
	
	//Tilt the Kinect up, set LED and depth callback and mode
	freenect_set_tilt_degs(dev, 10);
	freenect_set_led(dev, LED_GREEN);
	freenect_set_depth_callback(dev, depthCB);
	freenect_set_depth_mode(dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_11BIT));
	
	kinectReady = 1;
	
	//Start the depth stream
	printf("Starting depth stream.\n");
	freenect_set_led(dev, LED_BLINK_GREEN);
	freenect_start_depth(dev);
	
	while (!kill && freenect_process_events(ctx) >= 0) {
		//Do nothing but keep the stream going until its time to stop
	}
	
	printf("Stopping depth stream and shutting down Kinect.\n");
	
	//Stop depth stream
	freenect_stop_depth(dev);
	
	//Turn led off
	freenect_set_led(dev, LED_OFF);
	
	//Shutdown kinect
	freenect_close_device(dev);
	freenect_shutdown(ctx);
	
	return NULL;
}

//Updates the depth array
void depthCB(freenect_device* dev, void* tmpDepth, uint32_t timestamp) {
	//Lock the thread
	pthread_mutex_lock(&bufferSwapMutex);
	
	//Cast the void star to an int star
	uint16_t* depth = (uint16_t*)tmpDepth;
	
	
	//Cutoff the depth at 600
	for (int i = 0; i < 640*480; i++) {
		if (depth[i] < 600) {
			depthData[i] = 1;
		} else {
			depthData[i] = 0;
		}
	}
	
	//Signal that there is new data
	gotDepth++;
	pthread_cond_signal(&bufferCond);
	
	//Unlock
	pthread_mutex_unlock(&bufferSwapMutex);
}

/**
OpenGL Functions

Only used now for visual test of the image segmentation methods.
This entire section will be removed (most likely) later, when I am sure that this works.
While it will be still useful to the user, I will most likely just make a function to pass a GLuint
texture up out of this code, centralizing my graphics in the main app.
**/

//Initialize OpenGL window and start the OpenGL loop
void initGraphics(int* argc, char** argv) {
	//Boiler plate code
	glutInit(argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(640,480);
	glutInitWindowPosition(0,0);
	
	//Open my window
	window = glutCreateWindow("Hand Tracking");
	
	//Set callbacks
	glutDisplayFunc(&DrawGLScene);
	glutIdleFunc(&DrawGLScene);
	glutReshapeFunc(&ResizeGLScene);
	glutKeyboardFunc(&keyPressed);
	
	//Set up my texture
	glGenTextures(1, &depthTex);
	glBindTexture(GL_TEXTURE_2D, depthTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glEnable(GL_TEXTURE_2D);
	
}

//Deal with window resizes (really not necessary, but whatevs)
void ResizeGLScene(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 640, 480, 0, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

//Be able to exit the program without Ctrl-C :P
void keyPressed(unsigned char key, int x, int y) {
	if (key == 27) {
		kill = 1;
		pthread_join(kinectThread, NULL);
		glutDestroyWindow(window);
		free(depthData);
		printf("Shutting down.\n");
		exit(0);
	}
}

//Take the data that I have in the array and display it, for testing purposes
void DrawGLScene() {
	//Initialize RGB array
	uint8_t* depthRGB = (uint8_t*)malloc(640*480*3);

	//Lock the thread
	pthread_mutex_lock(&bufferSwapMutex);
	
	//Wait until there is new data
	while (!gotDepth) {
		pthread_cond_wait(&bufferCond, &bufferSwapMutex);
	}
	
	//Display the data in black and white
	if (gotDepth) {
		for (int i =0; i < 640*480; i++) {
			if (depthData[i]) {
				//Something is there, display as white
				depthRGB[3*i+0] = 255;
				depthRGB[3*i+1] = 255;
				depthRGB[3*i+2] = 255;
			} else {
				//Nothing is there, display black
				depthRGB[3*i+0] = 0;
				depthRGB[3*i+1] = 0;
				depthRGB[3*i+2] = 0;
			}
		}
		
		//Signal depth data was used
		gotDepth = 0;
	}
	
	//Unlock the thread
	pthread_mutex_unlock(&bufferSwapMutex);
	
	//Bind the byte array to the texture
	glBindTexture(GL_TEXTURE_2D, depthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, depthRGB);
	
	//Display the texture
	glBegin(GL_TRIANGLE_FAN);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoord2f(0,0); glVertex3f(0,0,0);
	glTexCoord2f(1,0); glVertex3f(640,0,0);
	glTexCoord2f(1,1); glVertex3f(640,480,0);
	glTexCoord2f(0,1); glVertex3f(0,480,0);
	glEnd();
	
	glutSwapBuffers();
	
	//Memory Management
	free(depthRGB);
}