#include "hds_face.h"
#include "hds_halfedge.h"

size_t HDS_Face::uid = 0;

HDS_Face::HDS_Face()
{
	isPicked = false;
	isCutFace = false;
	isConnector = false;
	isHole = false;
	//isFlap = false;
	index = -1;
	he = nullptr;
	scalingFactor = 1;
	isPlanar = true;
}

HDS_Face::~HDS_Face(){}

HDS_Face::HDS_Face(const HDS_Face &other)
{
	isPicked = other.isPicked;
	isCutFace = other.isCutFace;
	isConnector = other.isConnector;
	isHole = other.isHole;
	isPlanar = other.isPlanar;
	//isFlap = other.isFlap;
	index = other.index;
	n = other.n;
	he = nullptr;
	scalingFactor = other.scalingFactor;

}

HDS_Face HDS_Face::operator=(const HDS_Face &other)
{
	throw "Not implemented.";
}

set<HDS_Face *> HDS_Face::connectedFaces()
{
	set<HDS_Face*> faces;
	queue<HDS_Face*> Q;
	Q.push(this);
	while( !Q.empty() )
	{
		auto cur = Q.front();
		Q.pop();
		faces.insert(cur);
		auto curHE = cur->he;
		do {
			auto f = curHE->flip->f;
			if( faces.find(f) == faces.end() ) {
				faces.insert(f);
			}
			curHE = curHE->next;
		} while( curHE != he );
	}
	cout << "#connected faces = " << faces.size() << endl;
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

void HDS_Face::scaleDown(double factor)
{
	checkPlanar();

	scalingFactor = factor;
	if(isPlanar) {
		QVector3D c = center();
		/// update the vertex position
		auto vertices = corners();
		for (auto v : vertices) {
			QVector3D vec_cv = v->pos - c;
			v->pos = c + scalingFactor * vec_cv;
		}
	}else {
		//scale down non-planar face
	}
}

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
