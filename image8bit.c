/// image8bit - A simple image processing module.
///
/// This module is part of a programming project
/// for the course AED, DETI / UA.PT
///
/// You may freely use and modify this code, at your own risk,
/// as long as you give proper credit to the original and subsequent authors.
///
/// João Manuel Rodrigues <jmr@ua.pt>
/// 2013, 2023

// Student authors (fill in below):
// NMec:107322  Name: Bernardo Marujo
// NMec:76758  Name: José Diogo Cerqueira
// 
// 
// 
// Date:20/11/2023
//

#include "image8bit.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "instrumentation.h"
#include <string.h>
#include <math.h>

// The data structure
//
// An image is stored in a structure containing 3 fields:
// Two integers store the image width and height.
// The other field is a pointer to an array that stores the 8-bit gray
// level of each pixel in the image.  The pixel array is one-dimensional
// and corresponds to a "raster scan" of the image from left to right,
// top to bottom.
// For example, in a 100-pixel wide image (img->width == 100),
//   pixel position (x,y) = (33,0) is stored in img->pixel[33];
//   pixel position (x,y) = (22,1) is stored in img->pixel[122].
// 
// Clients should use images only through variables of type Image,
// which are pointers to the image structure, and should not access the
// structure fields directly.

// Maximum value you can store in a pixel (maximum maxval accepted)
const uint8 PixMax = 255;

// Internal structure for storing 8-bit graymap images
struct image {
  int width;
  int height;
  int maxval;   // maximum gray value (pixels with maxval are pure WHITE)
  uint8* pixel; // pixel data (a raster scan)
};


// This module follows "design-by-contract" principles.
// Read `Design-by-Contract.md` for more details.

/// Error handling functions

// In this module, only functions dealing with memory allocation or file
// (I/O) operations use defensive techniques.
// 
// When one of these functions fails, it signals this by returning an error
// value such as NULL or 0 (see function documentation), and sets an internal
// variable (errCause) to a string indicating the failure cause.
// The errno global variable thoroughly used in the standard library is
// carefully preserved and propagated, and clients can use it together with
// the ImageErrMsg() function to produce informative error messages.
// The use of the GNU standard library error() function is recommended for
// this purpose.
//
// Additional information:  man 3 errno;  man 3 error;

// Variable to preserve errno temporarily
static int errsave = 0;

// Error cause
static char* errCause;

/// Error cause.
/// After some other module function fails (and returns an error code),
/// calling this function retrieves an appropriate message describing the
/// failure cause.  This may be used together with global variable errno
/// to produce informative error messages (using error(), for instance).
///
/// After a successful operation, the result is not garanteed (it might be
/// the previous error cause).  It is not meant to be used in that situation!
char* ImageErrMsg() { ///
  return errCause;
}


// Defensive programming aids
//
// Proper defensive programming in C, which lacks an exception mechanism,
// generally leads to possibly long chains of function calls, error checking,
// cleanup code, and return statements:
//   if ( funA(x) == errorA ) { return errorX; }
//   if ( funB(x) == errorB ) { cleanupForA(); return errorY; }
//   if ( funC(x) == errorC ) { cleanupForB(); cleanupForA(); return errorZ; }
//
// Understanding such chains is difficult, and writing them is boring, messy
// and error-prone.  Programmers tend to overlook the intricate details,
// and end up producing unsafe and sometimes incorrect programs.
//
// In this module, we try to deal with these chains using a somewhat
// unorthodox technique.  It resorts to a very simple internal function
// (check) that is used to wrap the function calls and error tests, and chain
// them into a long Boolean expression that reflects the success of the entire
// operation:
//   success = 
//   check( funA(x) != error , "MsgFailA" ) &&
//   check( funB(x) != error , "MsgFailB" ) &&
//   check( funC(x) != error , "MsgFailC" ) ;
//   if (!success) {
//     conditionalCleanupCode();
//   }
//   return success;
// 
// When a function fails, the chain is interrupted, thanks to the
// short-circuit && operator, and execution jumps to the cleanup code.
// Meanwhile, check() set errCause to an appropriate message.
// 
// This technique has some legibility issues and is not always applicable,
// but it is quite concise, and concentrates cleanup code in a single place.
// 
// See example utilization in ImageLoad and ImageSave.
//
// (You are not required to use this in your code!)


// Check a condition and set errCause to failmsg in case of failure.
// This may be used to chain a sequence of operations and verify its success.
// Propagates the condition.
// Preserves global errno!
static int check(int condition, const char* failmsg) {
  errCause = (char*)(condition ? "" : failmsg);
  return condition;
}


