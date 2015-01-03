//
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:     http://safari.informit.com/9780321563835
//           http://www.opengles-book.com
//

// MultiTexture.c
//
//    This is an example that draws a quad with a basemap and
//    lightmap to demonstrate multitexturing.
//
#include "TekenBeesten.h"

#define NUM_BOIDS 1000
#define NUM_ATTRACTORS 5 
#define MAX( a, b )     ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#define MIN( a, b )     ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
///
// Initialize the shader and program object
//
int init ( ESContext *esContext )
{
   UserData *userData = esContext->userData;
   GLbyte bounceVertShader[] =  
      "precision mediump float;     \n"
      "attribute vec4 a_position;   \n"
      "attribute vec2 a_texCoord;   \n"
      "varying vec2 v_texCoord;     \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = a_position; \n"
      "   v_texCoord = a_texCoord;  \n"
      "}                            \n";

   
   GLbyte bounceFragShader[] =  
      "precision highp float;                            \n"
      "varying vec2 v_texCoord;                            \n"
      "uniform sampler2D s_sampler;                        \n"
      "uniform highp float f_fade;                       \n"
      "void main()                                         \n"
      "{                                                   \n"
      "  vec4 color = texture2D( s_sampler, v_texCoord );  \n"
      "  color -= vec4( f_fade );                          \n"
      "  vec2 edge = pow( 2.0 * ( gl_FragCoord.xy / vec2( 600.0, 600.0 ) - vec2( 0.5 ) ), vec2( 42.0 ) );\n"
      "  edge = clamp( vec2( vec2( 1.0 ) - 0.01 * edge ), 0.0, 1.0 ); \n"      
      "  gl_FragColor = color * edge.x * edge.y;                             \n"
      "}                                                   \n";

   GLbyte passThroughVertShader[] =  
      "precision mediump float;     \n"
      "attribute vec2 a_position;   \n"
      "attribute vec4 a_color;      \n"
      "varying vec4 v_color;        \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = vec4( a_position, 0.0, 1.0 ); \n"
      "   v_color = a_color;        \n"
      "}                            \n";
   
   GLbyte passThroughFragShader[] =  
      "precision mediump float;                            \n"
      "varying vec4 v_color;                               \n"
      "void main()                                         \n"
      "{                                                   \n"
      "  gl_FragColor = v_color;                            \n"
      "}                                                   \n";

   GLubyte * data;
   int width = 2048;
   int height = 1024;
   int i;

   // Load the shaders and get a linked program object
   userData->bounceProgram = esLoadProgram ( bounceVertShader, bounceFragShader );

   // Get the attribute locations
   userData->bouncePositionLoc = glGetAttribLocation ( userData->bounceProgram, "a_position" );
   userData->bounceTexCoordLoc = glGetAttribLocation ( userData->bounceProgram, "a_texCoord" );
   userData->bounceSamplerLoc = glGetUniformLocation ( userData->bounceProgram, "s_sampler" );
   userData->bounceFadeLoc = glGetUniformLocation ( userData->bounceProgram, "f_fade" );

   assertNoError( esContext );
   printf( "%d\n", userData->bounceFadeLoc );




   // Load the shaders and get a linked program object
   userData->passThroughProgram = esLoadProgram ( passThroughVertShader, passThroughFragShader );

   // Get the attribute locations
   userData->passThroughPositionLoc = glGetAttribLocation ( userData->passThroughProgram, "a_position" );
   userData->passThroughColorLoc = glGetAttribLocation ( userData->passThroughProgram, "a_color" );

   data = ( GLubyte * )calloc( width * height * 4, sizeof( GLubyte ) );
   glGenTextures ( 1, &( userData->baseMapTexId ) );
   glBindTexture ( GL_TEXTURE_2D, userData->baseMapTexId );
   glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

   glGenFramebuffers( 1, &( userData->baseFramebuffer ) );
   glBindFramebuffer( GL_FRAMEBUFFER, userData->baseFramebuffer );
   glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, userData->baseMapTexId, 0 );
   assertNoError( esContext );

   glGenTextures ( 1, &( userData->bounceMapTexId ) );
   glBindTexture ( GL_TEXTURE_2D, userData->bounceMapTexId );
   glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

   glGenFramebuffers( 1, &( userData->bounceFramebuffer ) );
   glBindFramebuffer( GL_FRAMEBUFFER, userData->bounceFramebuffer );
   glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, userData->bounceMapTexId, 0 );

   assertNoError( esContext );
   glBindFramebuffer( GL_FRAMEBUFFER, 0 );

   userData->boids = NULL;
   userData->attractors = NULL;
   userData->numBoids = 0;
   userData->numAttractors = 0;

   for( i = 0; i < NUM_ATTRACTORS; i++ ){
      addAttractor( esContext, ( ( random() % 32767 ) / 32767.0f - 0.5f ), ( ( random() % 32767 ) / 32767.0f - 0.5f ), ( random() % 32767 ) / 32767.0f, ( random() % 32767 ) / 32767.0f, ( random() % 32767 ) / 32767.0f, M_PI * ( random() % 32767 ) / 32767.0f, 0.1f * ( random() % 32767 ) / 32767.0f, 0.4f );
   }
   for( i = 0; i < NUM_BOIDS; i++ ){
      addBoid( esContext, ( ( random() % 32767 ) / 32767.0f - 0.5f ), ( ( random() % 32767 ) / 32767.0f - 0.5f ), random() );
   }

   glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
   glEnable( GL_BLEND );
   glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

   free( data );
   return TRUE;
}

