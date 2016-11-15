#include "Maxflow_hpr_gap.h"

Maxflow_hpr_gap::Maxflow_hpr_gap(Mat& image, const int PRECISION, const double alpha) : Graph(image, PRECISION, alpha) {
	
	vertex_label_count = vector<int>(graph_size * 2, 0);
	actives = vector<queue<int>>(graph_size*2);

}


void Maxflow_hpr_gap::maxflow(int s, int t){
	initialize(s, t);
	int u = activeNode();
	while (u != -1){
		cout << "discharge " << u <<" with excess "<< graph[u].excess<< " with height " << graph[u].height <<endl;
		if (!discharge(u, s, t)){
			relabel(u);
		}
		cout << 1 << endl;
		u = activeNode();
		cout << 2 << endl;
	}
	
}

void Maxflow_hpr_gap::initialize(int s, int t){
	max_label = graph_size;
	min_label = 0;

	int sz = graph[s].edges.size();
	for (int i = 0; i < sz; ++i){
		int v = graph[s].edges[i].first;
		int flow = graph[s].edges[i].second.r_weight;
		if (flow > 0){
			graph[s].edges[i].second.r_weight -= flow;
			graph[s].excess -= flow;
			graph[v][s].r_weight += flow;
			graph[v].excess += flow;

			graph[v].height = 1;
			actives[1].push(v);
		}
		

	}

}

bool Maxflow_hpr_gap::discharge(int u, int source, int sink){
	
	bool eligible = true;
	while (graph[u].excess > 0 && eligible){
		
		eligible = false;
		int sz = graph[u].edges.size();
		for (int i = 0; i < sz; ++i){

			int v = graph[u].edges[i].first;
			if (graph[u].height > graph[v].height && graph[u].edges[i].second.r_weight > 0){
				
				int flow = min(graph[u].excess, graph[u].edges[i].second.r_weight);
				cout << "push " << flow << endl;
				if (graph[v].excess == 0 && v != source && v != sink){
					actives[graph[v].height].push(v);
				}
				graph[u].edges[i].second.r_weight -= flow;
				graph[v][u].r_weight += flow;
				graph[u].excess -= flow;
				graph[v].excess += flow;

				eligible = true;

				break;
			}
		}
	}
	cout << actives[graph[u].height].size() << " " << graph[u].excess << endl;
	if (graph[u].excess == 0){
		cout << "pop " << u << endl;
		actives[graph[u].height].pop();
	}
	
	return eligible;
}

void Maxflow_hpr_gap::relabel(int u){
	cout << "relabel " << u << endl;
	int temp_height = INT_MAX;
	int old_height = graph[u].height;

	actives[graph[u].height].pop();
	int sz = graph[u].edges.size();
	for (int i = 0; i < sz; ++i){
		int v = graph[u].edges[i].first;
		if (graph[u].edges[i].second.r_weight > 0 && temp_height > graph[v].height){
			temp_height = graph[v].height;
		}
	}
	graph[u].height = temp_height + 1;
	actives[graph[u].height].push(u);
	cout << "pushed " << u << " to level " << graph[u].height << " now have size " << actives[graph[u].height].size() << endl;

	if (graph[u].height > max_label){
		max_label = graph[u].height;
	}

	//if (actives[old_height].empty() && old_height < graph_size){
	//	gap(old_height);
	//}
	cout << "finish relabel ..." << endl;
}

void Maxflow_hpr_gap::gap(int height){
	cout << "gap..." << endl;
	for (int i = height; i < graph_size; ++i){
		while (!actives[i].empty()){
			int v = actives[i].front();
			actives[i].pop();
			graph[v].height = graph_size;
			actives[graph_size].push(v);
		}
	}
	if (max_label < graph_size){
		max_label = graph_size;
	}
	
}

int Maxflow_hpr_gap::activeNode(){
	for (int i = graph_size*2-1; i >= min_label; --i){
		if (!actives[i].empty()){
			return actives[i].front();
		}
		max_label = i;
	}
	return -1;
}