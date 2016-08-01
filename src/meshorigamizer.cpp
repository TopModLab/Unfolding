#include "meshorigamizer.h"
using namespace std;

typedef HDS_Face face_t;
typedef HDS_HalfEdge he_t;
typedef HDS_Vertex vert_t;

double MeshOrigamizer::tucked_length;
double MeshOrigamizer::tucked_smooth;
double MeshOrigamizer::tucked_angle;
double MeshOrigamizer::origami_scale;
int MeshOrigamizer::bridger_type;

vector<QVector3D> MeshOrigamizer::ctrlPoints_front;
vector<QVector3D> MeshOrigamizer::ctrlPoints_back;
vector<face_t*> MeshOrigamizer::bridger_faces;
vector<he_t*> MeshOrigamizer::bridger_hes;
vector<vert_t*> MeshOrigamizer::bridger_verts;

void MeshOrigamizer::initBridger(const confMap &conf) {
	origami_scale = conf.at("scale");
	tucked_length = conf.at("tucked_length");
	tucked_smooth = tucked_length * conf.at("tucked_smooth");
	tucked_angle = conf.at("angle") * Pi;
	bridger_type = (int)conf.at("bridger_type");
}

HDS_Face* MeshOrigamizer::bridging(HDS_HalfEdge* he1, HDS_HalfEdge* he2, face_t* cutFace1, face_t* cutFace2) {
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

	bridger_hes.push_back(he_v1e_v2s);
	bridger_hes.push_back(he_v2e_v1s);

	return bridgeFace;
}

