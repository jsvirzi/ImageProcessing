#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include <iostream>
#include <math.h>

using namespace cv;

double mean_image_intensity(Mat &mat) {
	Scalar mean_intensity = mean(mat);
	return mean_intensity.val[0];
};

/* makes a histogram of the image contents.
   if the input is a CV_32F (floating point) it is assumed the values run between 0.0 and 1.0.
   if the input is a CV_8U, the values are assumed to run between 0 and 255.
 */
bool make_pdf(Mat &mat, double *pdf) {
	int i, index, row, rows = mat.rows, col, cols = mat.cols;
	float *buff = (float *)mat.data;
	bool rc = true;
	long int histogram[256];

	for(i=0;i<256;++i) histogram[i] = 0;

	for(row=0;row<rows;++row) {
		for(col=0;col<cols;++col) {
			float y = buff[row * cols + col]; 
			if(y < 0.0) { rc = false; y = 0.0; }
			if(y > 1.0) { rc = false; y = 1.0; }
			index = floor(255.0 * y);
			++histogram[index];
		}
	}

/* make pdf from histogram */
	double pixels = (double)rows * cols;
	for(i=0;i<256;++i) pdf[i] = ((double)histogram[i]) / pixels;

	return rc;
}

double image_entropy(Mat &mat, double *pdf) {
    int index, row, rows = mat.rows, col, cols = mat.cols;
    float *buff = (float *)mat.data;
    double entropy = 0;
    for(row=0;row<rows;++row) {
        for(col=0;col<cols;++col) {
            float y = buff[row * cols + col]; 
            if(y < 0.0) y = 0.0;
            if(y > 1.0) y = 1.0;
            index = floor(255.0 * y);
            double p = pdf[index];
// printf("prob(row=%d,col=%d) = %f\n", row, col, p);
            entropy = entropy - p * log(p);
        }
    }

// getchar();

    return entropy;
}

