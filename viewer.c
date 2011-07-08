#include <viewer.h>


// モデルに関する変数 
GLuint modelList = 0; // display list for object
GLMmodel* model; //model from object file
GLuint textureData[1]; //テクスチャのデータ
GLfloat scale; //objectのスケール
GLuint materialMode = GLM_TEXTURE;
GLboolean facetNormal = GL_FALSE;

//ウィンドウサイズ
int winWidth, winHeight;


//カメラに関する変数
GLfloat zoom;
GLfloat fov; //zoomによって決定される視野角
double baseLine; //ステレオの基線長
double apertureSize;//開口直径
double f = 1.0 ; //focal length

GLdouble eye[3] = {0.0, 0.0, 3.0}; //視点
GLdouble lookPt[3] = {0.0, 0.0, 0.0};

//GLUI
GLUI *glui;
GLUI_Rotation *objRot;

//private method
void display(void);
void keyboard( unsigned char key, int x, int y );
void mouse( int button, int state, int x, int y);
void resize( int w, int h);
void lists(void);

//call back function of interface
void resetRotation(int num);
void resetLookPoint( int num);
void setPerspective(int num);
void saveButton(int num);
void depthButton(int num);

void changePSF( int id);
int listBoxVar;

void changeDTPParam( int id );
GLUI_EditText* tBox[2];

void takeBlurredImage( int num );
void takeStereoImage( int num );
void takeStereoBlurredImage( int num );


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
  winWidth = 1024;
  winHeight = 1024;
  glutInitWindowPosition( 256, 0 );
  glutInitWindowSize( winWidth, winHeight);
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
  if( argc < 3 )
    setTexture( (char*)TEXTURE_FILE_PATH );
  else
    setTexture( argv[2] );

  //object
  if( argc < 2 )
    initObject( (char*)OBJECT_FILE_PATH );
  else
    initObject(argv[1]);


  //glui
  glui = GLUI_Master.create_glui(argv[0], 0, 0, 0);
  glui->add_statictext(argv[0]);
  glui->set_main_gfx_window( winNumber );
  GLUI_Master.set_glutIdleFunc(NULL);

  //call back
  GLUI_Master.set_glutReshapeFunc(resize);
  GLUI_Master.set_glutKeyboardFunc( keyboard );
  GLUI_Master.set_glutMouseFunc( mouse );


  //rotation
  objRot = glui->add_rotation( "rot", rotMat );
  glui->add_button("reset Rotation", 1, resetRotation);

  glui->add_button("reset look Point", 1, resetLookPoint);

  //translation
  GLUI_Translation* objTrans =  glui->add_translation("pan", GLUI_TRANSLATION_XY, shift);
  objTrans->set_speed( 0.1 );

  //zoom
  GLUI_Spinner* zoomSpinner = glui->add_spinner( "zoom", GLUI_SPINNER_FLOAT, &zoom, 1, setPerspective);
  zoomSpinner->set_float_limits( 1.0, 10.0, GLUI_LIMIT_CLAMP);
  zoomSpinner->set_speed( 0.33 );

  //save button
  glui->add_button("save image", 1, saveButton);

  //save depth map
  glui->add_button("save depth map", 1, depthButton);

  
  //change psf
  GLUI_Listbox* listBox = glui->add_listbox("PSF", &listBoxVar, 1, changePSF);
  listBox->add_item( 0, "circle" );
  listBox->add_item( 1, "Zhou" );
  listBox->add_item( 2, "MLS" );

  //DTPparam
  tBox[0] = glui->add_edittext( "a:", GLUI_EDITTEXT_FLOAT, NULL, 0, changeDTPParam);
  tBox[1] = glui->add_edittext( "b:", GLUI_EDITTEXT_FLOAT, NULL, 0, changeDTPParam);

  //take blurred image
  glui->add_button("take blurred image", 1, takeBlurredImage);

  //take stereo image
  glui->add_button( "take stereo image", 1, takeStereoImage);

  glui->add_button( "take stereo blurred image", 1, takeStereoBlurredImage);


  //initial values
  //psf : Circle
  changePSF(0);

  //DTPparam
  // size = disp * 2.5 + 1.0
  tBox[0]->set_float_val( 2.5 );
  tBox[1]->set_float_val( 1.0 );
  setDispSizeParam( 1.5, 1.0 );

  //zoom 
  zoomSpinner->set_float_val( 1.0 );
  
  //phisical parameters
  fov = (GLfloat)(2.0*atan( tan(40.0*M_PI/180.0) / (double)zoom ) *180.0/M_PI);
  baseLine = MAX_DISPARITY * 2.0  * tan( fov * M_PI / 360.0 ) / (double)winWidth;
  

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
  
  IplImage* img = cvCreateImage( cvSize( winWidth, winHeight), IPL_DEPTH_32F, 3);
  
  //read pixel
  glReadPixels( 0, 0, img->width, img->height,
		format, type, img->imageData);

  return img;

}

IplImage* readPixelAtEye( double x, double y)
{

  GLint format = GL_BGR;
  GLint type = GL_FLOAT;
  
  double prevEye[3];
  prevEye[0] = eye[0]; prevEye[1] = eye[1]; prevEye[2] = eye[2];

  eye[0] = x; eye[1] = y;
  setPerspective(0);

  IplImage* img = cvCreateImage( cvSize( winWidth, winHeight), IPL_DEPTH_32F,3);

  glReadPixels(0, 0, img->width, img->height,
	       format, type, img->imageData);

  eye[0] = prevEye[0];
  eye[1] = prevEye[1];

  return img;

}


