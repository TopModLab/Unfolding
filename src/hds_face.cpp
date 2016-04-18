#include "hds_face.h"
#include "hds_vertex.h"
#include "hds_halfedge.h"
#include <qdebug.h>

hdsid_t HDS_Face::uid = 0;

HDS_Face::HDS_Face()
{
	isPicked = false;
	isCutFace = false;
	isBridger = false;
	isHole = false;
	//isFlap = false;
	index = -1;
	refid = 0;
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
	refid = other.refid;
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

	return faces;
}

set<HDS_Face *> HDS_Face::linkedFaces()
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

//from http://paulbourke.net/geometry/pointlineplane/lineline.c
/*
   Calculate the line segment PaPb that is the shortest route between
   two lines P1P2 and P3P4. Calculate also the values of mua and mub where
	  Pa = P1 + mua (P2 - P1)
	  Pb = P3 + mub (P4 - P3)
   Return FALSE if no solution exists.
*/
void HDS_Face::LineLineIntersect(
   QVector3D p1,QVector3D p2,QVector3D p3,QVector3D p4,QVector3D *pa)
{
	//QVector3D *pb;
   QVector3D p13,p43,p21;
   double d1343,d4321,d1321,d4343,d2121;
   double numer,denom;
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
    QVector3D c = center();
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

   QVector3D v1_n = curHe->next->v->pos;
   QVector3D v2_n = curHe->next->next->v->pos;
   QVector3D v1_p = curHe->prev->v->pos;
   QVector3D v2_p = curHe->prev->prev->v->pos;

   QVector3D v01_n = (1 - scalingFactor/2)* v0 + scalingFactor/2 *v1_n;
   QVector3D v12_p = (1 - scalingFactor/2)* v1_p + scalingFactor/2 *v2_p;

   QVector3D v01_p = (1 - scalingFactor/2)* v0 + scalingFactor/2 *v1_p;
   QVector3D v12_n = (1 - scalingFactor/2)* v1_n + scalingFactor/2 *v2_n;

   //get intersection point
   QVector3D v0_scaled;
   HDS_Face::LineLineIntersect(v01_n, v12_p, v01_p, v12_n, &v0_scaled);
   return v0_scaled;*/

}



vector<QVector3D> HDS_Face::getScaledCorners()
{
	checkPlanar();

	if(isPlanar) {
		// update the vertex position
		auto vertices = corners();
		for (auto v : vertices) {
			scaledCorners.insert(scaledCorners.end(), scaleCorner(v));
		}
	} else {
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

uint16_t HDS_Face::getFlag() const
{
	return (uint16_t)(-(int16_t)isPicked) & PICKED
		| (uint16_t)(-(int16_t)isCutFace) & CUTFACE
		| (uint16_t)(-(int16_t)isHole) & HOLE
		| (uint16_t)(-(int16_t)isBridger) & BRIDGER
		| (uint16_t)(-(int16_t)isPlanar) & PLANAR;
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
		if (fabsf(dot) > 0.3){
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
