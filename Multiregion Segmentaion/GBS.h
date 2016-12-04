
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


struct Vertex{
	int rank = 0;
	Vertex* parent = this;
	int set_size = 1;

	int idx = 0;  // node index

};


class Edge{
private:
	Vertex* vertex[2];
	double weight;

public:
	Edge(Vertex* v, Vertex* u,double d_weight) {
		vertex[0] = v;
		vertex[1] = u;
		weight = d_weight;	
	}

	bool operator < (Edge& e2) const {
		return weight < e2.weight;
	}

	friend class GBS;
};


class GBS{
private:
	vector<Vertex> nodes;
	vector<Edge> edge;
	int pixel_number, edge_number, rows, cols;
public:
	GBS(Mat& image){
		pixel_number = image.total();
		rows = image.rows;
		cols = image.cols;
		edge_number = pixel_number * 4 - 2 * (rows + cols);

		nodes = vector<Vertex>(pixel_number);
		edge.reserve(edge_number);

		// initialize all vertex
		for (int i = 0; i < pixel_number; ++i){
			nodes[i].idx = i;
		}

		// build the graph, initialize all edges
		Vec3b x, y;
		double weight;
		for (int i = 0; i < rows ; ++i){
			for (int j = 0; j < cols ; ++j){

				x = image.at<Vec3b>(i, j);
				if (j != cols - 1){
					y = image.at<Vec3b>(i, j + 1);
					weight = pow(x[0] - y[0], 2) + pow(x[1] - y[1], 2) + pow(x[2] - y[2], 2);
					insertEdge(i * cols + j, i * cols + j + 1, weight);
				}
				if (i != rows -1){
					y = image.at<Vec3b>(i + 1, j);
					weight = pow(x[0] - y[0], 2) + pow(x[1] - y[1], 2) + pow(x[2] - y[2], 2);
					insertEdge(i * cols + j, (i + 1) * cols + j, weight);
				}

			}
		}

	}

	void insertEdge(int u, int v, double weight){
		edge.push_back(Edge(&nodes[u], &nodes[v], weight));
	}

	Vertex* findSet(Vertex* u){
		if (u->parent != u){
			u->parent = findSet(u->parent);
		}
		return u->parent;
	}

	void linkSet(Vertex* u, Vertex* v){
		if (u->rank > v->rank){
			v->parent = u;
			u->set_size += v->set_size;
		}
		else{
			u->parent = v;
			v->set_size += u->set_size;
			if (u->rank == v->rank){
				v->rank++;
			}
		}
	}

	void unionSet(Vertex* u, Vertex* v){
		linkSet(u, v);
	}


	void segmentImage(double C = 5000, double sigma = 1, int min_size = 5000){
		// sort all edges by weight
		sort(edge.begin(), edge.end());

		// An array that keeps the threshold value for each set
		// All vlaues are initialized to 1/C
		double* threshold = new double[nodes.size()];
		memset(threshold, 1 / C, nodes.size()*sizeof(int));

		// loop through all edges with non-decreasing weight order
		int edge_number = edge.size();
		for (int i = 0; i < edge_number; i++) {
			// current scanning edge
			Edge& cedge = edge[i];

			// components conected by this edge
			Vertex* u_p = findSet(cedge.vertex[0]);
			Vertex* v_p = findSet(cedge.vertex[1]);

			if (u_p != v_p) {
				if ((cedge.weight <= threshold[u_p->idx]) &&
					(cedge.weight <= threshold[v_p->idx])) {
					unionSet(u_p, v_p);
					Vertex* root_p = findSet(u_p);
					threshold[root_p->idx] = cedge.weight + C / root_p->set_size;
				}
			}
			
		}

		delete threshold;
		// Merge small segmentations
		for (int i = 0; i < edge_number; ++i) {
			Edge& cedge = edge[i];

			Vertex* u_p = findSet(cedge.vertex[0]);
			Vertex* v_p = findSet(cedge.vertex[1]);

			if ((u_p != v_p) && ((u_p->set_size < min_size) || (v_p->set_size < min_size))){
				unionSet(u_p, v_p);
			}

		}
	}

	void returnSegmentation(Mat& segmentation, Mat& image){
		segmentation = Mat(rows, cols, CV_8UC3, Scalar(0, 0, 0));

		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				Vertex* root = findSet(&nodes[i * cols + j]);
				int idx = root->idx;
				segmentation.at<Vec3b>(i, j) = image.at<Vec3b>((int)idx / cols, idx%cols);
			}
		}

	}

};