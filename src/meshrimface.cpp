#include "meshrimface.h"
#include "MeshExtender.h"
#include "common.h"

#define PI 3.14159265358

void MeshRimFace::computePlaneCornerOnEdge(vert_t* v, he_t* he, vector<QVector3D> &vpos) {
	///get planes on the edge
	vert_t* vp = he->flip->prev->v;
	vert_t* v1 = he->flip->v;
	vert_t* vn = he->prev->v;
	//get normals for plane prev and plane next
	QVector3D np = QVector3D::normal(v->pos, vp->pos, v1->pos);
	QVector3D nn = QVector3D::normal(v->pos, v1->pos, vn->pos);
	QVector3D n = np + nn;
	n.normalize();

	//get face_n face_p center
	QVector3D fn_center = he->f->center();
	QVector3D fp_center = he->flip->f->center();

	//get middle point f the edge
	QVector3D v_mid = (v->pos + v1->pos)/2;

	//get projected centers
	cout<<fn_center.distanceToPlane(v->pos,n)<<endl;
	QVector3D vn_max = fn_center - n * fn_center.distanceToPlane(v->pos,n);
	QVector3D vp_max = fp_center - n * fp_center.distanceToPlane(v->pos,n);

	QVector3D vn_mid = planeWidth * vn_max + (1- planeWidth) * v_mid;
	QVector3D vp_mid = planeWidth * vp_max + (1- planeWidth) * v_mid;

	QVector3D vn_i_pos = ( planeHeight * v->pos + (1 - planeHeight) * vn_mid);
	QVector3D vn_o_pos = (-planeHeight * v->pos + (1 + planeHeight) * vn_mid);
	QVector3D vp_i_pos = ( planeHeight * v->pos + (1 - planeHeight) * vp_mid);
	QVector3D vp_o_pos = (-planeHeight * v->pos + (1 + planeHeight) * vp_mid);

	vpos.push_back(vn_i_pos);
	vpos.push_back(vn_o_pos);
	vpos.push_back(vp_i_pos);
	vpos.push_back(vp_o_pos);

}

void MeshRimFace::computePlaneCornerOnFace(vert_t* v, he_t* he, vector<QVector3D> &control_points_p, vector<QVector3D> &control_points_n) {

	//get center of the face
	QVector3D top = v->pos;
	QVector3D center = he->f->center();
	vert_t* vp = he->prev->v;
	vert_t* vn = he->flip->v;

	QVector3D bot;
	HDS_Face::LineLineIntersect(v->pos, center,vn->pos, vp->pos, &bot);

	QVector3D v_up = ( planeHeight * top + (1 - planeHeight) * center);
	QVector3D v_down = ( planeHeight * bot + (1 - planeHeight) * center);

	vert_t* vup = new vert_t(v_up);
	vert_t* vdown = new vert_t(v_down);
	verts_new.push_back(vup);
	verts_new.push_back(vdown);
	he_t* he_mid = HDS_Mesh::insertEdge(vup, vdown);
	he_mid->refid = he->refid;
	hes_new.push_back(he_mid);
	control_edges.push_back(he_mid);

	//get vector vp to vn
	QVector3D cross = (vn->pos - vp->pos);
	cross.normalize();

	//get v_up_prev, v_down_prev, v_up_next, v_down_next boundaries on edge
	QVector3D v_up_prev_max;
	HDS_Face::LineLineIntersect(v->pos, vp->pos, v_up, v_up - cross, &v_up_prev_max);
	QVector3D v_down_prev_max;
	HDS_Face::LineLineIntersect(v->pos, vp->pos, v_down, v_down - cross, &v_down_prev_max);

	QVector3D v_up_next_max;
	HDS_Face::LineLineIntersect(v->pos, vn->pos, v_up, v_up + cross, &v_up_next_max);
	QVector3D v_down_next_max;
	HDS_Face::LineLineIntersect(v->pos, vn->pos, v_down, v_down + cross, &v_down_next_max);

	//get v_up_prev, v_down_prev, v_up_next, v_down_next
	QVector3D v_up_prev = ( planeWidth * v_up_prev_max + (1 - planeWidth) * v_up);
	QVector3D v_down_prev = ( planeWidth * v_down_prev_max + (1 - planeWidth) * v_down);
	QVector3D v_up_next = ( planeWidth * v_up + (1 - planeWidth) * v_up_next_max);
	QVector3D v_down_next = ( planeWidth * v_down + (1 - planeWidth) * v_down_next_max);

	control_points_p.push_back(v_up_next);
	control_points_p.push_back(v_down_next);
	control_points_n.push_back(v_up_prev);
	control_points_n.push_back(v_down_prev);
}

