#include "Graph.h"

class Maxflow_hpr_gap : public Graph{
private:

	vector<queue<int> > actives;

	int highest_level = 1;

	vector<int> vertex_label_count;

	

public:
	Maxflow_hpr_gap(Mat& image, const int PRECISION = 256, const double alpha = 1);

	void maxflow(int s, int t);

	void initialize(int s, int t);

	bool discharge(int u, int source, int sink);

	bool push(int u);

	void relabel(int u);

	void gap(int u);

	int activeNode(int t);

};