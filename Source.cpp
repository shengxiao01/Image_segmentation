#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <queue>

#include "Maxflow_fifo_gap.h"
#include "Maxflow_rtf.h"
#include "Maxflow_pr.h"
#include "Maxflow_hpr_gap.h"


#include <ctime>
#include <stdlib.h> 
#include <conio.h>

using namespace std;
bool preProcessing(string file_name, Mat& image, Mat& original_image, double downsample_width);
void postProcessing(vector<int>& cut, Mat& image, Mat& original_img);

int main()
{
	
		Mat image, original_image;
		string file = ".//test//carriage.jpg";
		double downsample_width = 300;
		if (!preProcessing(file, image, original_image, downsample_width)) return 0;
		
		clock_t begin = clock();
		//Maxflow_fifo_gap graph(image, 256, 1);
		Maxflow_hpr_gap graph(image, 256, 1);
		//Maxflow_rtf graph(image, 256, 1);
		clock_t mid = clock();

		graph.maxflow(image.total(), image.total() + 1);
		clock_t end = clock();
		vector<int> cut = graph.HeightCut();

		//clock_t end = clock();
		double secs1 = double(mid - begin) / CLOCKS_PER_SEC;
		double secs2 = double(end - mid) / CLOCKS_PER_SEC;
		cout << "Time elapsed for graph bulding: " << secs1 << endl;
		cout << "Time elapsed for max flow: " << secs2 << endl;

		postProcessing(cut, image, original_image);
		
	return 0;
}

void postProcessing(vector<int>& cut, Mat& image, Mat& original_img){
	Mat seg = Mat::zeros(image.rows, image.cols, CV_8UC1);
	int pixel_number = image.total();
	int cols = image.cols;
	int rows = image.rows;

	for (int i = 0; i < cut.size(); ++i){   // convert graph cut to image segmentation results
		if (cut[i] < pixel_number){
			seg.at<uchar>((int)cut[i] / cols, cut[i] % cols) = 255;
		}
	}

	Mat ele = getStructuringElement(MORPH_RECT, Size(3, 3));

	dilate(seg, seg, ele);
	erode(seg, seg, ele);

	erode(seg, seg, ele);
	dilate(seg, seg, ele);

	resize(seg, seg, Size(original_img.cols, original_img.rows));  // upscale the segmentation results
	threshold(seg, seg, 127, 255, 0);

	vector<Mat> channels;
	channels.push_back(Mat::zeros(original_img.rows, original_img.cols, CV_8UC1));
	channels.push_back(Mat::zeros(original_img.rows, original_img.cols, CV_8UC1));
	channels.push_back(seg);

	Mat segmented_img;
	merge(channels, segmented_img);
	addWeighted(original_img, 0.7, segmented_img, 0.3, 0.0, segmented_img); // overlay segmentation and original image for visualization

	namedWindow("Segmentation", WINDOW_NORMAL);   // display segmentation results
	imshow("Segmentation", segmented_img);
	imwrite("./Segmented_alpha_100.jpg", segmented_img);

	namedWindow("Display window", WINDOW_NORMAL);  // display original image
	imshow("Display window", original_img);
	waitKey(0); // Wait for a keystroke in the window
}
bool preProcessing(string file_name, Mat& image, Mat& original_image, double downsample_width){

	original_image = imread(file_name, IMREAD_COLOR); // Read the file

	if (original_image.empty())                      // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		waitKey(0);
		return false;
	}

	resize(original_image, image, Size(), downsample_width / (double)original_image.rows, downsample_width / (double)original_image.rows);
	return true;
}