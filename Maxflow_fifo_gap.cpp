#include "Maxflow_fifo_gap.h"

Maxflow_fifo_gap::Maxflow_fifo_gap(Mat& image, const int PRECISION, const double alpha) : Graph(image, PRECISION, alpha) {
	vertex_label_count = vector<int>(graph_size * 2, 0);
	active = vector<bool>(graph_size, false);
}


void Maxflow_fifo_gap::maxflow(int s, int t){

	initialize(s, t);
	int cou = 0;
	while (!active_vertex.empty()){
		cou++;
		int u = active_vertex.front();
		active_vertex.pop();
		active[u] = false;
		discharge(u);
		
	}
	cout << cou << endl;
	cout << "Gap operations: " << operation_count << endl;
	cout << "Max flow value: " << graph[t].excess << endl;
}

void Maxflow_fifo_gap::initialize(int s, int t){
	vertex_label_count[0] = graph_size - 1;
	vertex_label_count[graph_size] = 1;
	graph[s].height = graph_size;
	active[s] = true;
	active[t] = true;

	for (int i = 0; i < graph[s].edges.size(); ++i){
		int end_vertex = graph[s].edges[i].first;
		int temp_flow = graph[s].edges[i].second.r_weight;

		graph[s].edges[i].second.r_weight -= temp_flow;
		graph[end_vertex][s].r_weight += temp_flow;
		graph[s].excess -= temp_flow;
		graph[end_vertex].excess += temp_flow;

		active[end_vertex] = true;
		active_vertex.push(end_vertex);
	}
}

void Maxflow_fifo_gap::discharge(int u){
	int sz = graph[u].edges.size();
	for (int i = 0; graph[u].excess > 0 && i < sz; ++i) {
		int v = graph[u].edges[i].first;
		// push flow 
		if (graph[u].edges[i].second.r_weight > 0 && graph[u].height == graph[v].height + 1){
			int temp_flow = min(graph[u].edges[i].second.r_weight, graph[u].excess);

			graph[u].edges[i].second.r_weight -= temp_flow;
			graph[v][u].r_weight += temp_flow;
			graph[u].excess -= temp_flow;
			graph[v].excess += temp_flow;
			enqueue(v);
		}
	}

	if (graph[u].excess > 0) {
		if (vertex_label_count[graph[u].height] == 1){
			operation_count++;
			gap(u);
		}
		else{
			relabel(u);
		}
	}

}

void Maxflow_fifo_gap::relabel(int u){
	vertex_label_count[graph[u].height]--;
	graph[u].height = 2 * graph_size;
	int sz = graph[u].edges.size();
	int temp_height = INT_MAX;
	for (int i = 0; i < sz; i++){
		int v = graph[u].edges[i].first;
		if (graph[u].edges[i].second.r_weight > 0 && temp_height > graph[v].height){
			temp_height = graph[v].height;
		}
	}
	graph[u].height = temp_height + 1;
	vertex_label_count[graph[u].height]++;
	enqueue(u);
}

void Maxflow_fifo_gap::gap(int u){
	for (int i = 0; i < graph_size; ++i) {
		if (graph[i].height < graph[u].height) continue;
		vertex_label_count[graph[i].height]--;
		graph[i].height = max(graph[i].height, graph_size + 1);
		vertex_label_count[graph[i].height]++;
		enqueue(i);
	}
}

void Maxflow_fifo_gap::enqueue(int u) {
	if (!active[u] && graph[u].excess > 0) {
		active[u] = true;
		active_vertex.push(u);
	}
}