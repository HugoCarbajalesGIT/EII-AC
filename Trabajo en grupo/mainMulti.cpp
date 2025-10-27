/*
 * Main.cpp
 *
 * Created on: Fall 2019
 */

#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <CImg.h>

using namespace cimg_library;

// Data type for image components
// Change this type according to your group assignment
typedef float data_t; 
const char* SOURCE_IMG      = "bailarina.bmp";
const char* GRADIENT_IMG    = "background_V.bmp";
const char* DESTINATION_IMG = "bailarina2MULTI.bmp";
const int NUMBER_OF_ITERATIONS = 45;
const int NUM_THREADS = 12; // Number of threads used for filtering the images

// Filter argument data type
typedef struct {
    data_t* pRsrc; // Pointers to the R, G, and B components of the source image
    data_t* pGsrc;
    data_t* pBsrc;
    data_t* pRdst;
    data_t* pGdst;
    data_t* pBdst;
    data_t* pRgrd;
    data_t* pGgrd;
    data_t* pBgrd;

    uint pixelCount; // Size of the image in pixels
} filter_args_t;
filter_args_t filter_args;

/**
 * Structure that will store the starting and ending pixels for threads.
 */
struct struPixel {
    uint startInd; 
    uint endInd;
};

/**
 * Thread function for applying the filter.
 * Each thread processes a range of pixels defined in the `struPixel` structure.
 */
void* filter(void* arg) {
    struPixel *pixels;
    pixels = (struPixel *)arg;
    for (uint i = pixels->startInd; i < pixels->endInd; i++) {
        *(filter_args.pRdst + i) = std::min(255.0f , ((256 * (*(filter_args.pRgrd + i))) / ((255 - (*(filter_args.pRsrc + i))) + 1)));
        *(filter_args.pGdst + i) = std::min(255.0f , ((256 * (*(filter_args.pGgrd + i))) / ((255 - (*(filter_args.pGsrc + i))) + 1)));
        *(filter_args.pBdst + i) = std::min(255.0f , ((256 * (*(filter_args.pBgrd + i))) / ((255 - (*(filter_args.pBsrc + i))) + 1)));
    }
    return NULL;
}

int main() {
    if(access(SOURCE_IMG,F_OK)==-1 || access(GRADIENT_IMG,F_OK)==-1){
		perror("At least one of the images does not exist");
		exit(-1);
	}
    // Open file and object initialization
    CImg<data_t> srcImage(SOURCE_IMG);
    CImg<data_t> gradientImage(GRADIENT_IMG);

    data_t *pDstImage; // Pointer to the new image pixels

    /***************************************************
     * Variables initialization.
     * - Prepare variables for the algorithm
     * - This is not included in the benchmark time
     */
    struct timespec tStart, tEnd;
    double dElapsedTimeS;

    // Displays the source image
    uint width = srcImage.width(); // Getting width of the source image
    uint height = srcImage.height(); // Getting height of the source image
    uint nComp = srcImage.spectrum(); // Number of components in the source image
        // Common values for spectrum (number of image components):
        //  B&W images = 1
        //  Normal color images = 3 (RGB)
        //  Special color images = 4 (RGB and alpha/transparency channel)

    // Calculating image size in pixels
    uint widthSecondImg = gradientImage.width();
    uint heightSecondImg = gradientImage.height();

    // Validate dimensions of the two images
    if (width != widthSecondImg) {
        perror("The width of both images is different");
        exit(-2);
    }
    if (height != heightSecondImg) {
        perror("The height of both images is different");
        exit(-2);
    }

    filter_args.pixelCount = width * height;

    // Allocate memory space for destination image components
    pDstImage = (data_t *) malloc(filter_args.pixelCount * nComp * sizeof(data_t));
    if (pDstImage == NULL) {
        perror("Allocating destination image");
        exit(-2);
    }

    // Pointers to the component arrays of the source image
    filter_args.pRsrc = srcImage.data(); // Points to the R component array
    filter_args.pGsrc = filter_args.pRsrc + filter_args.pixelCount; // Points to the G component array
    filter_args.pBsrc = filter_args.pGsrc + filter_args.pixelCount; // Points to the B component array

    // Pointers to the component arrays of the gradient image
    filter_args.pRgrd = gradientImage.data(); // Points to the R component array
    filter_args.pGgrd = filter_args.pRgrd + filter_args.pixelCount; // Points to the G component array
    filter_args.pBgrd = filter_args.pGgrd + filter_args.pixelCount; // Points to the B component array

    // Pointers to the RGB arrays of the destination image
    filter_args.pRdst = pDstImage;
    filter_args.pGdst = filter_args.pRdst + filter_args.pixelCount;
    filter_args.pBdst = filter_args.pGdst + filter_args.pixelCount;

    /************************************************
     * Algorithm initialization.
     * Asign the pixels to the threads
     */
    pthread_t threads[NUM_THREADS];
    struPixel dividedThreads[NUM_THREADS];
    int pixelsPerThread = filter_args.pixelCount / NUM_THREADS; // Pixels to process per thread
    int startPixel = 0;
    int endPixel = pixelsPerThread;

    for (int i = 0; i < NUM_THREADS; i++) {
        struPixel sp; // Instance of the struPixel structure
        if (i == NUM_THREADS - 1) {
            sp.startInd = startPixel; // Starting pixel for the last thread
            sp.endInd = filter_args.pixelCount; // Last pixel for the last thread
        } else {
            sp.startInd = startPixel; // Starting pixel for current thread
            sp.endInd = endPixel; // Last pixel for current thread
        }
		// Update pixel ranges for the next thread
        startPixel += pixelsPerThread; 
        endPixel += pixelsPerThread;
        dividedThreads[i] = sp; // Assign structure to the thread divisions array
    }
    /***********************************************
     * Algorithm start.
     * - Measure initial time
     */
    if (clock_gettime(CLOCK_REALTIME, &tStart) < 0) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }


    // Thread creation, execution and desstruction
    for (int i = 0; i < NUMBER_OF_ITERATIONS; i++) {
        for (int j = 0; j < NUM_THREADS; j++) {
            if (pthread_create(&threads[j], NULL, filter, (void *) &dividedThreads[j])) {
                perror("Creating thread");
                exit(EXIT_FAILURE);
            }
        }
		// Wait for all threads to complete
		for (int i = 0; i < NUM_THREADS; i++) {
			pthread_join(threads[i], NULL);
		}
    }


    /***********************************************
     * End of the algorithm.
     * - Measure the end time
     * - Calculate the elapsed time
     */
    if (clock_gettime(CLOCK_REALTIME, &tEnd) < 0) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }
    dElapsedTimeS = (tEnd.tv_sec - tStart.tv_sec);
    dElapsedTimeS += (tEnd.tv_nsec - tStart.tv_nsec) / 1e+9;
    printf("Elapsed time:   %f s.\n", dElapsedTimeS);

    // Create a new image object with the calculated pixels
    // In case of normal color images use nComp=3,
    // In case of B/W images use nComp=1.
    CImg<data_t> dstImage(pDstImage, width, height, 1, nComp);

    // Store destination image in disk
    dstImage.save(DESTINATION_IMG);

    // Display destination image
    // dstImage.display();

    // Free memory
    free(pDstImage);

    return 0;
}