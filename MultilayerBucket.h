#include <vector>


template <typename C>
class myVector{
private:
	vector<C> data;
	vector<int> idx;

public:
	myVector(int sz){
		data.reserve(sz);
		idx = vector<int>(sz, 0);
	}
};