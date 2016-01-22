#include "meshrimface.h"
#include "MeshExtender.h"
#include "common.h"

void MeshRimFace::rimMesh3D(HDS_Mesh *mesh, float planeWidthScale, float planeHeight)
{
	cout<<planeWidthScale<<"   "<<planeHeight<<endl;
	initiate();
	cur_mesh = mesh;


	for(auto v: cur_mesh->verts()) {
		///assign cut faces
		face_t* cutFace = new face_t;
		cutFace->isCutFace = true;
		faces_new.push_back(cutFace);
		vector<face_t*> pieces;

		for (auto he: mesh->incidentEdges(v)) {

			///get planes on the edge
			vert_t* vp = he->flip->prev->v;
			vert_t* v1 = he->flip->v;
			vert_t* vn = he->prev->v;
			//get normals for plane prev and plane next
			QVector3D np = QVector3D::normal(v->pos, vp->pos, v1->pos);
			QVector3D nn = QVector3D::normal(v->pos, v1->pos, vn->pos);
			QVector3D n = np + nn;
			n.normalize();

			//get middle point f the edge
			QVector3D v_mid = (v->pos + v1->pos)/2;

			//get perpendicular plane
			QVector3D v_v1 = v1->pos - v->pos;
			v_v1.normalize();

			QVector3D cross = QVector3D::crossProduct(n, v_v1);

			//get scaling
			float scale_wn = vn->pos.distanceToLine(v->pos, v_v1)*planeWidthScale/2;
			float scale_wp = vp->pos.distanceToLine(v->pos, v_v1)*planeWidthScale/2;

			//get mid points on plane
			QVector3D vn_mid = v_mid + scale_wn * cross;
			QVector3D vp_mid = v_mid - scale_wp * cross;

			vert_t* vn_i = new vert_t( planeHeight * v->pos + (1 - planeHeight) * vn_mid);
			vert_t* vn_o = new vert_t(-planeHeight * v->pos + (1 + planeHeight) * vn_mid);
			vert_t* vp_i = new vert_t( planeHeight * v->pos + (1 - planeHeight) * vp_mid);
			vert_t* vp_o = new vert_t(-planeHeight * v->pos + (1 + planeHeight) * vp_mid);


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
		}
		///connect planes

		for(int i = 0; i < pieces.size(); i++) {
			face_t* curFace = pieces[i];
			face_t* nextFace;
			if(i < pieces.size() - 1)
				nextFace = pieces[i+1];
			else {
				//duplicate pieces[0]
				vector<vert_t*> vertices;
				for (auto v: pieces[0]->corners()) {
					vertices.push_back( new vert_t(v->pos));
				}
				nextFace = createFace(vertices, cutFace);
				nextFace->refid = pieces[0]->refid;
				faces_new.push_back(nextFace);
				verts_new.insert(verts_new.end(), vertices.begin(), vertices.end());
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

			addBridger(he2->flip, he1->flip, v1, v2);
		}

	}

	updateNewMesh();
}


MeshRimFace::MeshRimFace()
{

}


MeshRimFace::~MeshRimFace()
{

}