void rgb2hsl( float r, float g, float b, float *h, float *s, float *l ){
    float M = 0.0, m = 0.0, c = 0.0;
     M = MAX( r, MAX( g, b ) );
     m = MIN( r, MIN( g, b ) );
     c = M - m;
     *h = 0.0;
     *s = 0.0;
     *l = 0.5 * ( M + m );
     if (c != 0.0)
     {
         if (M == r)
         {
             *h = fmod(((g - b) / c), 6.0);
         }
         else if (M == g)
         {
             *h = ((b - r) / c) + 2.0;
         }
         else/*if(M==b)*/
         {
             *h = ((r - g) / c) + 4.0;
         }
         *h *= 60.0;
         *s = c / ( 1.0 - fabs( 2.0 * ( *l ) - 1.0 ) );
     }
}

void hsl2rgb( float h, float s, float l, float *r, float *g, float *b ){
   float _r;
   float _g;
   float _b;
   while( h < 0.0 ){
       h += 360.0 ;   
   }
   while( h > 360.0 ){
       h -= 360.0;   
   }
   
   if( h < 120.0 ){   
      ( *r ) = ( 120.0 - h ) / 60.0 ;   
      ( *g ) = h / 60.0 ;   
      ( *b ) = 0.0;   
   }   
   else if( h < 240.0 ){   
      ( *r ) = 0.0;
      ( *g ) = ( 240.0 - h) / 60.0 ;   
      ( *b ) = ( h - 120.0 ) / 60.0 ;     
   }   
   else{   
      ( *r ) = ( h - 240.0 ) / 60.0 ;    
      ( *g ) = 0.0;   
      ( *b ) = ( 360.0 - h ) / 60.0 ;                  
   }   

   ( *r ) = MIN( ( *r ), 1.0 ) ;   
   ( *g ) = MIN( ( *g ), 1.0 ) ;   
   ( *b ) = MIN( ( *b ), 1.0 ) ;   

   _r = 2.0 * s * ( *r ) + ( 1.0 - s );     
   _g = 2.0 * s * ( *g ) + ( 1.0 - s );   
   _b = 2.0 * s * ( *b ) + ( 1.0 - s );   

   if( l < 0.5 ){   
     ( *r ) =  l * _r ;     
     ( *g ) =  l * _g ;     
     ( *b ) =  l * _b ;     
   }   
   else {   
     ( *r ) = ( 1.0 - l ) * _r + 2.0 * l - 1.0 ;   
     ( *g ) = ( 1.0 - l ) * _g + 2.0 * l - 1.0 ;    
     ( *b ) = ( 1.0 - l ) * _b + 2.0 * l - 1.0 ;    
   }   
}


