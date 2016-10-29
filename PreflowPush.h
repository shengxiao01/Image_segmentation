#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <utility>
#include <limits.h>
#include <queue>
#include <list>

#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ml.hpp>

using namespace std;
using namespace cv;
using namespace cv::ml;

struct Edge2{
	int weight;   // original capacity
	int r_weight; // residual capacity
};


class Vertex{
private:

	map<int, Edge2> edges;
	int height;        // height of each vertex
	int excess;        // excess flow of each vertex

public:

	Vertex();

	Vertex(int d_height, int d_excess);

	void insert_edge(int vertex, int weight);

	Edge2& operator[](int vertex_idx);

	map<int, Edge2>::iterator begin();

	map<int, Edge2>::iterator end();

	friend class Graph;

};

class Graph{
private:
	vector<Vertex> graph;

public:
	Graph(int size);

	Graph(Mat& image);

	void insert_edge(int start, int end, int weight);

	void addFlow(int u, int v, int flow);

	void initialize(int source);

	int positiveExcessIdx(int t);

	// push flow out of vertex v
	bool push(int u);

	void relabel(int u);

	bool discharge(int u);

	int maxFlow(int s, int t);

	int maxFlow_rtf(int s, int t);

	void moveToFront(int i, int *A);

	vector<int> findCut(int s);
	double estimateNoise(Mat& image);
	inline double neighbourPenality(int x, int y, double sigma);
	void guassMixModel(Mat& image, Mat& labels, Mat& probs, Mat& means, vector<Mat>& covs);
};