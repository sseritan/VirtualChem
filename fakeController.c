//Stefan Seritan, July 13th 2012
//fakeController.c, a fake controller, to simulate the fact that
//handTracking will actually be a library to be used, moving towards finishing this part
//of the project

//Basic includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "fakeController.h"

//Hand tracking includes
#include "main.h"
#include "depthStream.h"
#include "handTracking.h"

//OpenGL includes
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>


//OpenGL globals
GLuint depthTex;
int window;

//Main function
int main(int argc, char** argv) {
	if (initKinect()) {
		printf("Shutting down.\n");
		return 0;
	}
	
	initGraphics(&argc, argv);
	
	glutMainLoop();
	
	return 0;
}

//Update the depth texture, with new depth data
void updateDepthTexture() {
	//Initialize RGB array
	uint8_t* depthRGB = (uint8_t*)malloc(640*480*3);
	
	uint8_t* depth = getDepthData();
	
	//Display the data in black and white
	for (int i = 0; i < 640*480; i++) {
		if (depth[i]) {
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
	
	//Bind the byte array to the texture
	glBindTexture(GL_TEXTURE_2D, depthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, depthRGB);
	
	free(depthRGB);
	free(depth);
}

/**
OpenGL Functions

Standard OpenGL initialization and callbacks
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
		stopKinect();
		glutDestroyWindow(window);
		printf("Shutting down.\n");
		exit(0);
	}
}

//Take the data that I have in the array and display it, for testing purposes
void DrawGLScene() {
	//Get the depth texture updated
	updateDepthTexture();
	
	//Display the texture
	glBegin(GL_TRIANGLE_FAN);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoord2f(0,0); glVertex3f(0,0,0);
	glTexCoord2f(1,0); glVertex3f(640,0,0);
	glTexCoord2f(1,1); glVertex3f(640,480,0);
	glTexCoord2f(0,1); glVertex3f(0,480,0);
	glEnd();
	
	glutSwapBuffers();
}