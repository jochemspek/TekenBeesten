//=========================================================================================//
//    The OpenCV-CamHeadSystem-Project                                                   
//    by                                                                    
//    Rytta Communications Inc.                                           
//    (c) Version 0.3. May 2009                                       
//===========================================================================================// 
//    cvgl_3dAxisDemo3.cpp 
//    --------------------  
//
//    begin     : Fri. 15.May 13.15:00 GMT 2009                            
//    copyright : (C) 2008/2009 by  s.morf                                         
//    email     : [email]stevenmorf@bluewin.ch[/email] 
//
//    compile with   
//    opencv & opengl:  
//    g++ -I/usr/include/GL -I/usr/local/include/opencv -L /usr/local/lib -lcxcore -lcv -lhighgui -lcvaux -lml -I/usr/X11R6/include -L/usr/X11R6/lib -o cvgl_3dAxisDemo3 cvgl_3dAxisDemo3.cpp -lglui -lglut -lGLU -lGL                  
//    
//    run as    : ./cvgl_3dAxisDemo3
//
//    for: Basics of: ComputerVisions
//    basics with 
//    opencv(webcam /images)with logitech QuickCam(s) E3500 Plus 
//    opengl (3d-cube movements with mouse, you can look inside      
//    of the cube
//    - opengl (show 3D-Axis (study)) 
// 
/****************************************************************************************************/ 
//     THIS PROGRAMM IS UNDER THE GNU-LICENCE                              
/****************************************************************************************************  
    This program is free software; you can redistribute it and/or modify    
it under the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option) any
later version.                                                                
*****************************************************************************************************
//  References
//
//  Gordon Wetzstein
//  The University of British Columbia
//    [email]wetzste1@cs.ubc.ca[/email]
//    @author Gordon Wetzstein, now @ The University of British Columbia [wetzste1@cs.ubc.ca]
//    @date 09/11/06
//
//  This is just a simple test for combining OpenCV's image loadig interface 
//  and OpenGL.
//            
//  verwendete programme:
//  from /root/Desktop/OPENCV/opencv-tutorial.pdf (german)
//  <Bildverarbeitung mit OpenCV, author Johannes Wienke, Uni Bielefeld 2008>
//
// OpenCV_Beispiele From RoboWeb
// Mehrere Bilder in einem Fenster
// from [url]http://opencvlibrary.sourceforge.net/DisplayManyImages[/url].
 
  Display video from webcam
 
  Author  Nash
  License GPL
  Website http:/nashruddin.com
/************************************************************************************************
REFERNECE STARTING WEBCAM:
 
1) on console
   if you have to reset you cam for any reason, do: 
1. unplug your cam 
2. modprobe -r uvcvideo 
3. modprobe -r snd_usb_audio 
4. replug your cam  
 
/************************************************************************************************
 
**************************************************************************************************/
#include <GL/freeglut.h>
#include <GL/gl.h>
#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <ctype.h>
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#ifndef random
#define random rand
#endif
#include <GL/glut.h>
#include <ctype.h>
#include "cv.h"
#include "highgui.h"
 
#include <iostream>
 
bool bool_makePic;
bool quit;
bool done;
 
//***********************************************************************************************/
// GLUT callbacks and functions
 
void initGlut(int argc, char **argv);
void displayFunc(void);
void idleFunc(void);
void reshapeFunc(int width, int height);
void mouseFunc(int button, int state, int x, int y);
void mouseMotionFunc(int x, int y);
void keyboardFunc(unsigned char key, int x, int y);
void specialFunc(int key, int x, int y);
 
/***********************************************************************************************/
 
// other [OpenGL] functions
void countFrames(void);
void renderBitmapString(float x, float y, float z, void *font, char *string);
 
/***********************************************************************************************/
 
bool bFullsreen = false;
int nWindowID;
 
/***********************************************************************************************/
 
// parameters for the framecounter nicht verwendet
char pixelstring[30];
int cframe = 0;
int timebase = 0;
 
 
/***********************************************************************************************/
// camera attributes
float viewerPosition[3]    = { 0.0, 0.0, -50.0 };
float viewerDirection[3]   = { 0.0, 0.0, 0.0 };
float viewerUp[3]          = { 0.0, 1.0, 0.0 };
 
// rotation values for the navigation
float navigationRotation[3]   = { 0.0, 0.0, 0.0 };
 
// parameters for the navigation
 
// position of the mouse when pressed
int mousePressedX = 0, mousePressedY = 0;
float lastXOffset = 0.0, lastYOffset = 0.0, lastZOffset = 0.0;
// mouse button states
int leftMouseButtonActive = 0, middleMouseButtonActive = 0, rightMouseButtonActive = 0;
// modifier state
int shiftActive = 0, altActive = 0, ctrlActive = 0;
 
