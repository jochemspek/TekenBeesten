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

#define NUM_BOIDS           1000
#define NUM_ATTRACTORS      5
#define MAX( a, b )         ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#define MIN( a, b )         ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#define TEXTURE_WIDTH       2048
#define TEXTURE_HEIGHT      1024
#define WINDOW_WIDTH        1280
#define WINDOW_HEIGHT       800
#define SPEED               2.0
#define CENTERGRAV          0.0007f
#define ATTRACTORGRAV       0.003f//0.5f
#define RANDOMVAR           0.010//0.03f
#define ATTRACTORRADIUS     100.0//0.03f
#define COLLISIONRADIUS     15.0
#define DOCOLLISION         TRUE
#define SATURATIONVARIATION 0.1
#define LUMAVARIATION       0.3
#define BOIDALPHA           0.4
#define BOIDRADIUS          10.0

typedef struct
{
   GLfloat x;
   GLfloat y;

   GLfloat x_;
   GLfloat y_;

   GLfloat r;
   GLfloat g;
   GLfloat b;

   GLfloat h;
   GLfloat s;
   GLfloat l;

   int enabled;
   int index;
   float power;
   float amplitude;
   float frequency;
   float phase;
   float radius;

} Attractor;

typedef struct
{
   GLfloat x;
   GLfloat y;

   GLfloat r;
   GLfloat g;
   GLfloat b;

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
   GLint bounceFadeLoc;
   GLint bounceWidthLoc;
   GLint bounceHeightLoc;
   GLint bounceProjectionLoc;

   // Attribute locations
   GLint  passThroughPositionLoc;
   GLint  passThroughColorLoc;
   GLint passThroughProjectionLoc;
   // Texture handles
   GLuint baseMapTexId;
   GLuint bounceMapTexId;

   GLuint baseFramebuffer;
   GLuint bounceFramebuffer;

   GLfloat     boidVertices[ NUM_BOIDS * 6 ];
   float       centerX[ NUM_ATTRACTORS ];
   float       centerY[ NUM_ATTRACTORS ];
   int         counter[ NUM_ATTRACTORS ];
   Boid        boids[ NUM_BOIDS ];
   Attractor   attractors[ NUM_ATTRACTORS ];

} UserData;

void  rgb2hsl( float r, float g, float b, float *h, float *s, float *l );
void  hsl2rgb( float h, float s, float l, float *r, float *g, float *b );
void  addAttractor( ESContext * esContext, int index, float x, float y, float r, float g, float b, float phase, float frequency, float amplitude );
void  addBoid( ESContext * esContext, int index, float x, float y, int attractor );
void  updateBoids( ESContext * esContext );
void  updateAttractors( ESContext * esContext );
void  setAttractorPosition( ESContext * esContext, int which, float x, float y );
void  enableAttractor( ESContext * esContext, int which );
void  disableAttractor( ESContext * esContext, int which );
void  drawBoids( ESContext * esContext );
void  setBoidColor( Boid * boid );
int   init ( ESContext *esContext );
void  assertNoError( ESContext *esContext );
void  update ( ESContext *esContext );
void  shutDown ( ESContext *esContext );
