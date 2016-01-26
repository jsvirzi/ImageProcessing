#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>

using namespace cv;

/// Global variables

int edgeThresh = 1;
int lowThreshold;
int const max_lowThreshold = 100;
int ratio = 3;
int kernel_size = 3;
char* window_name = "Edge Map";

/**
 * @function CannyThreshold
 * @brief Trackbar callback - Canny thresholds input with a ratio 1:3
 */
void CannyThreshold(Mat &src_gray, Mat &detected_edges, Mat &src, Mat &dst, int, void*)
{
  /// Reduce noise with a kernel 3x3
  blur( src_gray, detected_edges, Size(3,3) );

  /// Canny detector
  Canny( detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size );

  /// Using Canny's output as a mask, we display our result
  dst = Scalar::all(0);

  src.copyTo( dst, detected_edges);
  imshow( window_name, dst );
 }


/** @function main */
int main(int argc, char** argv) {

	std::string ifile = "lena.jpg";
	int i, wait = 30;
	bool debug = false;

	for(i=1;i<argc;++i) {
		if(strcmp(argv[i], "-debug") == 0) debug = true;
		else if(strcmp(argv[i], "-i") == 0) ifile = argv[++i]; 
		else if(strcmp(argv[i], "-threshold") == 0) lowThreshold = atoi(argv[++i]);
		else if(strcmp(argv[i], "-wait") == 0) wait = atoi(argv[++i]);
	}

	const char *filename = ifile.c_str(); 
	VideoCapture cap(filename); /* open input stream - camera or file */ 
	const char *windowName = "main";
	namedWindow(windowName, WINDOW_AUTOSIZE);
	int rows = 480, cols = 640;

	while(true) {

		Mat raw_frame, frame, gray_frame, detected_edges;

		cap >> raw_frame;

		Size size(640, 480);
		resize(raw_frame, frame, size);

  	/// Create a matrix of the same type and size as src (for dst)
		Mat dst = Mat::zeros(frame.size(), frame.type());

  	/// Convert the image to grayscale
  		cvtColor(frame, gray_frame, CV_BGR2GRAY );

  /// Create a Trackbar for user to enter threshold
  // createTrackbar( "Min Threshold:", windowName, &lowThreshold, max_lowThreshold, CannyThreshold );

  /// Show the image
		CannyThreshold(gray_frame, detected_edges, frame, dst, 0, 0);

  /// Wait until user exit program by pressing a key
  		waitKey(wait);
	}

	return 0;

}