/// Init Image library.  (Call once!)
/// Currently, simply calibrate instrumentation and set names of counters.
void ImageInit(void) { ///
  InstrCalibrate();
  InstrCount[0] = 0;  // InstrCount[0] will count pixel array acesses
  InstrCount[1] = 0;  // InstrCount[1] will count additions
  // Name other counters here...
  
}

// Macros to simplify accessing instrumentation counters:
#define PIXMEM InstrCount[0]
#define ADDS InstrCount[1]
// Add more macros here...

// TIP: Search for PIXMEM or InstrCount to see where it is incremented!


/// Image management functions

/// Create a new black image.
///   width, height : the dimensions of the new image.
///   maxval: the maximum gray level (corresponding to white).
/// Requires: width and height must be non-negative, maxval > 0.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageCreate(int width, int height, uint8 maxval) { ///
  assert (width >= 0);
  assert (height >= 0);
  assert (0 < maxval && maxval <= PixMax);
  Image img = (Image)malloc(sizeof(struct image));
  img->height = height;
  img->width = width;
  img->maxval = maxval;
  img->pixel = (uint8*)malloc(width*height*sizeof(uint8));
  //start all the pixels with 0
  memset(img->pixel, 0, width*height*sizeof(uint8));
  return img;
}

/// Destroy the image pointed to by (*imgp).
///   imgp : address of an Image variable.
/// If (*imgp)==NULL, no operation is performed.
/// Ensures: (*imgp)==NULL.
/// Should never fail, and should preserve global errno/errCause.
void ImageDestroy(Image* imgp) { ///
  assert (imgp != NULL);
  if (*imgp != NULL) {
    free((*imgp)->pixel);
    free(*imgp);
    *imgp = NULL;
  }
}


/// PGM file operations

// See also:
// PGM format specification: http://netpbm.sourceforge.net/doc/pgm.html

// Match and skip 0 or more comment lines in file f.
// Comments start with a # and continue until the end-of-line, inclusive.
// Returns the number of comments skipped.
static int skipComments(FILE* f) {
  char c;
  int i = 0;
  while (fscanf(f, "#%*[^\n]%c", &c) == 1 && c == '\n') {
    i++;
  }
  return i;
}

/// Load a raw PGM file.
/// Only 8 bit PGM files are accepted.
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageLoad(const char* filename) { ///
  int w, h;
  int maxval;
  char c;
  FILE* f = NULL;
  Image img = NULL;

  int success = 
  check( (f = fopen(filename, "rb")) != NULL, "Open failed" ) &&
  // Parse PGM header
  check( fscanf(f, "P%c ", &c) == 1 && c == '5' , "Invalid file format" ) &&
  skipComments(f) >= 0 &&
  check( fscanf(f, "%d ", &w) == 1 && w >= 0 , "Invalid width" ) &&
  skipComments(f) >= 0 &&
  check( fscanf(f, "%d ", &h) == 1 && h >= 0 , "Invalid height" ) &&
  skipComments(f) >= 0 &&
  check( fscanf(f, "%d", &maxval) == 1 && 0 < maxval && maxval <= (int)PixMax , "Invalid maxval" ) &&
  check( fscanf(f, "%c", &c) == 1 && isspace(c) , "Whitespace expected" ) &&
  // Allocate image
  (img = ImageCreate(w, h, (uint8)maxval)) != NULL &&
  // Read pixels
  check( fread(img->pixel, sizeof(uint8), w*h, f) == w*h , "Reading pixels" );
  PIXMEM += (unsigned long)(w*h);  // count pixel memory accesses

  // Cleanup
  if (!success) {
    errsave = errno;
    ImageDestroy(&img);
    errno = errsave;
  }
  if (f != NULL) fclose(f);
  return img;
}

/// Save image to PGM file.
/// On success, returns nonzero.
/// On failure, returns 0, errno/errCause are set appropriately, and
/// a partial and invalid file may be left in the system.
int ImageSave(Image img, const char* filename) { ///
  assert (img != NULL);
  int w = img->width;
  int h = img->height;
  uint8 maxval = img->maxval;
  FILE* f = NULL;

  int success =
  check( (f = fopen(filename, "wb")) != NULL, "Open failed" ) &&
  check( fprintf(f, "P5\n%d %d\n%u\n", w, h, maxval) > 0, "Writing header failed" ) &&
  check( fwrite(img->pixel, sizeof(uint8), w*h, f) == w*h, "Writing pixels failed" ); 
  PIXMEM += (unsigned long)(w*h);  // count pixel memory accesses

  // Cleanup
  if (f != NULL) fclose(f);
  return success;
}