// Rotation amounts
static GLfloat xRot = 0.0f;
static GLfloat yRot = 0.0f;
/***********************************************************************************************/
// OpenCV variables
 
IplImage *image = 0;
GLuint cameraImageTextureID;
 
//  here cv_cam-functions
CvCapture *capture = 0;
// IplImage  *frame   = 0;  -> image
int            key = 0;
 
bool bInit = false;
/***********************************************************************************************/
// Función para dibujar unos ejes del tamaño dado
void drawAxis(float length) 
{
     glBegin(GL_LINES);
        glDisable( GL_TEXTURE_2D );
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(length, 0.0f, 0.0f);
 
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, length, 0.0f);
 
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, length);
     glEnd();
}
 
void gltDrawUnitAxes(void)
{
   GLUquadricObj *pObj; // Temporary, used for quadrics
 
   // Measurements
   float fAxisRadius = 0.25f;
   float fAxisHeight = 10.0f;
   float fArrowRadius = 0.6f;
   float fArrowHeight = 1.0f;
 
 // Black background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f );   
 
   // Setup the quadric object
   pObj = gluNewQuadric();
   gluQuadricDrawStyle(pObj, GLU_FILL);
   gluQuadricNormals(pObj, GLU_SMOOTH);
   gluQuadricOrientation(pObj, GLU_OUTSIDE);
   gluQuadricTexture(pObj, GLU_FALSE);
 
   // fehler no color from Opengl.org forum
   glDisable( GL_TEXTURE_2D );
 
   ///////////////////////////////////////////////////////
   // Draw the blue Z axis first, with arrowed head
   glColor3f(0.0f, 0.0f, 1.0f);
   // from opencv -> cvScalar(0xff,0x00,0x00); 
   gluCylinder(pObj,fAxisRadius, fAxisRadius,fAxisHeight, 10, 1);
   glPushMatrix();
      glTranslatef(0.0f, 0.0f, 1.0f);
      gluCylinder(pObj, fArrowRadius, 0.0f, fArrowHeight, 10, 1);
      glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
      gluDisk(pObj, fAxisRadius, fArrowRadius, 10, 1);
   glPopMatrix();
 
   ///////////////////////////////////////////////////////
   // Draw the Red X axis 2nd, with arrowed head
   glColor3f(1.0f, 0.0f, 0.0f);
   glPushMatrix();
     glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
     gluCylinder(pObj,fAxisRadius,fAxisRadius,fAxisHeight, 10,1);
     glPushMatrix();
       glTranslatef(0.0f, 0.0f, 1.0f);
       gluCylinder(pObj,fArrowRadius, 0.0f, fArrowHeight, 10, 1);
       glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
       gluDisk(pObj, fAxisRadius, fArrowRadius, 10, 1);
     glPopMatrix();
   glPopMatrix();
 
   ///////////////////////////////////////////////////////
   // Draw the Green Y axis 3rd, with arrowed head
   glColor3f(0.0f, 1.0f, 0.0f);
   glPushMatrix();
     glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
     gluCylinder(pObj,fAxisRadius,fAxisRadius,fAxisHeight, 10,1);
     glPushMatrix();
       glTranslatef(0.0f, 0.0f, 1.0f);
       gluCylinder(pObj,fArrowRadius, 0.0f, fArrowHeight, 10, 1);
       glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
       gluDisk(pObj, fAxisRadius, fArrowRadius, 10, 1);
     glPopMatrix();
   glPopMatrix();
 
   ////////////////////////////////////////////////////////
   // White Sphere at origin
   glColor3f(1.0f, 1.0f, 1.0f);
   gluSphere(pObj, 0.05f, 15, 15);
 
   glEnable( GL_TEXTURE_2D );
 
   // Delete the quadric
   gluDeleteQuadric(pObj);
}
 
