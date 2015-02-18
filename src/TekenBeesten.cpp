#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include "TekenBeesten.h"
#include "tracking.h"
#include "settings.h"


Tracks TrackPositions;
boost::mutex track_mutex; 

int init ( ESContext *esContext )
{
  UserData *userData = (UserData *) esContext->userData;
  Settings& settings = userData->settings;
  
  char bounceVertShader[] =  
    "#version 120                 \n"
    "attribute vec4 a_position;   \n"
    "attribute vec2 a_texCoord;   \n"
    "varying vec2 v_texCoord;     \n"
    "uniform mat4 m_projection;   \n"
    "void main()                  \n"
    "{                            \n"
    "   gl_Position = m_projection * vec4( a_position.x, a_position.y, 0.0, 1.0 ); \n"
    "   v_texCoord = a_texCoord;  \n"
    "}                            \n";

  char bounceFragShader[] = 
    "#version 120                                         \n"  
    "varying vec2 v_texCoord;                             \n"
    "uniform sampler2D s_sampler;                         \n"
    "uniform float f_fade;                                \n"
    "uniform float f_width;                               \n"
    "uniform float f_height;                              \n"
    "void main()                                          \n"
    "{                                                    \n"
    "  vec4 color = texture2D( s_sampler, v_texCoord );   \n"
    "  color -= vec4( f_fade );                           \n"
    "  vec2 edge = vec2( 1.0 ) - 0.01 * pow( 2.0 * abs( gl_FragCoord.xy / vec2( f_width, f_height ) - vec2( 0.5 ) ), vec2( 22.0 ) );    \n"
    "  gl_FragColor = vec4( color.xyz * edge.x * edge.y, 1.0 );                             \n"
    "}                                                    \n";

  char passThroughVertShader[] =  
    "#version 120                 \n"  
    "attribute vec2 a_position;   \n"
    "attribute vec4 a_color;      \n"
    "varying vec4 v_color;        \n"
    "uniform mat4 m_projection;   \n"
    "void main()                  \n"
    "{                            \n"
    "   gl_PointSize = 5.0;       \n"
    "   gl_Position = m_projection * vec4( a_position, 0.0, 1.0 ); \n"
    "   v_color = a_color;        \n"
    "}                            \n";

  char passThroughFragShader[] =  
    "#version 120                                        \n"  
    "varying vec4 v_color;                               \n"
    "void main()                                         \n"
    "{                                                   \n"
    "   float edge = min( 0.707 - distance( gl_PointCoord, vec2( 0.5, 0.5 ) ), 1.0 ); \n"
    "   gl_FragColor = vec4( v_color.xyz, 0.7 * edge );                            \n"
     "}                                                   \n";

  GLubyte * data;
  int i, j;
  

  // Load the shaders and get a linked program object
  userData->bounceProgram = esLoadProgram ( bounceVertShader, bounceFragShader );

  // Get the attribute locations
  userData->bouncePositionLoc = glGetAttribLocation ( userData->bounceProgram, "a_position" );
  userData->bounceTexCoordLoc = glGetAttribLocation ( userData->bounceProgram, "a_texCoord" );
  userData->bounceSamplerLoc = glGetUniformLocation ( userData->bounceProgram, "s_sampler" );
  userData->bounceFadeLoc = glGetUniformLocation ( userData->bounceProgram, "f_fade" );
  userData->bounceWidthLoc = glGetUniformLocation ( userData->bounceProgram, "f_width" );
  userData->bounceHeightLoc = glGetUniformLocation ( userData->bounceProgram, "f_height" );
  userData->bounceProjectionLoc = glGetUniformLocation ( userData->bounceProgram, "m_projection" );

  assertNoError( esContext );

  // Load the shaders and get a linked program object
  userData->passThroughProgram = esLoadProgram ( passThroughVertShader, passThroughFragShader );

  // Get the attribute locations
  userData->passThroughPositionLoc = glGetAttribLocation ( userData->passThroughProgram, "a_position" );
  userData->passThroughColorLoc = glGetAttribLocation ( userData->passThroughProgram, "a_color" );
  userData->passThroughProjectionLoc = glGetUniformLocation ( userData->passThroughProgram, "m_projection" );

  data = ( GLubyte * )calloc( TEXTURE_WIDTH * TEXTURE_HEIGHT * 4, sizeof( GLubyte ) );

  glGenTextures ( 1, &( userData->baseMapTexId ) );
  glBindTexture ( GL_TEXTURE_2D, userData->baseMapTexId );
  glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
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
  glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

  glGenFramebuffers( 1, &( userData->bounceFramebuffer ) );
  glBindFramebuffer( GL_FRAMEBUFFER, userData->bounceFramebuffer );
  glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, userData->bounceMapTexId, 0 );

  assertNoError( esContext );
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );

  for( i = 0; i < settings.graphics_num_attractors; i++ ){
    addAttractor( esContext, i, ( ( random() % 32767 ) / 32767.0f ) * settings.graphics_window_width, ( ( random() % 32767 ) / 32767.0f ) * settings.graphics_window_height, ( random() % 32767 ) / 32767.0f, ( random() % 32767 ) / 32767.0f, ( random() % 32767 ) / 32767.0f, M_PI * ( random() % 32767 ) / 32767.0f, 0.1f * ( random() % 32767 ) / 32767.0f, 0.4f );
  }
  for( i = 0; i < settings.graphics_num_boids; i++ ){
    addBoid( esContext, i, ( ( random() % 32767 ) / 32767.0f ) * settings.graphics_window_width, ( ( random() % 32767 ) / 32767.0f ) * settings.graphics_window_height, random() );
  }

  glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
  glEnable( GL_BLEND );
  glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glPointSize( 5.0 );
  glEnable( GL_POINT_SPRITE );


  free( data );
  return TRUE;
}

