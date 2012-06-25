//(c) 2012 Stefan Seritan
// Based on code from glview.c, from OpenKinect Project. For licenses, see under the README or the Kinect folder
// Basic depth streamer from the Kinect

//Basic includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "depthStream.h"

//Include the OpenGL graphics libraries
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

//Include the libfreenect library
#include "libfreenect.h"

//Include threading
#include <pthread.h>

//Initialize all globals
//Thread globals
pthread_t fThread;
pthread_mutex_t glBufferMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t glFrameCond = PTHREAD_COND_INITIALIZER;
volatile int die = 0;

//Freenect globals
freenect_context * fCtx;
freenect_device * fDev;
int fAngle = 0;

//Video buffer globals
//Using triple layer callback system:
//Back is within libfreenect
//Middle is the callback layer, getting ready to be drawn
//Front is within OpenGL, being drawn
uint8_t *depthMid, *depthFront;
//This is currently used to color things according to their depth
//TODO: Use a different way to display depth, and cut off at a few feet
uint16_t depthGamma[2048];

//Other required globals
int window;
int globArgc; char ** globArgv;
GLuint glDepthTex;
int gotDepth = 0;

//Main function
int main(int argc, char **argv) {
	//Initialize all the arrays to store the data
	//Currently they are store 3 pixels because the depth data is turned into RGB
	//This may change
	depthMid = (uint8_t*)malloc(640*480*3);
	depthFront = (uint8_t*)malloc(640*480*3);
	
	//Initialize the different levels for depth (I assume this is what it does, it is in the original code)
	//I will most likely not use this in the final product
	for (int i=0;i<2048; i++) {
		float v = i/2048.0;
		v = powf(v,3)*6;
		depthGamma[i]=v*6*256;
	}
	
	//Set the globals
	globArgc = argc; globArgv = argv;
	
	//Start Kinect initialization
	printf("Starting Kinect...\n");
	
	//Initializes and checks for errors
	if (freenect_init(&fCtx,NULL) < 0) {
		printf("freenect_init() failed.\n");
		return 1;
	}
	
	//Set log level NOTICE, logging important messages
	freenect_set_log_level(fCtx, FREENECT_LOG_NOTICE);
	//Select camera and motor
	freenect_select_subdevices(fCtx, (freenect_device_flags)(FREENECT_DEVICE_MOTOR | FREENECT_DEVICE_CAMERA));
	
	//See how many Kinects are connected
	int numOfDevs = freenect_num_devices(fCtx);
	printf("Found %d Kinect devices connected. \n",numOfDevs);
	
	//Stop if 0 Kinects
	if (numOfDevs == 0) {
		freenect_shutdown(fCtx);
		return 1;
	}
	
	//Try to assign the device to fDev, and stop if fail
	if (freenect_open_device(fCtx, &fDev, 0) < 0) {
		printf("Failed to open the device.\n");
		freenect_shutdown(fCtx);
		return 1;
	}
	
	//Try to create another thread to receive the video data, and stop if fail
	if (pthread_create(&fThread, NULL, fThreadFunc, NULL)) {
		printf("Failed to create a thread for freenect.\n");
		freenect_shutdown(fCtx);
		return 1;
	}
	
	//Run OpenGL on main thread
	glThreadFunc(NULL);
	
	return 0;
}

//fThreadFunc, sets up the Kinect, and starts and stops the video
void * fThreadFunc(void * arg) {
	//Set up tilt, led, and video modes and callbacks
	freenect_set_tilt_degs(fDev, fAngle);
	freenect_set_led(fDev, LED_GREEN);
	freenect_set_depth_callback(fDev, depthCallback);
	freenect_set_depth_mode(fDev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_11BIT));
	
	printf("'w' for tilt up, 's' for tilt down, 'ESC' to exit.\n");
	
	//Start depth video
	freenect_start_depth(fDev);
	
	while(!die && freenect_process_events(fCtx) >=0) {
		//Do nothing until the process will end
	}
	
	printf("Shutting down.\n");
	
	//Stop depth video
	freenect_stop_depth(fDev);
	
	//Close and shutdown
	freenect_close_device(fDev);
	freenect_shutdown(fCtx);
	
	return NULL;
}

