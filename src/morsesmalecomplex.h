#pragma once

#include "common.h"

#include "hds_vertex.h"

class MorseSmaleComplex
{
public:
	struct Edge {
	Edge(){}
	Edge(HDS_Vertex *s, HDS_Vertex *t) :s(s), t(t){}
	HDS_Vertex *s, *t;
	};
	struct Path {
	vector<Edge> edges;
	};

	MorseSmaleComplex(){}
	MorseSmaleComplex(const unordered_set<HDS_Vertex*> &criticalPoints);
	~MorseSmaleComplex();

	const vector<Path>& getPaths() const { return paths; }
private:
	vector<Path> paths;
};

