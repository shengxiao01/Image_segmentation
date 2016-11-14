#ifndef GAUSSMODEL_H
#define GAUSSMODEL_H

#include <vector>
#include <cmath>

#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ml.hpp>

using namespace std;
using namespace cv;
using namespace cv::ml;

double estimateNoise(Mat& image);

double neighbourPenality(Vec3b x, Vec3b y, double sigma);

void guassMixModel(Mat& image, Mat& labels, Mat& probs, Mat& means, vector<Mat>& covs);

#endif