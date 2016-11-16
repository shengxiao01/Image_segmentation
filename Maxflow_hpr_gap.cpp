#include "Maxflow_hpr_gap.h"

Maxflow_hpr_gap::Maxflow_hpr_gap(Mat& image, const int PRECISION, const double alpha) : Graph(image, PRECISION, alpha) {
	
	active_node = new queue<int>[graph_size + 2];

	active_list.reserve(graph_size);
	active = vector<bool>(graph_size, false);
}


Maxflow_hpr_gap::~Maxflow_hpr_gap(){
	delete[] active_node;
}


void Maxflow_hpr_gap::maxflow(int s, int t){
	initialize(s, t);


	int u = active_node[highest_level].front();
	int cou = 0;

	while (!active_node[highest_level].empty()){

		u = active_node[highest_level].front();

		active_node[highest_level].pop();

		active[u] = false;
		
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
		int end_vertex = graph[source].edges[i].first;
		if (temp_flow > 0){
			

			graph[source].edges[i].second.r_weight -= temp_flow;
			graph[end_vertex][source].r_weight += temp_flow;
			graph[source].excess -= temp_flow;
			graph[end_vertex].excess += temp_flow;

			active_node[0].push(end_vertex);  // if flow is pushed, end_vertex becomes active
			active_list.push_back(end_vertex);
			active[end_vertex] = true;
		}
		
	}

	highest_level = 0;

}

bool Maxflow_hpr_gap::discharge(int u, int source, int sink){
	
	push(u, sink);

	if (graph[u].excess > 0){

			relabel(u);

			if (graph[u].height >= graph_size){
				while (active_node[highest_level].empty() && highest_level >0){
					highest_level--;
				}
			}
			else{
				active_node[graph[u].height].push(u);
				highest_level = graph[u].height;
				active[u] = true;
			}
	}
	else{
		if (active_node[graph[u].height].empty()){
			//gap(graph[u].height);
			graph[u].height = graph_size;
			while (active_node[highest_level].empty() && highest_level > 0){
				highest_level--;
			}
		}
	}


	return false;
}


void Maxflow_hpr_gap::push(int u, int sink){
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

			if (v != sink){
				if (!active[v]){
					active_node[graph[v].height].push(v);
					active_list.push_back(v);
					active[v] = true;
				}
			}
		}
	}
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
	
	//for (int i = 0; i < active_list.size();){
	//	if (graph[active_list[i]].height > height){
	//		
	//		graph[active_list[i]].height = graph_size;
	//		active_list.erase(active_list.begin() + i);
	//	}
	//	else{
	//		++i;
	//	}
	//}

	for (int i = 0; i < graph_size; ++i){
		if (graph[i].height >= height){
			active[i] = false;
			graph[i].height = graph_size;
		}
	}

}