void MeshOrigamizer::setControlPoints(HDS_HalfEdge* he1, HDS_HalfEdge* he2, double theta, double folding_length, double smooth_length) {
	if (he1->flip->f != nullptr && !he1->flip->f->isCutFace)
		he1->setCutEdge(false);
	if (he2->flip->f != nullptr && !he2->flip->f->isCutFace)
		he2->setCutEdge(false);

	//clear control points
	ctrlPoints_front.clear();
	ctrlPoints_back.clear();

	//set face normals
	QVector3D normal1, normal2;
	normal1 = he1->flip->f->computeNormal();
	if (!he1->isCutEdge && !he2->isCutEdge) {
		normal2 = he2->flip->f->computeNormal();
	}
	else {
		normal2 = he1->bridgeTwin->flip->f->computeNormal();
	}


	/////////////////////////////////////////////////////////////////////
	//end points of scaled mesh edges
	HDS_Vertex *v1s, *v1e, *v2s, *v2e;
	v1s = he1->v;
	v1e = he1->flip->v;
	v2s = he2->v;
	v2e = he2->flip->v;

	//4 pair of new vertices, 
	QVector3D vn1s, vn1e, vn2s, vn2e, vn3s, vn3e, vn4s, vn4e;

	QVector3D crease_s = (v1e->pos + v2s->pos) / 2;
	QVector3D crease_e = (v2e->pos + v1s->pos) / 2;
	QVector3D horizontal = crease_e - crease_s;
	horizontal.normalize();

	//get vector perpendicular to the edge and point into the mesh
	QVector3D vertical = -(normal1 + normal2) / 2;
	vertical.normalize();

	//input angle theta should be in Radians
	//rotate horizontal by theta, around local axis perpenducular to crease plane
	/*QVector3D interior_folding = horizontal * cos(theta) + vertical * sin(theta);
	interior_folding.normalize();*/

	double crease_len = (crease_e - crease_s).length();


	vn1s = (v1e->pos * 2.0 / 3.0 + v2s->pos * 1.0 / 3.0 + vertical * smooth_length);
	vn1e = (v1s->pos * 2.0 / 3.0 + v2e->pos * 1.0 / 3.0 + vertical * smooth_length);
	vn4s = (v1e->pos * 1.0 / 3.0 + v2s->pos * 2.0 / 3.0 + vertical * smooth_length);
	vn4e = (v1s->pos * 1.0 / 3.0 + v2e->pos * 2.0 / 3.0 + vertical * smooth_length);

	QVector3D tmp1 = vn1s - v1e->pos;
	QVector3D tmp2 = tmp1.length() * tan(Pi / 2 - theta) * (-horizontal);
	vn1s += tmp2;
	vn1e += tmp2;
	vn4s += tmp2;
	vn4e += tmp2;
	QVector3D tmp3 = (folding_length - smooth_length) * tan(Pi / 2 - theta) * (-horizontal);
	QVector3D tmp4 = (folding_length - smooth_length) * vertical;
	QVector3D tmp5 = tmp3 + tmp4;

	if (theta <= Pi / 2) {
		vn2s = vn1s + tmp5;
		vn3s = vn4s + tmp5;

		vn2e = vn1e + tmp5 + tmp5.normalized() * crease_len * sin(Pi / 2 - theta);
		vn3e = vn4e + tmp5 + tmp5.normalized() * crease_len * sin(Pi / 2 - theta);
	}else {
		vn2e = vn1e + tmp5;
		vn3e = vn4e + tmp5;

		vn2s = vn1s + tmp5 + tmp5.normalized() * crease_len * sin(theta - Pi / 2);
		vn3s = vn4s + tmp5 + tmp5.normalized() * crease_len * sin(theta - Pi / 2);
	}

	//#ifdef _DEBUG
	//	if (!he1->isCutEdge) {
	//		std::cout << "vertical:  x: " << vertical.x() << "  y: " << vertical.y() << "  z: " << vertical.z() << std::endl;
	//		std::cout << "normal1:  x: " << normal1.x() << "  y: " << normal1.y() << "  z: " << normal1.z() << std::endl;
	//		std::cout << "normal2:  x: " << normal2.x() << "  y: " << normal2.y() << "  z: " << normal2.z() << std::endl;
	//	}
	//#endif

	//#ifdef _DEBUG
	//std::cout << (vn2s - vn3s).length() << std::endl;
	//std::cout << (vn2e - vn3e).length() << std::endl;
	//#endif

	if (he2->flip->isCutEdge) {
		ctrlPoints_front.push_back(vn1s);
		ctrlPoints_front.push_back(vn2s);
		ctrlPoints_front.push_back((vn2s + vn3s) / 2);
		ctrlPoints_back.push_back(vn1e);
		ctrlPoints_back.push_back(vn2e);
		ctrlPoints_back.push_back((vn2e + vn3e) / 2);
	}
	else {
		ctrlPoints_front.push_back(vn1s);
		ctrlPoints_front.push_back(vn2s);
		ctrlPoints_front.push_back(vn3s);
		ctrlPoints_front.push_back(vn4s);
		ctrlPoints_back.push_back(vn1e);
		ctrlPoints_back.push_back(vn2e);
		ctrlPoints_back.push_back(vn3e);
		ctrlPoints_back.push_back(vn4e);
	}
}

