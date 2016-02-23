#include "meshrimface.h"
#include "MeshExtender.h"
#include "common.h"

#define PI 3.14159265358

void MeshRimFace::rimMesh3D(HDS_Mesh *mesh, float planeWidth, float planeHeight)
{
	initiate();
	cur_mesh = mesh;
	HDS_Bridger::setSamples(32);

	unordered_map <hdsid_t, he_t*> ori_map = ori_mesh->halfedgesMap();


	for(vert_t* v: cur_mesh->verts()) {
		///assign cut faces
		face_t* cutFace = new face_t;
		cutFace->isCutFace = true;
		faces_new.push_back(cutFace);
		vector<he_t*> control_edges;
		vector<QVector3D> control_points_n;
		vector<QVector3D> control_points_p;

		for (he_t* he: mesh->incidentEdges(v)) {
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

			control_points_n.push_back(v_up_next);
			control_points_n.push_back(v_down_next);
			control_points_p.push_back(v_up_prev);
			control_points_p.push_back(v_down_prev);

		}
		///connect pieces

		float angleSum = 0;
			control_points_p.push_back(control_points_p[0]);
			control_points_p.push_back(control_points_p[1]);
			control_points_p.erase(control_points_p.begin(), control_points_p.begin()+2);

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

