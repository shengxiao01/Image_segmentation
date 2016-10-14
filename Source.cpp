#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ml.hpp>

#include <limits.h>
#include <algorithm>
#include <cmath>

#include <iostream>
#include <string>
#include <vector>
#include <queue>

#include "graph.h"

#include <time.h>
#include <stdlib.h> 

using namespace cv;
using namespace cv::ml;
using namespace std;

#define DEBUG 3


bool BFS(vector<vector<Edge> >& graph, int s, int t, vector<int>& path);
int maxFlow(vector<vector<Edge> >& graph, int s, int t, vector<vector<Edge> >& residual_graph);
vector<int> findCut(vector<vector<Edge> >& graph, int s);
inline double neighbour_penality(double x, double y, double sigma);
double estimateNoise(Mat& image);
vector<vector<Edge> > buildGraph(Mat& image);
void guassMixModel(Mat& image, Mat& labels, Mat& probs, Mat& means, vector<Mat>& covs);

int main()
{


	if (DEBUG == 1){

		vector<vector<Edge> > graph(6);

		graph[0].push_back(Edge(1, 16));
		graph[0].push_back(Edge(2, 13));
		graph[1].push_back(Edge(2, 10));
		graph[1].push_back(Edge(3, 12));
		graph[2].push_back(Edge(1, 4));
		graph[2].push_back(Edge(4, 14));
		graph[3].push_back(Edge(2, 9));
		graph[3].push_back(Edge(5, 20));
		graph[4].push_back(Edge(3, 7));
		graph[4].push_back(Edge(5, 4));
		vector<vector<Edge> > residual_graph(graph);  // this is deep copy of a vector


		/*vector<vector<int> > graph = { { 0, 16, 13, 0, 0, 0 },
		{ 0, 0, 10, 12, 0, 0 },
		{ 0, 4, 0, 0, 14, 0 },
		{ 0, 0, 9, 0, 0, 20 },
		{ 0, 0, 0, 7, 0, 4 },
		{ 0, 0, 0, 0, 0, 0 }
		};*/

		cout << "The maximum possible flow is " << maxFlow(graph, 0, 5, residual_graph) << endl;


	}
	else if (DEBUG == 2){
		for (int n = 10; n < 101;){
			int k = 500 * n;
			vector<vector<Edge> > graph(k);
			for (int i = 1; i < k * 5; ++i){
				graph[int(i / 5)].push_back(Edge(rand() % k, 1));
			}
			vector<vector<Edge> > residual_graph(graph);  // this is deep copy of a vector

			clock_t start, stop;
			start = clock();
			cout << "The maximum possible flow is " << maxFlow(graph, 0, 5, residual_graph) << endl;
			stop = clock();
			double elapsed = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;
			cout << "Elapsed time: " << elapsed << endl;
			n = n + 10;
		}
	}
	else if (DEBUG == 3){
		Mat image, seg;
		image = imread(".//test//statue.jpg", IMREAD_COLOR); // Read the file
		resize(image, image, Size(), 100/(double)image.rows, 100/(double)image.rows);

		image.copyTo(seg);
		cvtColor(image, image, CV_BGR2GRAY);

		const int rows = image.rows;
		const int cols = image.cols;
		const int pixel_number = rows * cols;

		if (image.empty())                      // Check for invalid input
		{
			cout << "Could not open or find the image" << std::endl;
			return 0;
		}
		
		vector<vector<Edge> > graph = buildGraph(image);
		vector<vector<Edge> > residual_graph(graph);  // this is deep copy of a vector

		cout << "The maximum possible flow is " << maxFlow(graph, pixel_number, pixel_number + 1, residual_graph) << endl;

		vector<int> cut = findCut(residual_graph, pixel_number);

		for (int i = 0; i < cut.size(); ++i){
			if (cut[i] < pixel_number){
				seg.at<Vec3b>((int)cut[i] / cols, cut[i] % cols) = Vec3b(125, 45, 178);
			}
		}

		namedWindow("Segmentation", WINDOW_NORMAL); // Create a window for display.
		imshow("Segmentation", seg);

		namedWindow("Display window", WINDOW_NORMAL); // Create a window for display.
		imshow("Display window", image);                // Show our image inside it.
		waitKey(0); // Wait for a keystroke in the window

	}

	return 0;
}