void assertNoError( ESContext * esContext ){
   GLenum error = glGetError();
   GLubyte * errorString = 0;
   switch( error ){
      case GL_INVALID_ENUM:
         errorString = "GL_INVALID_ENUM\n"
                       "An unacceptable value is specified for an enumerated argument.\n"
                       "The offending command is ignored\n"
                       "and has no other side effect than to set the error flag.\n";
                       break;
      case GL_INVALID_VALUE:
         errorString = "GL_INVALID_VALUE\n"
                       "A numeric argument is out of range.\n"
                       "The offending command is ignored\n"
                       "and has no other side effect than to set the error flag.\n";
                       break;
      case GL_INVALID_OPERATION:
         errorString = "GL_INVALID_OPERATION\n"
                       "The specified operation is not allowed in the current state.\n"
                       "The offending command is ignored\n"
                       "and has no other side effect than to set the error flag.\n";
                       break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
         errorString = "GL_INVALID_FRAMEBUFFER_OPERATION\n"
                       "The command is trying to render to or read from the framebuffer\n"
                       "while the currently bound framebuffer is not framebuffer \n"
                       "complete (i.e. the return value from \n"
                       "glCheckFramebufferStatus\n"
                       "is not GL_FRAMEBUFFER_COMPLETE).\n"
                       "The offending command is ignored\n"
                       "and has no other side effect than to set the error flag.\n";
                       break;
      case GL_OUT_OF_MEMORY:
         errorString = "GL_OUT_OF_MEMORY\n"
                       "There is not enough memory left to execute the command.\n"
                       "The state of the GL is undefined,\n"
                       "except for the state of the error flags,\n"
                       "after this error is recorded.\n";
                       break;
   }                    
   GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
   if( status != GL_FRAMEBUFFER_COMPLETE ){
      printf( "glCheckFramebufferStatus: error making framebuffer\n" );
      shutDown( esContext );
      exit( 0 );
   }

   if( errorString ){
      printf( "%s", errorString );
      shutDown( esContext );
      exit( 0 );
   }      
}

void addAttractor( ESContext * esContext, float x, float y, float r, float g, float b, float phase, float frequency, float amplitude ){
   static int counter = 0;
   UserData *userData = esContext->userData;
   userData->numAttractors++;
   userData->attractors = ( Attractor * )realloc( userData->attractors, userData->numAttractors * sizeof( Attractor ) );

   Attractor * attractor = &( userData->attractors[ userData->numAttractors - 1 ] );
   attractor->x = x;
   attractor->y = y;
   attractor->x_ = x;
   attractor->y_ = y;

   attractor->r = r;
   attractor->g = g;
   attractor->b = b;

   float h, s, l;
   rgb2hsl( r, g, b, &h, &s, &l );
   attractor->h = h;
   attractor->s = s;
   attractor->l = l;

   attractor->index = counter++;
   attractor->power = 0.0f;
   attractor->phase = phase;
   attractor->frequency = frequency;
   attractor->amplitude = amplitude;
}

