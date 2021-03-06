/* taken from: http://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/canny_detector/canny_detector.html */

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>

/* ROOT includes */
#include <TGraph.h>
#include <TCanvas.h>
#include <TApplication.h>

using namespace cv;

/// Global variables

int edgeThresh = 1;
int lowThreshold;
int const max_lowThreshold = 100;
int ratio = 3;
int kernel_size = 3;
char* window_name = "Edge Map";

void smear(Mat &imat, Mat &omat, float scale) {
	int i, col, row, rows = imat.rows, cols = imat.cols;
	imat.copyTo(omat);
	float *buff = (float *)omat.data;
	for(row=0;row<rows;++row) {
		double y = row;
		for(col=0;col<cols;++col) {
			double x = col;
		}
	}
}

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

double planarFlow(Mat &mat) {
	double accX = 0.0, accY = 0.0, accXX = 0.0, accXY = 0.0, accYY = 0.0, sumOfWeights = 0.0;
	int i, col, row, rows = mat.rows, cols = mat.cols;
	float *buff = (float *)mat.data;
	for(row=0;row<rows;++row) {
		double y = row;
		for(col=0;col<cols;++col) {
			double x = col;
			double weight = buff[row * cols + col];
if(weight > 10.0) weight = 1.0;
else weight = 0.0;
			sumOfWeights += weight;
			accX += weight * x;
			accY += weight * y;
			accXX += weight * x * x;
			accXY += weight * x * y;
			accYY += weight * y * y;
		}
	}

	// double meanX = accX / sumOfWeights;
	// double meanY = accY / sumOfWeights;
	// double momXX = accXX / sumOfWeights - meanX * meanX;
	// double momXY = accXY / sumOfWeights - meanX * meanY;
	// double momYY = accYY / sumOfWeights - meanY * meanY;
	double det = accXX * accYY - accXY * accXY;
	double trace = accXX + accYY;
	double flow = 4.0 * det / (trace * trace);
	printf("sqrt(det) = %f. trace = %f\n", sqrt(fabs(det)), trace);
	return flow;
}

double pearsonCorrelation(Mat &mat) {
	double accX = 0.0, accY = 0.0, accXX = 0.0, accXY = 0.0, accYY = 0.0, sumOfWeights = 0.0;
	int i, col, row, rows = mat.rows, cols = mat.cols;
	float *buff = (float *)mat.data;
	for(row=0;row<rows;++row) {
		double y = row;
		for(col=0;col<cols;++col) {
			double x = col;
			double weight = buff[row * cols + col];
if(weight > 10.0) weight = 1.0;
else weight = 0.0;
			sumOfWeights += weight;
			accX += weight * x;
			accY += weight * y;
			accXX += weight * x * x;
			accXY += weight * x * y;
			accYY += weight * y * y;
printf("x/y=%f/%f WEIGHT=%f\n", x, y, weight); 
		}
	}

// sumOfWeights = sumOfWeights * 10.0;

	double meanX = accX / sumOfWeights;
	double meanY = accY / sumOfWeights;
	double momXX = accXX / sumOfWeights - meanX * meanX;
	double momXY = accXY / sumOfWeights - meanX * meanY;
	double momYY = accYY / sumOfWeights - meanY * meanY;
	double corr = momXY / sqrt(fabs(momXX * momYY));
	return corr;
}

bool detectVerticalEdge(Mat &mat, int rowMin, int rowMax, int colMin, int colMax, double *correlation) {

/* extract the bounding box as a separate image */
	Point topLeft = Point(colMin, rowMin);
printf("topLeft = (C=%d, R=%d)\n", topLeft.x, topLeft.y);
	Point bottomRight = Point(colMax, rowMax);
printf("bottomRight = (C=%d, R=%d)\n", bottomRight.x, bottomRight.y);
	Rect rect(topLeft, bottomRight);
	Mat grayBoundingBox, colorBoundingBox = mat(rect);
	cvtColor(colorBoundingBox, grayBoundingBox, CV_BGR2GRAY);
	grayBoundingBox.convertTo(grayBoundingBox, CV_32F);
/* at this point, we have color and grayscale versions of the bounding box image */

	float *data = (float *)grayBoundingBox.data;
	int i, col, row, nRows = grayBoundingBox.rows, nCols = grayBoundingBox.cols, width = 3; /* width should be odd */
printf("nRows = %d nCols = %d\n", nRows, nCols);
	const char *windowName = "boundingbox";
	imshow(windowName, grayBoundingBox);
	waitKey(0);
	// for(i=0;i<nCols;++i) correlation[i] = 0.0;
/* if using planar flow */
	double *histogram = new double [ nRows ];
	Scalar color(0xff, 0xff, 0xff);
	width = 5;
	for(col=0;col<(nCols-width);++col) {
		for(row=0;row<nRows;++row) {
			double acc = 0.0;
			for(i=0;i<width;++i) {
				int tCol = col + i;
				int index = row * nCols + tCol;
				acc += data[index];
			}
			histogram[row] = acc;
			// printf("col=%d row=%d. acc=%f\n", col, row, acc);
		}
		correlation[col] = 0.0;
		for(row=0;row<nRows;++row) {
			if(histogram[row] > 50.0) correlation[col] += 1.0;
		}

		Rect rect(col, 0, width, nRows); 
		Mat tmpBox = grayBoundingBox(rect);
		Mat drawBox;
		colorBoundingBox.copyTo(drawBox);
		// correlation[col] = planarFlow(tmpBox);
		// correlation[col] = pearsonCorrelation(tmpBox);
		printf("col=%d corr=%f\n", col, correlation[col]);
		rectangle(drawBox, rect, color);
		imshow(windowName, drawBox);
		// waitKey(0);
	}
	delete [] histogram;
}

