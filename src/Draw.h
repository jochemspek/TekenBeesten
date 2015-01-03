#include "TekenBeesten.h"

void  rgb2hsl( float r, float g, float b, float *h, float *s, float *l );
void  hsl2rgb( float h, float s, float l, float *r, float *g, float *b );
void  drawBoids( ESContext * esContext );
int   init ( ESContext *esContext );
void  assertNoError( ESContext *esContext );
void  update ( ESContext *esContext );
void  shutDown ( ESContext *esContext );
