#include <vector>
#include <unordered_map>
#include <map>
#include <ctime>

using namespace std;

// undirected graph edge weight
struct Edge{   
	int weight;
	int r_weight;
	int vertex[2];
	Edge(){
		weight = 0;
		r_weight = 0;
	}
	Edge(int w, int v[2]){
		weight = w;
		r_weight = w;
		vertex[0] = v[0];
		vertex[1] = v[1];
	}
	Edge(int w, int r_w, int v[2]){
		weight = w;
		r_weight = r_w;
		vertex[0] = v[0];
		vertex[1] = v[1];
	}
};


// a disjoing set structure for storing vertices
class Vertex{                   
private:
	int excess;
	int height;

	vector<pair<int, Edge*> > edges;  // first = connected vertex, second = corresponding edge

	Vertex* parent = this;  // initiate each vertex with make set operation
	double int_diff = 0;    // internal difference of each set
	int set_size = 1;
	int rank = 0;   
public:
	Vertex(){
		height = 0;
		excess = 0;
		edges.reserve(6);
	}
	Vertex(int d_height, int d_excess){
		height = d_height;
		excess = d_excess;
		edges.reserve(16);
	}

	void insert_edge(int vertex, Edge* p_edge){

		edges.push_back(make_pair(vertex, p_edge));
	}

	Edge& operator[](int vertex_idx){
		vector<pair<int, Edge*>>::iterator it = lower_bound(edges.begin(), edges.end(),
			make_pair(vertex_idx, &Edge{0,0}),
			[](const pair<int, Edge*>& lhs, const pair<int, Edge*>& rhs)
		{
			return lhs.first < rhs.first;
		});
		return *((&(*it))->second);
	}

	void Vertex::sort_edge(){
		sort(edges.begin(), edges.end(), [](pair<int, Edge*> &left, pair<int, Edge*> &right) {
			return left.first < right.first;
		});
	}

	// find the root of each vertex
	Vertex* root(){
		if (parent != this){
			parent = parent->root();
		}
		return parent;
	}


	friend class Graph;
};

class Graph{
private:
	vector<Vertex> node;
	vector<Edge> edge;
	vector<Edge*> p_edge;   // pointers to the edge for sorting by weight

public:
	Graph(int node_count, int edge_count){
		node = vector<Vertex> (node_count);
		edge.reserve(edge_count);
		p_edge.reserve(edge_count);
	}

	void insertEdge(int u, int v, int weight){
		int vertex[2] = { u, v };
		edge.push_back(Edge(weight, vertex));
		Edge* p_temp = &(*(edge.end() - 1));
		p_edge.push_back(p_temp);
		node[u].insert_edge(v, p_temp);
		node[v].insert_edge(u, p_temp);
	}

	void sortEdge(){
		sort(p_edge.begin(), p_edge.end(), [](Edge* a, Edge* b) { return (*a).weight > (*b).weight; });
	}

	void segmentation(int K = 30){
		for (int i = 0; i < p_edge.size(); ++i){
			int u = (*p_edge[i]).vertex[0];
			int v = (*p_edge[i]).vertex[1];
			if (node[u].root() != node[v].root()){
				double min_int_diff = MInt(node[u], node[v], K);

				if ((double)(*p_edge[i]).weight < min_int_diff){
					unionNodes(node[u], node[v]);
					node[v].root()->int_diff = min_int_diff;
					node[v].root()->set_size = node[v].root()->set_size + node[v].root()->set_size;
					//cout << "Union nodes " << u << " and " << v << endl;
				}
			}
		}
	}

	double MInt(Vertex u, Vertex v,const double K){
		double c1 = u.root()->int_diff + K / (double)u.root()->set_size;
		double c2 = v.root()->int_diff + K / (double)v.root()->set_size;
		return min(c1, c2);
	}

	void linkNodes(Vertex& u, Vertex& v){
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

	void unionNodes(Vertex& u, Vertex& v){
		linkNodes(*u.root(), *v.root());
	}
	
	void printEdge(){
		int temp = 0;
		for (int i = 0; i < p_edge.size(); ++i){
			if ((*p_edge[i]).weight == temp){
				continue;
			}
			temp = (*p_edge[i]).weight;
			cout << temp << endl;
		}
	}

	void printSet(){
		int temp = 0;
		for (int i = 0; i < node.size(); ++i){
			int p = node[i].root() - &node[0];
			if (temp != p){
				cout << p << endl;
				temp = p;
			}
		}
	}

	map<int, vector<int> > showSegResult(){
		map<int, vector<int> > segs;
		int p;
		for (int i = 0; i < node.size(); ++i){
			p = node[i].root() - &node[0];
			segs[p].push_back(i);
		}
		return segs;
	}


};