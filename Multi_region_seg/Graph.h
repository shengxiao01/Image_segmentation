#include <vector>

using namespace std;

// undirected graph edge weight
struct Edge{   
	int weight = 0;
	int r_weight = 0;
};


// a disjoing set structure for storing vertices
class Vertex{                   
private:
	int excess;
	int height;
	vector<Edge*> edge;        // pointers to outgoing edge
	vector<Vertex*> neighbor;

	Vertex* parent = this;  // initiate each vertex with make set operation
	int rank = 0;   
public:
	Vertex() :excess(0), height(0){
		edge.reserve(4);
		neighbor.reserve(4);
	};

	Vertex(int d_excess, int d_height) : excess(d_excess), height(d_height){};

	void insert_edge();

	friend class Graph;
};

class Graph{
private:
	vector<Vertex> node;
	vector<Edge> edge;
	vector<Edge*> p_edge;   // pointers to the edge for sorting by weight
	vector<Edge>::iterator insert_edge_position;

public:
	Graph(int node_count, int edge_count){
		node = vector<Vertex> (node_count);
		edge = vector<Edge> (edge_count);
		p_edge = vector<Edge*> (edge_count, NULL);
		insert_edge_position = edge.begin();
	}

	void insertEdge(int u, int v, int weight){
		insert_edge_position->weight = weight;
		insert_edge_position->r_weight = weight;
		node[u].edge.push_back(&(*insert_edge_position));
		node[v].edge.push_back(&(*insert_edge_position));
		insert_edge_position++;
	}

	void linkNodes(Vertex u, Vertex v){
		if (u.rank > v.rank) {
			v.parent = &u;
		}
		else {
			u.parent = &v;
			if (u.rank == v.rank){
				v.rank += 1;
			}
		}
	}

	Vertex findNode(Vertex u){
		if (u.parent != &u){
			u.parent = &findNode(*u.parent);
		}
		return *u.parent;
	}

	void unionNodes(Vertex u, Vertex v){
		linkNodes(findNode(u), findNode(v));
	}





};