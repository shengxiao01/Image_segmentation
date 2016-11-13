#include "PreflowPush.h"

Vertex::Vertex(){
	height = 0;
	excess = 0;
	edges.reserve(6);
}
Vertex::Vertex(int d_height, int d_excess){
	height = d_height;
	excess = d_excess;
	edges.reserve(6);
}

void Vertex::insert_edge(int vertex, int weight){
	//edges[vertex] = Edge2{ weight, weight };
	edges.push_back(make_pair(vertex, Edge2{ weight, weight }));
}

Edge2& Vertex::operator[](int vertex_idx){

	vector<pair<int, Edge2>>::iterator it = lower_bound(edges.begin(), edges.end(),
		make_pair(vertex_idx, Edge2{ 0, 0 }),
		[](const pair<int, Edge2>& lhs, const pair<int, Edge2>& rhs)
	{
		return lhs.first < rhs.first;
	});
	return it->second;
}

vector<pair<int, Edge2> >::iterator Vertex::begin(){
	return edges.begin();
}

vector<pair<int, Edge2> >::iterator Vertex::end(){
	return edges.end();
}


void Vertex::sort_edge(){
	sort(edges.begin(), edges.end(), [](pair<int, Edge2> &left, pair<int, Edge2> &right) {
		return left.first < right.first;
	});
}

Graph::Graph(int d_size){
	graph = vector<Vertex>(d_size);
	graph_size = d_size;
}
void Graph::guassMixModel(Mat& image, Mat& labels, Mat& probs, Mat& means, vector<Mat>& covs){

	Mat samples = image.reshape(1, image.rows * image.cols);

	Ptr<EM> em_model = EM::create();
	em_model->setClustersNumber(2);
	em_model->setCovarianceMatrixType(EM::COV_MAT_SPHERICAL);
	em_model->setTermCriteria(TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 300, 0.1));
	em_model->trainEM(samples, noArray(), labels, probs);

	em_model->getCovs(covs);    // covariance matrices of each cluster

	means = em_model->getMeans();  // means of each cluster
}

inline double Graph::neighbourPenality(Vec3b x, Vec3b y, double sigma){
	//cout << "x: " << x << " y: " << y << " sigma: " << sigma << endl;
	double sqrt_sum = pow(((double)x[0] - (double)y[0]), 2)
		+ pow(((double)x[1] - (double)y[1]), 2)
		+ pow(((double)x[2] - (double)y[2]), 2);
	return exp(-sqrt_sum / (3 * pow(sigma, 2)));
	//return exp(-pow(((double)x - (double)y), 2) / 3);
}

