#include "Graph.h"

class Maxflow_hpr_gap : public Graph{
private:

	//vector<queue<int> > actives;
	queue<int>* actives;

	int highest_level = 1;

	int *vertex_label_count;

	vector<int> active_list;

public:
	Maxflow_hpr_gap(Mat& image, const int PRECISION = 256, const double alpha = 1);
	~Maxflow_hpr_gap();

	void maxflow(int s, int t);

	void initialize(int s, int t);

	bool discharge(int u, int source, int sink);

	void push(int u, int sink);

	void relabel(int u);

	void gap(int u);

	int activeNode(int t);

	int findHighestLabel(int height);

};