//
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com
//

// ESUtil.c
//
//    A utility library for OpenGL ES.  This library provides a
//    basic common framework for the example applications in the
//    OpenGL ES 2.0 Programming Guide.
//

///
//  Includes
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include "esUtil.h"


//////////////////////////////////////////////////////////////////
//
//  Public Functions
//
//

///
//  esInitContext()
//
//      Initialize ES utility context.  This must be called before calling any other
//      functions.
//
void ESUTIL_API esInitContext ( ESContext *esContext )
{
   if ( esContext != NULL )
   {
      memset( esContext, 0, sizeof( ESContext) );
   }
}


///
//  esCreateWindow()
//
//      title - name for title bar of window
//      width - width of window to create
//      height - height of window to create
//      flags  - bitwise or of window creation flags 
//          ES_WINDOW_ALPHA       - specifies that the framebuffer should have alpha
//          ES_WINDOW_DEPTH       - specifies that a depth buffer should be created
//          ES_WINDOW_STENCIL     - specifies that a stencil buffer should be created
//          ES_WINDOW_MULTISAMPLE - specifies that a multi-sample buffer should be created
//
GLboolean ESUTIL_API esCreateWindow ( ESContext *esContext, const char* title, GLint width, GLint height, GLuint flags )
{
    int argc = 1;
    char * argv[] = { "TekenBeesten", 0 } ;
    
    esContext->width = width;
    esContext->height = height;
    
    glutInit(&argc, argv);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(40,40);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);

    glutCreateWindow(title);

    if( flags & ES_WINDOW_FULLSCREEN ){
        glutFullScreen();
    }
    return GL_TRUE;
}

///
//  esMainLoop()
//
//    Start the main loop for the OpenGL ES application
//

static ESContext * _context = NULL;
static struct timeval t1, t2;
static struct timezone tz;
static float deltatime;
static float totaltime = 0.0f;
static unsigned int frames = 0;

void display()
{
    if (_context == NULL) return;

    gettimeofday(&t2, &tz);
    deltatime = (float)(t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6);
    t1 = t2;
            
    if (_context->updateFunc != NULL)
        _context->updateFunc(_context, deltatime);
    if (_context->drawFunc != NULL)
        _context->drawFunc(_context);
    
    totaltime += deltatime;
    frames++;
    if (totaltime >  2.0f)
    {
        printf("%4d frames rendered in %1.4f seconds -> FPS=%3.4f\n", frames, totaltime, frames/totaltime);
        totaltime -= 2.0f;
        frames = 0;
    }
    glutSwapBuffers();
    
}

void
reshape(int w, int h)
{
}

void mouse()
{
    
}

void motion()
{
    
}

void
kbd(unsigned char key, int x, int y)
{
    switch((char)key) {
        case 'q':
        case 27:    /* ESC */
            exit(0);
        default:
            break;
    }

    return;
}

void ESUTIL_API esMainLoop ( ESContext *esContext )
{
    gettimeofday ( &t1 , &tz );
    
    _context = esContext;
    glutDisplayFunc (display);
    glutIdleFunc(display);
    glutReshapeFunc(reshape); 
    glutMouseFunc(mouse); 
    glutMotionFunc(motion);     
    glutKeyboardFunc(kbd);
    glutMainLoop ();
}


///
//  esRegisterDrawFunc()
//
void ESUTIL_API esRegisterDrawFunc ( ESContext *esContext, void (ESCALLBACK *drawFunc) (ESContext* ) )
{
   esContext->drawFunc = drawFunc;
}


///
//  esRegisterUpdateFunc()
//
void ESUTIL_API esRegisterUpdateFunc ( ESContext *esContext, void (ESCALLBACK *updateFunc) ( ESContext*, float ) )
{
   esContext->updateFunc = updateFunc;
}


///
//  esRegisterKeyFunc()
//
void ESUTIL_API esRegisterKeyFunc ( ESContext *esContext,
                                    void (ESCALLBACK *keyFunc) (ESContext*, unsigned char, int, int ) )
{
   esContext->keyFunc = keyFunc;
}


///
// esLogMessage()
//
//    Log an error message to the debug output for the platform
//
void ESUTIL_API esLogMessage ( const char *formatStr, ... )
{
    va_list params;
    char buf[BUFSIZ];

    va_start ( params, formatStr );
    vsprintf ( buf, formatStr, params );
    
    printf ( "%s", buf );
    
    va_end ( params );
}


///
// esLoadTGA()
//
//    Loads a 24-bit TGA image from a file. This is probably the simplest TGA loader ever.
//    Does not support loading of compressed TGAs nor TGAa with alpha channel. But for the
//    sake of the examples, this is sufficient.
//

char* ESUTIL_API esLoadTGA ( char *fileName, int *width, int *height )
{
    char *buffer = NULL;
    FILE *f;
    unsigned char tgaheader[12];
    unsigned char attributes[6];
    unsigned int imagesize;

    f = fopen(fileName, "rb");
    if(f == NULL) return NULL;

    if(fread(&tgaheader, sizeof(tgaheader), 1, f) == 0)
    {
        fclose(f);
        return NULL;
    }

    if(fread(attributes, sizeof(attributes), 1, f) == 0)
    {
        fclose(f);
        return 0;
    }

    *width = attributes[1] * 256 + attributes[0];
    *height = attributes[3] * 256 + attributes[2];
    imagesize = attributes[4] / 8 * *width * *height;
    buffer = malloc(imagesize);
    if (buffer == NULL)
    {
        fclose(f);
        return 0;
    }

    if(fread(buffer, 1, imagesize, f) != imagesize)
    {
        free(buffer);
        return NULL;
    }
    fclose(f);
    return buffer;
}
