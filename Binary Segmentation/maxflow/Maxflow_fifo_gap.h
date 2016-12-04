#ifndef MAXFLOW_FIFO_GAP_H
#define MAXFLOW_FIFO_GAP_H

#include "Graph.h"

class Maxflow_fifo_gap: public Graph{
private:
	vector<int> vertex_label_count;

	vector<bool> active;

	queue<int> active_vertex;
	int operation_count = 0;

public:
	Maxflow_fifo_gap(Mat& image, const int PRECISION = 256, const double alpha = 1);

	int maxflow(int s, int t);

	void initialize(int s, int t);

	void discharge(int u);

	void relabel(int u);

	void gap(int u);

	void enqueue(int u);

};

#endif