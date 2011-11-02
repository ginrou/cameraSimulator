#include <viewer.h>


// モデルに関する変数 
GLuint modelList = 0; // display list for object
GLMmodel* model; //model from object file
GLuint textureData[1]; //テクスチャのデータ
GLfloat scale; //objectのスケール
GLuint materialMode = GLM_TEXTURE;
GLboolean facetNormal = GL_FALSE;

#define VIEW_PERSPECTIVE 1
#define VIEW_FRUSTUM 2
GLdouble frustum[4]; // 順に left right, bottom, top

// 現在使用中のカメラ
int cam;

//GLUIで使用しているインタフェース
GLUI *glui;
GLUI_Rotation *objRot;

float zoomView;
int listBoxVar[CAM_NUM];// apertureの番号
GLUI_EditText* tBox[CAM_NUM][2];// DTPParamの値
GLUI_EditText* fdepthBox[CAM_NUM];// 焦点位置
GLUI_EditText* baseLineBox; // 基線長
GLUI_EditText* apertureSizeBox; // 開口径
GLUI_StaticText* maxDisparityText; // 最大視差
GLUI_StaticText* maxPSFText; // 最大PSF径


//Rotation matrix in glui
GLfloat rotMat[16] = {
  1.0, 0.0, 0.0, 0.0,
  0.0, 1.0, 0.0, 0.0,
  0.0, 0.0, 1.0, 0.0,
  0.0, 0.0, 0.0, 1.0
};

//Translation matrix
GLfloat shift[2] = {0.1, 0.1};



