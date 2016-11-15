#include "Maxflow_hpr_gap.h"

Maxflow_hpr_gap::Maxflow_hpr_gap(Mat& image, const int PRECISION, const double alpha) : Graph(image, PRECISION, alpha) {
	
	vertex_label_count = vector<int>(graph_size+2, 0);
	actives = vector<queue<int>>(graph_size+2);

}


void Maxflow_hpr_gap::maxflow(int s, int t){
	initialize(s, t);

	int u = actives[highest_level].front();
	int cou = 0;
	while (highest_level != -1){
		
		u = actives[highest_level].front();
		actives[graph[u].height].pop();
		
		cou++;
		
		discharge(u, s, t);

	}
	cout << cou << endl;

}

void Maxflow_hpr_gap::initialize(int source, int sink){
	graph[source].height = graph_size;
	int sz = graph[source].edges.size();

	for (int i = 0; i < sz; ++i){
		int temp_flow = graph[source].edges[i].second.r_weight;
		if (temp_flow > 0){
			int end_vertex = graph[source].edges[i].first;

			graph[source].edges[i].second.r_weight -= temp_flow;
			graph[end_vertex][source].r_weight += temp_flow;
			graph[source].excess -= temp_flow;
			graph[end_vertex].excess += temp_flow;

			actives[0].push(end_vertex);  // if flow is pushed, end_vertex becomes active
			
			
		}
	}

	highest_level = 0;

}

bool Maxflow_hpr_gap::discharge(int u, int source, int sink){
	
	int i = 0;
	int sz = graph[u].edges.size();
	int temp_height = INT_MAX;
	
	for (int i = 0; i < sz && graph[u].excess > 0; ++i){
		int v = graph[u].edges[i].first;
		// push flow 
		if (graph[u].edges[i].second.r_weight > 0 && graph[u].height == graph[v].height + 1){
			int temp_flow = min(graph[u].edges[i].second.r_weight, graph[u].excess);

			graph[u].edges[i].second.r_weight -= temp_flow;
			graph[v][u].r_weight += temp_flow;
			graph[u].excess -= temp_flow;
			graph[v].excess += temp_flow;

			if (graph[v].excess == temp_flow && v != sink){
				actives[graph[v].height].push(v);
				
				
			}
		}
	}

	int old_height = graph[u].height;
	if (graph[u].excess > 0 && graph[u].height < graph_size){

			relabel(u);
			actives[graph[u].height].push(u);
			vertex_label_count[graph[u].height]++;
			highest_level = graph[u].height;
	}
	else{
		if (actives[graph[u].height].empty()){
			gap(graph[u].height);
			
			highest_level = -1;
			for (int h = graph[u].height - 1; h > -1; --h){
				if (!actives[h].empty()){
					highest_level = h;
					break;
				}
			}
		}
	}

	return false;
}


void Maxflow_hpr_gap::relabel(int u){
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

void Maxflow_hpr_gap::gap(int height){

	for (int i = 0; i < graph_size; ++i){
		if (graph[i].height >= height){
			graph[i].height = graph_size;
		}
	}

	for (int i = height; i < graph_size; ++i){
		while (!actives[i].empty()){
			actives[i].pop();
		}
	}
}

