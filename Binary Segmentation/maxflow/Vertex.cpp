#include "Vertex.h"

Vertex::Vertex(){
	height = 0;
	excess = 0;
	edges.reserve(6);
}
Vertex::Vertex(int d_height, int d_excess){
	height = d_height;
	excess = d_excess;
	edges.reserve(6);
}

void Vertex::insert_edge(int vertex, int weight){
	edges.push_back(make_pair(vertex, Edge{ weight, weight }));
}

Edge& Vertex::operator[](int vertex_idx){

	vector<pair<int, Edge>>::iterator it = lower_bound(edges.begin(), edges.end(),
		make_pair(vertex_idx, Edge{ 0, 0 }),
		[](const pair<int, Edge>& lhs, const pair<int, Edge>& rhs)
	{
		return lhs.first < rhs.first;
	});
	return it->second;
}

vector<pair<int, Edge> >::iterator Vertex::begin(){
	return edges.begin();
}

vector<pair<int, Edge> >::iterator Vertex::end(){
	return edges.end();
}


void Vertex::sort_edge(){
	sort(edges.begin(), edges.end(), [](pair<int, Edge> &left, pair<int, Edge> &right) {
		return left.first < right.first;
	});
}