/*
 * Main.cpp
 *
 *  Created on: Fall 2019
 */

#include <stdio.h>
#include <math.h>
#include <CImg.h>

using namespace cimg_library;

// Data type for image components
typedef float data_t;

const char* SOURCE_IMG      = "bailarina.bmp";
const char* GRADIENT_IMG      = "background_V.bmp";
const char* DESTINATION_IMG = "bailarina2.bmp";

//Number of times the function "filter" will be called
const int NUMBER_OF_ITERATIONS = 45;

// Filter argument data type
typedef struct {
	data_t *pRsrc; // Pointers to the R, G and B components
	data_t *pGsrc;
	data_t *pBsrc;
	data_t *pRgrd; // Pointers to the R, G and B components of gradient/second image
	data_t *pGgrd;
	data_t *pBgrd;
	data_t *pRdst; // Pointers to the R, G and B components of the final image
	data_t *pGdst;
	data_t *pBdst;
	uint pixelCount; // Size of the image in pixels
} filter_args_t;

/***********************************************
 * 
 * Algorithm. Image filter.
 * The algorithm is a components swap
 * *********************************************/
void filter (filter_args_t args) {
	/************************************************
	 * The algorithm is a components swap
	 */
    for (uint i = 0; i < args.pixelCount; i++) {


		*(args.pRdst + i) = std::min(255.0f ,((256 * (*(args.pRgrd + i)))/((255-(*(args.pRsrc + i)))+1)));  
		*(args.pGdst + i) = std::min(255.0f ,((256 * (*(args.pGgrd + i)))/((255-(*(args.pGsrc + i)))+1)));
		*(args.pBdst + i) = std::min(255.0f ,((256 * (*(args.pBgrd + i)))/((255-(*(args.pBsrc + i)))+1)));
	}

}

int main() {
	if(access(SOURCE_IMG,F_OK)==-1 || access(GRADIENT_IMG,F_OK)==-1){
		perror("At least one of the images does not exist");
		exit(-1);
	}
	// Open file and object initialization
	CImg<data_t> srcImage(SOURCE_IMG);
	CImg<data_t> gradientImage(GRADIENT_IMG);

	filter_args_t filter_args;
	data_t *pDstImage; // Pointer to the new image pixels


	/***************************************************
	 * Variables initialization.
	 *   - Prepare variables for the algorithm
	 *   - This is not included in the benchmark time
	 */
	struct timespec tStart, tEnd;
	double dElapsedTimeS;

	//srcImage.display(); // Displays the source image
	uint width = srcImage.width();// Getting information from the source image
	uint height = srcImage.height();	
	uint nComp = srcImage.spectrum();// source image number of components
	         // Common values for spectrum (number of image components):
				//  B&W images = 1
				//	Normal color images = 3 (RGB)
				//  Special color images = 4 (RGB and alpha/transparency channel)

	// Calculating image size in pixels
	uint widthSecondImg= gradientImage.width();
	uint heightSecondImg= gradientImage.height();

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
	pDstImage = (data_t *) malloc (filter_args.pixelCount * nComp * sizeof(data_t));
	if (pDstImage == NULL) {
		perror("Allocating destination image");
		exit(-2);
	}

	// Pointers to the componet arrays of the source image
	filter_args.pRsrc = srcImage.data(); // pRcomp points to the R component array
	filter_args.pGsrc = filter_args.pRsrc + filter_args.pixelCount; // pGcomp points to the G component array
	filter_args.pBsrc = filter_args.pGsrc + filter_args.pixelCount; // pBcomp points to B component array

	// Pointers to the componet arrays of the gradient/second image
	filter_args.pRgrd = gradientImage.data(); // pRcomp points to the R component array
	filter_args.pGgrd = filter_args.pRgrd + filter_args.pixelCount; // pGcomp points to the G component array
	filter_args.pBgrd = filter_args.pGgrd + filter_args.pixelCount; // pBcomp points to B component array
	
	// Pointers to the RGB arrays of the destination image
	filter_args.pRdst = pDstImage;
	filter_args.pGdst = filter_args.pRdst + filter_args.pixelCount;
	filter_args.pBdst = filter_args.pGdst + filter_args.pixelCount;


	/***********************************************
	 * Algorithm start.
	 *   - Measure initial time
	 */
	if (clock_gettime(CLOCK_REALTIME, &tStart)<0)
	{
		perror("clock_gettime");
		exit(EXIT_FAILURE);
	}


	/************************************************
	 * Algorithm.
	 */
	for (int i=0;i<NUMBER_OF_ITERATIONS;i++){
		filter(filter_args);
	}

	/***********************************************
	 * End of the algorithm.
	 *   - Measure the end time
	 *   - Calculate the elapsed time
	 */
	if (clock_gettime(CLOCK_REALTIME,&tEnd)<0){
		perror("clock_gettime");
		exit(EXIT_FAILURE);
	}
	dElapsedTimeS=(tEnd.tv_sec-tStart.tv_sec);
	dElapsedTimeS+=(tEnd.tv_nsec-tStart.tv_nsec)/1e+9;
	printf("Elapsed time:   %f s. ",dElapsedTimeS);



		
	// Create a new image object with the calculated pixels
	// In case of normal color images use nComp=3,
	// In case of B/W images use nComp=1.
	CImg<data_t> dstImage(pDstImage, width, height, 1, nComp);

	// Store destination image in disk
	dstImage.save(DESTINATION_IMG); 

	// Display destination image
	//dstImage.display();
	
	// Free memory
	free(pDstImage);

	return 0;
}
