#include "MeshFactory/MeshExtender.h"
#include "HDS/HDS_Mesh.h"

#include "Utils/utils.h"
#include "Utils/mathutils.h"


const HDS_Mesh* MeshExtender::ori_mesh;
HDS_Mesh* MeshExtender::cur_mesh;

vector<HDS_HalfEdge> MeshExtender::hes_new;
vector<HDS_Vertex> MeshExtender::verts_new;
vector<HDS_Face> MeshExtender::faces_new;

void MeshExtender::initiate()
{
	hes_new.clear();
	verts_new.clear();
	faces_new.clear();
}

void MeshExtender::setOriMesh(const HDS_Mesh* mesh)
{
	ori_mesh = mesh;
}

vector<QVector3D> MeshExtender::scaleBridgerEdge(
	const vert_t &v1, const vert_t &v2)
{
	vector <QVector3D> vpair;
#ifdef USE_LEGACY_FACTORY
    double scale = HDS_Bridger::getScale();
    //obsolete scale function
	QVector3D v1pos = v1.pos;
	QVector3D v2pos = v2.pos;

	QVector3D vmid = (v1pos + v2pos)/2.0;
	vpair.push_back( (1 - scale)* vmid + scale *v1pos );
	vpair.push_back( (1 - scale)* vmid + scale *v2pos )

#endif
	return vpair;
}

void MeshExtender::addBridger(
	HDS_HalfEdge* he1, HDS_HalfEdge* he2, vector<QVector3D> vpos)
{
#ifdef USE_LEGACY_FACTORY
	HDS_Bridger* Bridger = new HDS_Bridger(he1, he2, vpos);
	Bridger->setCutFace(he1->f, he2->f);
	createBridger(Bridger);
#endif
}

void MeshExtender::createBridger(HDS_Bridger* Bridger)
{
	//new Bridger object
	Bridger->createBridge();

	hes_new.insert( hes_new.end(), Bridger->hes.begin(), Bridger->hes.end());
	faces_new.insert( faces_new.end(), Bridger->faces.begin(), Bridger->faces.end());
	verts_new.insert( verts_new.end(), Bridger->verts.begin(), Bridger->verts.end() );

}

void MeshExtender::scaleFaces()
{
#ifdef USE_LEGACY_FACTORY
	for (auto f: cur_mesh->faces()) {
		if (!f.isCutFace){
			f.setScaleFactor(HDS_Bridger::getScale());
			auto fCorners = f.corners();
			auto fScaledCorners = f.getScaledCorners();
			vector<vert_t*> vertices;
			for (int i = 0; i < fCorners.size(); i++) {
				vert_t* v_new = new vert_t;
				v_new->pos = fScaledCorners[i];
				v_new->refid = fCorners[i]->refid;
				vertices.push_back(v_new);
				verts_new.push_back(*v_new);
			}

			face_t* newFace = createFace(vertices);
			newFace->refid = f.refid;
			newFace->isCutFace = false;
			newFace->isBridger = false;
			faces_new.push_back(*newFace);

			//assign half edge's refid
			//get current face's edges
			he_t* newHE = newFace->he;
			he_t* curHE = f.he;

			do {
				newHE->refid = curHE->refid;
				newHE->flip()->refid = curHE->flip()->refid;
				newHE->setCutEdge(curHE->isCutEdge);
				if (newHE->isCutEdge){
					face_t* newCutFace = new face_t(*(curHE->flip()->f));
					newHE->flip()->f = newCutFace;
					faces_new.push_back(*newCutFace);
				}
				newHE = newHE->next;
				curHE = curHE->next;
			}while(newHE!= newFace->he);
		}
	}
#endif
}

