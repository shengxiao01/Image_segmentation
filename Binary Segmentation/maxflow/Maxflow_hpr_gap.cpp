#include "Maxflow_hpr_gap.h"

Maxflow_hpr_gap::Maxflow_hpr_gap(Mat& image, const int PRECISION, const double alpha) : Graph(image, PRECISION, alpha) {
	
	active_node = new queue<int>[graph_size + 2];
	vertex_label_count = vector<int>(graph_size *2, 0);
}


Maxflow_hpr_gap::~Maxflow_hpr_gap(){
	delete[] active_node;
}


int Maxflow_hpr_gap::maxflow(int s, int t){
	initialize(s, t);


	int u = active_node[highest_level].front();
	int discharge_count = 0;

	while (!active_node[highest_level].empty()){
		discharge_count++;
		u = active_node[highest_level].front();
		active_node[highest_level].pop();		
		
		discharge(u, s, t);
	}
	cout << discharge_count << endl;
	cout << "Gap operations: " << operation_count << endl;
	cout << "Max flow value: " << graph[t].excess << endl;
	return graph[t].excess;
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

			active_node[1].push(end_vertex);  // if flow is pushed, end_vertex becomes active

		}
		graph[end_vertex].height = 1;
		
	}
	vertex_label_count[1] = graph_size - 2;

	highest_level = 1;

}

bool Maxflow_hpr_gap::discharge(int u, int source, int sink){
	
	Vertex& current_node = graph[u];

	//push(current_node, sink);
	int sz = current_node.edges.size();
	int temp_height = INT_MAX;

	for (int i = 0; i < sz && current_node.excess > 0; ++i){
		int v = current_node.edges[i].first;
		// push flow 
		if (current_node.edges[i].second.r_weight > 0 && current_node.height == graph[v].height + 1){
			int temp_flow = min(current_node.edges[i].second.r_weight, graph[u].excess);
			current_node.edges[i].second.r_weight -= temp_flow;
			graph[v][u].r_weight += temp_flow;
			current_node.excess -= temp_flow;
			graph[v].excess += temp_flow;
			if (graph[v].excess == temp_flow && v != sink){
				active_node[graph[v].height].push(v);

			}
		}
	}


	int old_height = current_node.height;
	if (current_node.excess > 0){
		
		if (vertex_label_count[old_height] == 1){
			gap(old_height);
			operation_count++;
		}
		else{
			vertex_label_count[old_height]--;
			relabel(current_node);

			if (current_node.height < graph_size){
				vertex_label_count[current_node.height]++;
				active_node[current_node.height].push(u);
				highest_level = current_node.height;
			}
		}
		
	}
	

	while (active_node[highest_level].empty() && highest_level > 0){
		highest_level--;
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
			if (graph[v].excess == temp_flow && v != sink){
					active_node[graph[v].height].push(v);

			}
		}
	}
}

void Maxflow_hpr_gap::relabel(Vertex& node){
	int temp_height = INT_MAX;
	int sz = node.edges.size();
	
	for (int i = 0; i < sz; ++i){
		int v = node.edges[i].first;
		if (node.edges[i].second.r_weight > 0 && temp_height > graph[v].height){
			temp_height = graph[v].height;
		}
	}
	node.height = temp_height + 1;
	
}



void Maxflow_hpr_gap::gap(int height){

	for (int i = 0; i < graph_size; ++i){
		int& cureent_vertex_height = graph[i].height;
		if (cureent_vertex_height >= height){		

			vertex_label_count[cureent_vertex_height]--;
			cureent_vertex_height = graph_size;
		}
	}

}
