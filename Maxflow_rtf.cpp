#include "Maxflow_rtf.h"

Maxflow_rtf::Maxflow_rtf(Mat& image, const int PRECISION, const double alpha) : Graph(image, PRECISION, alpha) {
	active_vertex = vector<int>(graph_size - 2);
}


int Maxflow_rtf::maxflow(int s, int t){
	initialize(s);

	int sz = active_vertex.size();
	for (int i = sz - 1; i != -1; --i){
		active_vertex[i] = sz - 1 - i;       // store the active vertex index backwards in a vector<int>
	}

	for (int i = sz - 1; i != -1;){
		int u = active_vertex[i];
		int height = graph[u].height;
		discharge(u);
		if (graph[u].height > height){
			active_vertex.erase(active_vertex.begin() + i);
			active_vertex.push_back(u);
			i = sz - 3;
		}
		--i;
	}

	return graph[t].excess;
}

void Maxflow_rtf::initialize(int source){
	graph[source].height = graph.size();
	int sz = graph[source].edges.size();
	for (int i = 0; i < sz; ++i){
		int v = graph[source].edges[i].first;
		int temp_flow = graph[source][v].r_weight;

		graph[source].edges[i].second.r_weight -= temp_flow;
		graph[v][source].r_weight += temp_flow;
		graph[source].excess -= temp_flow;
		graph[v].excess += temp_flow;
	}

}


void Maxflow_rtf::discharge(int u){
	int i = 0;
	int sz = graph[u].edges.size();

	while (graph[u].excess > 0){

		if (i == sz){
			relabel(u);
			i = 0;
		}

		else{
			int v = graph[u].edges[i].first;
			// push flow 
			if (graph[u].edges[i].second.r_weight > 0 && graph[u].height == graph[v].height + 1){
				int temp_flow = min(graph[u].edges[i].second.r_weight, graph[u].excess);

				graph[u].edges[i].second.r_weight -= temp_flow;
				graph[v][u].r_weight += temp_flow;
				graph[u].excess -= temp_flow;
				graph[v].excess += temp_flow;
			}
			++i;
		}
	}

}


void Maxflow_rtf::relabel(int u){
	int temp_height = INT_MAX;
	int sz = graph[u].edges.size();

	for (int i = 0; i < sz; ++i){
		int v = graph[u].edges[i].first;
		if (graph[u].edges[i].second.r_weight > 0 && temp_height > graph[v].height){
			temp_height = graph[v].height;
		}
	}

	graph[u].height = temp_height + 1;
}