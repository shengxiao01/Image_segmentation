#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <limits.h>
#include <algorithm>

#include <iostream>
#include <string>
#include <vector>
#include <queue>

#include "graph.h"

#include <time.h>
#include <stdlib.h> 
#include <conio.h>

using namespace cv;
using namespace std;

#define DEBUG 3


bool BFS(vector<vector<Edge> >& graph, int s, int t, vector<int>& path);
int maxFlow(vector<vector<Edge> >& graph, int s, int t, vector<vector<Edge> >& residual_graph);
void findCut(vector<vector<Edge> >& graph, int s, vector<int>& A);

/*class node {
int value;
vector<node*> neighbors;
};*/



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
		image = imread("test6.bmp", IMREAD_COLOR); // Read the file
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

		vector<vector<Edge> > graph(pixel_number + 2);  // first N pixel represent the pixels in the image,
		// second last vector is the source
		// last vector is the sink

		for (int i = 0; i < rows; ++i){


			for (int j = 0; j < cols; ++j){
				int current_pixel = (int)image.at<uchar>(i, j);
				int current_index = i * cols + j;
				int neighbor_index;
				int neighbor_pixel;

				graph[pixel_number].push_back(Edge(current_index, current_pixel));
				graph[current_index].push_back(Edge(pixel_number + 1, abs(255 - current_pixel)));

				if (i != 0){
					neighbor_index = (i - 1) * cols + j;
					neighbor_pixel = (int)image.at<uchar>(i - 1, j);
					graph[current_index].push_back(Edge(neighbor_index, abs(current_pixel - neighbor_pixel)));
				}
				if (i != rows - 1){
					neighbor_index = (i + 1) * cols + j;
					neighbor_pixel = (int)image.at<uchar>(i + 1, j);
					graph[current_index].push_back(Edge(neighbor_index, abs(current_pixel - neighbor_pixel)));
				}
				if (j != 0){
					neighbor_index = i * cols + j - 1;
					neighbor_pixel = (int)image.at<uchar>(i, j - 1);
					graph[current_index].push_back(Edge(neighbor_index, abs(current_pixel - neighbor_pixel)));
				}
				if (j != cols - 1){
					neighbor_index = i * cols + j + 1;
					neighbor_pixel = (int)image.at<uchar>(i, j + 1);
					graph[current_index].push_back(Edge(neighbor_index, abs(current_pixel - neighbor_pixel)));
				}
			}
			
		}

		
		vector<vector<Edge> > residual_graph(graph);  // this is deep copy of a vector
		cout << "The maximum possible flow is " << maxFlow(graph, pixel_number, pixel_number + 1, residual_graph) << endl;

		vector<int> A;
		findCut(residual_graph, pixel_number - 1, A);

		for (int i = 0; i < A.size(); ++i){
			if (A[i] < pixel_number){
				seg.at<Vec3b>((int)A[i] / cols, A[i] % cols) = Vec3b(125, 45, 178);
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



/* given an residual graph, a source vertex, find the cut*/
void findCut(vector<vector<Edge> >& graph, int s, vector<int>& A){

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
			if (visited[v] == false){
				q.push(v);
				A.push_back(v);
				visited[v] = true;
			}
		}
	}
}
/* given an adjacency matrix, a source vertex, and a sink vertex,
return a path if there exists one from s to t*/
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
			if (visited[v] == false){
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

			if (it->weight() == 0){

				residual_graph[u].erase(it);
			}

			it = residual_graph[v].begin();

			while (it != residual_graph[v].end()){
				if (it->vertex() != u) ++it;
				else break;
			}

			if (it == residual_graph[v].end()){
				residual_graph[v].push_back(Edge(u, path_flow));
			}
			else{
				it->change_weight(path_flow);
			}

		}
		max_flow += path_flow;
		cout << "Current max flow = " << max_flow << endl;

	}

	return max_flow;
}