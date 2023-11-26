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

int main(int argc, char* argv[]) {

  if (argc != 4) {
    error(1, 0, "Usage: imageTest input.pgm output.pgm method(blur/locate)");
  }

  if (strcmp(argv[3],"blur")==0){
    ImageInit();

    printf("# Image: %s\n", argv[1]);
    printf("# LOAD image\n");
    InstrReset(); // to reset instrumentation
    Image img1 = ImageLoad(argv[1]);
    if (img1 == NULL) {
      error(2, errno, "Loading %s: %s", argv[1], ImageErrMsg());
    }
    InstrPrint(); // to print instrumentation

    int levels[5] = {0, 1, 5, 10, 100};

    printf("# BLUR operations of %s\n", argv[1]);
    for (int i = 0; i < 5; i++) {
      InstrReset(); // to reset instrumentation
      printf("# BLUR level %d\n", levels[i]);
      ImageBlur(img1, levels[i], levels[i]);

      InstrPrint(); // to print instrumentation
    }

    if (ImageSave(img1, argv[2]) == 0) {
      error(2, errno, "%s: %s", argv[2], ImageErrMsg());
    }
    ImageDestroy(&img1);
    return 0;
  }

  if (strcmp(argv[3],"worseblur")==0){
    ImageInit();

    printf("# Image: %s\n", argv[1]);
    printf("# LOAD image\n");
    InstrReset(); // to reset instrumentation
    Image img1 = ImageLoad(argv[1]);
    if (img1 == NULL) {
      error(2, errno, "Loading %s: %s", argv[1], ImageErrMsg());
    }
    InstrPrint(); // to print instrumentation

    int levels[5] = {0, 1, 5, 10, 20};

    printf("# WORSE BLUR operations of %s\n", argv[1]);
    for (int i = 0; i < 5; i++) {
      InstrReset(); // to reset instrumentation
      printf("# BLUR level %d\n", levels[i]);
      WorseImageBlur(img1, levels[i], levels[i]);

      InstrPrint(); // to print instrumentation
    }

    if (ImageSave(img1, argv[2]) == 0) {
      error(2, errno, "%s: %s", argv[2], ImageErrMsg());
    }

  
    ImageDestroy(&img1);
    return 0;
  }

  if (strcmp(argv[3],"locate")==0){

    int x, y;

    ImageInit();


    InstrReset(); // to reset instrumentation
    Image img1 = ImageLoad(argv[1]);
    if (img1 == NULL) {
      error(2, errno, "Loading %s: %s", argv[1], ImageErrMsg());
    }


    InstrReset(); // to reset instrumentation
    Image img2 = ImageLoad(argv[2]);
    if (img2 == NULL) {
      error(2, errno, "Loading %s: %s", argv[2], ImageErrMsg());
    }

    InstrReset(); // to reset instrumentation

    printf("# LOCATE operation\n");

    printf("# LOCATING first image in second image\n");


    if (ImageLocateSubImage(img2, &x, &y, img1)) {
      printf("# FOUND (%d,%d)\n", x, y);
    } else {
      printf("# NOTFOUND\n");
    }

    InstrPrint(); // to print instrumentation
    ImageDestroy(&img1);
    ImageDestroy(&img2);
    return 0;
  }
  else{


    error(1, 0, "Usage: imageTest input.pgm output.pgm method(blur/locate)\nInvalid method!");


  }
}

