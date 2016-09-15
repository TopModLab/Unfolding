#include "hds_bridger.h"

size_t HDS_Bridger::uid = 0;

int HDS_Bridger::shape = 0;
double HDS_Bridger::scale = 0.2;
double HDS_Bridger::curv = 0.5;
int HDS_Bridger::nSamples = 4;
double HDS_Bridger::cp = 0;

void HDS_Bridger::setBridger(const confMap &config)
{
	//0 bezier 1 original 2 flat
	shape = static_cast<int>(config.at("shape"));

	scale = config.at("size");
	curv = config.at("curv");
	nSamples = static_cast<int>(config.at("samples"));
	cp = config.at("cp");
}

void HDS_Bridger::setSamples(int sample)
{
	nSamples = sample;
}

void HDS_Bridger::setScale(double size)
{
	scale = size;
}

void HDS_Bridger::setCutFace(face_t* face1, face_t* face2)
{
	cutFace1 = face1;
	cutFace2 = face2;
}




HDS_Bridger::HDS_Bridger(HDS_HalfEdge* he, HDS_HalfEdge* hef, vector<QVector3D> controlPoints)
{
	if (he->flip->f != nullptr && !he->flip->f->isCutFace)
		he->setCutEdge(false);
	if (hef->flip->f != nullptr && !hef->flip->f->isCutFace)
		hef->setCutEdge(false);
	this->he = he;
	this->hef = hef;

	if (shape != 2) {
		QVector3D p10, p11, p20, p21;
		p10 = he->flip->v->pos;
		p20 = hef->v->pos;
		p11 = he->v->pos;
		p21 = hef->flip->v->pos;

		if (controlPoints.size() == 2) {
			//quadratic bezier curve constructor

			bezierPos_front = quadraticBezierCurve(p10, controlPoints[1], p20);
			bezierPos_back = quadraticBezierCurve(p11, controlPoints[0], p21);
		} else {
			bezierPos_front = cubicBezierCurve(p10, controlPoints[1], controlPoints[3], p20);
			bezierPos_back = cubicBezierCurve(p11, controlPoints[0], controlPoints[2], p21);
		}
	}
}

void HDS_Bridger::createBridge()
{
	if (shape != 2) { // not flat
		//push back all internal edges
		for (int i = 0; i < nSamples - 1; i++)
		{
			HDS_Vertex vs, ve;

			vs.pos = bezierPos_front[i];
			ve.pos = bezierPos_back[i];
			HDS_HalfEdge* he_new = HDS_Mesh::insertEdge(&vs, &ve);

			hes.push_back(*he_new);
			verts.push_back(vs);
			verts.push_back(ve);
		}

		//create bridge segments
		vector<he_t> hes_ori;
		hes_ori.reserve(hes.size() + 2);
		hes_ori.push_back(*he->flip);
		hes_ori.insert(hes_ori.end(), hes.begin(), hes.end());
		hes_ori.push_back(*hef);
		for (auto he = hes_ori.begin(); he != prev(hes_ori.end()); he++) {
				auto he_next = next(he);
				//bridge each pair of edges
				//get bridge faces, set to Bridger->faces
				HDS_Face* bridgeFace = bridging(&(*he->flip), &(*he_next));
				//fix face
				//bridgeFace->index = HDS_Face::assignIndex();
				bridgeFace->isCutFace = false;
				bridgeFace->isBridger = true;
				//add face to mesh
				faces.push_back(*bridgeFace);
		}
	}
}

QVector3D getPt( QVector3D n1 , QVector3D n2 , float perc )
{
	QVector3D diff = n2 - n1;

	return n1 + ( diff * perc );
}


//	///quadratic bezier curve calculator
//	///
//	//           _________p0'
//	//         p0      c2/__\
//	//      c1/__\      /    \
//	//       /    \    p1'    p2'
//	//      p1    p2
//	///
// ///
// ///


vector<QVector3D> HDS_Bridger::quadraticBezierCurve(QVector3D p0, QVector3D p1, QVector3D p2)
{
	vector<QVector3D> pos;
	if (shape == 1)
	{
		pos.push_back(p1);
	} else {

		for( float i = 1.0/(float)nSamples ; i < 1 ; i += 1.0/(float)nSamples )
		{
			QVector3D pa = getPt( p0 , p1 , i );
			QVector3D pb = getPt( p1 , p2 , i );

			QVector3D p = getPt( pa , pb , i );
			pos.push_back(p);
		}
	}
	return pos;
}

vector<QVector3D> HDS_Bridger::cubicBezierCurve(QVector3D p0, QVector3D p1, QVector3D p2, QVector3D p3)
{
	vector<QVector3D> pos;
	if (shape == 1)
	{
		pos.push_back(p1);
		pos.push_back(p2);
	} else {

		for( float i = 1.0/(float)nSamples ; i < 1 ; i += 1.0/(float)nSamples )
		{
			QVector3D pa = getPt( p0 , p1 , i );
			QVector3D pb = getPt( p1 , p2 , i );
			QVector3D pc = getPt( p2 , p3 , i );
			QVector3D pab = getPt( pa, pb, i);
			QVector3D pbc = getPt( pb, pc, i);
			QVector3D p = getPt( pab, pbc, i);

			pos.push_back(p);
		}
	}
	return pos;
}

HDS_Face * HDS_Bridger::bridging(HDS_HalfEdge* he1, HDS_HalfEdge* he2)
{
	//get 4 vertices from h1 h2
	HDS_Vertex* v1s, *v1e, *v2s, *v2e;
	v1s = he1->v;
	v1e = he1->flip->v;
	v2s = he2->v;
	v2e = he2->flip->v;


	//build new face
	face_t * bridgeFace = new face_t;

	//link he1 and he2 to face
	he1->f = bridgeFace;
	he2->f = bridgeFace;
	bridgeFace->he = he1;

	//insert two cut edges
	he_t* he_v1e_v2s = HDS_Mesh::insertEdge(v1e, v2s, he1, he2);
	he_t* he_v2e_v1s = HDS_Mesh::insertEdge(v2e, v1s, he2, he1);

	he_v1e_v2s->f = bridgeFace;
	he_v2e_v1s->f = bridgeFace;

	he_v2e_v1s->flip->f = cutFace1;
	he_v1e_v2s->flip->f = cutFace2;
	cutFace1->he = he_v2e_v1s->flip;
	cutFace2->he = he_v1e_v2s->flip;

	he_v1e_v2s->setCutEdge(true);
	he_v2e_v1s->setCutEdge(true);

	hes.push_back(*he_v1e_v2s);
	hes.push_back(*he_v2e_v1s);

	return bridgeFace;
}

HDS_Bridger::~HDS_Bridger()
{
}

