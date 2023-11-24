// imageTest - A program that performs some image processing.
//
// This program is an example use of the image8bit module,
// a programming project for the course AED, DETI / UA.PT
//
// You may freely use and modify this code, NO WARRANTY, blah blah,
// as long as you give proper credit to the original and subsequent authors.
//
// João Manuel Rodrigues <jmr@ua.pt>
// 2023

#include <assert.h>
#include <errno.h>
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image8bit.h"
#include "instrumentation.h"
struct image {
  int width;
  int height;
  int maxval;   // maximum gray value (pixels with maxval are pure WHITE)
  uint8* pixel; // pixel data (a raster scan)
};
int main( int argc, char *argv[]) {

  int x = 10;

  Image img1 = ImageCreate(x, x, 1);
  memset(img1->pixel, 1, x*x*sizeof(uint8));
  for (int i = 1; i <= x; i+=2) {
      for (int j = 1; j <= x; j+=2) {
          img1 -> pixel[x*i+j] = 0;
      }
  }
  img1 -> pixel[x*x-1] = 1;


  



  if (ImageSave(img1, "worstcase_mainimage.pgm") == 0) {
    error(2, errno, "%s: %s", "worstcase_mainimage.pgm", ImageErrMsg());
  }
  /*
  if (ImageLocateSubImage(img1, &x, &y, img2)) {
    printf("# FOUND (%d,%d)\n", x, y);
  } else {
    printf("# NOTFOUND\n");
  }
  InstrPrint(); // to print instrumentation

  */
  ImageDestroy(&img1);
  //ImageDestroy(&img2);
  return 0;

}

