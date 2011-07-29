#include <stdio.h>

#include "settings.h"
#include <parameter.h>
#include <viewer.h>
#include <camera.h>

int main( int argc, char* argv[] ){
  
  setbuf( stdout, NULL);
  initViewer(argc, argv);
  return 0;
}
