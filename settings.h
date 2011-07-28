#ifndef __SETTINGS__
#define __SETTINGS__

#define MAX_PSF_RADIUS 32.0
#define MAX_DISPARITY 32.0
#define Z_MAX 128.0

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512

//#define __DEBUG__ 1

//file path
#define OBJECT_FILE_PATH "object/blocks.obj"
#define TEXTURE_FILE_PATH "object/wood.png"

//debug file path
#define DEPTH_BUFFER "depth.png"
#define SCREAN_IAMGE "screan.png"
#define DISPARITY_MAP "disparityMap.png"


//escape sequence
#define _ClearLine() { fputs( "\r\x1b[2K", stdout);fflush(stdout); }

#endif
