#include "Maxflow_hpr_gap.h"

Maxflow_hpr_gap::Maxflow_hpr_gap(Mat& image, const int PRECISION, const double alpha) : Graph(image, PRECISION, alpha) {
	
	//vertex_label_count = vector<int>(graph_size+2, 0);
	vertex_label_count = new int[graph_size + 2]{0};
	actives = new queue<int> [graph_size+2];

	active_list.reserve(graph_size);
}
Maxflow_hpr_gap::~Maxflow_hpr_gap(){
	delete[] actives;
	delete[] vertex_label_count;
}


void Maxflow_hpr_gap::maxflow(int s, int t){
	initialize(s, t);


	int u = actives[highest_level].front();
	int cou = 0;
	while (highest_level != -1){
		
		u = actives[highest_level].front();
		actives[graph[u].height].pop();
		vertex_label_count[graph[u].height]--;
		
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
			active_list.push_back(end_vertex);
			vertex_label_count[0]++;
		}
	}

	highest_level = 0;

}

bool Maxflow_hpr_gap::discharge(int u, int source, int sink){
	

	push(u, sink);


	if (graph[u].excess > 0){

			relabel(u);
			if (graph[u].height >= graph_size){
				highest_level = findHighestLabel(graph_size - 1);
			}
			else{
				actives[graph[u].height].push(u);
				vertex_label_count[graph[u].height]++;
				highest_level = graph[u].height;
			}
	}
	else{
		if (actives[graph[u].height].empty()){
			gap(graph[u].height);
			highest_level = findHighestLabel(graph[u].height - 1);
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

			if (graph[v].excess == temp_flow && v != sink){
				actives[graph[v].height].push(v);
				active_list.push_back(v);
				vertex_label_count[graph[v].height]++;

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

	for (int i = 0; i < active_list.size();){
		if (graph[active_list[i]].height >= height){
			graph[active_list[i]].height = graph_size;
			active_list.erase(active_list.begin() + i);
		}
		else{
			++i;
		}
	}


	//for (int i = height; i < graph_size; ++i){
		//int& count = vertex_label_count[i];
		
		//while (!actives[i].empty()){
		//	cout << 11 << endl;
		//	actives[i].pop();
		//	vertex_label_count[i]--;
		//}
	//}

}

int Maxflow_hpr_gap::findHighestLabel(int height){
	int temp_height = -1;
	for (int h = height; h > -1; --h){

		while (!actives[h].empty()){
				temp_height = h;
				break;	
		}
	}
	/*
	for (int h = height; h > -1; --h){
		if (!actives[h].empty()){
			temp_height = h;
			break;
		}
	}
	*/
	return temp_height;
}