/// Information queries

/// These functions do not modify the image and never fail.

/// Get image width
int ImageWidth(Image img) { ///
  assert (img != NULL);
  return img->width;
}

/// Get image height
int ImageHeight(Image img) { ///
  assert (img != NULL);
  return img->height;
}

/// Get image maximum gray level
int ImageMaxval(Image img) { ///
  assert (img != NULL);
  return img->maxval;
}

uint8 searchMax(uint8* a, int n) {
  int indexMax = 0;
  for( int i=1; i<n; i++ ) {
      if( a[i] > a[indexMax] ) {
        indexMax = i;
      }
    }
  return a[indexMax];
}


uint8 searchMin(uint8* a, int n) {
  int indexMin = 0; 
  for (int i = 1; i < n; i++) {
    if (a[i] < a[indexMin]) {
      indexMin = i; 
    }
  }
  return a[indexMin]; 
}

/// Pixel stats
/// Find the minimum and maximum gray levels in image.
/// On return,
/// *min is set to the minimum gray level in the image,
/// *max is set to the maximum.
void ImageStats(Image img, uint8* min, uint8* max) { ///
  assert (img != NULL);
  int pixelsize = img->width*img->height;
  *max = searchMax(img->pixel, pixelsize);
  *min = searchMin(img->pixel, pixelsize);

}

/// Check if pixel position (x,y) is inside img.
int ImageValidPos(Image img, int x, int y) { ///
  assert (img != NULL);
  return (0 <= x && x < img->width) && (0 <= y && y < img->height);
}

/// Check if rectangular area (x,y,w,h) is completely inside img.
int ImageValidRect(Image img, int x, int y, int w, int h) { ///
  assert (img != NULL);
  return (0 <= x && x < img->width) && (0 <= y && y < img->height) && (0 <= w && w < img->width) && (0 <= h && h < img->height);
}

/// Pixel get & set operations

/// These are the primitive operations to access and modify a single pixel
/// in the image.
/// These are very simple, but fundamental operations, which may be used to 
/// implement more complex operations.

// Transform (x, y) coords into linear pixel index.
// This internal function is used in ImageGetPixel / ImageSetPixel. 
// The returned index must satisfy (0 <= index < img->width*img->height)
static inline int G(Image img, int x, int y) { ///
  int index;
  index = y*img->width + x;                               // unidimensional array index
  assert (0 <= index && index < img->width*img->height);  // index must be in range
  return index;
}

/// Get the pixel (level) at position (x,y).
uint8 ImageGetPixel(Image img, int x, int y) { ///
  assert (img != NULL);
  assert (ImageValidPos(img, x, y));
  PIXMEM += 1;  // count one pixel access (read)
  return img->pixel[G(img, x, y)];  
} 

/// Set the pixel at position (x,y) to new level.
void ImageSetPixel(Image img, int x, int y, uint8 level) { ///
  assert (img != NULL);
  assert (ImageValidPos(img, x, y));
  PIXMEM += 1;  // count one pixel access (store)
  img->pixel[G(img, x, y)] = level;
} 


/// Pixel transformations

/// These functions modify the pixel levels in an image, but do not change
/// pixel positions or image geometry in any way.
/// All of these functions modify the image in-place: no allocation involved.
/// They never fail.


/// Transform image to negative image.
/// This transforms dark pixels to light pixels and vice-versa,
/// resulting in a "photographic negative" effect.
void ImageNegative(Image img) { ///
  assert (img != NULL);
  int pixelsize = img->width*img->height;
  for (int i = 0; i < pixelsize; i++) {
    img->pixel[i] = PixMax - img->pixel[i];
  }
}

/// Apply threshold to image.
/// Transform all pixels with level<thr to black (0) and
/// all pixels with level>=thr to white (maxval).
void ImageThreshold(Image img, uint8 thr) { ///
  assert (img != NULL);
  int pixelsize = img->width*img->height;
  for (int i = 0; i < pixelsize; i++) {
    if (img->pixel[i] < thr) {
      img->pixel[i] = 0;
    } else {
      img->pixel[i] = img->maxval;
    }
  }
}



/// Brighten image by a factor.
/// Multiply each pixel level by a factor, but saturate at maxval.
/// This will brighten the image if factor>1.0 and
/// darken the image if factor<1.0.
void ImageBrighten(Image img, double factor) { ///
  assert (img != NULL);
  assert (factor >= 0.0);
  int pixelsize = img->width*img->height;
  for (int i = 0; i < pixelsize; i++) {
    double pixel_factor = round((double)img->pixel[i] * factor);
    if (pixel_factor > img->maxval) {
      img->pixel[i] = img->maxval;
    } else {
      img->pixel[i] = pixel_factor;
    }
  }
}


