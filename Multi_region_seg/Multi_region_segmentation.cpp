#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ml.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <ctime>


#include "Graph.h"

using namespace cv;
using namespace cv::ml;


inline double neighbour_penality(double x, double y, double sigma);
double estimateNoise(Mat& image);
void guassMixModel(Mat& image, Mat& labels, Mat& probs, Mat& means, vector<Mat>& covs);
void buildGraph2(Graph& graph, Mat& image);

int main()
{

	Mat image, seg;
	image = imread(".//test//statue.jpg", IMREAD_COLOR); // Read the file
	resize(image, image, Size(), 200 / (double)image.rows, 200 / (double)image.rows);
	GaussianBlur(image, image, Size(11, 11), 5);
	image.copyTo(seg);
	cvtColor(image, image, CV_BGR2GRAY);

	const int rows = image.rows;
	const int cols = image.cols;
	const int pixel_number = rows * cols;
	const int edge_number = pixel_number * 2 - (rows + cols);
	cout << cols << endl;
	clock_t begin = clock();

	Graph graph(pixel_number, edge_number * 8);
	buildGraph2(graph, image);
	graph.sortEdge();
	clock_t end = clock();
	//graph.printEdge();
	graph.segmentation(5);
	//graph.printSet();
	double secs2 = double(end - begin) / CLOCKS_PER_SEC;
	cout << "Time elapsed for graph bulding: " << secs2 << endl;
	
	map<int, vector<int> > segs = graph.showSegResult();

	for (map<int, vector<int> >::iterator it = segs.begin(); it != segs.end(); ++it){
		int v1 = rand() % 256;
		int v2 = rand() % 256;
		int v3 = rand() % 256;

		for (int m = 0; m < it->second.size(); ++m){
			int idx = it->second[m];
			seg.at<Vec3b>((int)idx / cols, idx % cols) = Vec3b(v1, v2, v3);
		}
		
	}

	namedWindow("Display window", WINDOW_NORMAL); // Create a window for display.
	imshow("Display window", image);                // Show our image inside it.

	namedWindow("Display window2", WINDOW_NORMAL); // Create a window for display.
	imshow("Display window2", seg);                // Show our image inside it.
	waitKey(0); // Wait for a keystroke in the window
		

	return 0;
}
inline double neighbourPenality(int x, int y, double sigma){
	return exp(-pow(((double)x - (double)y), 2) / (2 * pow(sigma, 2)));
}
/*Given a grayscale image, estimate its noise variance. Reference, J. Immerkær, “Fast Noise Variance Estimation”, Computer Vision and Image Understanding, Vol. 64, No. 2, pp. 300-302, Sep. 1996*/
double estimateNoise(Mat& image){
	Mat filter = (Mat_<double>(3, 3) << 1, -2, 1, -2, 4, -2, 1, -2, 1);
	Mat filtered_image;
	filter2D(image, filtered_image, 64, filter, Point(-1, -1), 0, BORDER_REPLICATE);
	filtered_image = abs(filtered_image);
	double sigma = sum(filtered_image)[0];
	const double PI = 3.141592653589793238462643383279502884;
	sigma = sigma * sqrt(0.5 * PI) / (6 * (image.rows - 2) * (image.cols - 2));
	return sigma;
}
void guassMixModel(Mat& image, Mat& labels, Mat& probs, Mat& means, vector<Mat>& covs){

	Mat samples = image.reshape(0, image.rows * image.cols);

	Ptr<EM> em_model = EM::create();
	em_model->setClustersNumber(2);
	em_model->setCovarianceMatrixType(EM::COV_MAT_SPHERICAL);
	em_model->setTermCriteria(TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 300, 0.1));
	em_model->trainEM(samples, noArray(), labels, probs);

	em_model->getCovs(covs);    // covariance matrices of each cluster

	means = em_model->getMeans();  // means of each cluster
}

