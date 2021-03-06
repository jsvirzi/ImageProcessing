/* taken from: http://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/canny_detector/canny_detector.html */

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>

#include <iostream>

using namespace cv;
using namespace std;

int lowThreshold;
int const max_lowThreshold = 100;
int ratio = 3;
int kernel_size = 3;

void rotate_frame(Mat &src, Mat &dst, double angle) {
	// get rotation matrix for rotating the image around its center
	cv::Point2f center(src.cols/2.0, src.rows/2.0);
	cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
	// determine bounding rectangle
	cv::Rect bbox = cv::RotatedRect(center,src.size(), angle).boundingRect();
	// adjust transformation matrix
	rot.at<double>(0,2) += bbox.width/2.0 - center.x;
	rot.at<double>(1,2) += bbox.height/2.0 - center.y;
	cv::warpAffine(src, dst, rot, bbox.size());
}

void CannyThreshold(Mat &src_gray, Mat &detected_edges, Mat &src, Mat &dst, int, void*) {
	blur(src_gray, detected_edges, Size(3,3)); // Reduce noise with a kernel 3x3
	Canny(detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size); // Canny detector
	dst = Scalar::all(0); // Using Canny's output as a mask, we display our result
	src.copyTo( dst, detected_edges);
}

class LineDetection {
	public:
	LineDetection(int nX, int nY);
	~LineDetection();
	bool detect(Mat &mat);
	bool getHistogram(Mat &mat);
	int nX, nY, nThetaSteps;
	float *sinLut, *cosLut, *hist, xScale, yScale;
};

LineDetection::LineDetection(int nX, int nY) {
	this->nX = nX;
	this->nY = nY;
	nThetaSteps = nX + 1;
	sinLut = new float [ nX ];
	cosLut = new float [ nX ];
	hist = new float [ nX * nY ];
	xScale = yScale = 1.0;
	for(int i=0;i<nX;++i) {
		float a, theta = i * M_PI / nThetaSteps;
		a = sin(theta); 
		sinLut[i] = a;
		a = cos(theta); 
		cosLut[i] = a;
	}
}

LineDetection::~LineDetection() {
	delete [] sinLut;
	delete [] cosLut;
}

bool LineDetection::detect(Mat &mat) {
	int i, col, row, rows = mat.rows, cols = mat.cols;
	float rMax = sqrt(rows * rows + cols * cols + 1.0); /* add a little bit so r never maxes out completely */
	memset(hist, 0, nX * nY * sizeof(float));
	unsigned char *data = (unsigned char *)mat.data;
	for(row=0;row<rows;++row) {
		// float y = row;
		for(col=0;col<cols;++col) {
			// float x = col;
			int index = row * cols + col;
			if(data[index] == 0.0) continue;
			for(i=0;i<nX;++i) {
				float c = cosLut[i], s = sinLut[i];
				float r = col * c + row * s;
// printf("%d row=%d col=%d cos=%f sin=%f r=%f rmax=%f\n", i, row, col, c, s, r, rMax);
if(fabs(r) > rMax) { printf("whoa! %f %f\n", r, rMax); r = rMax; }
				r = (1.0 + r / rMax) * 0.5 * nY;
				index = floor(r);
				index = index * nX + i;
if(index < 0 || (index >= nX * nY)) { printf("help %d\n", index); getchar(); }
				++hist[index];
			}
// printf("RxC = %dx%d\n", rows, cols);
// getchar();
		}
	}

	return true;

}

bool LineDetection::getHistogram(Mat &mat) {
	Mat tmpMat = Mat(nY, nX, CV_32F, hist);
  	cvtColor(tmpMat, tmpMat, CV_GRAY2BGR); /* convert to color */
	tmpMat.convertTo(mat, CV_8UC3);
printf("mat type = %d vs %d\n", mat.type(), CV_32FC3);
}

int main(int argc, char** argv) {

	std::string ifile = "lena.jpg";
	int i, wait = 30, nX = 100, nY = 100;
	int rect_t, rect_b, rect_r, rect_l;
	bool debug = false, crop = false;
	double rotation = 0.0;

	lowThreshold = 50;

	for(i=1;i<argc;++i) {
		if(strcmp(argv[i], "-debug") == 0) debug = true;
		else if(strcmp(argv[i], "-i") == 0) ifile = argv[++i]; 
		else if(strcmp(argv[i], "-threshold") == 0) lowThreshold = atoi(argv[++i]);
		else if(strcmp(argv[i], "-n") == 0) { nX = atoi(argv[++i]); nY = atoi(argv[++i]); }
		else if(strcmp(argv[i], "-wait") == 0) wait = atoi(argv[++i]);
		else if(strcmp(argv[i], "-rotate") == 0) rotation = atof(argv[++i]);
		else if(strcmp(argv[i], "-crop") == 0) {
			rect_t = atoi(argv[++i]);
			rect_l = atoi(argv[++i]);
			rect_b = atoi(argv[++i]);
			rect_r = atoi(argv[++i]);
			crop = true;
		}
	}

	const char *filename = ifile.c_str(); 
	VideoCapture cap(filename); /* open input stream - camera or file */ 

	const char *windowName = "main";
	namedWindow(windowName, WINDOW_AUTOSIZE);

	LineDetection lineDetection(nX, nY);

	while(true) {


		Mat raw_frame, frame, gray_frame, detected_edges;

		cap >> raw_frame;

		if(raw_frame.data == 0) { printf("end of input stream\n"); break; }
		printf("new frame\n");

		Mat rotated_frame;
		if(rotation != 0.0) {
			rotate_frame(raw_frame, rotated_frame, rotation);
		} else {
			raw_frame.copyTo(rotated_frame);
		}

		Size size(640, 360);
		resize(rotated_frame, frame, size);
//		raw_frame.copyTo(frame);

		if(crop) {
			Point topLeft = Point(rect_l, rect_t);
			Point bottomRight = Point(rect_r, rect_b);
printf("topLeft = (C=%d, R=%d)\n", topLeft.x, topLeft.y);
printf("bottomRight = (C=%d, R=%d)\n", bottomRight.x, bottomRight.y);
			Rect rect(topLeft, bottomRight);

			// rectangle(frame, rect, Scalar(0x00, 0x00, 0xff));

			Mat newMat = frame(rect);
			newMat.copyTo(frame);
		}

  	/// Create a matrix of the same type and size as src (for dst)
		Mat dst = Mat::zeros(frame.size(), frame.type());

  		cvtColor(frame, gray_frame, CV_BGR2GRAY); /* convert to gray scale */

		CannyThreshold(gray_frame, detected_edges, frame, dst, 0, 0); // dst is a CV_8UC3

		Mat gray_edges;
		cvtColor(dst, gray_edges, CV_BGR2GRAY);
		lineDetection.detect(gray_edges);
		Mat mat;
		lineDetection.getHistogram(mat);
		normalize(mat, mat, 0.0, 255.0, CV_MINMAX);
		// imshow(windowName, gray_edges);
		imshow(windowName, mat);
		// imshow(windowName, frame);
		waitKey(0);

	}

}