/// Geometric transformations

/// These functions apply geometric transformations to an image,
/// returning a new image as a result.
/// 
/// Success and failure are treated as in ImageCreate:
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.

// Implementation hint: 
// Call ImageCreate whenever you need a new image!

/// Rotate an image.
/// Returns a rotated version of the image.
/// The rotation is 90 degrees anti-clockwise.
/// Ensures: The original img is not modified.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageRotate(Image img) {
  assert (img != NULL);
  Image img_rotated = ImageCreate(img->height, img->width, img->maxval);
  for (int i = 0; i < img->width; i++) {
    for (int j = 0; j < img->height; j++) {
      ImageSetPixel(img_rotated, j, img->width - 1 - i, ImageGetPixel(img, i, j));
    }
  }
  return img_rotated;
}

/// Mirror an image = flip left-right.
/// Returns a mirrored version of the image.
/// Ensures: The original img is not modified.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageMirror(Image img) { ///
  assert (img != NULL);
  Image img_mirrored = ImageCreate(img->width, img->height, img->maxval);
  for (int i = 0; i < img->height; i++) {
    for (int j = 0; j < img->width; j++) {
      ImageSetPixel(img_mirrored, j, i, ImageGetPixel(img, img->width - 1 - j, i)); 
    }
  }
  return img_mirrored;
}

/// Crop a rectangular subimage from img.
/// The rectangle is specified by the top left corner coords (x, y) and
/// width w and height h.
/// Requires:
///   The rectangle must be inside the original image.
/// Ensures:
///   The original img is not modified.
///   The returned image has width w and height h.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageCrop(Image img, int x, int y, int w, int h) { ///
  assert (img != NULL);
  assert (ImageValidRect(img, x, y, w, h));
  Image img_cropped = ImageCreate(w, h, img->maxval);
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) { 
      ImageSetPixel(img_cropped, j, i, ImageGetPixel(img, x + j, y + i));
    }
  }
  return img_cropped;
}


/// Operations on two images

/// Paste an image into a larger image.
/// Paste img2 into position (x, y) of img1.
/// This modifies img1 in-place: no allocation involved.
/// Requires: img2 must fit inside img1 at position (x, y).
void ImagePaste(Image img1, int x, int y, Image img2) { ///
  assert (img1 != NULL);
  assert (img2 != NULL);
  assert (ImageValidRect(img1, x, y, img2->width, img2->height));
  for (int i = 0; i < img2->height; i++) {
    for (int j = 0; j < img2->width; j++) {
      ImageSetPixel(img1, x + j, y + i, ImageGetPixel(img2, j, i));
    }
  }
}

/// Blend an image into a larger image.
/// Blend img2 into position (x, y) of img1.
/// This modifies img1 in-place: no allocation involved.
/// Requires: img2 must fit inside img1 at position (x, y).
/// alpha usually is in [0.0, 1.0], but values outside that interval
/// may provide interesting effects.  Over/underflows should saturate.
void ImageBlend(Image img1, int x, int y, Image img2, double alpha) { ///
    assert(img1 != NULL);
    assert(img2 != NULL);
    assert(ImageValidRect(img1, x, y, img2->width, img2->height));

    for (int i = 0; i < img2->height; i++) {
        for (int j = 0; j < img2->width; j++) {
            double blendedValue = ImageGetPixel(img1, x + j, y + i) * (1 - alpha) + ImageGetPixel(img2, j, i) * alpha;
            uint8 roundedValue = (uint8)(blendedValue + 0.5);
            ImageSetPixel(img1, x + j, y + i, roundedValue);
        }
    }
}

/// Compare an image to a subimage of a larger image.
/// Returns 1 (true) if img2 matches subimage of img1 at pos (x, y).
/// Returns 0, otherwise.
int ImageMatchSubImage(Image img1, int x, int y, Image img2) { ///
  assert (img1 != NULL);
  assert (img2 != NULL);
  assert (ImageValidPos(img1, x, y));
  assert (ImageValidRect(img1, x, y, img2->width, img2->height));

  // Compare the pixels of the subimage with the pixels of the image
  for (int i = 0; i < img2->height; i++) {
    for (int j = 0; j < img2->width; j++) {
      ADDS += 1;
      // If the pixels are different, return 0, continuing the search
      if (ImageGetPixel(img1, x + j, y + i) != ImageGetPixel(img2, j, i)) {
        return 0;
      }
    }
  }
  return 1;
}