void addBoid( ESContext * esContext, float x, float y, int attractor ){
   UserData *userData = esContext->userData;
   userData->numBoids++;
   userData->boids = ( Boid * )realloc( userData->boids, userData->numBoids * sizeof( Boid ) );

   Boid * boid = &( userData->boids[ userData->numBoids - 1 ] );
   boid->x = x;
   boid->y = y;
   boid->x_ = x - 0.01f;
   boid->y_ = y;

   boid->attractor = & userData->attractors[ attractor % userData->numAttractors ];

   float r, g, b;
   float so = 0.1f * ( ( ( random() % 32767 ) / 32767.0 ) - 0.5 ) * 2.0;
   float s =  MIN( MAX( 0.0, boid->attractor->s + so ), 1.0 );
   float lo = 0.1f * ( ( ( random() % 32767 ) / 32767.0 ) - 0.5 ) * 2.0;
   float l =  MIN( MAX( 0.0, 0.7 + lo ), 1.0 );
   hsl2rgb( boid->attractor->h, s, l, &r, &g, &b );

   boid->r = r;
   boid->g = g;
   boid->b = b;
}

void updateBoids( ESContext * esContext ){
   UserData *userData = esContext->userData;
   int i;

   float left = -1.0f;
   float right = 1.0f;
   float bottom = -1.0f;
   float top = 1.0f;

   float speed = 0.0075f;
   float centerGrav = 0.02f;
   float attractorGrav = 0.5f;
   float randomVar = 0.03f;

   float * centerX = ( float * )calloc( userData->numAttractors, sizeof( float ) );
   float * centerY = ( float * )calloc( userData->numAttractors, sizeof( float ) );
   int * counter = ( int * )calloc( userData->numAttractors, sizeof( int ) );

   for( i = 0; i < userData->numBoids; i++ ){
      Boid * boid = &( userData->boids[ i ] );
      centerX[ boid->attractor->index ] += boid->x;
      centerY[ boid->attractor->index ] += boid->y;
      counter[ boid->attractor->index ]++;
   }
   for( i = 0; i < userData->numAttractors; i++ ){
      centerX[ i ] /= (float)counter[ i ];
      centerY[ i ] /= (float)counter[ i ];
   }

   for( i = 0; i < userData->numBoids; i++ ){
      Boid * boid = &( userData->boids[ i ] );
      float dx = boid->x - boid->x_;
      float dy = boid->y - boid->y_;
      boid->x_ = boid->x;
      boid->y_ = boid->y;

      float hyp = dx * dx + dy * dy;
      if( hyp > 0.0f ){
         float len = sqrtf( hyp );

         dx /= len;
         dy /= len;

         if( boid->x < left ){
            boid->x = left;
            dx *= -1.0f;
         }
         else if( boid->x > right ){
            boid->x = right;
            dx *= -1.0f;
         }
         if( boid->y < bottom ){
            boid->y = bottom;
            dy *= -1.0f;
         }
         else if( boid->y > top ){
            boid->y = top;
            dy *= -1.0f;
         }

         float angle = M_PI * 2.0 * randomVar * 2.0 * ( ( random() % 32767 ) / 32767.0f - 0.5f );
         dx = ( dx * cos( angle ) ) - ( dy * sin( angle ) );
         dy = ( dy * cos( angle ) ) + ( dx * sin( angle ) );

         dx += randomVar * ( ( random() % 32767 ) / 32767.0f - 0.5f );
         dy += randomVar * ( ( random() % 32767 ) / 32767.0f - 0.5f );

         float bax = boid->attractor->x - boid->x;
         float bay = boid->attractor->y - boid->y;

         float hyp = bax * bax + bay * bay;
         if( hyp > 0.0f ){
            float len = sqrtf( hyp );
            bax /= len;
            bay /= len;

            float attraction = attractorGrav * boid->attractor->power;
            dx = ( 1.0 - attraction ) * dx + attraction * bax;
            dy = ( 1.0 - attraction ) * dy + attraction * bay;

            float cx = centerX[ boid->attractor->index ] - boid->x;
            float cy = centerY[ boid->attractor->index ] - boid->y;

            float hyp = cx * cx + cy * cy;
            if( hyp > 0.0f ){
               float len = sqrtf( hyp );
               cx /= len;
               cy /= len;

               dx = ( 1.0 - centerGrav ) * dx + centerGrav * cx;
               dy = ( 1.0 - centerGrav ) * dy + centerGrav * cy;

               float hyp = dx * dx + dy * dy;
               if( hyp > 0.0f ){
                  float len = sqrtf( hyp );
                  dx /= len;
                  dy /= len;

                  boid->x += dx * speed;
                  boid->y += dy * speed;
               }
            }
         }
      }
   }
   free( centerX );
   free( centerY );
   free( counter );
}

