




class Edge{
	int d_vertex;   // end vertex

	int d_weight;   // weight

public:
	Edge(int vertex, int weight) : d_weight(weight), d_vertex(vertex) {}

	int vertex() const { return d_vertex; }

	int weight() const { return d_weight; }

	void change_weight(int delta) {
		d_weight = d_weight + delta;
	}
};