void MeshRimFace::rimMesh3D(HDS_Mesh *mesh, int rimType, float planeWidth, float planeHeight)
{
	initiate();
	cur_mesh = mesh;
	this->planeWidth = planeWidth;
	this->planeHeight = planeHeight;
	HDS_Bridger::setSamples(32);

	unordered_map <hdsid_t, he_t*> ori_map = ori_mesh->halfedgesMap();


	for(vert_t* v: cur_mesh->verts()) {
		///assign cut faces
		face_t* cutFace = new face_t;
		cutFace->isCutFace = true;
		faces_new.push_back(cutFace);
		vector<face_t*> pieces;
		vector<he_t*> control_edges;
		vector<QVector3D> control_points_n;
		vector<QVector3D> control_points_p;

		for (he_t* he: mesh->incidentEdges(v)) {

			switch(rimType) {
			case EdgeWithBezier: {
				vector<QVector3D> vpos;
				computePlaneCornerOnEdge(he, vpos);
				vert_t* vn_i = new vert_t(vpos[0]);
				vert_t* vn_o = new vert_t(vpos[1]);
				vert_t* vp_i = new vert_t(vpos[2]);
				vert_t* vp_o = new vert_t(vpos[3]);

				vn_i->refid = v->refid;
				vn_o->refid = v1->refid;
				vp_i->refid = v->refid;
				vp_o->refid = v1->refid;

				vector<vert_t*> vertices;
				vertices.push_back(vp_i);
				vertices.push_back(vp_o);
				vertices.push_back(vn_o);
				vertices.push_back(vn_i);

				verts_new.insert(verts_new.end(), vertices.begin(), vertices.end());

				face_t* newFace = createFace(vertices, cutFace);
				newFace->refid = he->refid;
				faces_new.push_back(newFace);
				pieces.push_back(newFace);
				break;
			}
			case EdgeWithCubic: {
				vector<QVector3D> vpos;
				computePlaneCornerOnEdge(he, vpos);
				control_points_n.push_back(vpos[0]);
				control_points_n.push_back(vpos[1]);
				control_points_p.push_back(vpos[2]);
				control_points_p.push_back(vpos[3]);

				vert_t* vmid_i = new vert_t( planeHeight * v->pos + (1 - planeHeight) * v_mid);
				vert_t* vmid_o = new vert_t(-planeHeight * v->pos + (1 + planeHeight) * v_mid);
				verts_new.push_back(vmid_i);
				verts_new.push_back(vmid_o);
				he_t* he_mid = HDS_Mesh::insertEdge(vmid_i, vmid_o);
				he_mid->refid = he->refid;
				hes_new.push_back(he_mid);
				control_edges.push_back(he_mid);
				break;
			}
			case FaceWithCubic:
				computePlaneCornerOnFace(v,he,control_points_p,control_points_n);
				break;
			}

		}
		///connect pieces

		float angleSum = 0;

		switch(rimType) {
		case EdgeWithBezier: {
			for(int i = 0; i < pieces.size(); i++) {
				face_t* curFace = pieces[i];
				face_t* nextFace;

				//check for negative curvature vertices,
				//if angleSum exceeds 360 degree,
				//cut it, duplicate current piece and start a new cutFace
				if (angleSum > 1.6* PI) {
					cutFace = new face_t;
					cutFace->isCutFace = true;
					faces_new.push_back(cutFace);
					curFace = duplicateFace(pieces[i], cutFace);

					angleSum = 0;
				}

				//when it comes to final piece, duplicate it
				if(i < pieces.size() - 1)
					nextFace = pieces[i+1];
				else {
					//duplicate pieces[0]
					nextFace = duplicateFace(pieces[0], cutFace);
				}

				//calculate angle between pieces' original he
				he_t* curHE = ori_map[(curFace->refid)>>2];
				he_t* nxtHE = ori_map[(nextFace->refid)>>2];
				QVector3D curHE_v = curHE->flip->v->pos - curHE->v->pos;
				QVector3D nxtHE_v = nxtHE->flip->v->pos - nxtHE->v->pos;
				double param = QVector3D::dotProduct(curHE_v, nxtHE_v);
				double angle = acos (param);
				angleSum += angle;

				//update nextFace 's cutFace
				if (nextFace->he->flip->f != cutFace) {
					cout<<"assigning new cut face....."<<endl;
					assignCutFace(nextFace, cutFace);
				}

				//get curFace he1
				he_t* he1 = curFace->he;
				//get nextFace he2
				he_t* he2 = nextFace->he->next->next;
				he1->setCutEdge(false);
				he2->setCutEdge(false);

				vector<vert_t*> curCorners = curFace->corners();
				vector<vert_t*> nxtCorners = nextFace->corners();

				QVector3D v1;
				HDS_Face::LineLineIntersect(nxtCorners[0]->pos, nxtCorners[3]->pos, curCorners[3]->pos, curCorners[0]->pos, &v1);

				QVector3D v2;
				HDS_Face::LineLineIntersect(nxtCorners[1]->pos, nxtCorners[2]->pos, curCorners[2]->pos, curCorners[1]->pos, &v2);
				vector<QVector3D> vpos;
				vpos.push_back(v1);
				vpos.push_back(v2);
				addBridger(he2->flip, he1->flip, vpos);
			}
			break;
		}
		case EdgeWithCubic: case FaceWithCubic: {
			control_points_n.push_back(control_points_n[0]);
			control_points_n.push_back(control_points_n[1]);
			control_points_n.erase(control_points_n.begin(), control_points_n.begin()+2);

			for (int index = 0; index < control_edges.size(); index++) {
				he_t* curEdge = control_edges[index];
				he_t* nxtEdge;

				if (angleSum > 1.6* PI) {
					cutFace = new face_t;
					cutFace->isCutFace = true;
					faces_new.push_back(cutFace);
					curEdge = duplicateEdge(curEdge);

					angleSum = 0;
				}

				if (index < control_edges.size() - 1)
					nxtEdge = control_edges[index+1];
				else {
					nxtEdge = duplicateEdge(control_edges[0]);
				}

				//calculate angle between pieces' original he
				he_t* curHE = ori_map[(curEdge->refid)>>2];
				he_t* nxtHE = ori_map[(nxtEdge->refid)>>2];
				QVector3D curHE_v = curHE->flip->v->pos - curHE->v->pos;
				QVector3D nxtHE_v = nxtHE->flip->v->pos - nxtHE->v->pos;
				double param = QVector3D::dotProduct(curHE_v, nxtHE_v);
				double angle = acos (param);
				angleSum += angle;

				vector<QVector3D> vpos;
				vpos = {control_points_n[2*index+1], control_points_n[2*index],
						control_points_p[2*index+1], control_points_p[2*index]};

				curEdge->f = cutFace;
				curEdge->flip->f = cutFace;
				nxtEdge->f = cutFace;
				nxtEdge->flip->f = cutFace;
				addBridger(curEdge->flip, nxtEdge, vpos);

			}

			break;
		}

	//cur_mesh->processType = HDS_Mesh::RIMMED_PROC;
	updateNewMesh();
}


MeshRimFace::MeshRimFace()
{

}


MeshRimFace::~MeshRimFace()
{

}

