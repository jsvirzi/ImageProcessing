#ifndef ENTROPY_H
#define ENTROPY_H

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"

double mean_image_intensity(cv::Mat &mat);
bool make_pdf(cv::Mat &mat, double *pdf);
double image_entropy(cv::Mat &mat, double *pdf);

#endif