IplImage* readDepthBuffer(void)
{
  float* zBuffer = (float*)malloc( sizeof(float) * winWidth * winHeight);
  glReadPixels( 0, 0, winWidth, winHeight,
		GL_DEPTH_COMPONENT, GL_FLOAT, zBuffer);
  
  IplImage* depth = cvCreateImage( cvSize( winWidth, winHeight), IPL_DEPTH_32F, 1);
  float n = 1.0;
  float f = Z_MAX;

  for( int h = 0; h < winHeight; ++h){
    for( int w = 0; w < winWidth; ++w){
      float z = f*n / ( (f-n)*zBuffer[h*winWidth+w] - f);
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

void getdepth2PSFSize( double dst[2] )
{
  dst[0] = (double)winWidth * baseLine / (2.0 * tan( fov *M_PI / 360.0 ));
  dst[1] = 0;
  return;
}

double getApertureSize(void)
{
  return apertureSize;
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
  if( button == GLUT_LEFT_BUTTON && state == GLUT_DOWN ){
    IplImage *depthBuffer = readDepthBuffer();

    float depth = - CV_IMAGE_ELEM( depthBuffer, float, winHeight-y, x);

    //disparity - psfSize parameter
    double disparity = (double)winWidth*baseLine/(2.0*depth*tan( fov*M_PI/360.0 ));
    
    cvReleaseImage( &depthBuffer);
    double pInf = (double)winWidth*baseLine/(2.0*Z_MAX*tan( fov*M_PI/360.0 ));
    float a = MAX_DISPARITY / ( disparity - pInf );
    float b = - a * disparity ;

    printf("depth at %d, %d = %f, disparity = %lf\n", y, x, depth, disparity);

    tBox[0]->set_float_val( a );
    tBox[1]->set_float_val( b );
    setDispSizeParam( a, b);

    //clicked point in object cordinate
    GLdouble model[16], proj[16];
    GLint view[4];
    
    glGetDoublev( GL_MODELVIEW_MATRIX, model);
    glGetDoublev( GL_PROJECTION_MATRIX, proj);
    glGetIntegerv( GL_VIEWPORT, view);

    gluUnProject( x, winHeight - y, depth,
		  model, proj, view,
		  &lookPt[0], &lookPt[1], &lookPt[2]);

    for(int i = 0; i < 3; ++i)
      lookPt[i] -= eye[i];

    printf("look at %lf, %lf, %lf\n", lookPt[0], lookPt[1], lookPt[2]);
    apertureSize
      = 2.0 * depth * Z_MAX * MAX_PSF_RADIUS * tan( fov*M_PI/360.0 )
      / ((double)winWidth * (Z_MAX - depth ));

    printf("aperture size = %lf\n", apertureSize);

    setPerspective(0);
  }
}


void resize( int w, int h)
{
  winWidth = w;  
  winHeight = h;
  setPerspective(0);
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
  lookPt[0] =  lookPt[1] =  lookPt[2] = 0.0;
  setPerspective(0);
}

void setPerspective(int num)
{
  glViewport( 0, 0, winWidth, winHeight );
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  fov = (GLfloat)(2.0*atan( tan(40.0*M_PI/180.0) / (double)zoom ) *180.0/M_PI);

  baseLine = MAX_DISPARITY * 2.0 * f * tan( fov * M_PI / 360.0 ) / (double)winWidth;

  gluPerspective( fov, (GLfloat)winHeight / (GLfloat)winWidth, 1.0, Z_MAX );
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt( eye[0], eye[1], eye[2], 
	     lookPt[0], lookPt[1], lookPt[2],
	     0.0, 1.0, 0.0);

  display();

}

void saveButton(int num)
{
  saveImage((char*)SCREAN_IAMGE);
}

void depthButton(int num)
{
  saveDepthMap((char*)DEPTH_BUFFER);
}


void changePSF( int id)
{
  char psfFile[256];
  switch(id){
  case 0: //circle
    sprintf(psfFile,"images/circle.png");
    break;
    
  case 1: //Zhou
    sprintf(psfFile, "images/Zhou0002.png");
    break;

  case 2://MLS
    sprintf( psfFile, "images/MLS.png");
    break;
    
  default:
    break;
  }
  setAperture(psfFile);
}

void changeDTPParam( int id){
  float a = tBox[0]->get_float_val();
  float b = tBox[1]->get_float_val();
  setDispSizeParam( a, b);
}

void takeBlurredImage( int num )
{
  //blur((char*)"blurred.png");
  blur2( (char*)"hoge");
  
}

void takeStereoImage( int num )
{


  baseLine = MAX_DISPARITY * 2.0 * f * tan( fov * M_PI / 360.0 ) / (double)winWidth;
  printf("baseLine = %f\n",baseLine);
  printf("d = %lf, f = %lf, fov = %lf, winWidth = %d\n",
	 MAX_DISPARITY, f, fov, winWidth);

  eye[0] = 0.0;
  lookPt[0] = 0.0;
  setPerspective(0);
  saveImage( (char*)"left.png" );

  eye[0] = baseLine;
  lookPt[0] = baseLine;
  setPerspective(0);
  saveImage( (char*)"right.png" );

  eye[0] = 0.0;
  lookPt[0] = 0.0;
  setPerspective(0);

}

void takeStereoBlurredImage( int num )
{
  double f = 1.0; //focal length
  baseLine = MAX_DISPARITY * 2.0 * f * tan( fov * M_PI / 360.0 ) / (double)winWidth;
  
  eye[0] = 0.0;
  lookPt[0] = 0.0;
  setPerspective(0);
  blur( (char*)"blurredLeft.png");

  eye[0] = baseLine;
  lookPt[0] = baseLine;
  setPerspective(0);
  blur( (char*)"blurredRight.png");
  
  eye[0] = 0.0;
  lookPt[0] = 0.0;
  setPerspective(0);

}