bool detectHorizontalEdge(Mat &mat, int rowMin, int rowMax, int colMin, int colMax, double *correlation) {

/* extract the bounding box as a separate image */
	Point topLeft = Point(colMin, rowMin);
printf("topLeft = (C=%d, R=%d)\n", topLeft.x, topLeft.y);
	Point bottomRight = Point(colMax, rowMax);
printf("bottomRight = (C=%d, R=%d)\n", bottomRight.x, bottomRight.y);
	Rect rect(topLeft, bottomRight);
	Mat grayBoundingBox, colorBoundingBox = mat(rect);
	cvtColor(colorBoundingBox, grayBoundingBox, CV_BGR2GRAY);
	grayBoundingBox.convertTo(grayBoundingBox, CV_32F);
/* at this point, we have color and grayscale versions of the bounding box image */

	float *data = (float *)grayBoundingBox.data;
	int i, col, row, nRows = grayBoundingBox.rows, nCols = grayBoundingBox.cols, width = 3; /* width should be odd */
printf("nRows = %d nCols = %d\n", nRows, nCols);
	const char *windowName = "boundingbox";
	imshow(windowName, grayBoundingBox);
	waitKey(0);
	// for(i=0;i<nCols;++i) correlation[i] = 0.0;
/* if using planar flow */
	double *histogram = new double [ nRows ];
	Scalar color(0xff, 0xff, 0xff);
	width = 5;
	for(row=0;row<(nRows-width);++row) {
		for(col=0;col<nCols;++col) {
			double acc = 0.0;
			for(i=0;i<width;++i) {
				int tRow = row + i;
				int index = tRow * nCols + col;
				acc += data[index];
			}
			histogram[col] = acc;
			// printf("col=%d row=%d. acc=%f\n", col, row, acc);
		}
		correlation[row] = 0.0;
		for(col=0;col<nCols;++col) {
			if(histogram[col] > 50.0) correlation[row] += 1.0;
		}

		Rect rect(0, row, nCols, width); 
		Mat tmpBox = grayBoundingBox(rect);
		Mat drawBox;
		colorBoundingBox.copyTo(drawBox);
		// correlation[col] = planarFlow(tmpBox);
		// correlation[col] = pearsonCorrelation(tmpBox);
		printf("row=%d corr=%f\n", row, correlation[row]);
		rectangle(drawBox, rect, color);
		imshow(windowName, drawBox);
		// waitKey(0);
	}
	delete [] histogram;
}

#if 0

bool detectVerticalEdge(Mat &mat, int rowMin, int rowMax, int colMin, int colMax, double *correlation) {
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
		correlation[i] = corr;
		printf("i=%d. mean=%f corr=%f\n", i, a[0], corr);
		Mat tmpBox;
		boundingBox.copyTo(tmpBox);
		rectangle(tmpBox, rect, color);
		imshow(windowName, tmpBox);
		waitKey(0);
	}
	for(;i<nCols;++i) correlation[i] = 0.0; /* finish it out */
}

float correlationToVerticalLine(Mat &mat, int lineCol) {
	int row, col, rows = mat.rows, cols = mat.cols;
printf("rows=%d cols=%d\n", rows, cols);
	float *data = (float *)mat.data;

	float sumx1 = 0.0, sumx2 = 0.0, sumy1 = 0.0, sumy2 = 0.0, sumxy = 0.0, sumOfWeights = 0.0;
	for(row=0;row<rows;++row) {
		float y = row;
		for(col=0;col<cols;++col) {
			float weight = data[row * cols + col]; /* weight = intensity */
			float x = col; 
			sumOfWeights += weight;
			sumx1 += weight * x;
			sumy1 += weight * y;
			sumx2 += weight * x * x;
			sumy2 += weight * y * y;
			sumxy += weight * x * y;
		}
	}
	float xMean = sumx1 / sumOfWeights, yMean = sumy1 / sumOfWeights;
	float xRms = sqrt(fabs(sumx2 / sumOfWeights - xMean * xMean));
	float yRms = sqrt(fabs(sumy2 / sumOfWeights - yMean * yMean));

	float corr = (sumxy / sumOfWeights - xMean * yMean) / (xRms * yRms);
	printf("means = (%f,%f) rms = (%f,%f)\n", xMean, yMean, xRms, yRms);
	return corr;
}

#endif

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

	TApplication theApp("App", &argc, argv);

	TCanvas canvas("main", "main", 500, 350);

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
		int col = colMin, row = rowMin, width = 1 + colMax - colMin, height = 1 + rowMax - rowMin;
		double *correlation = new double [ width ]; 
		double *xvalues = new double [ width ];
		for(i=0;i<width;++i) correlation[i] = 0.0;
		detectVerticalEdge(dst, rowMin, rowMax, colMin, colMax, correlation);
		cv::Rect rect(col, row, width, height);
		rectangle(dst, rect, color);

		for(i=0;i<width;++i) {
			if(correlation[i] > 38.0) {
				printf("col=%d wins\n", i);
				Point p1(col+i, rowMin);
				Point p2(col+i, rowMax);
				line(dst, p1, p2, Scalar(0x00, 0x00, 0xff));
			}
		}

		imshow(window_name, dst);

		for(int i=0;i<width;++i) xvalues[i] = i;

		TGraph graph(width, xvalues, correlation);
		graph.Draw("AL");
		canvas.Update();
		canvas.Draw();

  /// Wait until user exit program by pressing a key
  		waitKey(wait);

		delete [] correlation;
		delete [] xvalues;
	}

	return 0;

}

