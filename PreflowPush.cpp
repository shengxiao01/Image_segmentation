#include "PreflowPush.h"

Vertex::Vertex(){
	height = 0;
	excess = 0;
}
Vertex::Vertex(int d_height, int d_excess){
	height = d_height;
	excess = d_excess;
}

void Vertex::insert_edge(int vertex, int weight){
	edges[vertex] = Edge2{ weight, weight };
}

Edge2& Vertex::operator[](int vertex_idx){
	return edges[vertex_idx];
}

map<int, Edge2>::iterator Vertex::begin(){
	return edges.begin();
}

map<int, Edge2>::iterator Vertex::end(){
	return edges.end();
}



Graph::Graph(int size){
	graph = vector<Vertex>(size);
}

void Graph::insert_edge(int start, int end, int weight){
	graph[start].insert_edge(end, weight);
}

void Graph::addFlow(int u, int v, int flow){

	graph[u][v].r_weight -= flow;
	graph[v][u].r_weight += flow;
	graph[u].excess -= flow;
	graph[v].excess += flow;
}


//initialize push-rebel algorithm
void Graph::initialize(int source){
	graph[source].height = graph.size();

	for (auto it = graph[source].begin(); it != graph[source].end(); ++it){
		int end_vertex = it->first;
		int temp_flow = graph[source][end_vertex].r_weight;


		it->second.r_weight -= temp_flow;
		graph[end_vertex][source].r_weight += temp_flow;
		graph[source].excess -= temp_flow;
		graph[end_vertex].excess += temp_flow;

	}
}


int Graph::positiveExcessIdx(int t){
	for (int i = 0; i < graph.size(); ++i){
		if (graph[i].excess > 0 && i != t){
			return i;
		}
	}
	return -1;
}

bool Graph::push(int u){
	for (auto it = graph[u].begin(); it != graph[u].end(); ++it){
		int v = it->first;
		if (it->second.r_weight > 0 && graph[u].height > graph[v].height){
			int temp_flow = min(it->second.r_weight, graph[u].excess);

			it->second.r_weight -= temp_flow;
			graph[v][u].r_weight += temp_flow;
			graph[u].excess -= temp_flow;
			graph[v].excess += temp_flow;
		}
	}
	return false;
}

void Graph::relabel(int u){
	int temp_height = INT_MAX;
	for (auto it = graph[u].begin(); it != graph[u].end(); ++it){
		int v = it->first;
		if (it->second.r_weight > 0 && temp_height > graph[v].height){
			temp_height = graph[v].height;
		}
	}
	graph[u].height = temp_height + 1;
}


// Find the max flow of the graph
int Graph::maxFlow(int s, int t){
	initialize(s);
	int u = positiveExcessIdx(t);
	while (u != -1){
		if (!push(u)){
			relabel(u);
		}
		u = positiveExcessIdx(t);
	}
	return graph[t].excess;
}

// Find an s-t cut of the graph according to the residual flow
vector<int> Graph::findCut(int s){
	vector<int> cut;

	const int vertex_count = graph.size();   // the number vertex in the graph

	vector<bool> visited(vertex_count, false);    // flag if a verex has been visited

	queue<int> q;      //  a queue for current vertex index
	q.push(s);
	visited[s] = true;

	while (!q.empty()){
		int u = q.front();     // current scanning vertex
		q.pop();

		for (auto it = graph[u].begin(); it != graph[u].end(); ++it){

			int v = it->first;     //(u,v) is the current scanning edge
			if (it->second.r_weight > 0 && visited[v] == false){
				q.push(v);
				cut.push_back(v);
				visited[v] = true;
			}
		}
	}
	return cut;
}