#include <parameter.h>


// パラメータ
int winWidth = WINDOW_WIDTH;
int winHeight = WINDOW_HEIGHT;
double zoom = 1.0;
double fov; // 視野角(縦方向)
double f = 1.0;
double baseLine; // 基線長
double eye[CAM_NUM][3]; // カメラの視点
double focalDepth[CAM_NUM]; // カメラの焦点奥行き
int aperturePattern[CAM_NUM]; // カメラの開口形状
double apertureSize; // カメラの開口径
double DTPparam[CAM_NUM][2]; // PSFSize <--> 視差 の変換パラメータ


double tmf; // = tan ( FOVの最小値 ) : zoomが変化しても一定値をとるので

//init
void initParam(void)
{
  zoom = 1.0;
  tmf = tan(MIN_FOV*M_PI/180.0);

  double zMin, zMax;
  IplImage *zBuffer = readDepthBuffer();
  if(zBuffer != NULL ){
    cvConvertScale( zBuffer, zBuffer, -1.0, 0.0 );
    cvMinMaxLoc( zBuffer, &zMin, &zMax, NULL, NULL, NULL);
    apertureSize 
      = 2.0 * MAX_PSF_RADIUS * zMin * zMax * tmf / ( (zMax - zMin) * (double)winWidth * zoom);
  }

  baseLine = MAX_DISPARITY * 2.0 * tmf / (zoom * (double)winWidth );

  // 最も近い所にピントを合わせる
  focalDepth[LEFT_CAM] = zMin;
  focalDepth[RIGHT_CAM] = zMin;

  eye[CENTER_CAM][0] = 0.0;  eye[CENTER_CAM][1] = 0.0;  eye[CENTER_CAM][2] = 0.0;
  eye[LEFT_CAM][0] = -baseLine/2.0;   eye[LEFT_CAM][1] = 0.0;  eye[LEFT_CAM][2] = 0.0;
  eye[RIGHT_CAM][0] = baseLine/2.0;   eye[RIGHT_CAM][1] = 0.0;  eye[RIGHT_CAM][2] = 0.0;

  aperturePattern[LEFT_CAM] = INIT_APERTURE;
  aperturePattern[RIGHT_CAM] = INIT_APERTURE;
  
}



//setter
void setZoom(double val)
{ 
  double prevZoom = zoom;
  zoom = val;
  baseLine *= (prevZoom / zoom );
  apertureSize *= (prevZoom / zoom );
  eye[LEFT_CAM][0] = -baseLine / 2.0;
  eye[RIGHT_CAM][0] = baseLine / 2.0;
}

void setWindowSize( int width, int height)
{ 
  int prevWinWidth = winWidth;
  winWidth = width;  
  winHeight = height;
  baseLine *= ( (double)prevWinWidth / (double)winWidth );
  apertureSize *= ( (double)prevWinWidth / (double)winWidth );
}

void setEye( double val[3], int cam)
{  eye[cam][0] = val[0];  eye[cam][1] = val[1];  eye[cam][2] = val[2];}

void setFocalDepth( double val, int cam)
{  focalDepth[cam] = val;}

void setAperturePattern(int val, int cam)
{  aperturePattern[cam] = val;}

void setApertureSize( double val )
{ apertureSize = val;}

void setBaseLine( double val)
{ baseLine = val;
  eye[LEFT_CAM][0] = -baseLine / 2.0;
  eye[RIGHT_CAM][0] = baseLine / 2.0;
}




//getter
double getFov(void)
{
  return fov = 2.0 * atan( tmf / zoom ) * 180.0 / M_PI;
}

double getBaseLine(void)
{  return baseLine; }

void getEye( int cam , double dst[])
{
  dst[0] = eye[cam][0];  dst[1] = eye[cam][1];  dst[2] = eye[cam][2];
}

double getFocalDepth( int cam )
{  return focalDepth[cam]; }


int getAperturePattern( int cam )
{  return aperturePattern[cam]; }

double getApertureSize(void)
{  return apertureSize; }

void getDTPParam( int cam , double dst[])
{
  dst[0] = DTPparam[cam][0] = apertureSize / baseLine;
  dst[1] = DTPparam[cam][1] 
    = -(double)winWidth * apertureSize / (2.0 * focalDepth[cam] * tmf / zoom );
}

int getWindowWidth(void)
{  return winWidth; }

int getWindowHeight(void)
{  return winHeight; }

double getFLength(void)
{  return f; }

int getMaxDisparity(void)
{  return baseLine * zoom * (double)winWidth / ( 2.0 * tmf ); }

int getMaxPSFSize(void)
{
  double maxDisparity;
  double minDisparity;
  IplImage *zBuffer = readDepthBuffer();
  if(zBuffer != NULL ){
    double near, far;
    cvConvertScale( zBuffer, zBuffer, -1.0, 0.0 );
    cvMinMaxLoc( zBuffer, &near, &far, NULL, NULL, NULL);
    maxDisparity = disparityFromDepth(near);
    minDisparity = disparityFromDepth(far);
    cvReleaseImage(&zBuffer);
  }  


  double size[4];
  size[0] = fabs( DTPparam[LEFT_CAM][0] * maxDisparity + DTPparam[LEFT_CAM][1] );
  size[1] = fabs( DTPparam[LEFT_CAM][0] * minDisparity + DTPparam[LEFT_CAM][1] );
  size[2] = fabs( DTPparam[RIGHT_CAM][0] * maxDisparity + DTPparam[RIGHT_CAM][1] );
  size[3] = fabs( DTPparam[RIGHT_CAM][0] * minDisparity + DTPparam[RIGHT_CAM][1] );

  double max = 0.0;
  for(int i = 0; i < 4; ++i){
    if( max < size[i]) max = size[i];
  }


  return max;

}


// convert
double disparityFromDepth( double depth )
{  return (double)winWidth*baseLine/(2.0*depth*tmf/zoom); }

