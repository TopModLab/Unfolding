#pragma once
#ifndef UTILS_H
#define UTILS_H

#include "Utils/common.h"
#include <QVector3D>
#include <QtGui/QColor>

namespace Utils {
template <typename T>
inline T Lerp(const T &v1, const T &v2, double t)
{
	return v1 * (1 - t) + v2 * t;
}

template <typename Container>
Container interpolate(Container c0, Container c1, double alpha) {
	assert(c0.size() == c1.size());
	Container c(c0.size());
	for (int i = 0; i < c0.size(); ++i) {
		c[i] = Lerp(c0[i], c1[i], alpha);
	}
	return c;
}

inline bool exists(const string &filename) {
	return std::ifstream(filename).good();
}

template <typename Container, typename Pred>
Container filter(Container c, Pred p) {
	Container res;
	for (auto x : c) {
		if (p(x))
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

// Only for pairs of std::hash-able types for simplicity.
// You can of course template this struct to allow other hash functions
struct pair_hash {
	template <class T1, class T2>
	std::size_t operator () (const std::pair<T1, T2> &p) const {
		auto h1 = std::hash<T1>{}(p.first);
		auto h2 = std::hash<T2>{}(p.second);

		// Mainly for demonstration purposes, i.e. works but is overly simple
		// In the real world, use sth. like boost.hash_combine
		return h1 ^ h2;
	}
};
inline void printColor(QColor clr) {
	cout << '(' << clr.red() << ' ' << clr.green() << ' ' << clr.blue() << ')' << endl;
}

template<typename T, typename mesh_t>
vector<T> laplacianSmooth(
	const vector<T> &val, mesh_t *mesh,
	double lambda = 0.25, double sigma = 1.0)
{
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

//from http://paulbourke.net/geometry/pointlineplane/lineline.c
/*
   Calculate the line segment PaPb that is the shortest route between
   two lines P1P2 and P3P4. Calculate also the values of mua and mub where
	  Pa = P1 + mua (P2 - P1)
	  Pb = P3 + mub (P4 - P3)
   Return FALSE if no solution exists.
*/
inline void LineLineIntersect(
	QVector3D p1, QVector3D p2,
	QVector3D p3, QVector3D p4, QVector3D *pa)
{
	//QVector3D *pb;
	QVector3D p13, p43, p21;
	double d1343, d4321, d1321, d4343, d2121;
	double numer, denom;
	//   double EPS = 0.0001;

	p13 = p1 - p3;
	p43 = p4 - p3;
	p21 = p2 - p1;

	//   if (fabsf(p43.x) < EPS && fabsf(p43.y) < EPS && fabsf(p43.z) < EPS)
	//	  return(false);
	//   if (fabsf(p21.x) < EPS && fabsf(p21.y) < EPS && fabsf(p21.z) < EPS)
	//	  return(false);

	d1343 = QVector3D::dotProduct(p13, p43);
	d4321 = QVector3D::dotProduct(p43, p21);
	d1321 = QVector3D::dotProduct(p13, p21);
	d4343 = QVector3D::dotProduct(p43, p43);
	d2121 = QVector3D::dotProduct(p21, p21);

	denom = d2121 * d4343 - d4321 * d4321;
	//   if (fabsf(denom) < EPS)
	//	  return(false);
	numer = d1343 * d4321 - d1321 * d4343;

	double mua = numer / denom;
	//double mub = (d1343 + d4321 * (mua)) / d4343;
	*pa = p1 + mua * p21;
	//*pb = p3 + mub * p43;
}

}

#endif // UTILS_HPP