void MeshOrigamizer::setControlPoints2(HDS_HalfEdge* he1, HDS_HalfEdge* he2, double theta, double folding_length, double smooth_length)
{
	if (he1->flip->f != nullptr && !he1->flip->f->isCutFace)
		he1->setCutEdge(false);
	if (he2->flip->f != nullptr && !he2->flip->f->isCutFace)
		he2->setCutEdge(false);

	//clear control points
	ctrlPoints_front.clear();
	ctrlPoints_back.clear();

	//set face normals
	QVector3D normal1, normal2;
	normal1 = he1->flip->f->computeNormal();
	if (!he1->isCutEdge && !he2->isCutEdge) {
		normal2 = he2->flip->f->computeNormal();
	}
	else {
		normal2 = he1->bridgeTwin->flip->f->computeNormal();
	}
	

	/////////////////////////////////////////////////////////////////////
	//end points of scaled mesh edges
	HDS_Vertex *v1s, *v1e, *v2s, *v2e;
	v1s = he1->v;
	v1e = he1->flip->v;
	v2s = he2->v;
	v2e = he2->flip->v;

	//4 pair of new vertices, 
	QVector3D vn1s, vn1e, vn2s, vn2e, vn3s, vn3e, vn4s, vn4e;

	QVector3D crease_s = (v1e->pos + v2s->pos) / 2;
	QVector3D crease_e = (v2e->pos + v1s->pos) / 2;
	QVector3D horizontal = crease_e - crease_s;
	horizontal.normalize();

	//get vector perpendicular to the edge and point into the mesh
	QVector3D vertical = -(normal1 + normal2) / 2;
	vertical.normalize();

	//input angle theta should be in Radians
	//rotate horizontal by theta, around local axis perpenducular to crease plane
	/*QVector3D interior_folding = horizontal * cos(theta) + vertical * sin(theta);
	interior_folding.normalize();*/

	double crease_len = (crease_e - crease_s).length();


	vn1s = (v1e->pos * 2.0 / 3.0 + v2s->pos * 1.0 / 3.0 + vertical * smooth_length);
	vn1e = (v1s->pos * 2.0 / 3.0 + v2e->pos * 1.0 / 3.0 + vertical * smooth_length);
	vn2s = (v1e->pos * 2.0 / 3.0 + v2s->pos * 1.0 / 3.0 + vertical * folding_length);
	vn2e = (v1s->pos * 2.0 / 3.0 + v2e->pos * 1.0 / 3.0 + vertical * folding_length);
	vn3s = (v1e->pos * 1.0 / 3.0 + v2s->pos * 2.0 / 3.0 + vertical * folding_length);
	vn3e = (v1s->pos * 1.0 / 3.0 + v2e->pos * 2.0 / 3.0 + vertical * folding_length);
	vn4s = (v1e->pos * 1.0 / 3.0 + v2s->pos * 2.0 / 3.0 + vertical * smooth_length);
	vn4e = (v1s->pos * 1.0 / 3.0 + v2e->pos * 2.0 / 3.0 + vertical * smooth_length);


	if (theta <= Pi / 2) {
		vn2s = vn2s + vertical * crease_len * tan(Pi / 2 - theta);
		vn3s = vn3s + vertical * crease_len * tan(Pi / 2 - theta);
	}else {
		vn2e = vn2e + vertical * crease_len * tan(theta - Pi / 2);
		vn3e = vn3e + vertical * crease_len * tan(theta - Pi / 2);
	}



//#ifdef _DEBUG
//	if (!he1->isCutEdge) {
//		std::cout << "vertical:  x: " << vertical.x() << "  y: " << vertical.y() << "  z: " << vertical.z() << std::endl;
//		std::cout << "normal1:  x: " << normal1.x() << "  y: " << normal1.y() << "  z: " << normal1.z() << std::endl;
//		std::cout << "normal2:  x: " << normal2.x() << "  y: " << normal2.y() << "  z: " << normal2.z() << std::endl;
//	}
//#endif

	if (he2->flip->isCutEdge) {
		ctrlPoints_front.push_back(vn1s);
		ctrlPoints_front.push_back(vn2s);
		ctrlPoints_front.push_back((vn2s + vn3s) / 2);
		ctrlPoints_back.push_back(vn1e);
		ctrlPoints_back.push_back(vn2e);
		ctrlPoints_back.push_back((vn2e + vn3e) / 2);
	}
	else {
		ctrlPoints_front.push_back(vn1s);
		ctrlPoints_front.push_back(vn2s);
		ctrlPoints_front.push_back(vn3s);
		ctrlPoints_front.push_back(vn4s);
		ctrlPoints_back.push_back(vn1e);
		ctrlPoints_back.push_back(vn2e);
		ctrlPoints_back.push_back(vn3e);
		ctrlPoints_back.push_back(vn4e);
	}
}


