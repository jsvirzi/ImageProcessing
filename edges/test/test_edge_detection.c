/* taken from: http://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/canny_detector/canny_detector.html */

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

 }

float correlationToVerticalLine(Mat &mat, int lineCol) {
	int row, col, rows = mat.rows, cols = mat.cols;
printf("rows=%d cols=%d\n", rows, cols);
	float *data = (float *)mat.data;
	float chi = 0.0, yTot = 0.0;
	for(row=0;row<rows;++row) {
		for(col=0;col<cols;++col) {
			float y = data[row * cols + col];
			yTot += y;
			chi += y * (col - lineCol);
		}
	}
	float corr = chi / yTot;
	printf("yTot = %f. chi = %f. mean = %f. correlation = %f\n", yTot, chi, yTot / (rows * cols), corr);
	return corr;
}

bool detectVerticalEdge(Mat &mat, int rowMin, int rowMax, int colMin, int colMax) {
	Point topLeft = Point(colMin, rowMin);
printf("topLeft = (C=%d, R=%d)\n", topLeft.x, topLeft.y);
	Point bottomRight = Point(colMax, rowMax);
printf("bottomRight = (C=%d, R=%d)\n", bottomRight.x, bottomRight.y);
	Rect rect(topLeft, bottomRight);
	Mat boundingBox = mat(rect);
	int nRows = boundingBox.rows, nCols = boundingBox.cols;
printf("nRows = %d nCols = %d\n", nRows, nCols);
	int i, width = 5, height = nRows;
	Scalar color(0xff, 0x00, 0x00);
	const char *windowName = "sliver";
	imshow(windowName, boundingBox);
	waitKey(0);
	for(i=0;i<(nCols-width);++i) {
		Rect rect(i, 0, width, height); 
		Mat sliver, colorSliver = boundingBox(rect); 
        	cvtColor(colorSliver, sliver, CV_BGR2GRAY);
		sliver.convertTo(sliver, CV_32F);
printf("types = %d %d. chans = %d %d\n", sliver.type(), boundingBox.type(), sliver.channels(), boundingBox.channels());
		Scalar a = mean(sliver); 
		float corr = correlationToVerticalLine(sliver, width/2);
		printf("i=%d. mean=%f corr=%f\n", i, a[0], corr);
		Mat tmpBox;
		boundingBox.copyTo(tmpBox);
		rectangle(tmpBox, rect, color);
		imshow(windowName, tmpBox);
		waitKey(0);
	}
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

		Scalar color(0xff, 0x00, 0x00);
		int rowMin = 70, rowMax = 142, colMin = 170, colMax = 235;
		int col = colMin, row = rowMin, width = colMax - colMin, height = rowMax - rowMin;
		cv::Rect rect(col, row, width, height);
		rectangle(dst, rect, color);
		imshow(window_name, dst);

		detectVerticalEdge(dst, rowMin, rowMax, colMin, colMax);

  /// Wait until user exit program by pressing a key
  		waitKey(wait);
	}

	return 0;

}

