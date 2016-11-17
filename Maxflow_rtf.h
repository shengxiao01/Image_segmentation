#ifndef MAXFLOW_RTF
#define MAXFLOW_RTF

#include "Graph.h"

class Maxflow_rtf : public Graph{
private:
	vector<int> active_vertex;
public:

	Maxflow_rtf(Mat& image, const int PRECISION = 256, const double alpha = 1);

	int maxflow(int s, int t);

	void initialize(int s);

	void discharge(int u);

	void relabel(int u);
};

#endif