#include "Maxflow_pr.h"


Maxflow_pr::Maxflow_pr(Mat& image, const int PRECISION, const double alpha) : Graph(image, PRECISION, alpha) {

}

int Maxflow_pr::maxflow(int s, int t){
	initialize(s);

	int u = positiveExcessIdx(t);
	while (u != -1){
		//if (!push(u)){
		//	relabel(u);
		//}		
		discharge(u);
		u = positiveExcessIdx(t);
	}
	cout << graph_size << endl;
	return graph[t].excess;
}

void Maxflow_pr::initialize(int source){
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

int Maxflow_pr::positiveExcessIdx(int t){
	for (int i = 0; i < graph_size; ++i){
		if (graph[i].excess > 0 && i != t){
			return i;
		}
	}
	return -1;
}


bool Maxflow_pr::push(int u){
	for (auto it = graph[u].begin(); it != graph[u].end(); ++it){
		int v = it->first;
		if (it->second.r_weight > 0 && graph[u].height == graph[v].height + 1){
			int temp_flow = min(it->second.r_weight, graph[u].excess);

			it->second.r_weight -= temp_flow;
			graph[v][u].r_weight += temp_flow;
			graph[u].excess -= temp_flow;
			graph[v].excess += temp_flow;

			//return true;
		}
	}
	return false;
}

void Maxflow_pr::relabel(int u){
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

bool Maxflow_pr::discharge(int u){
	int i = 0;
	int sz = graph[u].edges.size();
	int temp_height = INT_MAX;

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

	return false;
}