void initViewer(int argc, char* argv[])
{
  glutInit(&argc, argv);
  
  //window
  glutInitWindowPosition( 600, 0 );
  glutInitWindowSize( WINDOW_WIDTH, WINDOW_HEIGHT);
  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  GLuint winNumber = glutCreateWindow( argv[0] );
  glutDisplayFunc( display );
  glClearColor( 0.0, 1.0, 1.0, 0.0 );

  //shading and lighting
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  //texture
  setTexture( (char*)TEXTURE_FILE_PATH );
  //object
  initObject( (char*)OBJECT_FILE_PATH );


  //glui
  glui = GLUI_Master.create_glui(argv[0], 0, 0, 0);
  glui->add_statictext(argv[0]);
  glui->set_main_gfx_window( winNumber );
  GLUI_Master.set_glutIdleFunc(NULL);

  //call back
  GLUI_Master.set_glutReshapeFunc(resize);
  GLUI_Master.set_glutKeyboardFunc( keyboard );
  GLUI_Master.set_glutMouseFunc( mouse );

  //top pannel
  GLUI_Panel* topPanel = glui->add_panel("", GLUI_PANEL_NONE);

  //geometry
  GLUI_Panel* geoPanel = glui->add_panel_to_panel(topPanel, "geometry");
  //rotation
  objRot = glui->add_rotation_to_panel(geoPanel,  "rot", rotMat );
  //translation
  GLUI_Translation* objTrans =  glui->add_translation_to_panel(geoPanel, "pan", GLUI_TRANSLATION_XY, shift);
  objTrans->set_speed( 0.1 );

  glui->add_button_to_panel(geoPanel, "reset Rotation", 1, resetRotation);
  glui->add_button_to_panel(geoPanel, "reset look Point", 1, resetLookPoint);

  glui->add_column_to_panel(topPanel, true);

  //save images
  GLUI_Panel* savePanel = glui->add_panel_to_panel(topPanel, "save image");
  GLUI_Button* btn;
  //save button
  btn = glui->add_button_to_panel(savePanel, "current image", 1, saveButton);
  btn->set_alignment(GLUI_ALIGN_LEFT);
  //save depth map
  btn = glui->add_button_to_panel(savePanel, "depth map", 1, depthButton);
  btn->set_alignment(GLUI_ALIGN_LEFT);
  //save disparity map
  btn = glui->add_button_to_panel(savePanel, "disparity map", 1, saveDispButton);
  btn->set_alignment(GLUI_ALIGN_LEFT);  
  //save blurred image
  btn = glui->add_button_to_panel(savePanel, "blurred image", 1, takeBlurredImage);
  btn->set_alignment(GLUI_ALIGN_LEFT);
  //save stereo image
  btn = glui->add_button_to_panel(savePanel, "stereo image", 1, takeStereoImage);
  btn->set_alignment(GLUI_ALIGN_LEFT);
  //save stereo blurred image
  btn = glui->add_button_to_panel(savePanel, "stereo blurred image", 1, takeStereoBlurredImage);
  btn->set_alignment(GLUI_ALIGN_LEFT);



  //Camera parameters
  GLUI_Panel* cameraPanel = glui->add_panel("camera parameters");

  GLUI_Panel* inPanel = glui->add_panel_to_panel(cameraPanel, "", GLUI_PANEL_NONE);
  GLUI_RadioGroup *camSet = glui->add_radiogroup_to_panel( inPanel, &cam, cam, changeCamera);
  glui->add_radiobutton_to_group( camSet, "left");
  glui->add_radiobutton_to_group( camSet, "center");
  glui->add_radiobutton_to_group( camSet, "right");

  glui->add_column_to_panel( inPanel, true);

  //zoom
  GLUI_Spinner* zoomSpinner 
    = glui->add_spinner_to_panel(inPanel, "zoom", GLUI_SPINNER_FLOAT, &zoomView, VIEW_PERSPECTIVE, changeZoom);
  zoomSpinner->set_float_limits( 1.0, 10.0, GLUI_LIMIT_CLAMP);
  zoomSpinner->set_speed( 0.33 );

  glui->add_separator_to_panel(cameraPanel);

  GLUI_Panel* paramPanel = glui->add_panel_to_panel( cameraPanel, "", GLUI_PANEL_NONE);
  // left camera
  GLUI_Panel* lcamPanel = glui->add_panel_to_panel( paramPanel, "left" );

  //change psf
  GLUI_Listbox* listBox = glui->add_listbox_to_panel(lcamPanel, "aperture", &listBoxVar[LEFT_CAM], LEFT_CAM, changePSF);
  listBox->add_item( 0, "circle" );
  listBox->add_item( 1, "Zhou" );
  listBox->add_item( 2, "MLS" );
  listBox->add_item( 3, "CA pair 1");
  listBox->add_item( 4, "CA pair 2");

  //focal distance
  fdepthBox[LEFT_CAM] = glui->add_edittext_to_panel(lcamPanel, "focal distance:", GLUI_EDITTEXT_FLOAT, NULL, LEFT_CAM, changeFocalDepth);
  //DTPparam
  tBox[LEFT_CAM][0] = glui->add_edittext_to_panel(lcamPanel, "a:", GLUI_EDITTEXT_FLOAT, NULL, LEFT_CAM, changeDTPParam);
  tBox[LEFT_CAM][1] = glui->add_edittext_to_panel(lcamPanel, "b:", GLUI_EDITTEXT_FLOAT, NULL, LEFT_CAM, changeDTPParam);
  glui->add_column_to_panel( paramPanel, true);

  //center camera
  GLUI_Panel* ccamPanel = glui->add_panel_to_panel( paramPanel, "center" );
  baseLineBox = glui->add_edittext_to_panel(ccamPanel, "base line:", GLUI_EDITTEXT_FLOAT, NULL, 0, changeBaseLine);
  apertureSizeBox = glui->add_edittext_to_panel(ccamPanel, "aperture size:", GLUI_EDITTEXT_FLOAT, NULL, 0, changeApertureSize);
  maxDisparityText = glui->add_statictext_to_panel( ccamPanel, "max disparity :  ");
  maxPSFText = glui->add_statictext_to_panel( ccamPanel, "max PSF :  ");
  glui->add_column_to_panel( paramPanel, true);

  //right camera
  GLUI_Panel* rcamPanel = glui->add_panel_to_panel( paramPanel, "right" );
  listBox = glui->add_listbox_to_panel(rcamPanel, "aperture", &listBoxVar[RIGHT_CAM], RIGHT_CAM, changePSF);
  listBox->add_item( 0, "circle" );
  listBox->add_item( 1, "Zhou" );
  listBox->add_item( 2, "MLS" );
  listBox->add_item( 3, "CA pair 1");
  listBox->add_item( 4, "CA pair 2");


  //focal distance
  fdepthBox[RIGHT_CAM] = glui->add_edittext_to_panel(rcamPanel, "focal distance:", GLUI_EDITTEXT_FLOAT, NULL, RIGHT_CAM, changeFocalDepth);
  //DTPparam
  tBox[RIGHT_CAM][0] = glui->add_edittext_to_panel(rcamPanel, "a:", GLUI_EDITTEXT_FLOAT, NULL, RIGHT_CAM, changeDTPParam);
  tBox[RIGHT_CAM][1] = glui->add_edittext_to_panel(rcamPanel, "b:", GLUI_EDITTEXT_FLOAT, NULL, RIGHT_CAM, changeDTPParam);
  

  initParam();

  //initalize prameter box
  camSet->set_int_val( CENTER_CAM );
  zoomSpinner->set_float_val( 1.0 );
  changePSF( LEFT_CAM );
  changePSF( RIGHT_CAM );
  fdepthBox[LEFT_CAM]->set_float_val( getFocalDepth( LEFT_CAM ) );
  fdepthBox[RIGHT_CAM]->set_float_val( getFocalDepth( RIGHT_CAM ) );
  baseLineBox->set_float_val( getBaseLine() );
  apertureSizeBox->set_float_val( getApertureSize() );


  batch();
  
  //main loop
  glutMainLoop();

  return ;

}

