//Stefan Seritan, June 27, 2012
//handTracking.c, takes depth data from the Kinect and turns it into events
//Will generate the number and location of current hands, as well as an OpenGL texture

//Basic includes
#include <stdio.h>
#include <stdlib.h>
#include "handTracking.h"
#include "imageProcessing.h"

//Include libfreenect library
#include "libfreenect.h"

//Include threading library
#include <pthread.h>

//For testing and development purposes, the easiest way to test this is visually
//Include OpenGL and GLUT (must run on main thread, not thread safe)
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
	
	//Init to an empty array
	for (int i = 0; i < 640*480; i++) {
		depthData[i] = 0;
	}
	
	printf("Initialized test array.\n");
	
	/**
	Commented out for testing purposes
	
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
	**/
	
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
Hand Tracking Functions

The actually important part of this file. Grabs the hands and their positions.
**/

//Get the hand regions
Node* getHandRegions(uint8_t* depth) {
	Point p1 = createPoint(0,0); Point p2 = createPoint(639, 479);
	Region full = createRegion(p1, p2);
	Node* fullRegion = createNode(full);
	
	//Segment the image
	Node* head = segmentRegions(fullRegion, depth, PREV_H, 0);
	
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
	
	/**
	Testing
	
	//Lock the thread
	pthread_mutex_lock(&bufferSwapMutex);
	
	//Wait until there is new data
	while (!gotDepth) {
		pthread_cond_wait(&bufferCond, &bufferSwapMutex);
	}
	**/
	
	//Display the data in black and white
	//if (gotDepth) {
	
		Node* head = getHandRegions(depthData);
	
		for (int i = 0; i < 640*480; i++) {
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
	//}
	
	//Unlock the thread
	//pthread_mutex_unlock(&bufferSwapMutex);
	
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