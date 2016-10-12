#include "meshsmoother.h"
#include "HDS/HDS_Mesh.h"
#include <iostream>


using namespace std;
MeshSmoother::MeshSmoother()
{
}

void MeshSmoother::smoothMesh_perVertex(HDS_Mesh *mesh) {
	auto vertex_comp = [](const HDS_Vertex *a, const HDS_Vertex *b) {
		return fabs(a->curvature) > fabs(b->curvature);
	};

	// put all non-zero curvature vertices into a heap
	const double CTHRES = 1e-6;
	vector<HDS_Vertex*> H;
	for (auto v : mesh->vertSet) {
		if ( fabs(v.curvature) > CTHRES )
			H.push_back(&v);
	}
	std::make_heap(H.begin(), H.end(), vertex_comp);

	// modify the curvature of the vertices one by one, making them 0
	double CTHRES2;
	cout << "threshold:" << endl;
	cin >> CTHRES2;
	while (!H.empty()) {
		auto v = H.front();
		std::pop_heap(H.begin(), H.end(), vertex_comp);
		H.pop_back();
		if (fabs(v->curvature) > CTHRES2) {
			H.clear();
			break;
		}

		// make the curvature at this vertex 0
		auto neighbors = v->neighbors();

		// update its neighbors
		map<HDS_Vertex*, double> entries;
		double sum_inv_dist = 0.0;
		for (auto neighbor : neighbors) {
#if 1
			if (fabs(neighbor->curvature) < CTHRES) {
				entries.insert(make_pair(neighbor, 0.0));
			}
			else {
				double inv_dist = 1.0 / v->pos.distanceToPoint(neighbor->pos);
				entries.insert(make_pair(neighbor, inv_dist));
				sum_inv_dist += inv_dist;
			}
#else
			double inv_dist = 1.0 / v->pos.distanceToPoint(neighbor->pos);
			entries.insert(make_pair(neighbor, inv_dist));
			sum_inv_dist += inv_dist;
#endif
		}

		if (sum_inv_dist > 0) {
			for (auto entry : entries) {
				double w = entry.second / sum_inv_dist;
				entry.first->curvature += w * v->curvature;
			}

			v->curvature = 0;

			// add back the neighbors if they are not in queue
			for (auto entry : entries) {
				if (find(H.begin(), H.end(), entry.first) == H.end()) {
					H.push_back(entry.first);
				}
			}
		}

		// remove zero curvature vertices
		auto newEnd = std::remove_if(H.begin(), H.end(), [=](const HDS_Vertex *a) {
			return fabs(a->curvature) <= CTHRES;
		});

		H.erase(newEnd, H.end());
		std::make_heap(H.begin(), H.end(), vertex_comp);
		cout << H.size() << ": " << H.front()->curvature << endl;
	}

	double sum_curvature = std::accumulate(mesh->vertSet.begin(), mesh->vertSet.end(), 0.0, [](double val, const HDS_Vertex &v) {
		return val + v.curvature;
	});

	cout << "sum = " << sum_curvature << endl;
}