inline double neighbourPenality(int x, int y, double sigma){
	return exp(-pow(((double)x - (double)y), 2) / (2 * pow(sigma, 2)));
}
/*Given a grayscale image, estimate its noise variance. Reference, J. Immerkær, “Fast Noise Variance Estimation”, Computer Vision and Image Understanding, Vol. 64, No. 2, pp. 300-302, Sep. 1996*/
double estimateNoise(Mat& image){
	Mat filter = (Mat_<double>(3,3) << 1, -2, 1, -2, 4, -2, 1, -2, 1);
	Mat filtered_image;
	filter2D(image, filtered_image, 64, filter, Point(-1, -1), 0, BORDER_REPLICATE);
	filtered_image = abs(filtered_image);
	double sigma = sum(filtered_image)[0];
	const double PI = 3.141592653589793238462643383279502884;
	sigma = sigma * sqrt(0.5 * PI) / (6 * (image.rows - 2) * (image.cols - 2));
	return sigma;
}
/* given an residual graph, a source vertex, find the cut*/
vector<int> findCut(vector<vector<Edge> >& graph, int s){

	vector<int> cut;

	const int vertex_count = graph.size();   // the number vertex in the graph

	vector<bool> visited(vertex_count, false);    // flag if a verex has been visited

	queue<int> q;      //  a queue for current vertex index
	q.push(s);
	visited[s] = true;

	while (!q.empty()){
		int u = q.front();     // current scanning vertex
		q.pop();

		for (vector<Edge>::iterator it = graph[u].begin(); it != graph[u].end(); ++it){

			int v = it->vertex();     //(u,v) is the current scanning edge
			if (it->weight() > 0 && visited[v] == false){
				q.push(v);
				cut.push_back(v);
				visited[v] = true;
			}
		}
	}
	return cut;
}
/* given an adjacency matrix, a source vertex, and a sink vertex, return a path if there exists one from s to t*/
bool BFS(vector<vector<Edge> >& graph, int s, int t, vector<int>& path){

	const int vertex_count = graph.size();   // the number vertex in the graph

	vector<bool> visited(vertex_count, false);    // flag if a verex has been visited

	queue<int> q;      //  a queue for current vertex index
	q.push(s);
	visited[s] = true;
	path[s] = -1;
	while (!q.empty()){

		int u = q.front();     // current scanning vertex
		q.pop();
		for (vector<Edge>::iterator it = graph[u].begin(); it != graph[u].end(); ++it){

			int v = it->vertex();     //(u,v) is the current scanning edge
			if (v < 0) { continue; }
			if (visited[v] == false && it->weight() > 0){
				q.push(v);
				path[v] = u;
				visited[v] = true;
			}
		}
	}
	return visited[t];   // if the sink vertex has been visited 
}
/*Calculate the max flow using Ford-Fulkerson method*/
int maxFlow(vector<vector<Edge> >& graph, int s, int t, vector<vector<Edge> >& residual_graph){
	const int vertex_count = graph.size();


	vector<int> path(vertex_count, -2);

	int max_flow = 0;
	int u, v;

	while (BFS(residual_graph, s, t, path)){
		int path_flow = INT_MAX;
		vector<Edge>::iterator it;
		// find an augmenting path and calculate the flow
		for (v = t; v != s; v = path[v]){
			u = path[v];

			it = residual_graph[u].begin();
			while (it->vertex() != v){
				++it;
			}

			path_flow = min(path_flow, it->weight());
		}
		// calculate the residual flow of the graph
		for (v = t; v != s; v = path[v]){
			u = path[v];

			it = residual_graph[u].begin();
			while (it->vertex() != v){
				++it;

			}

			it->change_weight(-path_flow);

			it = residual_graph[v].begin();

			while (it->vertex() != u){
				++it;
			}

			it->change_weight(path_flow);
		}
		max_flow += path_flow;
		cout << "Current max flow = " << max_flow << endl;

	}

	return max_flow;
}
/* Build an adjacency matrix graph given an image*/
vector<vector<Edge> > buildGraph(Mat& image){
	const int rows = image.rows;
	const int cols = image.cols;
	const int pixel_number = rows * cols;
	const int PRECISION = 256;
	const double alpha = 1;


	Mat labels, probs, means;
	vector<Mat> covs;
	guassMixModel(image, labels, probs, means, covs);

	log(probs, probs);       // turn linear probability to logrithmic scale
	probs = - PRECISION * probs;
	double sigma = estimateNoise(image);
	//probs.convertTo(probs, 8, 255, 0);


	vector<vector<Edge> > graph(pixel_number, vector<Edge>(6));  // first N pixel represent the pixels in the image,
	graph.push_back(vector<Edge>(pixel_number));   // Source vertex
	graph.push_back(vector<Edge>(pixel_number));        // Sink vertex

	for (int i = 0; i < rows; ++i){

		for (int j = 0; j < cols; ++j){
			int current_pixel = (int)image.at<uchar>(i, j);
			int current_index = i * cols + j;
			int neighbor_index;
			int neighbor_pixel;
			int penality;
			graph[pixel_number][current_index] = Edge(current_index, (int)probs.at<double>(current_index, 0));   // an edge from source
			graph[current_index][5] = Edge(pixel_number, 0); // an edge from current pixel to source with 0 capacity
			graph[current_index][0] = Edge(pixel_number + 1, (int)probs.at<double>(current_index, 1)); // an edge to sink
			graph[pixel_number + 1][current_index] = Edge(current_index, 0);      // an edge from sink to current_pixel with 0 capacities  

			if (i != 0){
				neighbor_index = (i - 1) * cols + j;
				neighbor_pixel = (int)image.at<uchar>(i - 1, j);
				penality = (int)PRECISION * alpha * neighbourPenality(current_pixel, neighbor_pixel, sigma);
				graph[current_index][1] = (Edge(neighbor_index, penality));
			}
			if (i != rows - 1){
				neighbor_index = (i + 1) * cols + j;
				neighbor_pixel = (int)image.at<uchar>(i + 1, j);
				penality = (int)PRECISION * alpha * neighbourPenality(current_pixel, neighbor_pixel, sigma);
				graph[current_index][2] = (Edge(neighbor_index, penality));
			}
			if (j != 0){
				neighbor_index = i * cols + j - 1;
				neighbor_pixel = (int)image.at<uchar>(i, j - 1);
				penality = (int)PRECISION * alpha * neighbourPenality(current_pixel, neighbor_pixel, sigma);
				graph[current_index][3] = (Edge(neighbor_index, penality));
			}
			if (j != cols - 1){
				neighbor_index = i * cols + j + 1;
				neighbor_pixel = (int)image.at<uchar>(i, j + 1);
				penality = (int)PRECISION * alpha * neighbourPenality(current_pixel, neighbor_pixel, sigma);
				graph[current_index][4] = (Edge(neighbor_index, penality));
			}
		}

	}

	return graph;

}
/*Bi-cluster gaussian mixture model*/
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