void initObject( char *filename )
{
  model = glmReadOBJ( filename );
  scale = glmUnitize( model );
  glmFacetNormals( model );
  facetNormal = GL_TRUE;
  
  if(model->nummaterials > 0)
    materialMode = GLM_TEXTURE;

  //create new display lists
  lists();
  
  //enable lighting
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );
  
  //enable depth test (多角形の前後関係を決定)
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  printf("load %s as object\n", filename);

  return;
}


IplImage* readPixel(void)
{
  
  GLint format = GL_BGR;
  GLint type = GL_FLOAT;
  CvSize size = cvSize( getWindowWidth(), getWindowHeight());
  IplImage* img = cvCreateImage( size, IPL_DEPTH_32F, 3);
  
  //read pixel
  glReadPixels( 0, 0, img->width, img->height,
		format, type, img->imageData);

  return img;

}

IplImage* readPixelAtEye( double x, double y)
{
  int winWidth = getWindowWidth();
  int winHeight = getWindowHeight();

  GLint format = GL_BGR;
  GLint type = GL_FLOAT;

  double prevEye[3];
  double far = Z_MAX;
  double eye[3];
  getEye( cam, eye);

  prevEye[0] = eye[0]; prevEye[1] = eye[1]; prevEye[2] = eye[2];

  //set frustum
  double d = getFocalDepth(cam);
  double f = getFLength();
  double tfov = tan( getFov() * M_PI / 360.0 );
  frustum[0] = x - f * tfov - f*x/d;
  frustum[1] = x + f * tfov - f*x/d;
  frustum[2] = y - f * tfov - f*y/d;
  frustum[3] = y + f * tfov - f*y/d;

  eye[0] += ( frustum[0] + frustum[1] ) / 2.0 * d / f;
  eye[1] += ( frustum[2] + frustum[3] ) / 2.0 * d / f;

  setEye( eye, cam);

  setPerspective( VIEW_FRUSTUM );

#ifdef __DEBUG__  
  printf("left = %lf,  right = %lf,  ", frustum[0], frustum[1]);
  printf("bottom = %lf,  top = %lf \n", frustum[2], frustum[3]);
  printf("x = %lf, y = %lf \n", eye[0], eye[1]);
#endif   

  IplImage* img = cvCreateImage( cvSize( winWidth, winHeight), IPL_DEPTH_32F,3);

  glReadPixels(0, 0, img->width, img->height,
	       format, type, img->imageData);

  setEye( prevEye, cam);


  return img;
}


