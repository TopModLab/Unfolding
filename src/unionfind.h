#ifndef UNIONFIND_H
#define UNIONFIND_H

#include "common.h"

class UnionFind {
public:
	UnionFind(int sz);

	bool Connected(int p, int q);
	void Union(int p, int q);

private:
	int find(int p);

private:
	struct IdRank {
	IdRank(int id, int rank):id(id), rank(rank){}
	int id;
	int rank;
	};

	int count;
	vector<IdRank> s;
};
#endif // UNIONFIND_H
