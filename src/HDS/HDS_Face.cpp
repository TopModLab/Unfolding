#include "HDS/hds_face.h"
#include "HDS/hds_vertex.h"
#include "HDS/hds_halfedge.h"
#include <qdebug.h>

hdsid_t HDS_Face::uid = 0;

HDS_Face::HDS_Face()
	: index(uid++), refid(sInvalidHDS)
	, heid(sInvalidHDS), flag(0)
	, scalingFactor(1)
{
}
/*
set<HDS_Face *> HDS_Face::connectedFaces() const
{
	// Find all faces that are directly connected to current face
	set<HDS_Face*> faces;

#ifdef USE_LEGACY_FACTORY
	//TODO: force cast
	faces.insert(const_cast<HDS_Face*>(this));
	auto curHE = this->he;
	do {
		auto f = curHE->flip()->f;
		if (faces.find(f) == faces.end()) {
			faces.insert(f);
		}
		curHE = curHE->next;
	} while (curHE != he);
#endif
	return faces;
}

set<HDS_Face *> HDS_Face::linkedFaces()
{
	// Find all linked faces
	set<HDS_Face*> faces;

#ifdef USE_LEGACY_FACTORY
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
			auto f = curHE->flip()->f;
			if (faces.find(f) == faces.end() && visitedFaces.find(f)== visitedFaces.end())
			{
				faces.insert(f);
				visitedFaces.insert(f);
				Q.push(f);
			}
			curHE = curHE->next;
		} while (curHE != fhe);
	}
#endif
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
	vector<HDS_Vertex*> corners;
#ifdef USE_LEGACY_FACTORY
	HDS_HalfEdge *curHE = he;
	do {
		corners.push_back(curHE->v);
		curHE = curHE->next;
	} while( curHE != he );
#endif
	return corners;
}

QVector3D HDS_Face::computeNormal() 
{
#ifdef USE_LEGACY_FACTORY
	QVector3D c = center();
	n = QVector3D::crossProduct(he->v->pos - c, he->next()->v->pos - c);
	n.normalize();
#endif
	return n;
}

QVector3D HDS_Face::computeNormal() const
{
#ifdef USE_LEGACY_FACTORY
	QVector3D c = center();
	QVector3D n = QVector3D::crossProduct(he->v->pos - c, he->next()->v->pos - c);
	n.normalize();
#endif
	return n;
}
*/
void HDS_Face::setScaleFactor(Float factor)
{
	scalingFactor = factor;
}


QVector3D HDS_Face::scaleCorner(HDS_Vertex* v)
{
	// v1_p ---------- v0
	//      |        |
	//      |    f   |
	//      |        |
	//      |        |
	// v2_  ---------- v1_n
	//    n/p

	//scale down based on center point

	// TODO: replace by hds_mesh function
	QVector3D c;// = center();
    QVector3D vec_cv = v->pos - c;
    return c + scalingFactor * vec_cv;

/*
	//scale down corner proportionally along edges
   QVector3D v0 = v->pos;


   //find half edge from v0 to v1_n
   HDS_HalfEdge* curHe = he;
   do {
       if (curHe->v == v) {
           break;
       }
       curHe = curHe->next;
   } while( curHe != he );

   QVector3D v1_n = curHe->next()->v->pos;
   QVector3D v2_n = curHe->next()->next()->v->pos;
   QVector3D v1_p = curHe->prev()->v->pos;
   QVector3D v2_p = curHe->prev()->prev()->v->pos;

   QVector3D v01_n = (1 - scalingFactor/2)* v0 + scalingFactor/2 *v1_n;
   QVector3D v12_p = (1 - scalingFactor/2)* v1_p + scalingFactor/2 *v2_p;

   QVector3D v01_p = (1 - scalingFactor/2)* v0 + scalingFactor/2 *v1_p;
   QVector3D v12_n = (1 - scalingFactor/2)* v1_n + scalingFactor/2 *v2_n;

   //get intersection point
   QVector3D v0_scaled;
   Utils::LineLineIntersect(v01_n, v12_p, v01_p, v12_n, &v0_scaled);
   return v0_scaled;*/

}



vector<QVector3D> HDS_Face::getScaledCorners()
{
#ifdef USE_LEGACY_FACTORY
	checkPlanar();

	if(!isNonPlanar) {
		// update the vertex position
		auto vertices = corners();
		for (auto v : vertices) {
			scaledCorners.insert(scaledCorners.end(), scaleCorner(v));
		}
	} else {
		//scale down non-planar face
	}
#endif
	return scaledCorners;
}

void HDS_Face::scaleDown()
{
#ifdef USE_LEGACY_FACTORY
	int n = 0;
	for (auto v : corners()) {
		v->pos = scaledCorners.at(n);
		n++;
	}
#endif
}

void HDS_Face::checkPlanar()
{
#ifdef USE_LEGACY_FACTORY
	auto vertices = corners();
	QVector3D normal = QVector3D::crossProduct(vertices[1]->pos - vertices[0]->pos, vertices[2]->pos - vertices[0]->pos);

	for(int i = 3; i < vertices.size(); i++) {
		Float dot = QVector3D::dotProduct(normal, vertices[i]->pos - vertices[0]->pos);
		if (fabsf(dot) > 0.3){
			isNonPlanar = true;
			break;
		}
	}
#endif
}

bool HDS_Face::isConnected(const HDS_Face *other)
{	
#ifdef USE_LEGACY_FACTORY
	auto curHe = he;
	do 
	{
		if (curHe->flip()->f == other)
		{
			return true;
		}
		curHe = curHe->next;
	} while (curHe != he);
#endif

	return false;
}