IplImage* readDepthBuffer(void)
{
  int winWidth = getWindowWidth();
  int winHeight = getWindowHeight();
  float* zBuffer = (float*)malloc( sizeof(float) * winWidth * winHeight);
  glReadPixels( 0, 0, winWidth, winHeight,
		GL_DEPTH_COMPONENT, GL_FLOAT, zBuffer);
  
  IplImage* depth = cvCreateImage( cvSize( winWidth, winHeight), IPL_DEPTH_32F, 1);
  float n = getFLength();
  float far = Z_MAX;

  for( int h = 0; h < winHeight; ++h){
    for( int w = 0; w < winWidth; ++w){
      float z = far*n / ( (far-n)*zBuffer[h*winWidth+w] - far);
      CV_IMAGE_ELEM(depth, float, h, w) = z;
    }
  }


  free(zBuffer);

  return depth;

}


void setTexture( char *filename )
{
  IplImage* img = cvLoadImage( filename, CV_LOAD_IMAGE_COLOR );
  if(img){
    printf("load %s as texture\n", filename);
  }else{
    printf("cannot load file %s \n",filename);
  }
  
  glEnable(GL_TEXTURE_2D);

  glPixelStorei( GL_UNPACK_ALIGNMENT, 1);
  glGenTextures(1, &textureData[0]);
  glBindTexture( GL_TEXTURE_2D, textureData[0] );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->width, img->height, 0,
	       GL_RGB, GL_UNSIGNED_BYTE, img->imageData);

  cvReleaseImage( &img );

}


void saveParameters( char *filename )
{
  char buf[256];
  double par[2];
  if( filename != NULL ){
    strcpy( buf, filename );
  }else{
    time_t timer;
    struct tm* tst;
    time(&timer);
    tst = localtime(&timer);
    sprintf(buf, "img/%02d%02d%02d-%02d%02d.txt", 
	    tst->tm_year-100, tst->tm_mon+1, tst->tm_mday, tst->tm_hour, tst->tm_min);
  }

  FILE *fp = fopen( buf, "w" );
  if( fp == NULL ){
    printf("error in save parameters. cannot open file\n");
    return;
  }else{
    fprintf( fp, "common parameters\n");
    fprintf( fp, "\tWindow Size [pixel] : height = %d, width = %d\n",  
	     getWindowWidth(), getWindowHeight());
    fprintf( fp, "\tbase line length [camera corrdinate]: %lf \n", getBaseLine() );
    fprintf( fp, "\taperture size [camera corrdinate]: %lf \n", getApertureSize() );
    fprintf( fp, "\tfov [angle] :  %lf \n", getFov());
    fprintf( fp, "\tMAX DISPARITY : %d\n", getMaxDisparity());
    fprintf( fp, "\tMAX PSF RADIUS : %d\n", getMaxPSFSize());

    fprintf( fp, "\n\nleft camera\n");
    fprintf( fp, "\tfocal depth : %lf \n", getFocalDepth(LEFT_CAM));
    fprintf( fp, "\taperture patern : %d\n", getAperturePattern(LEFT_CAM));
    getDTPParam( LEFT_CAM, par);
    fprintf( fp, "\tDTPParam : PSFSize = %lf * disparity + %lf\n", par[0], par[1]);

    fprintf( fp, "\nright camera\n");
    fprintf( fp, "\tfocal depth : %lf \n", getFocalDepth(RIGHT_CAM));
    fprintf( fp, "\taperture patern : %d\n", getAperturePattern(RIGHT_CAM));
    getDTPParam( RIGHT_CAM, par);
    fprintf( fp, "\tDTPParam : PSFSize = %lf * disparity + %lf\n", par[0], par[1]);

    fclose(fp);
  }

}