void updateAttractors( ESContext * esContext ){
   UserData *userData = esContext->userData;
   int i;

   float left = -1.0f;
   float right = 1.0f;
   float bottom = -1.0f;
   float top = 1.0f;

   float lerp = 0.98;

   for( i = 0; i < userData->numAttractors; i++ ){
      Attractor * attractor = &( userData->attractors[ i ] );

      attractor->x = lerp * attractor->x + ( 1.0 - lerp ) * attractor->x_;
      attractor->y = lerp * attractor->y + ( 1.0 - lerp ) * attractor->y_;

      attractor->power = attractor->amplitude * ( 0.5f + 0.5f * sin( attractor->phase ) );
      attractor->phase += attractor->frequency;
   }
}

void setAttractorPosition( ESContext * esContext, int which, float x, float y ){
   UserData *userData = esContext->userData;
   Attractor * attractor = &( userData->attractors[ which % userData->numAttractors ] );
   attractor->x_ = x;
   attractor->y_ = y;
}

void drawBoids( ESContext * esContext ){
   UserData *userData = esContext->userData;
   int i, j;
   float radius = 0.01;
/*
   GLfloat * segments = ( GLfloat * )malloc( ( 2 + 4 ) * 2 * userData->numBoids * sizeof( GLfloat ) );
   for( i = 0; i < userData->numBoids; i++ ){
      Boid * boid = &( userData->boids[ i ] );
      segments[ ( 2 + 4 ) * 2 * i + 0 ] = boid->x;
      segments[ ( 2 + 4 ) * 2 * i + 1 ] = boid->y;
      segments[ ( 2 + 4 ) * 2 * i + 2 ] = boid->attractor->r;
      segments[ ( 2 + 4 ) * 2 * i + 3 ] = boid->attractor->g;
      segments[ ( 2 + 4 ) * 2 * i + 4 ] = boid->attractor->b;
      segments[ ( 2 + 4 ) * 2 * i + 5 ] = 1.0;

      segments[ ( 2 + 4 ) * 2 * i + 6 ] = boid->x_;
      segments[ ( 2 + 4 ) * 2 * i + 7 ] = boid->y_;
      segments[ ( 2 + 4 ) * 2 * i + 8 ] = boid->attractor->r;
      segments[ ( 2 + 4 ) * 2 * i + 9 ] = boid->attractor->g;
      segments[ ( 2 + 4 ) * 2 * i + 10 ] = boid->attractor->b;
      segments[ ( 2 + 4 ) * 2 * i + 11 ] = 1.0;
   }
   glVertexAttribPointer ( userData->passThroughPositionLoc, 2, GL_FLOAT, 
                           GL_FALSE, 6 * sizeof( GLfloat ), segments );
   glVertexAttribPointer ( userData->passThroughColorLoc, 4, GL_FLOAT,
                           GL_FALSE, 6 * sizeof( GLfloat ), &( segments[ 2 ] ) );
   glEnableVertexAttribArray ( userData->passThroughPositionLoc );
   glEnableVertexAttribArray ( userData->passThroughColorLoc );
   glLineWidth( radius * 2048.0 );
   glDrawArrays( GL_LINES, 0, userData->numBoids * 2 );
   free( segments );
*/
   GLfloat circle[ 17 * ( 2 + 4 ) ];
   for( i = 0; i < userData->numBoids; i++ ){
      Boid * boid = &( userData->boids[ i ] );

      circle[ 0 ] = boid->x;
      circle[ 1 ] = boid->y;
      circle[ 2 ] = boid->r;
      circle[ 3 ] = boid->g;
      circle[ 4 ] = boid->b;
      circle[ 5 ] = 0.4;
      for( j = 0; j < 16; j++ ){
         float a = (float)j / 15.0f * 3.14165 * 2.0f;
         circle[ ( j + 1 ) * ( 2 + 4 ) + 0 ] = boid->x + cos( a ) * radius;
         circle[ ( j + 1 ) * ( 2 + 4 ) + 1 ] = boid->y + sin( a ) * radius;
         circle[ ( j + 1 ) * ( 2 + 4 ) + 2 ] = boid->r;
         circle[ ( j + 1 ) * ( 2 + 4 ) + 3 ] = boid->g;
         circle[ ( j + 1 ) * ( 2 + 4 ) + 4 ] = boid->b;
         circle[ ( j + 1 ) * ( 2 + 4 ) + 5 ] = 0.0;
      }
      glVertexAttribPointer ( userData->passThroughPositionLoc, 2, GL_FLOAT, 
                              GL_FALSE, 6 * sizeof( GLfloat ), circle );
      glVertexAttribPointer ( userData->passThroughColorLoc, 4, GL_FLOAT,
                              GL_FALSE, 6 * sizeof( GLfloat ), &( circle[ 2 ] ) );
      glEnableVertexAttribArray ( userData->passThroughPositionLoc );
      glEnableVertexAttribArray ( userData->passThroughColorLoc );
      glDrawArrays( GL_TRIANGLE_FAN, 0, 17 );
   }

}


