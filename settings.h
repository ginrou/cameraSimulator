#ifndef __SETTINGS__
#define __SETTINGS__

#define MAX_PSF_RADIUS 32.0
#define MAX_DISPARITY 32.0
#define Z_MAX 128.0

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512

#define MIN_FOV 40.0

//#define __DEBUG__ 1

//file path
#define OBJECT_FILE_PATH "object/blocks.obj"
#define TEXTURE_FILE_PATH "object/wood.png"

//image file path
#define DEPTH_BUFFER "depth.png"
#define SCREAN_IAMGE "center.png"
#define DISPARITY_MAP "disparityMap.png"

//Aperture Pattern Path
#define CIRCLE 0
#define CIRCLE_PATH "images/circle.png"
#define ZHOU 1
#define ZHOU_PATH "images/Zhou0002.png"
#define MLS 2
#define MLS_PATH "images/MLS.png"

#define INIT_APERTURE CIRCLE

//escape sequence
#define _ClearLine() { fputs( "\r\x1b[2K", stdout);fflush(stdout); }

#endif