//glThreadFunc, sets up the OpenGL loop on the main thread
void * glThreadFunc(void * arg) {
	//Initialize a bunch of OpenGL stuff
	glutInit(&globArgc, globArgv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowSize(640,480);
	glutInitWindowPosition(0,0);
	
	//Open window
	window = glutCreateWindow("Depth Stream");
	
	//Set OpenGL callbacks
	glutDisplayFunc(&DrawGlScene);
	glutIdleFunc(&DrawGlScene);
	glutReshapeFunc(&ResizeGlScene);
	glutKeyboardFunc(&keyPressed);
	
	//Do more of my own initialization
	InitGl(640,480);
	
	//Start the magic OpenGL loop
	glutMainLoop();
	
	return NULL;
}

//InitGL, more custom initialization
void InitGl(int w, int h) {
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);
    glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_TEXTURE_2D);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glShadeModel(GL_FLAT);

	glGenTextures(1, &glDepthTex);
	glBindTexture(GL_TEXTURE_2D, glDepthTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	ResizeGlScene(w,h);
}

//ResizeGlScene, handle window resize events (should not really happen at this stage)
void ResizeGlScene(int w, int h) {
	glViewport(0,0,w,h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho (0, 640, 480, 0, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

//keyPressed, handle key events for OpenGL
void keyPressed(unsigned char key, int x, int y) {
	//Exit if ESC pressed
	if (key == 27) {
		die = 1;
		pthread_join(fThread, NULL);
		glutDestroyWindow(window);
		free(depthMid);
		free(depthFront);
		exit(0);
	}
	
	//Tilt up/down if w/s
	if (key == 'w') {
		fAngle++;
		if (fAngle > 30) {
			fAngle = 30;
		}
	}
	if (key == 's') {
		fAngle--;
		if (fAngle < -30) {
			fAngle = -30;
		}
	}
	
	//Tilt
	freenect_set_tilt_degs(fDev, fAngle);
}

void DrawGlScene() {
	//Lock the thread
	pthread_mutex_lock(&glBufferMutex);
	
	//Wait for the signal that there is depth data
	while (!gotDepth) {
		pthread_cond_wait(&glFrameCond, &glBufferMutex);
	}
	
	//Initialize a temporary pointer, and swap the front and mid layers
	//Mid will be overwritten with the new data later
	uint8_t * tmp;
	if (gotDepth) {
		tmp = depthFront;
		depthFront = depthMid;
		depthMid = tmp;
		gotDepth = 0;
	}
	
	//Unlock thread
	pthread_mutex_unlock(&glBufferMutex);
	
	//Bind images to texture, and display the texture
	glBindTexture(GL_TEXTURE_2D, glDepthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, depthFront);
	
	glBegin(GL_TRIANGLE_FAN);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoord2f(0,0); glVertex3f(0,0,0);
	glTexCoord2f(1,0); glVertex3f(640,0,0);
	glTexCoord2f(1,1); glVertex3f(640,480,0);
	glTexCoord2f(0,1); glVertex3f(0,480,0);
	glEnd();
	
	glutSwapBuffers();	
}

void depthCallback(freenect_device * dev, void * tmpDepth, uint32_t timestamp) {
	//Lock the thread
	pthread_mutex_lock(&glBufferMutex);
	
	//Cast depth to int
	uint16_t * depth = (uint16_t *)tmpDepth;
	
	//Displays the colored depth
	for(int i = 0; i < 640*480; i++) {
		int pval = depthGamma[depth[i]];
		int lb = pval & 0xff;
		switch (pval>>8) {
			case 0:
				depthMid[3*i+0] = 255;
				depthMid[3*i+1] = 255-lb;
				depthMid[3*i+2] = 255-lb;
				break;
			case 1:
				depthMid[3*i+0] = 255;
				depthMid[3*i+1] = lb;
				depthMid[3*i+2] = 0;
				break;
			case 2:
				depthMid[3*i+0] = 255-lb;
				depthMid[3*i+1] = 255;
				depthMid[3*i+2] = 0;
				break;
			case 3:
				depthMid[3*i+0] = 0;
				depthMid[3*i+1] = 255;
				depthMid[3*i+2] = lb;
				break;
			case 4:
				depthMid[3*i+0] = 0;
				depthMid[3*i+1] = 255-lb;
				depthMid[3*i+2] = 255;
				break;
			case 5:
				depthMid[3*i+0] = 0;
				depthMid[3*i+1] = 0;
				depthMid[3*i+2] = 255-lb;
				break;
			default:
				depthMid[3*i+0] = 0;
				depthMid[3*i+1] = 0;
				depthMid[3*i+2] = 0;
				break;
		}
	}
	
	//Signal that there is new depth data
	gotDepth++;
	pthread_cond_signal(&glFrameCond);
	
	//Unlock the thread
	pthread_mutex_unlock(&glBufferMutex);
}