/// Locate a subimage inside another image.
/// Searches for img2 inside img1.
/// If a match is found, returns 1 and matching position is set in vars (*px, *py).
/// If no match is found, returns 0 and (*px, *py) are left untouched.
int ImageLocateSubImage(Image img1, int* px, int* py, Image img2) { ///
  assert (img1 != NULL);
  assert (img2 != NULL);
  assert (px != NULL);
  assert (py != NULL);

  // Search for the subimage in the image
  for (int i = 0; i <= img1->height - img2->height; i++) {
    for (int j = 0; j <= img1->width - img2->width; j++) {
      // If the subimage is found, set the position and end the search
      if (ImageMatchSubImage(img1, j, i, img2)) {
        *px = j;
        *py = i;
        return 1;
      }
    }
  }
  return 0;
}


/// Filtering

/// Blur an image by a applying a (2dx+1)x(2dy+1) mean filter.
/// Each pixel is substituted by the mean of the pixels in the rectangle
/// [x-dx, x+dx]x[y-dy, y+dy].
/// The image is changed in-place.

void ImageBlur(Image img, int dx, int dy) {
    assert(img != NULL);
    assert(dx >= 0);
    assert(dy >= 0);

    int width = img->width;
    int height = img->height;

    // Create an array to store cumulative sums
    double** cumSum = (double**)malloc(height * sizeof(double*));
    for (int i = 0; i < height; i++) {
        cumSum[i] = (double*)malloc(width * sizeof(double));
    }

    // Calculate cumulative sums
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            ADDS += 1;

            // Get the value of the current pixel
            cumSum[i][j] = ImageGetPixel(img, j, i);

            // Add the values of the pixels above and to the left
            if (j > 0) {
                cumSum[i][j] += cumSum[i][j - 1];
            }

            if (i > 0) {
                cumSum[i][j] += cumSum[i - 1][j];
            }

            // Subtract the intersection to avoid double addition
            if (i > 0 && j > 0) {
                cumSum[i][j] -= cumSum[i - 1][j - 1];
            }
        }
    }

    // Apply the filter
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            ADDS += 1;

            // Calculate the coordinates of the rectangle
            int iMin = (i - dy > 0) ? i - dy : 0;
            int iMax = (i + dy < height - 1) ? i + dy : height - 1;
            int jMin = (j - dx > 0) ? j - dx : 0;
            int jMax = (j + dx < width - 1) ? j + dx : width - 1;

            // Calculate the area of the rectangle
            int area = (iMax - iMin + 1) * (jMax - jMin + 1);

            // Calculate the sum of the pixels inside the rectangle
            double sum = cumSum[iMax][jMax];

            // Subtract the values of the pixels outside the rectangle
            if (iMin > 0) {
                sum -= cumSum[iMin - 1][jMax];
            }
            if (jMin > 0) {
                sum -= cumSum[iMax][jMin - 1];
            }

            // Add the value of the intersection to avoid double subtraction
            if (iMin > 0 && jMin > 0) {
                sum += cumSum[iMin - 1][jMin - 1];
            }

            // Calculate the mean and round it
            uint8 roundedValue = (uint8)(round(sum / area));
            ImageSetPixel(img, j, i, roundedValue);
        }
    }

    // Free the allocated memory
    for (int i = 0; i < height; i++) {
        free(cumSum[i]);
    }
    free(cumSum);
    
}


void WorseImageBlur(Image img, int dx, int dy) {
    assert(img != NULL);
    assert(dx >= 0);
    assert(dy >= 0);
    
    // Create a copy of the image
    int pixelsize = img->width * img->height;
    Image img_copy = ImageCreate(img->width, img->height, img->maxval);
    img_copy->pixel = (uint8*)malloc(pixelsize * sizeof(uint8));
    memcpy(img_copy->pixel, img->pixel, pixelsize * sizeof(uint8));
    
    // Apply the filter
    for (int i = 0; i < img->height; i++) {
        for (int j = 0; j < img->width; j++) {
            double sum = 0.0;
            int count = 0;
            // Calculate the coordinates of the rectangle
            for (int k = i - dy; k <= i + dy; k++) {
                for (int l = j - dx; l <= j + dx; l++) {
                    ADDS += 1;
                    // Check if the pixel is inside the image
                    if (ImageValidPos(img, l, k)) {
                        // Add the value of the pixel to the sum
                        sum += ImageGetPixel(img_copy, l, k);
                        count++;
                    }
                }
            }

            // Calculate the mean and round it
            uint8 roundedValue = (uint8)(round(sum / count));
            ImageSetPixel(img, j, i, roundedValue);
        }
    }
    free(img_copy->pixel);
    free(img_copy);
}