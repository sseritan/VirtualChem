//Stefan Seritan, July 13th 2012
//depthStream.c, handle all the Kinect functions

#include <stdio.h>
#include <stdlib.h>
#include "depthStream.h"

//Include libfreenect library
#include "libfreenect.h"

//Include threading library
#include <pthread.h>

//Threading globals, and flags
pthread_t kinectThread;
pthread_mutex_t bufferSwapMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t bufferCond = PTHREAD_COND_INITIALIZER;
static int gotDepth = 0;
static int kill = 0;
static int kinectReady = 0;

//Global array, needed to pass the data out
uint8_t* depthData;

//Initialize Kinect
int initKinect() {
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
	
	return kill;
}

void stopKinect() {
	kill = 1;
	pthread_join(kinectThread, NULL);
	free(depthData);
}

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

uint8_t* getDepthData() {
	uint8_t* depth = (uint8_t*)malloc(640*480);
		
	pthread_mutex_lock(&bufferSwapMutex);
	
	//Wait for new data
	if (!gotDepth) {
		pthread_cond_wait(&bufferCond, &bufferSwapMutex);
	}
	
	for (int i = 0; i < 640*480; i++) {
		depth[i] = depthData[i];
	}
	
	gotDepth = 0;
	
	pthread_mutex_unlock(&bufferSwapMutex);
	
	return depth;
}