void  enableAttractor( ESContext * esContext, int which ){  
    UserData *userData = (UserData *) esContext->userData;
    Settings& settings = userData->settings;
    
    Attractor * attractor = &( userData->attractors[ which % settings.graphics_num_attractors ] );

    if (attractor->enabled) return;

    attractor->enabled = 1;
    // count the number of enabled attractors (including this one)
    int i, count = 0;
    for( i = 0; i < settings.graphics_num_attractors; i++ ){
      count += userData->attractors[ i ].enabled;
    }
    if( count == 1 ){
      // this is the only attractor, so assign all boids to it
      for( i = 0; i < settings.graphics_num_boids; i++ ){
        userData->boids[ i ].attractor = attractor;
        setBoidColor( & userData->boids[ i ] );
      }
    }
    else{
      // attach a number of boids to this attractor
      for( i = 0; i < settings.graphics_num_boids / count; i++ ){
        int n = rand() % settings.graphics_num_boids;
        userData->boids[ n ].attractor = attractor;
        setBoidColor( & userData->boids[ n ] );
      }
    }

}

void  disableAttractor( ESContext * esContext, int which ){
  UserData *userData = (UserData *) esContext->userData;
  Settings& settings = userData->settings;
  
  Attractor * attractor = &( userData->attractors[ which % settings.graphics_num_attractors ] );

  // disable
  attractor->enabled = 0;

  // collect the number of enabled attractors (except this one)
  int i, count = 0;
  Attractor * enabledAttractors[ settings.graphics_num_boids ];
  for( i = 0; i < settings.graphics_num_attractors; i++ ){
    if( userData->attractors[ i ].enabled ){
      enabledAttractors[ count ] = & userData->attractors[ i ];
      count++;
    }
  }

  if( count ){
    // attach each boid that was attached to this attractor to an enabled attractor
    for( i = 0; i < settings.graphics_num_boids; i++ ){
      if( userData->boids[ i ].attractor->index == attractor->index ){
        int n = rand() % count;
        userData->boids[ i ].attractor = enabledAttractors[ n ];
        setBoidColor( & userData->boids[ i ] );
      }
    }
  }
  else{
    // there are NO attractors available, so set all boids to NULL
    for( i = 0; i < settings.graphics_num_boids; i++ ){
      userData->boids[ i ].attractor = NULL;
      setBoidColor( & userData->boids[ i ] );
    }
  }
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
   const char * errorString = 0;
   switch( error ){
      case GL_INVALID_ENUM:
         errorString = "GL_INVALID_ENUM\n";
         break;
      case GL_INVALID_VALUE:
         errorString = "GL_INVALID_VALUE\n";
         break;
      case GL_INVALID_OPERATION:
         errorString = "GL_INVALID_OPERATION\n";
         break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
         errorString = "GL_INVALID_FRAMEBUFFER_OPERATION\n";
         break;
      case GL_OUT_OF_MEMORY:
         errorString = "GL_OUT_OF_MEMORY\n";
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

void addAttractor( ESContext * esContext, int index, float x, float y, float r, float g, float b, float phase, float frequency, float amplitude ){
   static int counter = 0;
   UserData *userData = (UserData *) esContext->userData;
   Settings& settings = userData->settings;
   

   Attractor * attractor = &( userData->attractors[ index ] );
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

   attractor->enabled = 1;
   attractor->index = counter++;
   attractor->power = 0.0f;
   attractor->phase = phase;
   attractor->frequency = frequency;
   attractor->amplitude = amplitude;
   attractor->radius = ATTRACTORRADIUS;
}

void addBoid( ESContext * esContext, int index, float x, float y, int attractor ){
   UserData *userData = (UserData *) esContext->userData;
   Settings& settings = userData->settings;
   

   Boid * boid = &( userData->boids[ index ] );
   boid->x = x;
   boid->y = y;
   boid->x_ = x - 0.01f;
   boid->y_ = y;

   boid->attractor = & userData->attractors[ attractor % settings.graphics_num_attractors ];
   setBoidColor( boid );
}

void setBoidColor( Boid * boid ){

  float r = 1.0, g = 1.0, b = 1.0;
  if( boid->attractor ){  
    float so = SATURATIONVARIATION * ( ( ( random() % 32767 ) / 32767.0 ) - 0.5 ) * 2.0;
    float s =  MIN( MAX( 0.0, boid->attractor->s + so ), 1.0 );
    float lo = LUMAVARIATION * ( ( ( random() % 32767 ) / 32767.0 ) - 0.5 ) * 2.0;
    float l =  MIN( MAX( 0.0, 0.9 + lo ), 1.0 );
    hsl2rgb( boid->attractor->h, s, l, &r, &g, &b );
  }

  boid->r = r;
  boid->g = g;
  boid->b = b;
}

void updateBoids( ESContext * esContext ){
  UserData *userData = (UserData *) esContext->userData;
  Settings& settings = userData->settings;
  
  int i, j;

  float hyp, len;

  float left = 0.0f;
  float right = settings.graphics_window_width;
  float bottom = 0.0f;
  float top = settings.graphics_window_height;

/*  float left = -1.0f;
  float right = 1.0f;
  float bottom = -1.0f;
  float top = 1.0f;
*/
  for( i = 0; i < settings.graphics_num_attractors; i++ ){
    userData->counter[ i ] = 0;
    userData->centerX[ i ] = 0.0;
    userData->centerY[ i ] = 0.0;
  }
  for( i = 0; i < settings.graphics_num_boids; i++ ){
    Boid * boid = &( userData->boids[ i ] );
    if( boid->attractor ){  
      userData->centerX[ boid->attractor->index ] += boid->x;
      userData->centerY[ boid->attractor->index ] += boid->y;
      userData->counter[ boid->attractor->index ]++;
    }
  }
  for( i = 0; i < settings.graphics_num_attractors; i++ ){
    if( userData->counter[ i ] ){
      userData->centerX[ i ] /= (float)userData->counter[ i ];
      userData->centerY[ i ] /= (float)userData->counter[ i ];
    }
  }

  for( i = 0; i < settings.graphics_num_boids; i++ ){
    Boid * boid = &( userData->boids[ i ] );

    if( DOCOLLISION ){
      for( j = i + 1; j < settings.graphics_num_boids; j++ ){
        Boid * boid2 = &( userData->boids[ j ] );
        if( boid->attractor->index != boid2->attractor->index ){
          float dx = boid2->x - boid->x;
          float dy = boid2->y - boid->y;
          float hyp = dx * dx + dy * dy + 0.00001;
          float len = sqrtf( hyp );
          if( len < COLLISIONRADIUS ){
            dx /= len;
            dy /= len;
            float d = COLLISIONRADIUS - len;
            boid->x -= dx * d;
            boid->y -= dy * d;
            boid2->x += dx * d;
            boid2->y += dy * d;
          }
        }
      }
    }

    if( boid->attractor ){  
      float ax = boid->attractor->x - boid->x;
      float ay = boid->attractor->y - boid->y;
      hyp = ax * ax + ay * ay + 0.00001;
      len = sqrtf( hyp );

      for( j = 0; j < settings.graphics_num_attractors; j++ ){
        if( j != boid->attractor->index && userData->attractors[ j ].enabled ){
          float bx = boid->x - userData->attractors[ j ].x;
          float by = boid->y - userData->attractors[ j ].y;

          float hyp = bx * bx + by * by + 0.00001;
          if( sqrtf( hyp ) < len && random() % 100 == 0 ){
            boid->attractor = &( userData->attractors[ j ] );
            setBoidColor( boid );
          }
        }
      }
      ax = boid->attractor->x - boid->x;
      ay = boid->attractor->y - boid->y;
      hyp = ax * ax + ay * ay + 0.00001;
      len = sqrtf( hyp );

      ax /= len;
      ay /= len;

      ax = ( boid->attractor->x - ax * boid->attractor->radius );
      ay = ( boid->attractor->y - ay * boid->attractor->radius );

      float attraction = ATTRACTORGRAV * boid->attractor->power;
      boid->x = ( 1.0 - attraction ) * boid->x + attraction * ax;
      boid->y = ( 1.0 - attraction ) * boid->y + attraction * ay;

      boid->x = ( 1.0 - CENTERGRAV ) * boid->x + CENTERGRAV * userData->centerX[ boid->attractor->index ];
      boid->y = ( 1.0 - CENTERGRAV ) * boid->y + CENTERGRAV * userData->centerY[ boid->attractor->index ];
    }
    float dx = boid->x - boid->x_;
    float dy = boid->y - boid->y_;
    boid->x_ = boid->x;
    boid->y_ = boid->y;

    hyp = dx * dx + dy * dy + 0.00001;
    len = sqrtf( hyp );

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

    dx += RANDOMVAR * ( ( random() % 32767 ) / 32767.0f - 0.5f );
    dy += RANDOMVAR * ( ( random() % 32767 ) / 32767.0f - 0.5f );

    hyp = dx * dx + dy * dy + 0.00001;
    len = sqrtf( hyp );
    dx /= len;
    dy /= len;

    boid->x += dx * SPEED;
    boid->y += dy * SPEED;

  }
}

void updateAttractors( ESContext * esContext ){
   UserData *userData = (UserData *) esContext->userData;
   Settings& settings = userData->settings;
   
   int i;

  float left = 0.0f;
  float right = settings.graphics_window_width;
  float bottom = 0.0f;
  float top = settings.graphics_window_height;
/*
  float left = -1.0f;
  float right = 1.0f;
  float bottom = -1.0f;
  float top = 1.0f;
*/
   float lerp = 0.98;

   for( i = 0; i < settings.graphics_num_attractors; i++ ){
      Attractor * attractor = &( userData->attractors[ i ] );

      attractor->x = lerp * attractor->x + ( 1.0 - lerp ) * attractor->x_;
      attractor->y = lerp * attractor->y + ( 1.0 - lerp ) * attractor->y_;

      attractor->power = 1.0;//attractor->amplitude * ( 0.5f + 0.5f * sin( attractor->phase ) );
//      attractor->phase += attractor->frequency;

      attractor->x += 2.0f * ( ( random() % 32767 ) / 32767.0f - 0.5f ) * 2.0f;
      attractor->y += 2.0f * ( ( random() % 32767 ) / 32767.0f - 0.5f ) * 2.0f;
      if( attractor->x < left ){
        attractor->x = left;
      }
      if( attractor->y < bottom ){
        attractor->y = bottom;
      }
      if( attractor->x > right ){
        attractor->x = right;
      }
      if( attractor->y > top ){
        attractor->y = top;
      }
   }
}

void setAttractorPosition( ESContext * esContext, int which, float x, float y ){
   UserData *userData = (UserData *) esContext->userData;
   Settings& settings = userData->settings;

   Attractor * attractor = &( userData->attractors[ which % settings.graphics_num_attractors ] );
   attractor->x_ = x;
   attractor->y_ = y;
}

void drawBoids( ESContext * esContext ){
   UserData *userData = (UserData *) esContext->userData;
   Settings& settings = userData->settings;
   
   int i, j;

  ESMatrix projection;
  esMatrixLoadIdentity( & projection );

  esOrtho( &projection, 0.0, settings.graphics_window_width, 0.0, settings.graphics_window_height, -1.0f, 1.0f );
  for( i = 0; i < settings.graphics_num_boids; i++ ){
    Boid * boid = &( userData->boids[ i ] );

    userData->boidVertices[ i * 6 + 0 ] = boid->x;
    userData->boidVertices[ i * 6 + 1 ] = boid->y;
    userData->boidVertices[ i * 6 + 2 ] = boid->r;
    userData->boidVertices[ i * 6 + 3 ] = boid->g;
    userData->boidVertices[ i * 6 + 4 ] = boid->b;
    userData->boidVertices[ i * 6 + 5 ] = 1.0;
  }

  glUniformMatrix4fv( userData->passThroughProjectionLoc, 1, 0, ( const GLfloat * )( projection.m ) );

  glVertexAttribPointer ( userData->passThroughPositionLoc, 2, GL_FLOAT, 
                          GL_FALSE, 6 * sizeof( GLfloat ), userData->boidVertices );
  glVertexAttribPointer ( userData->passThroughColorLoc, 4, GL_FLOAT,
                          GL_FALSE, 6 * sizeof( GLfloat ), &( userData->boidVertices[ 2 ] ) );
  glEnableVertexAttribArray ( userData->passThroughPositionLoc );
  glEnableVertexAttribArray ( userData->passThroughColorLoc );
  glDrawArrays( GL_POINTS, 0, settings.graphics_num_boids );
}

void drawAttractors( ESContext * esContext ){
   UserData *userData = (UserData *) esContext->userData;
   Settings& settings = userData->settings;
   
   int i, j;
   float size = 0.1f;

   GLfloat * segments = ( GLfloat * )malloc( ( 2 + 4 ) * 4 * settings.graphics_num_attractors * sizeof( GLfloat ) );
   for( i = 0; i < settings.graphics_num_attractors; i++ ){
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
   glDrawArrays( GL_LINES, 0, settings.graphics_num_attractors * 4 );
   free( segments );
}

void setAttractors ( ESContext * esContext)
{
	UserData *userData = (UserData *) esContext->userData;
    Settings& settings = userData->settings;
    
	bool state[settings.graphics_num_attractors];

	for (int i=0; i<settings.graphics_num_attractors; i++) {
		state[i] = false;
	}

	if (settings.graphics_simulate_tracking) {
		if( random() % 100 == 0 ){
			setAttractorPosition( esContext, random(), settings.graphics_window_width * ( ( random() % 32767 ) / 32767.0f ), settings.graphics_window_height * ( ( random() % 32767 ) / 32767.0f ) );
		}
    } else {
		boost::unique_lock<boost::mutex> scoped_lock(track_mutex);
		for (Tracks::const_iterator it=TrackPositions.begin(); it != TrackPositions.end(); ++it) {
			// std::cout << "Track : " << it->id << " " << it->x << " " << it->y << std::endl;
			float x = settings.graphics_window_width * it->x + settings.graphics_window_width / 2.0;
			float y = settings.graphics_window_height * it->y + settings.graphics_window_height  / 2.0;
			setAttractorPosition( esContext, it->id, x, y );
			state[it->id % settings.graphics_num_attractors] = true;
			enableAttractor( esContext, it->id );
		}
		for (int i=0; i<settings.graphics_num_attractors; i++) {
			if (state[i] == false) {
				disableAttractor( esContext, i );
			}
		}
    }
}

void update ( ESContext *esContext )
{
  static int iter = 0;
  int i;
  iter++;
  UserData *userData = (UserData *) esContext->userData;
  Settings& settings = userData->settings;
  

  setAttractors(esContext);

  static ESMatrix projection;
  esMatrixLoadIdentity( & projection );
  esOrtho( &projection, 0.0, settings.graphics_window_width, 0.0, settings.graphics_window_height, -1.0f, 1.0f );

  GLfloat vBounceVertices[] = { 0.0f,  0.0f, 0.0f, 
                                0.0f,  0.0f,       
                                0.0f,  settings.graphics_window_height, 0.0f, 
                                0.0f,  (float)settings.graphics_window_height / (float)TEXTURE_HEIGHT,
                                settings.graphics_window_width, settings.graphics_window_height, 0.0f, 
                                (float)settings.graphics_window_width / (float)TEXTURE_WIDTH,  (float)settings.graphics_window_height / (float)TEXTURE_HEIGHT,       
                                settings.graphics_window_width,  0.0f, 0.0f, 
                                (float)settings.graphics_window_width / (float)TEXTURE_WIDTH,  0.0f       
                         };

  float xoff = -1.0f / TEXTURE_WIDTH;                           
  float yoff = -1.0f / TEXTURE_HEIGHT;                            
  GLfloat vBounceVertices2[] = { 0.0f,  0.0f, 0.0f, 
                                -xoff,  -yoff,       
                                0.0f,  settings.graphics_window_height, 0.0f, 
                                -xoff,  (float)settings.graphics_window_height / (float)TEXTURE_HEIGHT + yoff,       
                                settings.graphics_window_width, settings.graphics_window_height, 0.0f, 
                                (float)settings.graphics_window_width / (float)TEXTURE_WIDTH + xoff,  (float)settings.graphics_window_height / (float)TEXTURE_HEIGHT + yoff,       
                                settings.graphics_window_width,  0.0f, 0.0f, 
                                (float)settings.graphics_window_width / (float)TEXTURE_WIDTH + xoff,  -yoff       
                         };
  GLushort bounceIndices[] = { 0, 2, 1, 0, 3, 2 };

  // Set the viewport
  glViewport ( 0, 0, esContext->width, esContext->height );

  // bind the base map
  glBindFramebuffer( GL_FRAMEBUFFER, userData->baseFramebuffer );

  // draw the bounce map into the base map
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, userData->bounceMapTexId );
  glUseProgram( userData->bounceProgram );

  assertNoError( esContext );

  glUniformMatrix4fv( userData->bounceProjectionLoc, 1, 0, ( const GLfloat * )( projection.m ) );

  assertNoError( esContext );

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

  GLfloat fade0 = 0.0f;
  GLfloat fade1_255 = 1.0f / 255.0f;
  
  if( iter % 10 == 0 ){
    glUniform1fv( userData->bounceFadeLoc, 1, & fade1_255 );
  }
  else{
    glUniform1fv( userData->bounceFadeLoc, 1, & fade0 );
  }

  GLfloat width = (GLfloat)settings.graphics_window_width;
  GLfloat height = (GLfloat)settings.graphics_window_height;
  glUniform1fv( userData->bounceWidthLoc, 1, & width );
  glUniform1fv( userData->bounceHeightLoc, 1, & height );
  glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, bounceIndices );

  assertNoError( esContext );

  // draw the boids (into the base map)
  glUseProgram ( userData->passThroughProgram );

  updateBoids( esContext );
  updateAttractors( esContext );
  drawBoids( esContext );
  if (settings.graphics_draw_attractors) drawAttractors( esContext );

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
}

void shutDown ( ESContext *esContext )
{
   UserData *userData = (UserData *) esContext->userData;
   Settings& settings = userData->settings;
   

   // Delete texture object
   glDeleteTextures ( 1, &userData->baseMapTexId );
   glDeleteTextures ( 1, &userData->bounceMapTexId );

   // Delete program object
   glDeleteProgram ( userData->bounceProgram );
   glDeleteProgram ( userData->passThroughProgram );
}

void graphics(Settings& settings)
{
   ESContext esContext;
   UserData  userData;
   userData.settings = settings;

   esInitContext ( &esContext );
   esContext.userData = &userData;

   if (settings.graphics_full_screen) {
	   esCreateWindow( &esContext, "TekenBeesten", settings.graphics_window_width, settings.graphics_window_height, ES_WINDOW_RGB | ES_WINDOW_FULLSCREEN);
   } else {
	   esCreateWindow( &esContext, "TekenBeesten", settings.graphics_window_width, settings.graphics_window_height, ES_WINDOW_RGB );
   }
   
   if ( !init ( &esContext ) ){
      std::cout << "Unable to initialize graphics" << std::endl;
      return ;
   }

   esRegisterDrawFunc ( &esContext, update );
   
   esMainLoop ( &esContext );

   shutDown ( &esContext );
}

void tracking(Settings& settings)
{
	Tracks tracks;
    bool show_camera = settings.properties.get<bool>("tracking.show_camera");
	initCamera(settings, show_camera);
	while (1) {
		tracks = processCamera(settings);
		// if (tracks.size()) std::cout << "Track count : " << tracks.size() << std::endl;
		{
			boost::unique_lock<boost::mutex> scoped_lock(track_mutex);
			TrackPositions = tracks;
		}
	}	
}

int main ( int argc, char *argv[] )
{
    Settings settings;
    try {
        settings.load("tekenbeesten.json");
        std::cout << "Success\n";
    } catch (std::exception &e) {
        std::cout << "Error: " << e.what() << "\n";
    }
   
    boost::thread graphicsThread(graphics, settings);
    boost::thread camThread(tracking, settings);

    graphicsThread.join();
    camThread.join();
    return 0;
}
