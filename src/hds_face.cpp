#include "hds_face.h"
#include "hds_halfedge.h"

size_t HDS_Face::uid = 0;

HDS_Face::HDS_Face()
{
	isPicked = false;
	isCutFace = false;
	isBridger = false;
	isHole = false;
	//isFlap = false;
	index = -1;
	he = nullptr;
	scalingFactor = 1;
	isPlanar = true;

	//bound = nullptr;
}

HDS_Face::~HDS_Face()
{
	//delete bound;
}

HDS_Face::HDS_Face(const HDS_Face &other)
{
	isPicked = other.isPicked;
	isCutFace = other.isCutFace;
	isBridger = other.isBridger;
	isHole = other.isHole;
	isPlanar = other.isPlanar;
	//isFlap = other.isFlap;
	index = other.index;
	n = other.n;
	he = nullptr;
	scalingFactor = other.scalingFactor;

	//bound = nullptr;
}

HDS_Face HDS_Face::operator=(const HDS_Face &other)
{
	throw "Not implemented.";
}

set<HDS_Face *> HDS_Face::connectedFaces()
{
	// Find all faces that are directly connected to current face
	set<HDS_Face*> faces;
		
	faces.insert(this);
	auto curHE = this->he;
	do {
		auto f = curHE->flip->f;
		if (faces.find(f) == faces.end()) {
			faces.insert(f);
		}
		curHE = curHE->next;
	} while (curHE != he);
	cout << "#connected faces = " << faces.size() << endl;
	return faces;
}

set<HDS_Face *> HDS_Face::linkededFaces()
{
	// Find all linked faces
	set<HDS_Face*> faces;
	set<HDS_Face*> visitedFaces;
	queue<HDS_Face*> Q;
	Q.push(this);
	while (!Q.empty())
	{
		auto cur = Q.front();
		Q.pop();
		faces.insert(cur);
		if (visitedFaces.find(cur) == visitedFaces.end())
		{
			visitedFaces.insert(cur);
		}

		auto fhe = cur->he;
		auto curHE = fhe;
		do {
			auto f = curHE->flip->f;
			if (faces.find(f) == faces.end() && visitedFaces.find(f)== visitedFaces.end())
			{
				faces.insert(f);
				visitedFaces.insert(f);
				Q.push(f);
			}
			curHE = curHE->next;
		} while (curHE != fhe);
	}

	cout << "#linked faces = " << faces.size() << endl;
	return faces;

}

QVector3D HDS_Face::center() const
{
	auto cs = corners();
	QVector3D c;
	for(auto p : cs) {
		c += p->pos;
	}
	c /= (qreal) cs.size();
	return c;
}

vector<HDS_Vertex*> HDS_Face::corners() const
{
	HDS_HalfEdge *curHE = he;
	vector<HDS_Vertex*> corners;
	do {
		corners.push_back(curHE->v);
		curHE = curHE->next;
	} while( curHE != he );
	return corners;
}

QVector3D HDS_Face::computeNormal()
{
	QVector3D c = center();
	n = QVector3D::crossProduct(he->v->pos - c, he->next->v->pos - c);
	n.normalize();
	return n;
}

void HDS_Face::setScaleFactor(double factor)
{
	scalingFactor = factor;
}

QVector3D HDS_Face::scaleCorner(HDS_Vertex* v)
{
	QVector3D c = center();
	QVector3D vec_cv = v->pos - c;
	return c + scalingFactor * vec_cv;

}

vector<QVector3D> HDS_Face::getScaledCorners()
{
	checkPlanar();

	if(isPlanar) {
		/// update the vertex position
		auto vertices = corners();
		for (auto v : vertices) {
			scaledCorners.insert(scaledCorners.end(), scaleCorner(v));
		}
	}else {
		//scale down non-planar face
	}
	return scaledCorners;
}

void HDS_Face::scaleDown()
{
	int n = 0;
	for (auto v : corners()) {
		v->pos = scaledCorners.at(n);
		n++;
	}
}
/*
void HDS_Face::update_bbox()
{
	auto curHE = he;
	while(curHE->next != he)
	{
		bound->Union(curHE->v->pos);
		curHE = curHE->next;
	}

}*/
void HDS_Face::checkPlanar()
{
	isPlanar = true;

	auto vertices = corners();
	QVector3D normal = QVector3D::crossProduct(vertices[1]->pos - vertices[0]->pos, vertices[2]->pos - vertices[0]->pos);

	for(int i = 3; i < vertices.size(); i++) {
		float dot = QVector3D::dotProduct(normal, vertices[i]->pos - vertices[0]->pos);
		if (abs(dot) > 0.3){
			isPlanar = false;
			break;
		}
	}

}

bool HDS_Face::isConnected(const HDS_Face *other)
{	
	auto curHe = he;
	do 
	{
		if (curHe->flip->f == other)
		{
			return true;
		}
		curHe = curHe->next;
	} while (curHe != he);

	return false;
}