void buildGraph2(Graph& graph, Mat& image){
	clock_t t1 = clock();
	const int rows = image.rows;
	const int cols = image.cols;
	const int pixel_number = rows * cols;
	const int edge_number = pixel_number * 2 - (rows + cols);
	const int PRECISION = 256;
	const double alpha = 1;


	Mat labels, probs, means;
	vector<Mat> covs;
	
	guassMixModel(image, labels, probs, means, covs);
	

	log(probs, probs);       // turn linear probability to logrithmic scale
	probs = -PRECISION * probs;
	double sigma = estimateNoise(image);
	
	for (int i = 0; i < rows; ++i){

		for (int j = 0; j < cols; ++j){
			int current_pixel = (int)image.at<uchar>(i, j);
			int current_index = i * cols + j;
			int neighbor_index;
			int neighbor_pixel;
			int penality;
			
			if (i != rows - 1){
				neighbor_index = (i + 1) * cols + j;
				neighbor_pixel = (int)image.at<uchar>(i + 1, j);
				//penality = (int)PRECISION * alpha * neighbourPenality(current_pixel, neighbor_pixel, sigma);	
				penality = pow(pow((current_pixel - neighbor_pixel),2) +1,0.5);
				graph.insertEdge(current_index, neighbor_index, penality);

			}

			if (j != cols - 1){
				neighbor_index = i * cols + j + 1;
				neighbor_pixel = (int)image.at<uchar>(i, j + 1);
				//penality = (int)PRECISION * alpha * neighbourPenality(current_pixel, neighbor_pixel, sigma);
				penality = pow(pow((current_pixel - neighbor_pixel), 2) + 1, 0.5);
				graph.insertEdge(current_index, neighbor_index, penality);
			}
			/*
			if (i != rows - 1 && j != cols - 1){
				neighbor_index = (i + 1) * cols + j + 1;
				neighbor_pixel = (int)image.at<uchar>(i + 1, j +1);
				//penality = (int)PRECISION * alpha * neighbourPenality(current_pixel, neighbor_pixel, sigma);	
				penality = pow(pow((current_pixel - neighbor_pixel), 2) + 2, 0.5);
				graph.insertEdge(current_index, neighbor_index, penality);

			}
			if (i < rows - 2){
				neighbor_index = (i + 2) * cols + j;
				neighbor_pixel = (int)image.at<uchar>(i + 2, j);
				//penality = (int)PRECISION * alpha * neighbourPenality(current_pixel, neighbor_pixel, sigma);	
				penality = pow(pow((current_pixel - neighbor_pixel), 2) + 4, 0.5);
				graph.insertEdge(current_index, neighbor_index, penality);

			}

			if (j < cols - 2){
				neighbor_index = i * cols + j + 2;
				neighbor_pixel = (int)image.at<uchar>(i, j + 2);
				//penality = (int)PRECISION * alpha * neighbourPenality(current_pixel, neighbor_pixel, sigma);
				penality = pow(pow((current_pixel - neighbor_pixel), 2) + 4, 0.5);
				graph.insertEdge(current_index, neighbor_index, penality);
			}

			if (j < cols - 2 && i < rows - 2){
				neighbor_index = (i + 2) * cols + j + 2;
				neighbor_pixel = (int)image.at<uchar>(i + 2, j + 2);
				//penality = (int)PRECISION * alpha * neighbourPenality(current_pixel, neighbor_pixel, sigma);
				penality = pow(pow((current_pixel - neighbor_pixel), 2) + 8, 0.5);
				graph.insertEdge(current_index, neighbor_index, penality);
			}
			if (i < rows - 3){
				neighbor_index = (i + 3) * cols + j;
				neighbor_pixel = (int)image.at<uchar>(i + 2, j);
				//penality = (int)PRECISION * alpha * neighbourPenality(current_pixel, neighbor_pixel, sigma);	
				penality = pow(pow((current_pixel - neighbor_pixel), 2) + 9, 0.5);
				graph.insertEdge(current_index, neighbor_index, penality);

			}

			if (j < cols - 3){
				neighbor_index = i * cols + j + 3;
				neighbor_pixel = (int)image.at<uchar>(i, j + 3);
				//penality = (int)PRECISION * alpha * neighbourPenality(current_pixel, neighbor_pixel, sigma);
				penality = pow(pow((current_pixel - neighbor_pixel), 2) + 9, 0.5);
				graph.insertEdge(current_index, neighbor_index, penality);
			}
			*/
			
		}

	}
	clock_t t2 = clock();
	cout << double(t2 - t1) / CLOCKS_PER_SEC << endl;

}