/*----------------------------------------------------------------- 
  パブリック関数ここまで
  プライーベト関数ここから
  -----------------------------------------------------------------*/

void display(void)
{
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
  glPushMatrix();

  glMultMatrixf(rotMat);
  glTranslatef( shift[0], shift[1], 0);

  glEnable(GL_LIGHTING);
  glEnable(GL_COLOR_MATERIAL);
  glColor3f(0.5, 0.5, 0.5);
  
  //draw object using glCallList
  glBindTexture(GL_TEXTURE_2D, textureData[0] );
  glCallList( modelList );

  glPopMatrix();
  
  glutSwapBuffers();

}


void keyboard( unsigned char key, int x, int y )
{
  switch(key){
  case 'q':
  case 'Q':
  case '\033':
    exit(0);
  default:
    break;
  }
}

void mouse( int button, int state, int x, int y)
{
  if( button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && cam != CENTER_CAM){
    IplImage *depthBuffer = readDepthBuffer();
    float depth = - CV_IMAGE_ELEM( depthBuffer, float, getWindowHeight()-y, x);
    cvReleaseImage( &depthBuffer);
    fdepthBox[cam]->set_float_val( depth );
    changeFocalDepth( cam );
    printf("change focuced distance of camera %d as %f\n", cam, depth);
    setPerspective(VIEW_PERSPECTIVE );
  }
}


void resize( int w, int h)
{
  setWindowSize(w, h);
  setPerspective( VIEW_PERSPECTIVE );
}


void lists(void)
{
  GLfloat ambient[] = { 0.2, 0.2, 0.2, 1.0 };
  GLfloat diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
  GLfloat specular[] = { 0.3, 0.3, 0.3, 1.0 };
  GLfloat shininess = 65.0;

  if (modelList)
    glDeleteLists(modelList, 1);

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

  /* generate a list */
  if (materialMode == 0) { 
    if (facetNormal)
      modelList = glmList(model, GLM_FLAT);
    else
      modelList = glmList(model, GLM_SMOOTH);
  } else if (materialMode == 1) {
    if (facetNormal)
      modelList = glmList(model, GLM_FLAT | GLM_COLOR);
    else
      modelList = glmList(model, GLM_SMOOTH | GLM_COLOR);
  } else if (materialMode == 2) {
    if (facetNormal)
      modelList = glmList(model, GLM_FLAT | GLM_MATERIAL);
    else
      modelList = glmList(model, GLM_SMOOTH | GLM_MATERIAL);
  } else if (materialMode == GLM_TEXTURE){
    if(facetNormal)
      modelList = glmList(model, GLM_FLAT | GLM_TEXTURE);
    else
      modelList = glmList(model, GLM_SMOOTH | GLM_TEXTURE);
  }


}



/****************************************
  描画に関する関数ここまで
  コールバック関数ここから
 ****************************************/

void resetRotation(int num)
{
  printf("reset rotation\n");
  objRot->reset();
}

void resetLookPoint( int num)
{
  printf("reset look point\n");
  setPerspective( VIEW_PERSPECTIVE );
}

void setPerspective(int viewMode)
{

  int winWidth = getWindowWidth();
  int winHeight = getWindowHeight();
  
  glViewport( 0, 0, winWidth, winHeight );
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  double fov = getFov();
  double f = getFLength();
  double baseLine = getBaseLine();

  if( viewMode == VIEW_PERSPECTIVE )
    {
      gluPerspective( fov, (GLfloat)winHeight / (GLfloat)winWidth, f, Z_MAX );
    }
  else if( viewMode == VIEW_FRUSTUM )
    {
      glFrustum( frustum[0], frustum[1], frustum[2], frustum[3], f, Z_MAX);
    }


  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  glTranslatef( 0.0, 0.0, -3.0 );

  // translation for blurring image & stereo image
  double eye[3];
  getEye( cam, eye);
  glTranslated( eye[0], eye[1], eye[2] );

  display();

}

