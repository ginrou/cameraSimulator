#include <camera.h>

IplImage* aperture;
double param[2]; // psfsize = param[0] * disparity + param[1]


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
  cvConvertScale( glDepth, depth, -64.0, -1.0);
  
  cvSaveImage( filename, depth);
  
  cvReleaseImage(&glDepth);
  cvReleaseImage(&depth);

  return;
}


void setAperture( char filename[] )
{
  if(!aperture) cvReleaseImage(&aperture);
  aperture = cvLoadImage(filename, CV_LOAD_IMAGE_GRAYSCALE);
  printf("%s is loaded as aperture\n", filename);
  return;
}

void setDispSizeParam( double a, double b)
{
  param[0] = a;
  param[1] = b;
  printf("parameter = %lf, %lf\n", param[0], param[1]);
}

void blur(char filename[])
{
  // disparity = p[0] / depth + p[1];
  double disparity2PSFSize[2];
  getdepth2PSFSize( disparity2PSFSize );
  
  IplImage* screan = readPixel();
  IplImage* depthMap = readDepthBuffer();
  cvConvertScale( screan, screan, 255.0, 0.0);
  cvFlip( screan, NULL, 0);
  cvFlip( depthMap, NULL, 0);


  //psf
  int minDisparity = 0.0;
  int maxDisparity = MAX_DISPARITY;
  IplImage* psf[(int)(MAX_DISPARITY+1)];
  //makePSF( psf, minDisparity, maxDisparity);
  
  makeShiftPSF( psf, minDisparity, maxDisparity);

  //dst image
  IplImage* dst = cvCreateImage( cvGetSize( screan ), IPL_DEPTH_64F, screan->nChannels);
  cvSetZero(dst);
  
  //blurring
  for(int h = 0; h < dst->height; ++h){
    for( int w = 0; w < dst->width ; ++w){
      
      float depth = CV_IMAGE_ELEM( depthMap, float, h, w) ;
      int disparity = -disparity2PSFSize[0] / depth + disparity2PSFSize[1];

      CvSize sz = cvGetSize(psf[disparity]);

      for( int y = 0 ; y < sz.height; ++y){
	for( int x = 0; x < sz.width; ++x){

	  CvPoint pt = cvPoint( x + w - sz.width/2, y + h - sz.height/2 );
	  
	  if( pt.x < 0 || pt.x >= dst->width ||
	      pt.y < 0 || pt.y >= dst->height)
	    continue;
	  
	  for(int c = 0 ; c < dst->nChannels; ++c){
	    CV_IMAGE_ELEM( dst, double, h, w*dst->nChannels + c) 
	      += CV_IMAGE_ELEM( screan, float, pt.y, pt.x * screan->nChannels +c)
	      * CV_IMAGE_ELEM( psf[disparity], double, y, x);
	  }
	}
      }

      
    }
  }



  IplImage* ret = cvCreateImage( cvGetSize(dst), IPL_DEPTH_8U, dst->nChannels);
  cvConvert( dst, ret);
  cvSaveImage( filename, ret);


  for(int i = 0; i < 16; ++i)
    cvReleaseImage( &(psf[i]) );
  cvReleaseImage(&screan);
  cvReleaseImage(&depthMap);
  cvReleaseImage(&dst);
  cvReleaseImage(&ret);
  return;
}

void blur2(char saveFileName[])
{
  int x,y;
  double apertureSize = getApertureSize();
  IplImage *tmp;
  IplImage *img = cvCreateImage( cvSize(1024,1024), IPL_DEPTH_32F, 3);
  cvSetZero(img);
  int PSFWidth = 32;
  int PSFHeight = 32;
  for( x = 0; x < PSFWidth; ++x ){
    for( y=0 ; y<PSFHeight; ++y){


      tmp = readPixelAtEye( apertureSize*(double)x/(double)PSFWidth,
			    apertureSize*(double)y/(double)PSFHeight);
      cvConvertScale( tmp, tmp, 255.0, 0.0);

#ifdef __DEBUG__
      char filename[256];
      sprintf(filename, "images/sample%d%d.png", x, y);
      cvSaveImage( filename, tmp );
#endif

      cvConvertScale( tmp, tmp, 1.0/(double)(PSFHeight*PSFWidth), 0.0);

      for(int h = 0 ; h < tmp->height; ++h){
	for( int w = 0 ; w < tmp->width; ++w){
	  for( int c = 0 ; c < 3 ; ++c){
	  CV_IMAGE_ELEM( img,float,h,w*3+c) += CV_IMAGE_ELEM(tmp,float,h,w*3+c);
	  }
	}
      }

      cvReleaseImage(&tmp);
    }
  }

  cvFlip( img, NULL, 0);
  cvSaveImage( "images/blurred.png", img );

  return;
}