HDS_Face* MeshExtender::createFace(vector<HDS_Vertex*> vertices, face_t* cutFace)
{
#ifdef USE_LEGACY_FACTORY
	face_t * newFace = new face_t();
	if (HDS_Mesh::incidentEdge(vertices.front(), vertices.back()) == nullptr) {
		vertices.push_back(vertices.front());//form a loop
	}

	auto preV = vertices.front();
	for (int i = 1; i < vertices.size(); i++)
	{

		auto& curV = vertices[i];
		he_t* newHE = HDS_Mesh::insertEdge(preV, curV);

		if (cutFace != nullptr){
			newHE->flip()->f = cutFace;
			newHE->setCutEdge(true);
			if (cutFace->he == nullptr)
				cutFace->he = newHE->flip;
		}

		if(newFace->he == nullptr)
			newFace->he = newHE;

		newHE->f = newFace;


		hes_new.push_back(*newHE);
		preV = curV;
	}

	return newFace;
#else
	return nullptr;
#endif
}

HDS_Face* MeshExtender::duplicateFace(face_t* face, face_t* cutFace)
{
	vector<vert_t*> vertices;

	// TODO: replace by hds_mesh function
	vector<vert_t*> fCorners;// = face->corners();
	for (auto v : fCorners) {
		vertices.push_back(new vert_t(v->pos));
	}
	face_t* newFace = createFace(vertices, cutFace);
	newFace->refid = face->refid;
	faces_new.push_back(*newFace);
	for (auto newv : vertices)
	{
		verts_new.push_back(*newv);
	}
	return newFace;
}

HDS_HalfEdge* MeshExtender::duplicateEdge(he_t* edge)
{
#ifdef USE_LEGACY_FACTORY
	vert_t* vs = new vert_t(edge->v->pos);
	vert_t* ve = new vert_t(edge->flip()->v->pos);
	verts_new.push_back(*vs);
	verts_new.push_back(*ve);
	he_t* newEdge = HDS_Mesh::insertEdge(vs, ve);
	hes_new.push_back(*newEdge);

	edge->setCutEdge(true);
	newEdge->setCutEdge(true);
	return newEdge;
#else
	return nullptr;
#endif
}

void MeshExtender::assignCutFace(face_t* face, face_t* cutFace)
{
#ifdef USE_LEGACY_FACTORY
	he_t* curHE = face->he;
	do {
		if (cutFace->he == nullptr)
			cutFace->he = curHE->flip;
		curHE->flip()->f = cutFace;
	}while(curHE != face->he);
#endif
}

