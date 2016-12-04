#include "GaussModel.h"

void guassMixModel(Mat& image, Mat& labels, Mat& probs, Mat& means, vector<Mat>& covs){

	Mat samples = image.reshape(1, image.rows * image.cols);

	Ptr<EM> em_model = EM::create();
	em_model->setClustersNumber(2);
	em_model->setCovarianceMatrixType(EM::COV_MAT_SPHERICAL);
	em_model->setTermCriteria(TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 300, 0.1));
	em_model->trainEM(samples, noArray(), labels, probs);

	em_model->getCovs(covs);    // covariance matrices of each cluster

	means = em_model->getMeans();  // means of each cluster
}

double neighbourPenality(Vec3b x, Vec3b y, double sigma[]){
	//cout << "x: " << x << " y: " << y << " sigma: " << sigma << endl;
	double sqrt_sum = pow(((double)x[0] - (double)y[0]), 2) / pow(sigma[0], 2)
		+ pow(((double)x[1] - (double)y[1]), 2)/pow(sigma[1], 2)
		+ pow(((double)x[2] - (double)y[2]), 2) / pow(sigma[2], 2);
	return exp(-sqrt_sum);
	//return exp(-pow(((double)x - (double)y), 2) / 3);
}

void estimateNoise(Mat& image, double sigma[]){
	Mat x_filter = (Mat_<double>(2, 1) << 1, -1);
	Mat y_filter = (Mat_<double>(1, 2) << 1, -1);
	Mat x_grad, y_grad;
	filter2D(image, x_grad, 64, x_filter, Point(-1, -1), 0, BORDER_REPLICATE);
	filter2D(image, y_grad, 64, y_filter, Point(-1, -1), 0, BORDER_REPLICATE);
	x_grad = abs(x_grad);
	y_grad = abs(y_grad);
	sigma[0] = sum(x_grad + y_grad)[0];
	sigma[1] = sum(x_grad + y_grad)[1];
	sigma[2] = sum(x_grad + y_grad)[2];
	sigma[0] = sigma[0] / (image.rows*image.cols);
	sigma[1] = sigma[1] / (image.rows*image.cols);
	sigma[2] = sigma[2] / (image.rows*image.cols);

	//return sigma;
}