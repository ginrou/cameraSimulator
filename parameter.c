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


double tmf;

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
{ zoom = val;
  eye[LEFT_CAM][0] = -getBaseLine()/2.0;
  eye[RIGHT_CAM][0] = getBaseLine()/2.0;
}

void setWindowSize( int width, int height)
{  winWidth = width;  winHeight = height;}

void setEye( double val[3], int cam)
{  eye[cam][0] = val[0];  eye[cam][1] = val[1];  eye[cam][2] = val[2];}

void setFocalDepth( double val, int cam)
{  focalDepth[cam] = val;}

void setAperturePattern(int val, int cam)
{  aperturePattern[cam] = val;}






//getter
double getFov(void)
{
  return fov = 2.0 * atan( tmf / zoom ) * 180.0 / M_PI;
}

double getBaseLine(void)
{
  return baseLine 
    = MAX_DISPARITY * 2.0 * tmf / (zoom * (double)winWidth);
}

void getEye( int cam , double dst[])
{

  dst[0] = eye[cam][0];  dst[1] = eye[cam][1];  dst[2] = eye[cam][2];
}

double getFocalDepth( int cam )
{
  return focalDepth[cam];
}


int getAperturePattern( int cam )
{
  return aperturePattern[cam];
}

double getApertureSize(void)
{
  return apertureSize;
}

void getDTPParam( int cam , double dst[])
{
  double dInf = disparityFromDepth( Z_MAX );
  double disp = disparityFromDepth( focalDepth[cam] );
  dst[0] = DTPparam[cam][0] = (double)winWidth*baseLine/(disp - dInf);
  dst[1] = DTPparam[cam][1] = -DTPparam[cam][0] * disp ;
}

double disparityFromDepth( double depth )
{
  return (double)winWidth*baseLine/(2.0*depth*tmf/zoom);
}

int    getWindowWidth(void)
{
  return winWidth;
}

int    getWindowHeight(void)
{
  return winHeight;
}

double getFLength(void)
{
  return f;
}
