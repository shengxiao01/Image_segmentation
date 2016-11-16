#ifndef GRAPH_H
#define GRAPH_H


#include <vector>
#include <queue>

#include "Vertex.h"
#include "GaussModel.h"


class Graph{
protected:
	vector<Vertex> graph;
	int graph_size;

public:
	Graph(int d_size);

	Graph(Mat& image, const int PRECISION = 256, const double alpha = 1);

	void insert_edge(int start, int end, int weight);

	void addFlow(int u, int v, int flow);

	vector<int> HeightCut();

	vector<int> BFSCut(int s);

};

#endif