#if 0

/* (r, theta) ~ (rows, cols). nX = number of bins in x. number of cols = nX + 1 */
void detectLines(Mat &imat, double xScale, double yScale, float *hist, int nX, int nY) {
	int i, j, col, row, rows = imat.rows, cols = imat.cols;
	float rMax = sqrt(rows * rows + cols * cols);
	int nThetaSteps = nX, nRSteps = nY;
	// float *s = new float [ nThetaSteps + 1 ]; 

/* zero out histogram */
	for(j=0;j<(nY+1);++j) { for(i=0;i<(nX+1);++i) { hist[j * (nX + 1) + i] = 0.5; } }

	unsigned char *matBuff = (unsigned char *)imat.data;

	for(row=0;row<rows;++row) {
		double y = row;
		for(col=0;col<cols;++col) {
			double x = col;
			int index = row * cols + col;
			if(matBuff[index] == 0) continue;
			for(int k=0;k<nThetaSteps;++k) {
				double theta = k * M_PI / nThetaSteps;
				double r = x * cos(theta) + y * sin(theta);
				if(fabs(r) > rMax) { printf("whoa! %f %f\n", r, rMax); r = rMax; }
				r = (1.0 + r / rMax) * 0.5 * nY;
				int ocol = k;
				int orow = floor(r);
				index = orow * (nX + 1) + ocol;
if(index < 0 || (index >= (nX+1) * (nY+1))) { printf("help %d %d %d\n", ocol, orow, index); getchar(); }
				++hist[index];
			}
		}
	}
printf("all done\n");
}

int main(int argc, char** argv) {

	std::string ifile = "lena.jpg";
	int i, wait = 30, nX = 100, nY = 100;
	bool debug = false;

	lowThreshold = 50;

	for(i=1;i<argc;++i) {
		if(strcmp(argv[i], "-debug") == 0) debug = true;
		else if(strcmp(argv[i], "-i") == 0) ifile = argv[++i]; 
		else if(strcmp(argv[i], "-threshold") == 0) lowThreshold = atoi(argv[++i]);
		else if(strcmp(argv[i], "-n") == 0) { nX = atoi(argv[++i]); nY = atoi(argv[++i]); }
		else if(strcmp(argv[i], "-wait") == 0) wait = atoi(argv[++i]);
	}

	const char *filename = ifile.c_str(); 
	VideoCapture cap(filename); /* open input stream - camera or file */ 

	const char *windowName = "main";
	namedWindow(windowName, WINDOW_AUTOSIZE);

	int row, col, nRows = nY + 1, nCols = nX + 1;
	float *hist = new float [ (nX + 1) * (nY + 1) ];
	double xScale = 1.0, yScale = 1.0;

	while(true) {

		printf("new frame\n");

		Mat raw_frame, frame, gray_frame, detected_edges;

		cap >> raw_frame;

		Size size(640, 480);
		resize(raw_frame, frame, size);

  	/// Create a matrix of the same type and size as src (for dst)
		Mat dst = Mat::zeros(frame.size(), frame.type());

  		cvtColor(frame, gray_frame, CV_BGR2GRAY); /* convert to gray scale */

		CannyThreshold(gray_frame, detected_edges, frame, dst, 0, 0); // dst is a CV_8UC3

		Mat gray_edges;
		cvtColor(dst, gray_edges, CV_BGR2GRAY);

#if 1
// Mat detectLines(Mat &imat, double xScale, double yScale, double *hist, int nX, int nY) 
		// mat = Mat(nCols, nRows, CV_8UC3, Scalar(0,0,255));
// printf("types = %d %d %d %d %d %d %d\n", frame.type(), gray_frame.type(), dst.type(), CV_32F, CV_8UC3, CV_8UC1, mat.type());
		detectLines(gray_edges, xScale, yScale, hist, nX, nY);
		Mat mat = Mat(nRows, nCols, CV_32F, hist);
		// cout << mat;
#else
		dst.copyTo(mat);
#endif

#if 0
		float *buff = (float *)mat.data;
		for(row=0;row<nRows;++row) {
			for(col=0;col<nCols;++col) {
				int index = row * nCols + col;
				buff[index] = hist[index]; 
			}
		}
#endif

		// delete [] nThetaSteps;
		normalize(mat, mat, 0.0, 1.0, CV_MINMAX);

		// imshow(windowName, gray_edges);
		imshow(windowName, mat);
		waitKey(0);

	}

}

#endif