void drawAttractors( ESContext * esContext ){
   UserData *userData = esContext->userData;
   int i, j;
   float size = 0.1f;

   GLfloat * segments = ( GLfloat * )malloc( ( 2 + 4 ) * 4 * userData->numAttractors * sizeof( GLfloat ) );
   for( i = 0; i < userData->numAttractors; i++ ){
      Attractor * attractor = &( userData->attractors[ i ] );
      segments[ ( 2 + 4 ) * 4 * i + 0 ] = attractor->x - size;
      segments[ ( 2 + 4 ) * 4 * i + 1 ] = attractor->y;
      segments[ ( 2 + 4 ) * 4 * i + 2 ] = attractor->r;
      segments[ ( 2 + 4 ) * 4 * i + 3 ] = attractor->g;
      segments[ ( 2 + 4 ) * 4 * i + 4 ] = attractor->b;
      segments[ ( 2 + 4 ) * 4 * i + 5 ] = 1.0f;

      segments[ ( 2 + 4 ) * 4 * i + 6 ] = attractor->x + size;
      segments[ ( 2 + 4 ) * 4 * i + 7 ] = attractor->y;
      segments[ ( 2 + 4 ) * 4 * i + 8 ] = attractor->r;
      segments[ ( 2 + 4 ) * 4 * i + 9 ] = attractor->g;
      segments[ ( 2 + 4 ) * 4 * i + 10 ] = attractor->b;
      segments[ ( 2 + 4 ) * 4 * i + 11 ] = 1.0f;

      segments[ ( 2 + 4 ) * 4 * i + 12 ] = attractor->x;
      segments[ ( 2 + 4 ) * 4 * i + 13 ] = attractor->y - size;
      segments[ ( 2 + 4 ) * 4 * i + 14 ] = attractor->r;
      segments[ ( 2 + 4 ) * 4 * i + 15 ] = attractor->g;
      segments[ ( 2 + 4 ) * 4 * i + 16 ] = attractor->b;
      segments[ ( 2 + 4 ) * 4 * i + 17 ] = attractor->power;

      segments[ ( 2 + 4 ) * 4 * i + 18 ] = attractor->x;
      segments[ ( 2 + 4 ) * 4 * i + 19 ] = attractor->y + size;
      segments[ ( 2 + 4 ) * 4 * i + 20 ] = attractor->r;
      segments[ ( 2 + 4 ) * 4 * i + 21 ] = attractor->g;
      segments[ ( 2 + 4 ) * 4 * i + 22 ] = attractor->b;
      segments[ ( 2 + 4 ) * 4 * i + 23 ] = attractor->power;

   }
   glVertexAttribPointer ( userData->passThroughPositionLoc, 2, GL_FLOAT, 
                           GL_FALSE, 6 * sizeof( GLfloat ), segments );
   glVertexAttribPointer ( userData->passThroughColorLoc, 4, GL_FLOAT,
                           GL_FALSE, 6 * sizeof( GLfloat ), &( segments[ 2 ] ) );
   glEnableVertexAttribArray ( userData->passThroughPositionLoc );
   glEnableVertexAttribArray ( userData->passThroughColorLoc );
   glLineWidth( 2.0 );
   glDrawArrays( GL_LINES, 0, userData->numAttractors * 4 );
   free( segments );
}