#if 0
// smooth the mesh by minimizing vertex movement and removing as much as possible curvature deficit
void MeshSmoother::smoothMesh(HDS_Mesh *mesh)
{
	// compute the smoothed curvature at each vertex
	int nVerts = mesh->vertSet.size();

	// construct the smoothing matrix
	mat A(nVerts, nVerts, arma::fill::zeros);
	vec x(nVerts);
	unordered_map<int, int> vidxMap;
	int ridx = 0;
	for(auto v : mesh->vertSet) {
		vidxMap[v->index] = ridx;
		// set the entry in vector x
		x(ridx) = v->curvature;
		++ridx;
	}

	// assemble the matrix A
	const double CTHRES = 1e-10;
	for(auto v : mesh->vertSet) {
		int ridx = vidxMap[v->index];

		if( fabs(v->curvature) < CTHRES ) {
			A(ridx, ridx) = 1.0;
		}
		else {
			// set the entry in vector x
			// get all its neighbors
			auto neighbors = v->neighbors();
			vector<pair<int, double>> entries;
			double sum_inv_dist = 0.0;
			for(auto neighbor : neighbors) {
				if( fabs(neighbor->curvature) < CTHRES ) {
					entries.push_back(make_pair(vidxMap[neighbor->index], 0.0));
				}
				else {
					double inv_dist = 1.0 / v->pos.distanceToPoint(neighbor->pos);
					entries.push_back(make_pair(vidxMap[neighbor->index], inv_dist));
					sum_inv_dist += inv_dist;
				}
			}

			for(auto entry : entries) {
				double w = entry.second / (sum_inv_dist + 1.0);
				A(entry.first, ridx) = w;
			}

			A(ridx, ridx) = 1.0 / (sum_inv_dist + 1.0);
		}
	}

	//cout << A << endl;

	// compute new curvatures
	vec x_hat = A * x;
	double sum_curvature = 0.0;
	for(auto v : mesh->vertSet) {
		int ridx = vidxMap[v->index];

		// set the entry in vector x
		v->curvature = x_hat(ridx);
		sum_curvature += x_hat(ridx);
	}

	cout << "sum = " << sum_curvature << endl;

	// compute the new vertex positions using the new curvatures
}
#endif

// cost function for computing new vertex positions
void costfunction_vert(double *p, double *hx, int m, int n, void *adata) {
	HDS_Mesh *mesh = (HDS_Mesh*)adata;

	// assemble matrix A with p

	// compute the new curvatures

	// compute the new positions for each vertex

	// compute the cost vector
}

// cost function for computing new curvatures
void costfunction(double *p, double *hx, int m, int n, void *adata) {
	HDS_Mesh *mesh = (HDS_Mesh*)adata;

	// assemble matrix A with p

	// compute the new curvatures

	// compute the new positions for each vertex

	// compute the cost vector
}

void MeshSmoother::smoothMesh_wholeMesh(HDS_Mesh *mesh)
{
	// compute the smoothed curvature at each vertex
	int nVerts = mesh->vertSet.size();

	// update curvature and vertex normal
	for (auto v : mesh->vertSet) {
		v.computeCurvature();
		v.computeNormal();
	}

	// update face normals
	for (auto &f : mesh->faceSet) {
		// TODO: replace by hds_mesh function
		//f.computeNormal();
	}

	// find out the set of zero curvature and static curvature
	set<int> V0, VS;

	// find out the coefficients of the manipulation matrix A
	/*
	// interface
	extern int dlevmar_lec_dif(
	void(*func)(double *p, double *hx, int m, int n, void *adata),
	double *p, double *x, int m, int n, double *A, double *b, int k,
	int itmax, double *opts, double *info, double *work, double *covar, void *adata);
	*/

	// compute the new curvatures with A

	// compute the new positions for each vertex
	/*
	// interface
	extern int dlevmar_dif(
	void(*func)(double *p, double *hx, int m, int n, void *adata),
	double *p, double *x, int m, int n, int itmax, double *opts,
	double *info, double *work, double *covar, void *adata);
	*/
}

void MeshSmoother::smoothMesh_Laplacian(HDS_Mesh *mesh)
{
	const double lambda = 0.25;
	const double sigma = 1.0;
	int nn=1;                          // later added
	unordered_map<HDS_Vertex*, QVector3D> L(mesh->vertSet.size());

	vector <double> vec(mesh->vertSet.size(),1.0);  // later added

	mesh->colorVertices(vec);  // later added

	//  cout<<"mesh->vertSet"<<mesh->vertSet.size()<<endl;// later added
	for (auto vi : mesh->vertSet) {
		auto neighbors = vi.neighbors();

		double denom = 0.0;
		QVector3D numer(0, 0, 0);

		for (auto vj : neighbors) {
			double wij = 1.0 / (vi.pos.distanceToPoint(vj->pos) + sigma);
			denom += wij;
			numer += wij * vj->pos;
		}
		//cout<<"vi.pos"<<nn<<vi->pos<<endl;// later added
		nn+=1;                           // later added
		L.insert(make_pair(&vi, numer/denom-vi.pos));
	}

	for (auto vi : mesh->vertSet) {
		vi.pos += lambda * L.at(&vi);
	}
}
