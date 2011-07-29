#ifndef __PARAMETER__
#define __PARAMETER__

#include <stdio.h>
#include <settings.h>
#include <viewer.h>
#include <camera.h>

#include <cv.h>
#include <highgui.h>

#define CAM_NUM 3
#define LEFT_CAM 0
#define CENTER_CAM 1
#define RIGHT_CAM 2

//init
void initParam(void);

//setter
void setZoom(double val);
void setWindowSize( int width, int height);
void setEye( double val[3], int cam);
void setFocalDepth( double val, int cam);
void setAperturePattern(int val, int cam);

//getter
double getFov(void);
double getBaseLine(void);
void   getEye( int cam , double dst[]);
double getFocalDepth( int cam );
int    getAperturePattern( int cam );
double getApertureSize(void);
void   getDTPParam( int cam , double dst[]);
int    getWindowWidth(void);
int    getWindowHeight(void);
double getFLength(void);
//convert
double disparityFromDepth( double depth );


#endif