/***********************************************************************************************/
// new stuff from lk_glDemo.cpp
void displayFunc(void) 
{
     IplImage *frame = 0;
     frame = cvQueryFrame( capture );
 
     // initialze OpenGL texture    
     glEnable(GL_TEXTURE_RECTANGLE_ARB);
 
     glGenTextures(1, &amp;cameraImageTextureID);
     glBindTexture(GL_TEXTURE_RECTANGLE_ARB,
                   cameraImageTextureID);
 
     glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S,  
                     GL_CLAMP_TO_EDGE);
     glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T,
                     GL_CLAMP_TO_EDGE);
     glTexParameterf(GL_TEXTURE_RECTANGLE_ARB,  
                     GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, 
                     GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 
     glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
 
     if(frame->nChannels == 3)
{
           glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, 
                        frame->width, frame->height, 0, GL_BGR,  
                        GL_UNSIGNED_BYTE, frame->imageData);
} 
     else if(frame->nChannels == 4)
{
           glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, 
                        frame->width, frame->height, 0, GL_BGRA, 
                        GL_UNSIGNED_BYTE, frame->imageData);
}
 
     if(frame) 
{
           // clear the buffers
           glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
           glEnable(GL_DEPTH_TEST);
           glDisable(GL_LIGHTING);
           glEnable(GL_TEXTURE_RECTANGLE_ARB);
 
           glMatrixMode(GL_PROJECTION);
           glLoadIdentity();
 
           // original
           gluPerspective(50.0, 1.33, 1.0, 100.0);
           glMatrixMode(GL_MODELVIEW); 
           glLoadIdentity();
 
           glTranslatef( viewerPosition[0], viewerPosition[1], 
                         viewerPosition[2] );
 
           // add navigation rotation
           glRotatef( navigationRotation[0], 1.0f, 0.0f, 0.0f );
           glRotatef( navigationRotation[1], 0.0f, 1.0f, 0.0f );
 
           // bind texture
           glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 
                         cameraImageTextureID);
 
           // draw 4 rectangles
           glBegin(GL_QUADS);
              glTexCoord2i(0,frame->height);
              glVertex3f(-15.0,-15.0, 15.0);
              glTexCoord2i(frame->width,frame->height);
              glVertex3f(15.0,-15.0, 15.0);
              glTexCoord2i(frame->width,0);
              glVertex3f(15.0,15.0, 15.0);
              glTexCoord2i(0,0);
              glVertex3f(-15.0,15.0, 15.0);
           glEnd();
 
           glBegin(GL_QUADS);
              glTexCoord2i(0,frame->height);
              glVertex3f(15.0,-15.0, -15.0);
              glTexCoord2i(frame->width,frame->height);
              glVertex3f(15.0,-15.0, 15.0);
              glTexCoord2i(frame->width,0);
              glVertex3f(15.0,15.0, 15.0);
              glTexCoord2i(0,0);
              glVertex3f(15.0,15.0, -15.0);
           glEnd();
 
           glBegin(GL_QUADS);
              glTexCoord2i(0,frame->height);
              glVertex3f(15.0,-15.0, -15.0);
              glTexCoord2i(frame->width,frame->height);
              glVertex3f(-15.0,-15.0, -15.0);
              glTexCoord2i(frame->width,0);
              glVertex3f(-15.0,15.0, -15.0);
              glTexCoord2i(0,0);
              glVertex3f(15.0,15.0, -15.0);
           glEnd();
 
           glBegin(GL_QUADS);
              glTexCoord2i(0,frame->height);
              glVertex3f(-15.0,-15.0, -15.0);
              glTexCoord2i(frame->width,frame->height);
              glVertex3f(-15.0,-15.0, 15.0);
              glTexCoord2i(frame->width,0);
              glVertex3f(-15.0,15.0, 15.0);
              glTexCoord2i(0,0);
              glVertex3f(-15.0,15.0, -15.0);
           glEnd();
 
           // end webcam-texture
           // disable texture
           glDisable(GL_TEXTURE_RECTANGLE_ARB);
 
           // here start 3dAxis must be here else wrong drawings 
           // Save the matrix state and do the rotations
           glPushMatrix();
              // Move object back and do in place rotation
             glTranslatef(0.0f, 0.0f, 20.0f);   
             glRotatef(xRot, 1.0f, 0.0f, 0.0f);
             glRotatef(yRot, 0.0f, 1.0f, 0.0f);
 
             // Draw something
             gltDrawUnitAxes();  // from book
 
             // Restore the matrix state
           glPopMatrix();
 
           // Buffer swap
           glutSwapBuffers(); 
 
 
} // end image
}
 
/***********************************************************************************************/
 
void idleFunc(void) 
{
     glutPostRedisplay();
}
 
/***********************************************************************************************/
 
void reshapeFunc(int width, int height) 
{
     glViewport(0, 0, width, height);
}
 
/***********************************************************************************************/
 
