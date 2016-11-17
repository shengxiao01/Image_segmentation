#ifndef MAXFLOW_PR
#define MAXFLOW_PR

#include "Graph.h"

class Maxflow_pr : public Graph{

public:
	Maxflow_pr(Mat& image, const int PRECISION = 256, const double alpha = 1);

	int maxflow(int s, int t);

	void initialize(int s);

	int positiveExcessIdx(int t);

	bool push(int u);

	void relabel(int u);

	bool discharge(int u);

};

#endif