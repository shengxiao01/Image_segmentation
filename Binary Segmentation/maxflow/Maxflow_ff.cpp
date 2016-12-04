
#include "Maxflow_ff.h"


Maxflow_ff::Maxflow_ff(Mat& image, const int PRECISION, const double alpha) : Graph(image, PRECISION, alpha) {

}

int Maxflow_ff::maxflow(int s, int t){

	vector<int> path(graph_size, -2);

	int max_flow = 0;
	int u, v;
	
	while (BFS(s, t, path)){   // while there is a path from source to sink
		int path_flow = INT_MAX;
		// find an augmenting path and calculate the flow
		for (v = t; v != s; v = path[v]){
			u = path[v];
			path_flow = min(path_flow, graph[u][v].r_weight);
		}
		// calculate the residual flow of the graph
		for (v = t; v != s; v = path[v]){
			u = path[v];

			graph[u][v].r_weight -= path_flow;
			graph[v][u].r_weight += path_flow;
		}
		max_flow += path_flow;
	}

	return max_flow;
}
bool Maxflow_ff::BFS(int s, int t, vector<int>& path){

	const int vertex_count = graph.size();   // the number vertex in the graph

	vector<bool> visited(vertex_count, false);    // flag if a verex has been visited

	queue<int> q;      //  a queue for current vertex index
	q.push(s);
	visited[s] = true;
	path[s] = -1;
	while (!q.empty()){

		int u = q.front();     // current scanning vertex
		q.pop();
		for (auto it = graph[u].edges.begin(); it != graph[u].edges.end(); ++it){

			int v = it->first;     //(u,v) is the current scanning edge
			if (v < 0) { continue; }
			if (visited[v] == false && it->second.r_weight > 0){
				q.push(v);
				path[v] = u;
				visited[v] = true;
			}
		}
	}
	return visited[t];   // if the sink vertex has been visited 
}


