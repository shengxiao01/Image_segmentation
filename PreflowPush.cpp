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

	vector<pair<int,Edge2>>::iterator it = lower_bound(edges.begin(), edges.end(),
		make_pair(vertex_idx, Edge2{0,0}), 
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

Graph::Graph(int size){
	graph = vector<Vertex>(size);
}
void Graph::guassMixModel(Mat& image, Mat& labels, Mat& probs, Mat& means, vector<Mat>& covs){

	Mat samples = image.reshape(0, image.rows * image.cols);

	Ptr<EM> em_model = EM::create();
	em_model->setClustersNumber(2);
	em_model->setCovarianceMatrixType(EM::COV_MAT_SPHERICAL);
	em_model->setTermCriteria(TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 300, 0.1));
	em_model->trainEM(samples, noArray(), labels, probs);

	em_model->getCovs(covs);    // covariance matrices of each cluster

	means = em_model->getMeans();  // means of each cluster
}

inline double Graph::neighbourPenality(int x, int y, double sigma){
	return exp(-pow(((double)x - (double)y), 2) / (2 * pow(sigma, 2)));
}
/*Given a grayscale image, estimate its noise variance. Reference, J. Immerkær, “Fast Noise Variance Estimation”, Computer Vision and Image Understanding, Vol. 64, No. 2, pp. 300-302, Sep. 1996*/
double Graph::estimateNoise(Mat& image){
	Mat filter = (Mat_<double>(3, 3) << 1, -2, 1, -2, 4, -2, 1, -2, 1);
	Mat filtered_image;
	filter2D(image, filtered_image, 64, filter, Point(-1, -1), 0, BORDER_REPLICATE);
	filtered_image = abs(filtered_image);
	double sigma = sum(filtered_image)[0];
	const double PI = 3.141592653589793238462643383279502884;
	sigma = sigma * sqrt(0.5 * PI) / (6 * (image.rows - 2) * (image.cols - 2));
	return sigma;
}
Graph::Graph(Mat& image){
	const int rows = image.rows;
	const int cols = image.cols;
	const int pixel_number = rows * cols;
	const int PRECISION = 256;
	const double alpha = 1;


	Mat labels, probs, means;
	vector<Mat> covs;
	guassMixModel(image, labels, probs, means, covs);

	log(probs, probs);       // turn linear probability to logrithmic scale
	probs = -PRECISION * probs;
	double sigma = estimateNoise(image);
	//probs.convertTo(probs, 8, 255, 0);

	graph = vector<Vertex>(pixel_number + 2);

	for (int i = 0; i < rows; ++i){

		for (int j = 0; j < cols; ++j){
			int current_pixel = (int)image.at<uchar>(i, j);
			int current_index = i * cols + j;
			int neighbor_index;
			int neighbor_pixel;
			int penality;

			if (i != rows - 1){
				neighbor_index = (i + 1) * cols + j;
				neighbor_pixel = (int)image.at<uchar>(i + 1, j);
				penality = (int)PRECISION * alpha * neighbourPenality(current_pixel, neighbor_pixel, sigma);
				insert_edge(current_index, neighbor_index, penality);
				insert_edge(neighbor_index, current_index, penality);
			}
			if (j != cols - 1){
				neighbor_index = i * cols + j + 1;
				neighbor_pixel = (int)image.at<uchar>(i, j + 1);
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
	for (int i = 0; i < graph.size(); ++i){
		if (graph[i].excess > 0 && i != t){
			return i;
		}
	}
	return -1;
}

bool Graph::push(int u){
	for (auto it = graph[u].begin(); it != graph[u].end(); ++it){
		int v = it->first;
		if (it->second.r_weight > 0 && graph[u].height > graph[v].height){
			int temp_flow = min(it->second.r_weight, graph[u].excess);

			it->second.r_weight -= temp_flow;
			graph[v][u].r_weight += temp_flow;
			graph[u].excess -= temp_flow;
			graph[v].excess += temp_flow;

			return true;
		}
	}
	return false;
}

void Graph::relabel(int u){
	int temp_height = INT_MAX;
	for (auto it = graph[u].begin(); it != graph[u].end(); ++it){
		int v = it->first;
		if (it->second.r_weight > 0 && temp_height > graph[v].height){
			temp_height = graph[v].height;
		}
	}
	graph[u].height = temp_height + 1;
}

bool Graph::discharge(int u){
	while (graph[u].excess > 0){
		for (auto it = graph[u].begin(); it != graph[u].end(); ++it){
			int v = it->first;
			if (it->second.r_weight > 0 && graph[u].height > graph[v].height){
				int temp_flow = min(it->second.r_weight, graph[u].excess);

				it->second.r_weight -= temp_flow;
				graph[v][u].r_weight += temp_flow;
				graph[u].excess -= temp_flow;
				graph[v].excess += temp_flow;

			}
		}
		relabel(u);
	}
	return false;
}
// push relabel algorithm
int Graph::maxFlow(int s, int t){
	initialize(s);
	int u = positiveExcessIdx(t);

	while (u != -1){
		if (!push(u)){
			relabel(u);
		}
		u = positiveExcessIdx(t);
	}
	return graph[t].excess;
}
// relabel to front algorithm
int Graph::maxFlow_rtf(int s, int t){
	initialize(s);
	int u = positiveExcessIdx(t);
	list<int> active_vertex(graph.size() - 2);
	int i = 0;
	for (list<int>::iterator it = active_vertex.begin(); it != active_vertex.end(); ++it){
		*it = i;
		++i;
	}
	int p = 0;
	for (list<int>::iterator it = active_vertex.begin(); it != active_vertex.end();){
		int u = *it;
		int height = graph[u].height;
		discharge(u);
		if (graph[u].height > height){
			active_vertex.erase(it);
			active_vertex.push_front(u);
			it = active_vertex.begin();
		}
		++it;

	}
	return graph[t].excess;
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