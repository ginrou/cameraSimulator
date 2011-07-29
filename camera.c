#include <camera.h>

IplImage* aperture = NULL;
double param[2]; // psfsize = param[0] * disparity + param[1]

void setAperture( char filename[] );
char apertureFilePath[3][64] 
= { CIRCLE_PATH,
    ZHOU_PATH,
    MLS_PATH};

/*****************************************
  プライベート関数ここから
 *****************************************/
void makePSF( IplImage* psf[], int minDisparity, int maxDisparity);
void makeShiftPSF( IplImage* psf[], int minDisparity, int maxDisparity);

void makePSF( IplImage* psf[], int minDisparity, int maxDisparity)
{
  IplImage* apertureIn = cvCreateImage( cvGetSize(aperture), IPL_DEPTH_64F, 1);
  cvConvert( aperture, apertureIn );
  for( int disp = minDisparity; disp <= maxDisparity; ++disp)
    {
      double rad = param[0] * (double)disp + param[1];
      CvSize sz;
      if( fabs(rad) < 1.0 )
	sz = cvSize( 1, 1);
      else
	sz = cvSize( fabs(rad), fabs(rad) );

      psf[disp] = cvCreateImage( sz, IPL_DEPTH_64F, 1);
      cvResize( apertureIn, psf[disp], CV_INTER_AREA);
      
      cvNormalize( psf[disp], psf[disp], 1.0, 0.0, CV_L1, NULL);

      if(rad<0)
	cvFlip( psf[disp], NULL, 0);
      
    }

  cvReleaseImage( &apertureIn);

  return;
}


void makeShiftPSF( IplImage* psf[], int minDisparity, int maxDisparity)
{
  CvSize size = cvSize( maxDisparity+1, 1 );
  for( int i = minDisparity; i <= maxDisparity; ++i){
    psf[i] = cvCreateImage( size, IPL_DEPTH_64F, 1);
    cvSetZero( psf[i] );

    CV_IMAGE_ELEM( psf[i], double, size.height/2, i) = 1.0;

  }

}



/*****************************************
 *プライベート関数ここまで
 *パブリック関数ここから
 *****************************************/

void saveImage(char filename[])
{
  IplImage* glBuffer = readPixel();

  IplImage* img = cvCreateImage( cvGetSize(glBuffer), IPL_DEPTH_8U, 3);
  cvConvertScale( glBuffer, img, 255.0, 0);
  
  cvFlip( img, img, 0);
  cvSaveImage( filename, img );
  
  cvReleaseImage( &glBuffer );
  cvReleaseImage( &img );
  
  return;
  
}

void saveDepthMap(char filename[] )
{
  IplImage* glDepth = readDepthBuffer();

  IplImage* depth = cvCreateImage( cvGetSize(glDepth), IPL_DEPTH_8U, 1);
  cvConvertScale( glDepth, depth, -32.0, -1.0);
  cvFlip( depth, NULL, 0);
  cvSaveImage( filename, depth);
  
  cvReleaseImage(&glDepth);
  cvReleaseImage(&depth);

  return;
}

void saveDispMap( char filename[] )
{
  IplImage* glDepth = readDepthBuffer();
  IplImage* dst = cvCreateImage( cvGetSize(glDepth), IPL_DEPTH_8U, 1);
  cvSetZero(dst);


  for(int h = 0; h < dst->height; ++h){
    for( int w = 0 ; w < dst->width; ++w ){
      double depth = -CV_IMAGE_ELEM( glDepth, float, h, w);
      double disp;
      if( depth > 127 ){
	disp = 256/4.0;
      }else{
	disp = disparityFromDepth(depth);
      }

      CV_IMAGE_ELEM( dst, uchar, h, w) = disp * 4.0;

    }
  }
  
  cvFlip(dst, NULL, 0);
  cvSaveImage(filename, dst);
  cvReleaseImage(&glDepth);
  cvReleaseImage(&dst);
}


void setAperture( int PSF )
{

  if(!aperture) cvReleaseImage(&aperture);
  IplImage *load = cvLoadImage(apertureFilePath[PSF], CV_LOAD_IMAGE_GRAYSCALE);
  IplImage *tmp  = cvCreateImage( cvGetSize(load), IPL_DEPTH_64F, 1);
  aperture = cvCreateImage( cvSize( MAX_PSF_RADIUS, MAX_PSF_RADIUS), IPL_DEPTH_64F, 1);

  cvConvert( load, tmp);
  cvResize( tmp, aperture);
  cvNormalize( aperture, aperture, 1.0, 0.0, CV_L1, NULL);
  printf("%s is loaded as aperture\n", apertureFilePath[PSF]);

  cvReleaseImage( &load );
  cvReleaseImage( &tmp );

  return;
}

void setDispSizeParam( double a, double b)
{
  param[0] = a;
  param[1] = b;
  printf("parameter = %lf, %lf\n", param[0], param[1]);
}

void blur(char saveFileName[], int apertureID)
{
  
  setAperture( apertureID );
  int x,y;
  double apertureSize = getApertureSize();
  IplImage *tmp;
  IplImage *img = cvCreateImage( cvSize(512,512), IPL_DEPTH_32F, 3);
  cvSetZero(img);
  int PSFWidth = aperture->width;
  int PSFHeight = aperture->height;

  printf("blurring\n");

  for( x = 0; x < PSFWidth; ++x ){
    for( y = 0; y < PSFHeight; ++y){

      tmp = readPixelAtEye( apertureSize*(double)(x-PSFWidth/2)/(double)PSFWidth,
			    apertureSize*(double)(y-PSFHeight/2)/(double)PSFHeight);

      cvConvertScale( tmp, tmp, 255.0, 0.0);

#ifdef __DEBUG__
      cvFlip(tmp, tmp, 0);
      char filename[256];
      sprintf(filename, "images/sample%d%d.png", x, y);
      cvSaveImage( filename, tmp );
      cvFlip(tmp, tmp, 0);
#endif

      _ClearLine();
      printf("progress : %d %%",100*(x*PSFHeight+y)/(PSFWidth*PSFHeight));

      for(int h = 0 ; h < tmp->height; ++h){
	for( int w = 0 ; w < tmp->width; ++w){
	  for( int c = 0 ; c < 3 ; ++c){
	    CV_IMAGE_ELEM( img,float,h,w*3+c) += 
	      CV_IMAGE_ELEM( aperture, double, y, x)*CV_IMAGE_ELEM(tmp,float,h,w*3+c);
	  }
	}
      }
      
      cvReleaseImage(&tmp);
    }
  }

  printf("finished\n");

  cvFlip( img, NULL, 0);
  cvSaveImage( saveFileName, img );

  return;
}
