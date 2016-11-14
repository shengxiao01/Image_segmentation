#include "Graph.h"

class Maxflow_fifo_gap: public Graph{
private:
	vector<int> vertex_label_count;

	vector<bool> active;

	queue<int> active_vertex;

public:
	Maxflow_fifo_gap(Mat& image, const int PRECISION = 256, const double alpha = 1);

	void maxflow(int s, int t);

	void initialize(int s, int t);

	void discharge(int u);

	void relabel(int u);

	void gap(int u);

	void enqueue(int u);

};