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

    Image img1 = ImageCreate(2, 2, 1);
    memset(img1->pixel, 1, 2*2*sizeof(uint8));


    if (ImageSave(img1, "worstcase_subimage.pgm") == 0) {
      error(2, errno, "%s: %s", "worstcase_subimage.pgm", ImageErrMsg());
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

