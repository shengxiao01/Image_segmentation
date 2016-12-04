#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <queue>

#include "maxflow/Maxflow_fifo_gap.h"
#include "maxflow/Maxflow_rtf.h"
#include "maxflow/Maxflow_pr.h"
#include "maxflow/Maxflow_hpr_gap.h"
#include "maxflow/Maxflow_ff.h"


#include <ctime>
#include <stdlib.h> 
#include <conio.h>

using namespace std;
bool preProcessing(string file_name, Mat& image, Mat& original_image, double downsample_width);
void postProcessing(vector<int>& cut, Mat& image, Mat& original_img, string save_name);
vector<int> imageSegmentation(Mat& image, const string METHOD, const int PRECISION, const int ALPHA);


int main()
{
	string file = ".//test//carriage.jpg";
	string save_file_name = "./temp2_Segmented_fifo_alpha_1_HiRes.jpg";
	double max_dimension = 100;
	int PRECISION = 65535;
	int ALPHA = 1;
	string METHOD = "FF";

	Mat image, original_image;
	if (!preProcessing(file, image, original_image, max_dimension)) return 0;
	vector<int> cut(imageSegmentation(image, METHOD, PRECISION, ALPHA));
	postProcessing(cut, image, original_image, save_file_name);
		
	return 0;
}

void postProcessing(vector<int>& cut, Mat& image, Mat& original_img, string save_name){
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
	imwrite(save_name, segmented_img);

	namedWindow("Display window", WINDOW_NORMAL);  // display original image
	imshow("Display window", original_img);
	waitKey(0); // Wait for a keystroke in the window
}
bool preProcessing(string file_name, Mat& image, Mat& original_image, double max_dimension){

	original_image = imread(file_name, IMREAD_COLOR); // Read the file

	if (original_image.empty())                      // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		waitKey(0);
		return false;
	}

	if (max_dimension < (double)original_image.rows || max_dimension < (double)original_image.cols){
		double downsamlple_ratio = min(max_dimension / (double)original_image.rows, max_dimension / (double)original_image.cols);
		resize(original_image, image, Size(), downsamlple_ratio, downsamlple_ratio);
	}
	else{
		original_image.copyTo(image);
	}
	
	return true;
}
vector<int> imageSegmentation(Mat& image, const string METHOD, const int PRECISION, const int ALPHA){
	vector<int> cut;
	if (METHOD.compare("FF") == 0){
		clock_t begin = clock();
		Maxflow_ff graph(image, 65535, 1);
		clock_t mid = clock();
		graph.maxflow(image.total(), image.total() + 1);
		clock_t end = clock();
		cut = graph.BFSCut(image.total());
		double secs1 = double(mid - begin) / CLOCKS_PER_SEC;
		double secs2 = double(end - mid) / CLOCKS_PER_SEC;
		cout << "Time elapsed for graph bulding: " << secs1 << endl;
		cout << "Time elapsed for max flow: " << secs2 << endl;
	}
	else if (METHOD.compare("PR") == 0){
		clock_t begin = clock();
		Maxflow_pr graph(image, 65535, 1);
		clock_t mid = clock();
		graph.maxflow(image.total(), image.total() + 1);
		clock_t end = clock();
		cut = graph.HeightCut();
		double secs1 = double(mid - begin) / CLOCKS_PER_SEC;
		double secs2 = double(end - mid) / CLOCKS_PER_SEC;
		cout << "Time elapsed for graph bulding: " << secs1 << endl;
		cout << "Time elapsed for max flow: " << secs2 << endl;
	}
	else if (METHOD.compare("RTF") == 0){
		clock_t begin = clock();
		Maxflow_rtf graph(image, 65535, 1);
		clock_t mid = clock();
		graph.maxflow(image.total(), image.total() + 1);
		clock_t end = clock();
		cut = graph.HeightCut();
		double secs1 = double(mid - begin) / CLOCKS_PER_SEC;
		double secs2 = double(end - mid) / CLOCKS_PER_SEC;
		cout << "Time elapsed for graph bulding: " << secs1 << endl;
		cout << "Time elapsed for max flow: " << secs2 << endl;
	}
	else if (METHOD.compare("HPR_GAP") == 0){
		clock_t begin = clock();
		Maxflow_hpr_gap graph(image, 65535, 1);
		clock_t mid = clock();
		graph.maxflow(image.total(), image.total() + 1);
		clock_t end = clock();
		cut = graph.HeightCut();
		double secs1 = double(mid - begin) / CLOCKS_PER_SEC;
		double secs2 = double(end - mid) / CLOCKS_PER_SEC;
		cout << "Time elapsed for graph bulding: " << secs1 << endl;
		cout << "Time elapsed for max flow: " << secs2 << endl;
	}
	else if (METHOD.compare("FIFO_GAP") == 0){
		clock_t begin = clock();
		Maxflow_fifo_gap graph(image, 65535, 1);
		clock_t mid = clock();
		graph.maxflow(image.total(), image.total() + 1);
		clock_t end = clock();
		cut = graph.HeightCut();
		double secs1 = double(mid - begin) / CLOCKS_PER_SEC;
		double secs2 = double(end - mid) / CLOCKS_PER_SEC;
		cout << "Time elapsed for graph bulding: " << secs1 << endl;
		cout << "Time elapsed for max flow: " << secs2 << endl;
	}
	
	return cut;

}