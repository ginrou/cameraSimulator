#ifndef __CAMERA__
#define __CAMERA__

//OpenGL
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>

//OpenCV
#include <cv.h>
#include <highgui.h>
#include <cxcore.h>

//private 
#include "settings.h"
#include <viewer.h>

void saveImage(char filename[]);
void saveDepthMap(char filename[] );
void setAperture( char filename[] );
void setDispSizeParam( double a, double b);
void blur(char filename[]);
void blur2(char saveFileName[]);

#endif
