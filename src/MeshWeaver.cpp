#include "MeshWeaver.h"

float MeshWeaver::size = 0.3;
float MeshWeaver::roundness = 0.5;
float MeshWeaver::depth = 0.01;

void MeshWeaver::weaveMesh(HDS_Mesh *mesh)
{
	initiate();
	cur_mesh = mesh;
	planeHeight = size;
	cout<<size<<"' "<<roundness<<"  "<<depth<<endl;
	unordered_map<int, face_t*> top_pieces;
	unordered_map<int, pair<QVector3D, QVector3D>> top_piece_bounds;
	//get top pieces
	for (face_t* f: cur_mesh->faces()) {
		he_t* he = f->he;
		do {
			if (top_pieces.find(he->refid) == top_pieces.end())
			{
				vector<QVector3D> vpos;
				QVector3D vn_max, vp_max;
				computeDiamondCornerOnEdge(he, vpos, vn_max, vp_max);
				vector<vert_t*> vertices;
				for (QVector3D pos: vpos) {
					vert_t* vertex = new vert_t(pos);
					vertices.push_back(vertex);
				}
				vertices[0]->refid = he->v->refid;
				vertices[1]->refid = he->flip->refid;
				vertices[2]->refid = he->flip->v->refid;
				vertices[3]->refid = he->refid;

				verts_new.insert(verts_new.end(), vertices.begin(), vertices.end());

				face_t* cutFace = new face_t;
				cutFace->isCutFace = true;
				faces_new.push_back(cutFace);

				face_t* newFace = createFace(vertices, cutFace);
				newFace->refid = he->refid;
				faces_new.push_back(newFace);
				top_pieces[he->refid] = newFace;
				top_piece_bounds[he->refid] = make_pair(vp_max, vn_max);

			}
			he = he->next;
		}while(he != f->he);
	}
	cout<<"start weaving"<<endl;
	//start weaving
	for (face_t* f: cur_mesh->faces()) {
		he_t* he = f->he;
		do {
			//find next top piece
			face_t* top_piece = top_pieces[he->next->refid];
			face_t* cutFace = top_piece->he->flip->f;

			//create a bottom piece
			vector<QVector3D> vpos;
			QVector3D vn_max, vp_max;
			computeDiamondCornerOnEdge(he, vpos, vn_max, vp_max);
			//move inwards the vpos
			for (QVector3D& v: vpos) {
				v -= he->computeNormal()*depth;
			}
			vector<vert_t*> vertices;
			for (QVector3D pos: vpos) {

				vert_t* vertex = new vert_t(pos);
				vertices.push_back(vertex);
			}
			vertices[0]->refid = he->v->refid;
			vertices[1]->refid = he->flip->refid;
			vertices[2]->refid = he->flip->v->refid;
			vertices[3]->refid = he->refid;

			verts_new.insert(verts_new.end(), vertices.begin(), vertices.end());

			face_t* newFace = createFace(vertices, cutFace);
			newFace->refid = he->refid;
			faces_new.push_back(newFace);

			//find bezier control points
			QVector3D bound;
			he_t* startHE = top_piece->he;
			int offset = 0;
			do {
				if (startHE->v->refid == he->next->v->refid) {
					break;
				}
				startHE = startHE->next;
				offset++;
			}while (startHE != top_piece->he);

			if (offset > 1) bound = top_piece_bounds[he->next->refid].first;//vp_max
			else bound = top_piece_bounds[he->next->refid].second; //vn_max

			//find max cubic control points
			QVector3D cur_v_up_max, cur_v_down_max;
			HDS_Face::LineLineIntersect(vpos[1], vpos[2], he->flip->v->pos, vn_max, &cur_v_down_max);
			HDS_Face::LineLineIntersect(vpos[0], vpos[3], he->flip->v->pos, vn_max, &cur_v_up_max);

			QVector3D nxt_v_up_max, nxt_v_down_max;
			HDS_Face::LineLineIntersect(startHE->v->pos, startHE->next->v->pos, he->flip->v->pos, bound, &nxt_v_down_max);
			HDS_Face::LineLineIntersect(startHE->prev->v->pos, startHE->prev->prev->v->pos, he->flip->v->pos, bound, &nxt_v_up_max);

			QVector3D cur_v_down = (1 - roundness)* vpos[2] + roundness* cur_v_down_max;
			QVector3D cur_v_up = (1 - roundness) * vpos[3] + roundness* cur_v_up_max;

			QVector3D nxt_v_down = (1 - roundness)* startHE->v->pos + roundness* nxt_v_down_max;
			QVector3D nxt_v_up = (1 - roundness) * startHE->prev->v->pos + roundness* nxt_v_up_max;

			//add bridger

			vpos = {nxt_v_down, nxt_v_up,
					cur_v_down, cur_v_up};

			addBridger(startHE->prev->flip, newFace->he->next->next->flip, vpos);

			he = he->next;
		}while (he != f->he);
	}

	updateNewMesh();
}
