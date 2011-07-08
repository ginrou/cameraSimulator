#ifndef __VIEWER__
#define __VIEWER__

#include <stdio.h>
#include <math.h>

//OpenGL
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <GL/glui.h>

//OpenCV
#include <cv.h>
#include <cxcore.h>

//private files
#include "settings.h"
#include "glm.h"
#include <camera.h>


//methods
void initViewer(int argc, char* argv[]);
void initObject( char *filename );
void setTexture( char *filename );

//return value : IPL_DEPTH_32F, BGRA
IplImage* readPixel(void); //normalize between 0 and 1
IplImage* readDepthBuffer(void); // clipped from -128 to -1

IplImage* readPixelAtEye( double x, double y);

//encapsulation
void getdepth2PSFSize( double dst[2] );
double getApertureSize(void);

#endif