// mouse callback
void mouseFunc(int button, int state, int x, int y) 
{
     // get the mouse buttons
     if(button == GLUT_LEFT_BUTTON)
        if(state == GLUT_DOWN) 
{
              leftMouseButtonActive += 1;
} 
        else
          leftMouseButtonActive -= 1;
        else if(button == GLUT_MIDDLE_BUTTON)
 
           if(state == GLUT_DOWN) 
{
                 middleMouseButtonActive += 1;
                 lastXOffset = 0.0;
                 lastYOffset = 0.0;
} 
           else
              middleMouseButtonActive -= 1;
           else if(button == GLUT_RIGHT_BUTTON)
           if(state == GLUT_DOWN) 
{
                 rightMouseButtonActive += 1;
                 lastZOffset = 0.0;
} 
           else
              rightMouseButtonActive -= 1;
 
              mousePressedX = x;
              mousePressedY = y;
}
 
/***********************************************************************************************/
 
void mouseMotionFunc(int x, int y) 
{
     float xOffset = 0.0, yOffset = 0.0, zOffset = 0.0;
     // navigation
 
     // rotatation
     if(leftMouseButtonActive) 
{
           navigationRotation[0] += ((mousePressedY - y) * 180.0f) / 200.0f;
           navigationRotation[1] += ((mousePressedX - x) * 180.0f) / 200.0f;
 
           mousePressedY = y;
           mousePressedX = x;
}
 
      // panning
     else if(middleMouseButtonActive) 
{
           xOffset = (mousePressedX + x);
 
           if(!lastXOffset == 0.0) 
{
                 viewerPosition[0]  -= (xOffset - lastXOffset) / 8.0;
                 viewerDirection[0] -= (xOffset - lastXOffset) / 8.0;
}
 
           lastXOffset = xOffset;
 
           yOffset = (mousePressedY + y);
 
           if(!lastYOffset == 0.0) 
{
                 viewerPosition[1]  += (yOffset - lastYOffset) / 8.0;
                 viewerDirection[1] += (yOffset - lastYOffset) / 8.0;   
}
 
           lastYOffset = yOffset;
 
}
 
     // depth movement
     else if (rightMouseButtonActive) 
{
           zOffset = (mousePressedX + x);
 
           if(!lastZOffset == 0.0) 
{
                 viewerPosition[2] -= (zOffset - lastZOffset) / 5.0;
                 viewerDirection[2] -= (zOffset - lastZOffset) / 5.0;
}
 
           lastZOffset = zOffset;
}
}
 
/***********************************************************************************************/
 
void keyboardFunc(unsigned char key, int x, int y) 
{
     switch(key) 
{
           case '\033': // ESC 
              done = true;
              cvReleaseImage(&amp;image);
              glDeleteTextures(1, &amp;cameraImageTextureID);
              printf("Exit with ESC and bye.....\n");
              exit(0);
           break; 
 
  case '1':  
              printf("Pressed <1> = Initialitions\n"); 
              bool_makePic = false;
           break;
  case '2':
              printf("Pressed <2> = Starting\n");
              bool_makePic = true;
           break;
  case '3':
              printf("Pressed <3> = Stopp\n");
              bool_makePic = false;
           break;   
}
}
/***********************************************************************************************/
// for moveing 3dAxis
void specialFunc(int key, int x, int y) 
{
     if(key == GLUT_KEY_UP)
        xRot-= 5.0f;
 
     if(key == GLUT_KEY_DOWN)
        xRot += 5.0f;
 
     if(key == GLUT_KEY_LEFT)
        yRot -= 5.0f;
 
     if(key == GLUT_KEY_RIGHT)
        yRot += 5.0f;
 
     xRot = (GLfloat)((const int)xRot % 360);
     yRot = (GLfloat)((const int)yRot % 360);
 
     // Refresh the Window
     glutPostRedisplay();
}
 
/***********************************************************************************************/
 
int main(int argc, char **argv)
{
    // initialize camera 
    capture = cvCaptureFromCAM( 0 );
 
    // always check 
    if( !capture ) 
{
          printf( "Cannot open initialize WebCam!\n" );
          return 1;
}
 
    glutInit(&amp;argc,argv);
 
    glutInitWindowSize (640, 480);
    glutInitWindowPosition(100, 100);  
 
    glutInitDisplayMode ( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);        
    glutCreateWindow("FaceTracking");     
 
    glutReshapeFunc(reshapeFunc);
    glutKeyboardFunc(keyboardFunc);
    glutSpecialFunc(specialFunc);
    glutMouseFunc(mouseFunc);
    glutMotionFunc(mouseMotionFunc);
 
     // Register callbacks:
     glutDisplayFunc(displayFunc);
     glutIdleFunc(displayFunc);
 
     glutMainLoop();
 
     return (1);
 
}
/***********************************************************************************************/

