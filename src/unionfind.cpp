#include "unionfind.h"

UnionFind::UnionFind(int sz){
	count = sz;
	for(int i=0;i<sz;++i) {
		s.push_back(IdRank(i, 0));
	}
}

bool UnionFind::Connected(int p, int q) {
	return find(p) == find(q);
}

void UnionFind::Union(int p, int q) {
	int i = find(p);
	int j = find(q);
	if (i == j) return;

	// make root of smaller rank point to root of larger rank
	if      (s[i].rank < s[j].rank) s[i].id = j;
	else if (s[i].rank > s[j].rank) s[j].id = i;
	else {
		s[j].id = i;
		s[i].rank++;
	}
	count--;
}

int UnionFind::find(int p) {
	if (p < 0 || p >= s.size()) throw "Out of bound.";
	while (p != s[p].id) {
		s[p].id = s[s[p].id].id;    // path compression by halving
		p = s[p].id;
	}
	return p;
}
