#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <queue>

#include "GBS.h"


#include <ctime>
#include <stdlib.h> 
#include <conio.h>

using namespace std;
bool preProcessing(string file_name, Mat& image, Mat& original_image);

int main()
{
	
	string file = ".//test//flower.jpg";
	Mat image, original_image, segmented_image;
	if (!preProcessing(file, image, original_image)) return 0;

	GBS graph(image);
	graph.segmentImage();
	graph.returnSegmentation(segmented_image, image);

	namedWindow("Original", WINDOW_NORMAL);  // display original image
	imshow("Original", original_image);

	namedWindow("Display window", WINDOW_NORMAL);  // display original image
	imshow("Display window", segmented_image);

	waitKey(0); // Wait for a keystroke in the window
		
	return 0;
}


bool preProcessing(string file_name, Mat& image, Mat& original_image){

	original_image = imread(file_name, IMREAD_COLOR); // Read the file

	if (original_image.empty())                      // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		waitKey(0);
		return false;
	}

	original_image.copyTo(image);

	return true;
}