// Draw a triangle using the shader pair created in init()
//
void update ( ESContext *esContext )
{
   static int iter = 0;
   iter++;
   UserData *userData = esContext->userData;
   GLfloat vBounceVertices[] = { -1.0f,  1.0f, 0.0f, 
                                  0.0f,  0.0f,       
                                 -1.0f, -1.0f, 0.0f, 
                                  0.0f,  600.0f / 1024.0f,       
                                  1.0f, -1.0f, 0.0f, 
                                  600.0f / 2048.0f,  600.0f / 1024.0f,       
                                  1.0f,  1.0f, 0.0f, 
                                  600.0f / 2048.0f,  0.0f        
                           };

   float xoff = -1.0f / 2048.0f;                           
   float yoff = -1.0f / 1024.0f;                           
   GLfloat vBounceVertices2[] = { -1.0f,  1.0f, 0.0f, 
                                  -xoff,  -yoff,       
                                 -1.0f, -1.0f, 0.0f, 
                                  -xoff,  600.0f / 1024.0f + yoff,       
                                  1.0f, -1.0f, 0.0f, 
                                  600.0f / 2048.0f + xoff,  600.0f / 1024.0f + yoff,       
                                  1.0f,  1.0f, 0.0f, 
                                  600.0f / 2048.0f + xoff,  -yoff       
                           };
   GLushort bounceIndices[] = { 0, 1, 2, 0, 2, 3 };

   // Set the viewport
   glViewport ( 0, 0, esContext->width, esContext->height );

   // bind the base map
   glBindFramebuffer( GL_FRAMEBUFFER, userData->baseFramebuffer );

   // draw the bounce map into the base map
   glActiveTexture( GL_TEXTURE0 );
   glBindTexture( GL_TEXTURE_2D, userData->bounceMapTexId );
   glUseProgram( userData->bounceProgram );
   if( iter % 2 == 0 ){
      glVertexAttribPointer( userData->bouncePositionLoc, 3, GL_FLOAT, 
                              GL_FALSE, 5 * sizeof(GLfloat), vBounceVertices2 );
      glVertexAttribPointer( userData->bounceTexCoordLoc, 2, GL_FLOAT,
                              GL_FALSE, 5 * sizeof(GLfloat), &vBounceVertices2[ 3 ] );
   }
   else{
      glVertexAttribPointer( userData->bouncePositionLoc, 3, GL_FLOAT, 
                              GL_FALSE, 5 * sizeof(GLfloat), vBounceVertices );
      glVertexAttribPointer( userData->bounceTexCoordLoc, 2, GL_FLOAT,
                              GL_FALSE, 5 * sizeof(GLfloat), &vBounceVertices[ 3 ] );
   }
   glEnableVertexAttribArray( userData->bouncePositionLoc );
   glEnableVertexAttribArray( userData->bounceTexCoordLoc );
   glUniform1i( userData->bounceSamplerLoc, 0 );
   if( iter % 7 == 0 ){
      glUniform1f( userData->bounceFadeLoc, 2.0f / 255.0f );
   }
   else{
      glUniform1f( userData->bounceFadeLoc, 0.0f );
   }
   glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, bounceIndices );

   assertNoError( esContext );

   // draw the boids (into the base map)
   glUseProgram ( userData->passThroughProgram );

   if( random() % 100 == 0 ){
      setAttractorPosition( esContext, random(), ( ( random() % 32767 ) / 32767.0f - 0.5f ) * 2.0f, ( ( random() % 32767 ) / 32767.0f - 0.5f ) * 2.0f );
   }

   updateBoids( esContext );
   updateAttractors( esContext );

   drawBoids( esContext );
