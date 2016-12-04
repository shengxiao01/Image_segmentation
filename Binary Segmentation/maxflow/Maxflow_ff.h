#ifndef MAXFLOW_FF
#define MAXFLOW_FF

#include "Graph.h"

class Maxflow_ff : public Graph{

public:
	Maxflow_ff(Mat& image, const int PRECISION = 256, const double alpha = 1);

	int maxflow(int s, int t);

	bool BFS(int s, int t, vector<int>& path);

};

#endif