bool MeshExtender::extendMesh(HDS_Mesh *mesh)
{
#ifdef USE_LEGACY_FACTORY
	// TODO: store bridgeTwin locally
	initiate();
	cur_mesh = mesh;
	if (HDS_Bridger::getScale() == 1) {
		HDS_Bridger::setScale(0.99);
	}
	vector<vert_t> ori_vec = ori_mesh->verts();
	//unordered_map<hdsid_t, vert_t*> ori_map = ori_mesh->vertMap;

	scaleFaces();

	//get bridge pairs
	unordered_map<hdsid_t, he_t*> refidMap;
	for (auto he: hes_new) {
		if (refidMap.find(he.flip()->refid) == refidMap.end()) {
			refidMap.insert(make_pair(he.refid, &he));
		}else {
			he.flip()->setBridgeTwin(refidMap[he.refid]->flip);
		}
	}

//	for (auto f: cur_mesh->faces()) {
//		if(f->isCutFace){
//			faces_new.push_back(new face_t(*f));
//		}

//	}

	//assign flip s face
	for (auto he_inner: hes_new){
		he_t* he = he_inner.flip;

		if (he->f == nullptr){

			//find nearest cut face, if not found set a new one
			he_t* curHE = he;
			do {
				curHE = curHE->bridgeTwin->next;
				if (curHE->f != nullptr) {
					he->f = curHE->f;
					break;
				}
			}while (curHE != he);

			if (he->f == nullptr) {
				face_t* cutFace = new face_t;
				cutFace->isCutFace = true;
				he->f = cutFace;
				cutFace->he = he;
				faces_new.push_back(*cutFace);
			}
		}
	}


	for (auto& heMap: refidMap) {
		he_t* he = heMap.second->flip;
		if (!he->isCutEdge){
			///for all non-cut-edge edges, create bridge faces
			HDS_Vertex v1_ori = ori_vec[(he->v->refid)>>2];
			HDS_Vertex v2_ori = ori_vec[(he->bridgeTwin->v->refid)>>2];
			vector <QVector3D> vpair = scaleBridgerEdge(v1_ori, v2_ori);
			addBridger(he, he->bridgeTwin, vpair);

		}else {
			// for all cut-edge edges, create flaps
			cout<<"cut edge bridger"<<endl;
			he_t* twin_he = he->bridgeTwin;
			vert_t* flap_vs = new vert_t;
			vert_t* flap_ve = new vert_t;
			flap_vs->pos = twin_he->v->pos;
			flap_ve->pos = twin_he->flip()->v->pos;

			//warning. assign refid, to be tested
			flap_vs->refid = twin_he->v->refid;
			flap_ve->refid = twin_he->flip()->v->refid;

			verts_new.push_back(*flap_vs);
			verts_new.push_back(*flap_ve);

			he_t* flap_he = HDS_Mesh::insertEdge(flap_vs, flap_ve);
			flap_he->setCutEdge(true);
			flap_he->f = he->f;
			flap_he->flip()->f = he->f;
			flap_he->refid = twin_he->refid;
			hes_new.push_back(*flap_he);

			HDS_Vertex v1_ori = ori_vec[(he->v->refid)>>2];
			HDS_Vertex v2_ori = ori_vec[(flap_he->v->refid)>>2];
			vector <QVector3D> vpair = scaleBridgerEdge(v1_ori, v2_ori);


			vert_t* twin_flap_vs = new vert_t;
			vert_t* twin_flap_ve = new vert_t;
			twin_flap_vs->pos = he->v->pos;
			twin_flap_ve->pos = he->flip()->v->pos;

			//warning. assign refid, to be tested
			twin_flap_vs->refid = he->v->refid;
			twin_flap_ve->refid = he->flip()->v->refid;

			verts_new.push_back(*twin_flap_vs);
			verts_new.push_back(*twin_flap_ve);

			he_t* twin_flap_he = HDS_Mesh::insertEdge(twin_flap_vs, twin_flap_ve);
			twin_flap_he->setCutEdge(true);
			twin_flap_he->f = twin_he->f;
			twin_flap_he->flip()->f = twin_he->f;

			twin_flap_he->refid = he->refid;
			hes_new.push_back(*twin_flap_he);

			addBridger(he, flap_he, vpair);
			vector<QVector3D> vpair_reverse =scaleBridgerEdge(v2_ori, v1_ori);
			addBridger(twin_he, twin_flap_he, vpair_reverse);

		}

	}

	updateNewMesh();
	cur_mesh->processType = HDS_Mesh::GRS_PROC;
#ifdef _DEBUG
	cout << "extend succeed............." << endl;
#endif
#endif
	return true;

}

bool MeshExtender::updateNewMesh()
{
#ifdef USE_LEGACY_FACTORY
	//for debugging...
	int bridgerCount = 0;

	cur_mesh->releaseMesh();

	HDS_Vertex::resetIndex();
	HDS_HalfEdge::resetIndex();
	HDS_Face::resetIndex();

	for (auto f : faces_new) {
		f.index = HDS_Face::assignIndex();
		cur_mesh->addFace(f);
		if (f.isBridger) bridgerCount++;
	}
	//add new vertices and edges
	for (auto v : verts_new) {
		v.index = HDS_Vertex::assignIndex();
		cur_mesh->addVertex(v);
	}
	for (auto he : hes_new) {
		he.index = HDS_HalfEdge::assignIndex();
		he.flip()->index = HDS_HalfEdge::assignIndex();
		cur_mesh->addHalfEdge(he);
		cur_mesh->addHalfEdge(*(he.flip));
	}

	if (cur_mesh->validate()) return true;
	else return false;
#else
	return false;
#endif
}
