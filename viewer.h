#ifndef __VIEWER__
#define __VIEWER__

#include <stdio.h>
#include <math.h>
#include <time.h>

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


//functions
void initViewer(int argc, char* argv[]);
void initObject( char *filename );
void setTexture( char *filename );
void saveParameters( char *filename ); //if filename is NULL saved in yyddmm-ttmm.txt

//return value : IPL_DEPTH_32F, BGRA
IplImage* readPixel(void); //normalize between 0 and 1
IplImage* readDepthBuffer(void); // clipped from -128 to -1 (カメラ座標系)

IplImage* readPixelAtEye( double x, double y);

//encapsulation
void getdepth2PSFSize( double dst[2] );
double getApertureSize(void);
double getBaselineLength(void);
double getFieldOfView(void); //[degree]

#endif