/*****************************************
  視点変更の関数ここまで
  コールバック関数ここから
****************************************/

//save image の関数

void saveButton(int num)
{
  saveImage((char*)SCREAN_IAMGE);
}

void depthButton(int num)
{
  saveDepthMap((char*)DEPTH_BUFFER);
}

void saveDispButton( int num)
{
  saveDispMap( (char*)DISPARITY_MAP );
}

void takeBlurredImage( int num )
{
  if(cam == CENTER_CAM) changeCamera( LEFT_CAM );
  saveParameters(NULL); 
  setPerspective(VIEW_PERSPECTIVE);
  blur((char*)BLURRED , getAperturePattern(cam) );
}

void takeStereoImage( int num )
{
  int prevCam = cam;
  saveParameters(NULL);
  cam = LEFT_CAM;
  setPerspective(VIEW_PERSPECTIVE);
  saveImage( (char*)STEREO_LEFT );

  cam = RIGHT_CAM;
  setPerspective(VIEW_PERSPECTIVE);
  saveImage( (char*)STEREO_RIGHT );

  cam = prevCam;
  setPerspective(VIEW_PERSPECTIVE);
}

void takeStereoBlurredImage( int num )
{
  saveParameters(NULL);
  
  cam = LEFT_CAM;
  setPerspective(VIEW_PERSPECTIVE);
  blur( (char*)BLURRED_LEFT, getAperturePattern(cam));

  cam = RIGHT_CAM;
  setPerspective(VIEW_PERSPECTIVE);
  blur( (char*)BLURRED_RIGHT, getAperturePattern(cam));
  
  cam = CENTER_CAM;
  setPerspective(VIEW_PERSPECTIVE);

}


void changeCamera( int num)
{
  setPerspective(VIEW_PERSPECTIVE);
}

void changeZoom(int id)
{
  setZoom(zoomView);
  baseLineBox->set_float_val( getBaseLine() );
  apertureSizeBox->set_float_val( getApertureSize() );  

  char buf[256];
  sprintf( buf, "max disparity : %d", getMaxDisparity());
  maxDisparityText->set_text( buf );
  sprintf( buf, "max PSF : %d", getMaxPSFSize());
  maxPSFText->set_text(buf);

  setPerspective(id);
}

void changePSF( int camID)
{
  setAperturePattern( listBoxVar[camID], camID);
}

void changeDTPParam( int id){

  char buf[256];
  sprintf( buf, "max PSF : %d", getMaxPSFSize());
  maxPSFText->set_text(buf);

}

void changeFocalDepth( int id )
{
  setFocalDepth( (fdepthBox[id]->get_float_val()), id);
  double par[2];
  getDTPParam( id, par);
  tBox[id][0]->set_float_val( par[0] );
  tBox[id][1]->set_float_val( par[1] );

  char buf[256];
  sprintf( buf, "max PSF : %d", getMaxPSFSize());
  maxPSFText->set_text(buf);

}

void changeBaseLine( int id)
{
  setBaseLine( baseLineBox->get_float_val() );
  printf("base line = %lf\n", getBaseLine());
  char buf[256];
  sprintf( buf, "max disparity : %d", getMaxDisparity());
  maxDisparityText->set_text( buf );

}

void changeApertureSize( int id)
{
  setApertureSize( apertureSizeBox->get_float_val());

  double par[2];
  getDTPParam( LEFT_CAM, par);
  tBox[LEFT_CAM][0]->set_float_val( par[0] );
  tBox[LEFT_CAM][1]->set_float_val( par[1] );

  getDTPParam( RIGHT_CAM, par);
  tBox[RIGHT_CAM][0]->set_float_val( par[0] );
  tBox[RIGHT_CAM][1]->set_float_val( par[1] );

  char buf[256];
  sprintf( buf, "max PSF : %d", getMaxPSFSize());
  maxPSFText->set_text(buf);

}

