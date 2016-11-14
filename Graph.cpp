#include "Graph.h"

Graph::Graph(int d_size){
	graph = vector<Vertex>(d_size);
	graph_size = d_size;
}

Graph::Graph(Mat& image, const int PRECISION, const double alpha){
	const int rows = image.rows;
	const int cols = image.cols;
	const int pixel_number = rows * cols;


	Mat labels, probs, means;
	vector<Mat> covs;
	guassMixModel(image, labels, probs, means, covs);

	log(probs, probs);       // turn linear probability to logrithmic scale
	probs = -PRECISION * probs;
	double sigma = estimateNoise(image);
	//probs.convertTo(probs, 8, 255, 0);
	double min, max;
	minMaxLoc(probs, &min, &max);
	probs = (probs - min) * (256 / (max - min));

	Mat display;
	probs.convertTo(display, CV_8UC1, 255.0, 0);
	//applyColorMap(display, display, cv::COLORMAP_JET);
	//imshow("imagesc", display);


	graph = vector<Vertex>(pixel_number + 2);
	graph_size = graph.size();

	for (int i = 0; i < rows; ++i){

		for (int j = 0; j < cols; ++j){

			Vec3b current_pixel = image.at<Vec3b>(i, j);

			int current_index = i * cols + j;
			int neighbor_index;
			Vec3b neighbor_pixel;
			int penality;

			if (i != rows - 1){
				neighbor_index = (i + 1) * cols + j;
				neighbor_pixel = image.at<Vec3b>(i + 1, j);

				penality = (int)PRECISION * alpha * neighbourPenality(current_pixel, neighbor_pixel, sigma);
				insert_edge(current_index, neighbor_index, penality);
				insert_edge(neighbor_index, current_index, penality);
			}
			if (j != cols - 1){

				neighbor_index = i * cols + j + 1;
				neighbor_pixel = image.at<Vec3b>(i, j + 1);

				penality = (int)PRECISION * alpha * neighbourPenality(current_pixel, neighbor_pixel, sigma);
				insert_edge(current_index, neighbor_index, penality);
				insert_edge(neighbor_index, current_index, penality);
			}

			insert_edge(pixel_number, current_index, (int)probs.at<double>(current_index, 0));   // an edge from source
			insert_edge(current_index, pixel_number, 0);// an edge from current pixel to source with 0 capacity
			insert_edge(current_index, pixel_number + 1, (int)probs.at<double>(current_index, 1));// an edge to sink
			insert_edge(pixel_number + 1, current_index, 0); // an edge from sink to current_pixel with 0 capacities  

		}

	}

	for (int i = 0; i < rows; ++i){
		for (int j = 0; j < cols; ++j){
			graph[i*cols + j].sort_edge();
		}
	}


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

vector<int> Graph::BFSCut(int s){
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

vector<int> Graph::HeightCut(){
	vector<int> cut;
	for (int i = 0; i < graph_size - 2; ++i){
		if (graph[i].height >= graph_size){
			cut.push_back(i);
		}
	}
	return cut;
}