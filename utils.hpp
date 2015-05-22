#ifndef UTILS_HPP
#define UTILS_HPP

#include "common.h"

#include <QColor>

namespace Utils {
	template <typename Container>
	Container interpolate(Container c0, Container c1, double alpha) {    
	assert(c0.size() == c1.size());
	Container c(c0.size());
	for (int i = 0; i < c0.size(); ++i) {
		c[i] = c0[i] * (1 - alpha) + c1[i] * alpha;
	}
	return c;
	}

	inline bool exists(const string &filename) {
	return std::ifstream(filename).good();
	}

template <typename Container, typename Pred>
Container filter(Container c, Pred p) {
	Container res;
	for(auto x : c) {
	if( p(x) )
		res.push_back(x);
	}
	return res;
}

template <typename Container, typename Pred>
Container filter_set(Container c, Pred p) {
	Container res;
	for(auto x : c) {
	if( p(x) )
		res.insert(x);
	}
	return res;
}

template <typename Container, typename Func>
Container map(Container c, Func f) {
	Container res;
	for(auto x : c) {
		res.push_back(f(x));
	}
	return res;
}

template <typename Container, typename Func>
Container map_set(Container c, Func f) {
	Container res;
	for(auto x : c) {
		res.insert(f(x));
	}
	return res;
}

template <typename Container>
void print(Container c, ostream& os = cout) {
	for(auto x : c) {
	os << x << ' ';
	}
	os << endl;
}

inline void printColor(QColor clr) {
	cout << '(' << clr.red() << ' ' << clr.green() << ' ' << clr.blue() << ')' << endl;
}

template<typename T, typename mesh_t>
vector<T> laplacianSmooth(const vector<T> &val, mesh_t *mesh, double lambda = 0.25, double sigma = 1.0) {
	unordered_map<typename mesh_t::vert_t*, double> L(mesh->verts().size());
	vector<T> newval(mesh->verts().size());
	for (auto vi : mesh->verts()) {
	auto neighbors = vi->neighbors();

	double denom = 0.0;
	double numer = 0.0;

	for (auto vj : neighbors) {
		//double wij = 1.0 / (vi->pos.distanceToPoint(vj->pos) + sigma);
		double wij = 1.0 / neighbors.size();
		denom += wij;
		numer += wij * val[vj->index];
	}

	L.insert(make_pair(vi, numer / denom - val[vi->index]));
	}
	for (auto p : L) {
	newval[p.first->index] = val[p.first->index] + lambda * p.second;
	}

	return newval;
}
}

#endif // UTILS_HPP
