#ifndef __SETTINGS__
#define __SETTINGS__

#define MAX_PSF_RADIUS 24.0
#define MAX_DISPARITY 20.0
#define Z_MAX 128.0

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512

#define MIN_FOV 40.0

//#define __DEBUG__ 1

//file path
#define OBJECT_FILE_PATH "object/research.obj"
#define TEXTURE_NOISE_PATH "object/texture-noise.png"
#define TEXTURE_LINE_PATH  "object/texture-line.png"
#define TEXTURE_YOKO_PATH  "object/texture-line-yoko.png"

#define TEXTURE_FILE_PATH TEXTURE_YOKO_PATH

//image file path
#define DEPTH_BUFFER "img/depth.png"
#define SCREAN_IAMGE "img/center.png"
#define DISPARITY_MAP "img/disparityMap.png"
#define STEREO_LEFT  "img/stereoLeft.png"
#define STEREO_RIGHT "img/stereoRight.png"
#define BLURRED      "img/blurred.png"
#define BLURRED_LEFT "img/blurredLeft.png"
#define BLURRED_RIGHT "img/blurredRight.png"
#define IMAGE_LEFT_HEADER "img/leftImage"
#define IMAGE_RIGHT_HEADER "img/rightImage"


//Aperture Pattern Path
#define CIRCLE 0
#define CIRCLE_PATH "aperture/circle.png"
#define ZHOU 1
#define ZHOU_PATH "aperture/Zhou0002.png"
#define MLS 2
#define MLS_PATH "aperture/MLS.png"
#define CAPAIR1 3
#define CAPAIR1_PATH "aperture/CAPair1.png"
#define CAPAIR2 4
#define CAPAIR2_PATH "aperture/CAPair2.png"
extern char apertureFilePath[5][64];

#define INIT_APERTURE CIRCLE

//escape sequence
#define _ClearLine() { fputs( "\r\x1b[2K", stdout);fflush(stdout); }

#endif
