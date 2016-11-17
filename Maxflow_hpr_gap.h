#include "Graph.h"

class Maxflow_hpr_gap : public Graph{
private:

	//vector<queue<int> > actives;
	queue<int>* active_node;

	int highest_level;

	vector<int> vertex_label_count;

	int operation_count = 0;


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


};