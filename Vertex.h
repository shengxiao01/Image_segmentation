#ifndef VERTEX_H
#define VERTEX_H


#include <vector>
#include <utility>
#include <algorithm>

using namespace std;

struct Edge{
	int weight;   // original capacity
	int r_weight; // residual capacity
};


class Vertex{
private:

	//map<int, Edge2> edges;
	vector<pair<int, Edge> > edges;
	int height;        // height of each vertex
	int excess;        // excess flow of each vertex

public:

	Vertex();

	Vertex(int d_height, int d_excess);

	void insert_edge(int vertex, int weight);

	Edge& operator[](int vertex_idx);

	vector<pair<int, Edge> >::iterator begin();

	vector<pair<int, Edge> >::iterator end();

	void sort_edge();

	friend class Graph;
	friend class Maxflow_fifo_gap;
	friend class Maxflow_rtf;
	friend class Maxflow_pr;

};

#endif