#include "GeomUtils/morsesmalecomplex.h"
#include "Utils/utils.h"

//cut locus cutting method, follow gradient of saddle to min/maximum
MorseSmaleComplex::MorseSmaleComplex(const unordered_set<HDS_Vertex*> &criticalPoints)
{
	int ii=0;
	for (auto cp : criticalPoints) {
		if (cp->rtype == HDS_Vertex::Saddle) {
			// trace lines in each segment
			auto neighbors = cp->neighbors();
			doubles_t diffs;
			for (auto n : neighbors) {
				diffs.push_back(n->morseFunctionVal - cp->morseFunctionVal);
			}

			vector<vector<HDS_Vertex*>> groups(1, vector<HDS_Vertex*>());
			int ngroups = 1, sign = diffs.front() > 0 ? 1 : -1;
			groups.back().push_back(neighbors.front());
			for (int i = 1; i < diffs.size(); ++i) {
				if (diffs[i] * sign >= 0) {
					groups.back().push_back(neighbors[i]);
					continue;
				}
				else {
					sign = -sign;
					++ngroups;
					groups.push_back(vector<HDS_Vertex*>());
					groups.back().push_back(neighbors[i]);
				}
			}

			if (ngroups % 2 == 1) {
				ngroups = ngroups - 1;
				groups.front().insert(groups.front().end(), groups.back().begin(), groups.back().end());
				groups.pop_back();
			}
			// above find saddle points and their group;
			// for each group, find the steepest direction
			//  cout << "number of groups = " << groups.size() << endl;
			for (auto g : groups) {
				HDS_Vertex *steepestV = g.front();
				double vdiff = fabs(steepestV->morseFunctionVal - cp->morseFunctionVal);
				for (int i = 1; i < g.size(); ++i) {
					double gidiff = fabs(g[i]->morseFunctionVal - cp->morseFunctionVal);
					if (gidiff > vdiff) {
						steepestV = g[i];
						vdiff = gidiff;
					}
				}

				// trace the steepest vertex until reaches a maximum or minimum
				Path path;
				HDS_Vertex *v0 = cp, *v1 = steepestV;
				bool climbingHill = v1->morseFunctionVal > v0->morseFunctionVal;
				while (v0->rtype != HDS_Vertex::Maximum && v0->rtype != HDS_Vertex::Minimum) {
					path.edges.push_back(Edge(v0, v1));
					v0 = v1;
					// find the steepest neighbor of v1
					auto v1_neighbors = Utils::filter(v1->neighbors(), [&](HDS_Vertex* v){return v != v1; });

					if (climbingHill) {
						// find the highest neighbor of v1
						auto nv = std::max_element(v1_neighbors.begin(), v1_neighbors.end(), [&](HDS_Vertex *vstar, HDS_Vertex *v){
								return vstar->morseFunctionVal < v->morseFunctionVal;
					});
						v1 = *nv;
					}
					else {
						// find the lowest neighbor of v1
						auto nv = std::min_element(v1_neighbors.begin(), v1_neighbors.end(), [&](HDS_Vertex *vstar, HDS_Vertex *v){
								return vstar->morseFunctionVal < v->morseFunctionVal;
					});
						v1 = *nv;
					}
				}
				if (climbingHill)
					maxpaths.push_back(path);
				else
					minpaths.push_back(path);
			}


			/*liu wei's modification somehow has some bugs, can't produce correct path, hence commented out
				// trace the steepest vertex until reaches a maximum or minimum
				Path path;

				HDS_Vertex *v0 = cp, *v1 = steepestV;
				bool climbingHill = v1->morseFunctionVal >= v0->morseFunctionVal;//add an extra = sign, later added;
				while (v0->rtype != HDS_Vertex::Maximum && v0->rtype != HDS_Vertex::Minimum&&ii<1000) {   //sometimes it can't find max or min points, so recycle always runs;
					path.edges.push_back(Edge(v0, v1));
					v0 = v1;

					// find the steepest neighbor of v1
					auto v1_neighbors = Utils::filter(v1->neighbors(), [&](HDS_Vertex* v){return v != v1; });//find neighbors of steepest point

					if (climbingHill) {
					// find the highest neighbor of v1
					auto nv = std::max_element(v1_neighbors.begin(), v1_neighbors.end(), [&](HDS_Vertex *vstar, HDS_Vertex *v){
						return vstar->morseFunctionVal >= v->morseFunctionVal;//change < to >; later changed;//add an extral = sign, later added;
					});
					v1 = *nv;
					}
					else {
					// find the lowest neighbor of v1
					auto nv = std::min_element(v1_neighbors.begin(), v1_neighbors.end(), [&](HDS_Vertex *vstar, HDS_Vertex *v){
						return vstar->morseFunctionVal <= v->morseFunctionVal;//add an extral = sign, later added;
					});
					v1 = *nv;
					}
			//        cout<<"anything"<<endl;
					ii+=1;                          //later added;
				}
				paths.push_back(path);
			}
			*/
		}
	}
}


MorseSmaleComplex::~MorseSmaleComplex()
{
}
