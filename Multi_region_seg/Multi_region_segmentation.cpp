#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ml.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <queue>

#include "Graph.h"

using namespace cv;
using namespace cv::ml;
using namespace std;

int main()
{

	Mat image, seg;
	image = imread(".//test//plane.jpg", IMREAD_COLOR); // Read the file
	resize(image, image, Size(), 100 / (double)image.rows, 100 / (double)image.rows);

	image.copyTo(seg);
	cvtColor(image, image, CV_BGR2GRAY);

	const int rows = image.rows;
	const int cols = image.cols;
	const int pixel_number = rows * cols;

	Graph graph(10, 10);
	namedWindow("Display window", WINDOW_NORMAL); // Create a window for display.
	imshow("Display window", image);                // Show our image inside it.
	waitKey(0); // Wait for a keystroke in the window
		

	return 0;
}