//
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com
//

// MultiTexture.c
//
//    This is an example that draws a quad with a basemap and
//    lightmap to demonstrate multitexturing.
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "esUtil.h"

typedef struct
{
   GLfloat x;
   GLfloat y;

   GLfloat x_;
   GLfloat y_;

   GLfloat r;
   GLfloat g;
   GLfloat b;

   float power;
   float amplitude;
   float frequency;
   float phase;

} Attractor;

typedef struct
{
   GLfloat x;
   GLfloat y;

   GLfloat x_;
   GLfloat y_;

   Attractor * attractor;
} Boid;

typedef struct
{
   // Handle to a program object
   GLuint bounceProgram;
   GLuint passThroughProgram;

   // Attribute locations
   GLint bouncePositionLoc;
   GLint bounceTexCoordLoc;
   GLint bounceSamplerLoc;

   // Attribute locations
   GLint  passThroughPositionLoc;
   GLint  passThroughColorLoc;

   // Texture handles
   GLuint baseMapTexId;
   GLuint bounceMapTexId;

   GLuint baseFramebuffer;
   GLuint bounceFramebuffer;

   Boid * boids;
   Attractor * attractors;
   int numBoids;
   int numAttractors;

} UserData;

void  addAttractor( ESContext * esContext, float x, float y, float r, float g, float b, float phase, float frequency, float amplitude );
void  addBoid( ESContext * esContext, float x, float y, int attractor );
void  updateBoids( ESContext * esContext );
void  updateAttractors( ESContext * esContext );
void  setAttractorPosition( ESContext * esContext, int which, float x, float y );
void  drawBoids( ESContext * esContext );
int   init ( ESContext *esContext );
void  update ( ESContext *esContext );
void  shutDown ( ESContext *esContext );