void MeshOrigamizer::addBridger(HDS_HalfEdge* he1, HDS_HalfEdge* he2, double theta, double folding_length, double smooth_length)
{
	//set control points
	//bridger_type: 0 -> parallel bridger; 1 -> non-parallel bridger
	switch (bridger_type) {
	default:
	case 0:
		setControlPoints(he1, he2, theta, folding_length, smooth_length);
		break;
	case 1:
		setControlPoints2(he1, he2, theta, folding_length, smooth_length);
		break;
	}

	
	//set cut faces
	face_t* cutFace1 = he1->f;
	face_t* cutFace2 = he2->f;
	
	//create verts, hes, faces corresponding to new bridger
	bridger_verts.clear();
	bridger_hes.clear();
	bridger_faces.clear();

	for (int i = 0; i < ctrlPoints_front.size(); ++i) {
		if ((i == ctrlPoints_front.size() - 1) && (he2->flip->isCutEdge)) {
			//he2 is cutedge, just build a half-bridger 
			he2->v->pos = ctrlPoints_front[i];
			he2->flip->v->pos = ctrlPoints_back[i];
		}else {
			HDS_Vertex* vs = new HDS_Vertex;
			HDS_Vertex* ve = new HDS_Vertex;

			vs->pos = ctrlPoints_front[i];
			ve->pos = ctrlPoints_back[i];
			HDS_HalfEdge* he_new = HDS_Mesh::insertEdge(vs, ve);

			//save added bridger info
			bridger_hes.push_back(he_new);
			bridger_verts.push_back(vs);
			bridger_verts.push_back(ve);
		}
	}

	//create bridger segments
	vector<he_t*> hes_ori = bridger_hes;
	hes_ori.insert(hes_ori.begin(), he1->flip);
	hes_ori.push_back(he2);

	for (auto he = hes_ori.begin(); he != prev(hes_ori.end()); he++) {
		auto he_next = next(he);
		//bridge each pair of edges
		//get bridge faces, set to Bridger->faces
		HDS_Face* bridgeFace = bridging((*he)->flip, *he_next, cutFace1, cutFace2);
		//fix face
		bridgeFace->isCutFace = false;
		bridgeFace->isBridger = true;
		//add face to mesh
		bridger_faces.push_back(bridgeFace);
	}

	//copy bridger geomery info to new mesh
	hes_new.insert(hes_new.end(), bridger_hes.begin(), bridger_hes.end());
	faces_new.insert(faces_new.end(), bridger_faces.begin(), bridger_faces.end());
	verts_new.insert(verts_new.end(), bridger_verts.begin(), bridger_verts.end());

	bridger_verts.clear();
	bridger_hes.clear();
	bridger_faces.clear();
	ctrlPoints_front.clear();
	ctrlPoints_back.clear();
}