double Graph::estimateNoise(Mat& image){
	Mat x_filter = (Mat_<double>(2, 1) << 1, -1);
	Mat y_filter = (Mat_<double>(1, 2) << 1, -1);
	Mat x_grad, y_grad;
	filter2D(image, x_grad, 64, x_filter, Point(-1, -1), 0, BORDER_REPLICATE);
	filter2D(image, y_grad, 64, y_filter, Point(-1, -1), 0, BORDER_REPLICATE);
	x_grad = abs(x_grad);
	y_grad = abs(y_grad);
	double sigma = sum(x_grad + y_grad)[0];
	sigma = sigma / (2 * image.rows*image.cols);

	return sigma;
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


			//cout << (int)probs.at<double>(current_index, 0) << "  " << PRECISION * alpha *neighbourPenality(current_pixel, neighbor_pixel, sigma) << "  " << alpha <<endl;
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


//initialize push-rebel algorithm
void Graph::initialize(int source){
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


int Graph::positiveExcessIdx(int t){
	for (int i = 0; i < graph_size; ++i){
		if (graph[i].excess > 0 && i != t){
			return i;
		}
	}
	return -1;
}

int Graph::ActiveNodeIdx(int t){
	for (int i = 0; i < graph_size; ++i){
		if (graph[i].excess > 0 && graph[i].height < graph_size && i != t){
			return i;
		}
	}
	return -1;
}


vector<int> Graph::Cut(){
	vector<int> cut;

	for (int i = 0; i < graph_size - 2; ++i){
		if (graph[i].height >= graph_size){
			cut.push_back(i);
		}
	}
	return cut;
}

bool Graph::push(int u){
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

void Graph::relabel(int u){
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

bool Graph::discharge(int u){
	
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
// push relabel algorithm
int Graph::maxFlow_pr(int s, int t){
	initialize(s);
	int u = positiveExcessIdx(t);

	while (u != -1){
		//if (!push(u)){
		//	relabel(u);
		//}
		discharge(u);
		u = positiveExcessIdx(t);
	}
	
	return graph[t].excess;
}

// relabel to front algorithm
int Graph::maxFlow_rtf(int s, int t){
	initialize(s);
	int u = positiveExcessIdx(t);

	vector<int> active_vertex(graph.size() - 2);
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

int Graph::maxFlow_hpr(int s, int t){
	vector<vector<int> > actives(graph_size);
	int highest_level = 1;
	vector<int> next_level(graph_size);
	for (int i = 0; i < graph_size; ++i){
		next_level[i] = i;
	}

	// initialize and push preflow
	for (auto it = graph[s].begin(); it != graph[s].end(); ++it){
		int end_vertex = it->first;
		int temp_flow = graph[s][end_vertex].r_weight;

		it->second.r_weight -= temp_flow;
		graph[end_vertex][s].r_weight += temp_flow;
		graph[s].excess -= temp_flow;
		graph[end_vertex].excess += temp_flow;

		actives[highest_level - 1].push_back(end_vertex);
		next_level[highest_level] = highest_level - 1;
	}

	while (!actives[highest_level].empty()){
		int u = actives[highest_level][0];
		int height = graph[u].height;
		
		discharge(u);

		actives[highest_level].erase(actives[highest_level].begin());  // vertex has been fully discharged and shoould be removed

		if (graph[u].height > height){  // vertex u has been relabelled
			if (graph[u].height < graph_size){
				actives[graph[u].height].push_back(u);
				if (actives[highest_level].empty()){
					next_level[graph[u].height] = next_level[highest_level];
				}
				else{
					next_level[graph[u].height] = highest_level;
				}
			}
			else{
				if (actives[highest_level].empty()){
					highest_level = next_level[highest_level];
				}
			}
		}
	}


	return graph[t].excess;

}

bool Graph::discharge_hpr(int u, vector<vector<int> >& actives, int& highest_level, vector<int>& next_level){

	while (graph[u].excess > 0){
		int temp_height = INT_MAX;
		int sz = graph[u].edges.size();
		for (int i = 0; i < sz; ++i){
			int v = graph[u].edges[i].first;
			// push flow 
			if (graph[u].edges[i].second.r_weight > 0 && graph[u].height == graph[v].height+1){
				int temp_flow = min(graph[u].edges[i].second.r_weight, graph[u].excess);

				graph[u].edges[i].second.r_weight -= temp_flow;
				graph[v][u].r_weight += temp_flow;
				graph[u].excess -= temp_flow;
				graph[v].excess += temp_flow;
				// if node v previously is inactive
				if (graph[v].excess == temp_flow) {
					actives[highest_level - 1].push_back(v);
					next_level[highest_level] = highest_level - 1;
				}
			}
			// relabel u
			if (graph[u].edges[i].second.r_weight > 0 && temp_height > graph[v].height){
				temp_height = graph[v].height;
			}

		}
		graph[u].height = temp_height + 1;
		//relabel(u);
	}
	return false;
}


void Graph::moveToFront(int i, int *A) {
	int temp = A[i];
	int n;
	for (n = i; n > 0; n--){
		A[n] = A[n - 1];
	}
	A[0] = temp;
}

// Find an s-t cut of the graph according to the residual flow
vector<int> Graph::findCut(int s){
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