//   drawAttractors( esContext );



   // copy the base texture to the bounce buffer
   glBindFramebuffer( GL_FRAMEBUFFER, userData->bounceFramebuffer );

   glActiveTexture ( GL_TEXTURE0 );
   glBindTexture ( GL_TEXTURE_2D, userData->baseMapTexId );
   glUseProgram ( userData->bounceProgram );
   glVertexAttribPointer ( userData->bouncePositionLoc, 3, GL_FLOAT, 
                           GL_FALSE, 5 * sizeof(GLfloat), vBounceVertices );
   glVertexAttribPointer ( userData->bounceTexCoordLoc, 2, GL_FLOAT,
                           GL_FALSE, 5 * sizeof(GLfloat), & vBounceVertices[ 3 ] );

   glEnableVertexAttribArray ( userData->bouncePositionLoc );
   glEnableVertexAttribArray ( userData->bounceTexCoordLoc );
   glUniform1i( userData->bounceSamplerLoc, 0 );
   glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, bounceIndices );

   // draw the base texture onto the screen
   glBindFramebuffer( GL_FRAMEBUFFER, 0 );

   glActiveTexture ( GL_TEXTURE0 );
   glBindTexture ( GL_TEXTURE_2D, userData->bounceMapTexId );
   glUseProgram ( userData->bounceProgram );
   glVertexAttribPointer ( userData->bouncePositionLoc, 3, GL_FLOAT, 
                           GL_FALSE, 5 * sizeof(GLfloat), vBounceVertices );
   glVertexAttribPointer ( userData->bounceTexCoordLoc, 2, GL_FLOAT,
                           GL_FALSE, 5 * sizeof(GLfloat), & vBounceVertices[ 3 ] );

   glEnableVertexAttribArray ( userData->bouncePositionLoc );
   glEnableVertexAttribArray ( userData->bounceTexCoordLoc );
   glUniform1i ( userData->bounceSamplerLoc, 0 );
   glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, bounceIndices );

   eglSwapBuffers ( esContext->eglDisplay, esContext->eglSurface );
}

///
// Cleanup
//
void shutDown ( ESContext *esContext )
{
   UserData *userData = esContext->userData;

   // Delete texture object
   glDeleteTextures ( 1, &userData->baseMapTexId );
   glDeleteTextures ( 1, &userData->bounceMapTexId );

   // Delete program object
   glDeleteProgram ( userData->bounceProgram );
   glDeleteProgram ( userData->passThroughProgram );
}


int main ( int argc, char *argv[] )
{
   ESContext esContext;
   UserData  userData;

   esInitContext ( &esContext );
   esContext.userData = &userData;

   esCreateWindow( &esContext, "TekenBeesten", 600, 600, ES_WINDOW_RGB );
   
   if ( !init ( &esContext ) ){
      return 0;
   }

   esRegisterDrawFunc ( &esContext, update );
   
   esMainLoop ( &esContext );

   shutDown ( &esContext );
}