bool MeshOrigamizer::origamiMesh( HDS_Mesh * mesh )
{
	//////////////////////////////////////////////////////////
	HDS_Bridger::setScale(origami_scale);

	initiate();
	cur_mesh = mesh;
	unordered_map<hdsid_t, vert_t*> ori_map = ori_mesh->vertMap;

	//seperateFaces();
	scaleFaces();

	//get bridge pairs
	unordered_map<hdsid_t, he_t*> refidMap;
	unordered_map<he_t*, QVector2D> rotationMap;
	unordered_map<hdsid_t, he_t*> heIdMap;
	unordered_map<he_t*, he_t*> flipMap;
	unordered_map<hdsid_t, vector<he_t*>> vertHeMap;
	unordered_map<hdsid_t, bool> vertMarked;
	queue<hdsid_t> markQueue;

	for (auto& he : hes_new) {
		vertHeMap[he->v->refid].push_back(he);
		if (heIdMap[he->refid]) {
			he_t* flip = heIdMap[he->refid];
			heIdMap.erase(he->refid);
			flipMap[he] = flip;
			flipMap[flip] = he;
		}else {
			heIdMap[he->refid] = he;
		}
	}


	cout << "+++++++++++++++++++++++++++++++++++++++++++++" << endl;

	auto rotateVector2D = [](QVector2D vec, double angle) -> QVector2D {
		return QVector2D(vec.x() * cos(angle) + vec.y() * sin(angle), -vec.x() * sin(angle) + vec.y() * cos(angle));
	};

	auto rotateFace = [&rotationMap, &flipMap, &rotateVector2D, &markQueue, &vertMarked] (he_t* he, QVector2D vec) -> void {
		if (he->f->isMarked)
			return;
		he->f->isMarked = true;
		he_t* curHe = he;
		cout << "------ face angles ------------" << endl;
		do {
			rotationMap[curHe] = vec;
			he_t* nextHe = curHe->next;
			QVector3D v1 = curHe->v->pos - curHe->flip->v->pos;
			QVector3D v2 = nextHe->flip->v->pos - nextHe->v->pos;
			double nv1pnv2 = v1.length() * v2.length();
			double inv_nv1pnv2 = 1.0 / nv1pnv2;
			double cosVal = QVector3D::dotProduct(v1, v2) * inv_nv1pnv2;
			double angle = acos(clamp<double>(cosVal, -1.0, 1.0));
			cout << angle;
			cout << " : " << vec.x() << " : " << vec.y() << endl;
			//vec = QVector2D(-vec.x() * cos(angle) - vec.y() * sin(angle), vec.x() * sin(angle) - vec.y() * cos(angle));
			vec = rotateVector2D(-vec, angle);
			if (!vertMarked[curHe->v->refid]) {
				markQueue.push(curHe->v->refid);
			}
			curHe = nextHe;
		} while (curHe != he);
	};

	auto clockWiseRotate = [](QVector2D before, QVector2D after) -> double {
		double cosVal = QVector2D::dotProduct(before, after) / (before.length() * after.length());
		double angle = acos(clamp<double>(cosVal, -1.0, 1.0));
		double sinVal = (after.x() * before.y() - after.y() * before.x()) / (pow(before.x(), 2) + pow(before.y(), 2));
		if (sinVal < -0.000001) {
			angle = PI2 - angle;
		}
		return angle;
	};
	

	auto calcAngle3D = [](QVector3D v1, QVector3D v2) -> double {
		double nv1pnv2 = v1.length() * v2.length();
		double inv_nv1pnv2 = 1.0 / nv1pnv2;
		double cosVal = QVector3D::dotProduct(v1, v2) * inv_nv1pnv2;
		double angle = acos(clamp<double>(cosVal, -1.0, 1.0));
		return angle;
	};

	
	
	//randomly start from a vertex
	markQueue.push(hes_new[0]->v->refid);
	rotateFace(hes_new[0], QVector2D(1, 0));
	
	while (!markQueue.empty()) {
		hdsid_t vertId = markQueue.front();
		markQueue.pop();
		if (vertMarked[vertId])
			continue;
		vertMarked[vertId] = true;
		he_t* curHe = vertHeMap[vertId][0];
		//find first marked face connect to the vertex
		while (!curHe->f->isMarked) {
			curHe = flipMap[curHe]->next;
		}
		he_t* startHe = curHe;
		do {
			he_t* curFlip = flipMap[curHe];
			he_t* nextHe = curFlip->next;
			double meshAngleSum = 0;
			int anglesCnt = 0;
			vector<he_t*> flipsToRotate;
			while (!curFlip->f->isMarked) {
				flipsToRotate.push_back(curFlip);
				QVector3D v1 = curFlip->v->pos - curHe->flip->v->pos;
				QVector3D v2 = nextHe->flip->v->pos - nextHe->v->pos;
				double angle = calcAngle3D(v1, v2); 
				meshAngleSum += angle;
				anglesCnt++;
				curFlip = flipMap[nextHe];
				nextHe = curFlip->next;
			}
			double totalAngleSum = clockWiseRotate(rotationMap[curHe], -rotationMap[curFlip]);
			double averageAngle = (totalAngleSum - meshAngleSum) / (anglesCnt + 1);
			for (auto& he : flipsToRotate) {
				QVector2D rotatedVec = rotateVector2D(rotationMap[flipMap[he]], averageAngle);
				rotateFace(he, rotatedVec);
			}
			curHe = nextHe;
		} while (curHe != startHe);
	}
	
	

	for (auto& heList : vertHeMap) {
		cout << (heList.first >> 2) << " : ";
		for (auto& he : heList.second) {
			cout << (he->refid >> 2) << " - ";
		}
		cout << endl;
		auto he = heList.second[0];
		auto curHe = he;
		do{
			cout << (curHe->refid >> 2) << " - ";
			curHe = flipMap[curHe]->next;
		} while (curHe != he);
		cout << endl;
	}

	cout << "+++++++++++++++++++++++++++++++++++++++++++++" << endl;


	for (auto vert : verts_new) {
		//cout << (vert->he->refid >> 3) << endl;
		/*
		double sum = 0;
		auto he = vert->he;
		auto curHE = he->flip->next;
		bool hasCutFace = false;
		do {
			if (!curHE->f->isCutFace) {
				//calculate vertex angle defect
				QVector3D v1 = he->flip->v->pos - he->v->pos;
				QVector3D v2 = curHE->flip->v->pos - curHE->v->pos;
				double nv1pnv2 = v1.length() * v2.length();
				double inv_nv1pnv2 = 1.0 / nv1pnv2;
				double cosVal = QVector3D::dotProduct(v1, v2) * inv_nv1pnv2;
				double angle = acos(clamp<double>(cosVal, -1.0, 1.0));
				sum += angle;
			}
			else hasCutFace = true;
			he = curHE;

			curHE = he->flip->next;
		} while (he != vert->he);
		cout << "angle: " << sum << endl;
		*/
	}
	
	cout << "+++++++++++++++++++++++++++++++++++++++++++++" << endl;
	/*
	//print int in binary form
	auto print_bin = [](int n) -> void{
		int l = sizeof(n) * 8;
		int i;
		if (i == 0)
		{
			printf("0\n");
			return;
		}
		for (i = l - 1; i >= 0; i--)
		{
			if (n&(1 << i)) break;
		}

		for (; i >= 0; i--)
			printf("%d", (n&(1 << i)) != 0);
		printf("\n");
		return;
	};
	*/

	for (auto he : hes_new) {
		if(refidMap.find(he->flip->refid) == refidMap.end()) {
			refidMap.insert(make_pair(he->refid, he));
		}
		else {
			he->flip->setBridgeTwin(refidMap[he->refid]->flip);
		}
	}


	//assign flip s face
	for (auto& he_inner : hes_new) {
		he_t* he = he_inner->flip;
		if (he->f == nullptr) {
			//find nearest cut face, if not found set a new one
			he_t* curHE = he;

			//rotate around vertex to check if already exist a cutface
			do {
				curHE = curHE->bridgeTwin->next;
				if (curHE->f != nullptr) {
					he->f = curHE->f;
					break;
				}
			} while (curHE != he);

			//after check, if no cut face, create one
			if (he->f == nullptr) {
				face_t* cutFace = new face_t;
				cutFace->isCutFace = true;
				he->f = cutFace;
				cutFace->he = he;
				faces_new.push_back(cutFace);
			}
		}
	}

	vector<face_t*> ref_faces = faces_new;
	for (auto& ref_face : ref_faces) {
		;
	}

	
	cout << "+++++++++++++++++++++ Add Bridger ++++++++++++++++++++++++" << endl;

	for (auto& heMap : refidMap) {
		he_t* he = heMap.second->flip;
		if (!he->isCutEdge) {
			//for all non-cut-edge edges, create bridge faces
			double angle = clockWiseRotate(rotationMap[he->bridgeTwin->flip], -rotationMap[he->flip]);
			cout << angle << endl;
			//addBridger(he, he->bridgeTwin, tucked_angle, tucked_length, tucked_smooth);
			angle += Pi / 2;
			addBridger(he, he->bridgeTwin, angle, tucked_length, tucked_smooth);
		}
		else {
			// for all cut-edge edges, create flaps
			std::cout << "cut edge bridger" << std::endl;
			he_t* twin_he = he->bridgeTwin;
			vert_t* flap_vs = new vert_t;
			vert_t* flap_ve = new vert_t;
			flap_vs->pos = twin_he->v->pos;
			flap_ve->pos = twin_he->flip->v->pos;

			//warning. assign refid, to be tested
			flap_vs->refid = twin_he->v->refid;
			flap_ve->refid = twin_he->flip->v->refid;

			verts_new.push_back(flap_vs);
			verts_new.push_back(flap_ve);

			he_t* flap_he = HDS_Mesh::insertEdge(flap_vs, flap_ve);
			flap_he->setCutEdge(true);
			flap_he->f = he->f;
			flap_he->flip->f = he->f;
			flap_he->refid = twin_he->refid;
			hes_new.push_back(flap_he);


			vert_t* twin_flap_vs = new vert_t;
			vert_t* twin_flap_ve = new vert_t;
			twin_flap_vs->pos = he->v->pos;
			twin_flap_ve->pos = he->flip->v->pos;

			//warning. assign refid, to be tested
			twin_flap_vs->refid = he->v->refid;
			twin_flap_ve->refid = he->flip->v->refid;

			verts_new.push_back(twin_flap_vs);
			verts_new.push_back(twin_flap_ve);

			he_t* twin_flap_he = HDS_Mesh::insertEdge(twin_flap_vs, twin_flap_ve);
			twin_flap_he->setCutEdge(true);
			twin_flap_he->f = twin_he->f;
			twin_flap_he->flip->f = twin_he->f;

			twin_flap_he->refid = he->refid;
			hes_new.push_back(twin_flap_he);

			//new angle calc method
			double angle = clockWiseRotate(rotationMap[he->flip], -rotationMap[twin_he->flip]);
			cout << angle << endl;
			angle += Pi / 2;

			//addBridger(he, flap_he, tucked_angle, tucked_length, tucked_smooth);
			//addBridger(twin_he, twin_flap_he, Pi - tucked_angle, tucked_length, tucked_smooth);
			addBridger(he, flap_he, angle, tucked_length, tucked_smooth);
			addBridger(twin_he, twin_flap_he, Pi - angle, tucked_length, tucked_smooth);
		}
	}


	updateNewMesh();
	cur_mesh->setProcessType(HDS_Mesh::ORIGAMI_PROC);
#ifdef _DEBUG
	std::cout << "origami succeed............." << std::endl;
#endif
	return true;
}

void MeshOrigamizer::seperateFaces() {
	for (auto f : cur_mesh->faces()) {
		if (!f->isCutFace) {
			auto fCorners = f->corners();
			vector<vert_t*> vertices;
			for (int i = 0; i < fCorners.size(); ++i) {
				vert_t* v_new = new vert_t;
				v_new->pos = fCorners[i]->pos;
				v_new->refid = fCorners[i]->refid;
				vertices.push_back(v_new);
				verts_new.push_back(v_new);
			}

			face_t* newFace = createFace(vertices);
			newFace->refid = f->refid;
			newFace->isCutFace = false;
			newFace->isBridger = false;
			faces_new.push_back(newFace);

			he_t* newHE = newFace->he;
			he_t* curHE = f->he;

			do {
				newHE->refid = curHE->refid;
				newHE->flip->refid = curHE->flip->refid;
				newHE->setCutEdge(curHE->isCutEdge);
				if (newHE->isCutEdge) {
					face_t* newCutFace = new face_t(*(curHE->flip->f));
					newHE->flip->f = newCutFace;
					faces_new.push_back(newCutFace);
				}
				newHE = newHE->next;
				curHE = curHE->next;
			} while (newHE != newFace->he);
		}
	}
}


