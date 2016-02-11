#include "opencv2/opencv.hpp"

#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;

#define ESC 27

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

void filter_color(Mat &src, Mat &dst, int color) {
	int row, col, rows = src.rows, cols = src.cols;
	unsigned char *src_buff = (unsigned char *)src.data;
	unsigned char *dst_buff = new unsigned char [ 3 * rows * cols ]; 

	for(row=0;row<rows;++row) {
		for(col=0;col<cols;++col) {
			int index = 3 * (row * cols + col);
			dst_buff[index + 0] = (color == 0) ? src_buff[index + 0] : 0;
			dst_buff[index + 1] = (color == 1) ? src_buff[index + 1] : 0;
			dst_buff[index + 2] = (color == 2) ? src_buff[index + 2] : 0;
			// dst_buff[index + 0] = src_buff[index + 0];
			// dst_buff[index + 1] = src_buff[index + 1];
			// dst_buff[index + 2] = src_buff[index + 2];
		}
	}

	dst = Mat(rows, cols, CV_8UC3, dst_buff);
	// delete [] dst_buff; jsv memory leak but the above Mat ctor wants this to stick around
}

int main(int argc, char **argv) {

	int frame_counter = 0, wait = 0, display_rows = 480, display_cols = 640;
	bool is_color = true, calibrate = false;
	std::string ifile;
	Mat camera_matrix, dist_coeffs;
	int i, rect_t, rect_b, rect_r, rect_l;
	double rotation = 0.0;
	for(i=1;i<argc;++i) {
		if(strcmp(argv[i], "-i") == 0) ifile = argv[++i];
		else if(strcmp(argv[i], "-wait") == 0) wait = atoi(argv[++i]);
		else if(strcmp(argv[i], "-rotate") == 0) rotation = atof(argv[++i]);
		else if(strcmp(argv[i], "-crop") == 0) {
			rect_t = atoi(argv[++i]);
			rect_l = atoi(argv[++i]);
			rect_b = atoi(argv[++i]);
			rect_r = atoi(argv[++i]);
		} 
	}

	VideoCapture cap(ifile.c_str()); /* open input stream - camera or file */ 
	if(!cap.isOpened()) return -1;

	double iwidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
	double iheight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

	printf("input video parameters: WxH = %.fx%.f\n", iwidth, iheight);
	
	namedWindow("main", 1);
	while(true) {
		Mat input_frame, frame;
		cap >> input_frame;
		if(input_frame.data == NULL) {
			printf("frame data == NULL. terminating\n");
			break;
		}
		++frame_counter;

		if(rotation != 0.0) {
			rotate_frame(input_frame, frame, rotation);
		} else {
			input_frame.copyTo(frame);
		}

		Size size(display_cols, display_rows);
		resize(frame, frame, size);

		// filter_color(frame, frame, 2);

		printf("new frame %d\n", frame_counter);
		imshow("main", frame);
		int ch = waitKey(wait);
		if(ch == ESC) { break;
		} else if(ch == 'p') { ch = waitKey(0);
		}
	}

	cap.release();
	return 0;
}

