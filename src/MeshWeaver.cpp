#include "MeshWeaver.h"

float MeshWeaver::size = 0.3f;
float MeshWeaver::roundness = 0.5f;
float MeshWeaver::depth = 0.01f;
float MeshWeaver::pivot = 0.5;
float MeshWeaver::flapSize = 0.0;
bool MeshWeaver::isBilinear = true;
bool MeshWeaver::isCone = true;

void MeshWeaver::weaveLinearScaledPiece() {
	//for linear scaled piece
	unordered_map<int, face_t*> top_pieces;
	unordered_map<int, pair<QVector3D, QVector3D>> top_piece_bounds;
	//get top pieces
	for (auto f: cur_mesh->faces()) {
		he_t* he = f.he;
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

					for (auto newv : vertices)
					{
						verts_new.push_back(*newv);
					}

					face_t* cutFace = new face_t;
					cutFace->isCutFace = true;
					faces_new.push_back(*cutFace);

					face_t* newFace = createFace(vertices, cutFace);
					newFace->refid = he->refid;
					faces_new.push_back(*newFace);
					top_pieces[he->refid] = newFace;
					top_piece_bounds[he->refid] = make_pair(vp_max, vn_max);
			}
			he = he->next;
		}while(he != f.he);
	}
	cout<<"start weaving"<<endl;
	//start weaving
	for (face_t f: cur_mesh->faces()) {
		he_t* he = f.he;
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

			for (auto newv : vertices)
			{
				verts_new.push_back(*newv);
			}

			face_t* newFace = createFace(vertices, cutFace);
			newFace->refid = he->refid;
			faces_new.push_back(*newFace);

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
			Utils::LineLineIntersect(vpos[1], vpos[2], he->flip->v->pos, vn_max, &cur_v_down_max);
			Utils::LineLineIntersect(vpos[0], vpos[3], he->flip->v->pos, vn_max, &cur_v_up_max);

			QVector3D nxt_v_up_max, nxt_v_down_max;
			Utils::LineLineIntersect(startHE->v->pos, startHE->next->v->pos, he->flip->v->pos, bound, &nxt_v_down_max);
			Utils::LineLineIntersect(startHE->prev->v->pos, startHE->prev->prev->v->pos, he->flip->v->pos, bound, &nxt_v_up_max);

			QVector3D cur_v_down = Utils::Lerp(vpos[2], cur_v_down_max, roundness);
			QVector3D cur_v_up = Utils::Lerp(vpos[3], cur_v_up_max, roundness);

			QVector3D nxt_v_down = Utils::Lerp(startHE->v->pos, nxt_v_down_max, roundness);
			QVector3D nxt_v_up = Utils::Lerp(startHE->prev->v->pos, nxt_v_up_max, roundness);

			//add bridger

			vpos = {nxt_v_down, nxt_v_up,
					cur_v_down, cur_v_up};

			addBridger(startHE->prev->flip, newFace->he->next->next->flip, vpos);

			he = he->next;
		}while (he != f.he);
	}
}

void MeshWeaver::weaveBilinearScaledPiece() {
	//for bilinear scaled piece

	unordered_map<int, int> ori_v_id;
	unordered_map<int, he_t*> control_edges;
	unordered_map<int, pair<QVector3D, QVector3D>> control_points_n;
	unordered_map<int, pair<QVector3D, QVector3D>> control_points_p;

	unordered_map<int, pair<QVector3D, QVector3D>> bot_control_points_n;
	unordered_map<int, pair<QVector3D, QVector3D>> bot_control_points_p;
	unordered_map<int, pair<QVector3D, QVector3D>> bot_control_points_mid;

	//get top pieces
	for (face_t f: cur_mesh->faces()) {
		he_t* he = f.he;
		do {
			vert_t* v = he->v;
			vert_t* flip_v = he->flip->v;
			if (control_edges.find(he->refid) == control_edges.end())
			{
				ori_v_id[he->refid] = v->refid;
				//compute top piece
				QVector3D vn_max, vp_max;
				projectFaceCenter(v, he, vn_max, vp_max);

				//do bilinear interpolation
				QVector3D vp_pivot = Utils::Lerp(vp_max, v->pos, pivot);
				QVector3D vn_pivot = Utils::Lerp(vn_max, flip_v->pos, pivot);

				QVector3D vp_up = Utils::Lerp(vp_pivot, v->pos, planeHeight);
				QVector3D vp_down = Utils::Lerp(vp_pivot, vp_max, planeHeight);
				QVector3D vn_up = Utils::Lerp(vn_pivot, vn_max, planeHeight);
				QVector3D vn_down = Utils::Lerp(vn_pivot, flip_v->pos, planeHeight);

				//get middle edge as control edges
				QVector3D mid_up, mid_down, vp_up_scaled, vp_down_scaled, vn_up_scaled, vn_down_scaled;
				if (isCone) {
					QVector3D vp_scaled = Utils::Lerp(flip_v->pos, vp_max, roundness);
					QVector3D vn_scaled = Utils::Lerp(v->pos, vn_max, roundness);

					Utils::LineLineIntersect(v->pos, flip_v->pos, vp_up, vn_up, &mid_up);
					Utils::LineLineIntersect(v->pos, flip_v->pos, vp_down, vn_down, &mid_down);

					Utils::LineLineIntersect(v->pos, vp_scaled, vp_up, vn_up, &vp_up_scaled);
					Utils::LineLineIntersect(v->pos, vp_scaled, vp_down, vn_down, &vp_down_scaled);
					Utils::LineLineIntersect(flip_v->pos, vn_scaled, vp_up, vn_up, &vn_up_scaled);
					Utils::LineLineIntersect(flip_v->pos, vn_scaled, vp_down, vn_down, &vn_down_scaled);

				}else {
					mid_up = (vp_up + vn_up)/2;
					mid_down = (vp_down + vn_down)/2;

					vp_up_scaled = Utils::Lerp(mid_up, vp_up, roundness);
					vp_down_scaled = Utils::Lerp(mid_down, vp_down, roundness);
					vn_up_scaled = Utils::Lerp(mid_up, vn_up, roundness);
					vn_down_scaled = Utils::Lerp(mid_down, vn_down, roundness);
				}

				//do bilinear interpolation on bottom piece
				QVector3D bot_vp_pivot = Utils::Lerp(vn_max, v->pos, pivot);
				QVector3D bot_vn_pivot = Utils::Lerp(vp_max, flip_v->pos, pivot);

				QVector3D bot_vp_up = Utils::Lerp(bot_vp_pivot, vn_max, planeHeight);
				QVector3D bot_vp_down = Utils::Lerp(bot_vp_pivot, v->pos, planeHeight);
				QVector3D bot_vn_up = Utils::Lerp(bot_vn_pivot, flip_v->pos, planeHeight);
				QVector3D bot_vn_down = Utils::Lerp(bot_vn_pivot, vp_max, planeHeight);

				//get middle edge as control edges
				QVector3D bot_vmid_up, bot_vmid_down,
						  bot_vp_up_scaled, bot_vp_down_scaled, bot_vn_up_scaled, bot_vn_down_scaled;

				if (isCone) {
					QVector3D vp_scaled = Utils::Lerp(flip_v->pos, vn_max, roundness);
					QVector3D vn_scaled = Utils::Lerp(v->pos, vp_max, roundness);

					Utils::LineLineIntersect(v->pos, flip_v->pos, bot_vp_up, bot_vn_up, &bot_vmid_up);
					Utils::LineLineIntersect(v->pos, flip_v->pos, bot_vp_down, bot_vn_down, &bot_vmid_down);

					Utils::LineLineIntersect(v->pos, vp_scaled, bot_vp_up, bot_vn_up, &bot_vp_up_scaled);
					Utils::LineLineIntersect(v->pos, vp_scaled, bot_vp_down, bot_vn_down, &bot_vp_down_scaled);
					Utils::LineLineIntersect(flip_v->pos, vn_scaled, bot_vp_up, bot_vn_up, &bot_vn_up_scaled);
					Utils::LineLineIntersect(flip_v->pos, vn_scaled, bot_vp_down, bot_vn_down, &bot_vn_down_scaled);

				}else {
				 bot_vmid_up = (bot_vp_up + bot_vn_up)/2;
				 bot_vmid_down = (bot_vp_down + bot_vn_down)/2;

				 bot_vp_up_scaled = Utils::Lerp(bot_vmid_up, bot_vp_up, roundness);
				 bot_vp_down_scaled = Utils::Lerp(bot_vmid_down, bot_vp_down, roundness);
				 bot_vn_up_scaled = Utils::Lerp(bot_vmid_up, bot_vn_up, roundness);
				 bot_vn_down_scaled = Utils::Lerp(bot_vmid_down, bot_vn_down, roundness);
				}

				control_points_n[he->refid] = make_pair(vn_up_scaled, vn_down_scaled);
				control_points_p[he->refid] = make_pair(vp_up_scaled, vp_down_scaled);

				bot_control_points_n[he->refid] = make_pair(bot_vn_up_scaled, bot_vn_down_scaled);
				bot_control_points_p[he->refid] = make_pair(bot_vp_up_scaled, bot_vp_down_scaled);
				bot_control_points_mid[he->refid] = make_pair(bot_vmid_up, bot_vmid_down);




				vert_t* vmid_up = new vert_t(mid_up);
				vert_t* vmid_down = new vert_t(mid_down);
				vmid_up->refid = v->refid;
				vmid_down->refid = flip_v->refid;
				verts_new.push_back(*vmid_up);
				verts_new.push_back(*vmid_down);

				he_t* he_mid = HDS_Mesh::insertEdge(vmid_up, vmid_down);
				he_mid->refid = he->refid;

				hes_new.push_back(*he_mid);
				control_edges[he->refid] = he_mid;

				//assign cutface
				face_t* cutFace = new face_t;
				cutFace->isCutFace = true;
				faces_new.push_back(*cutFace);
				he_mid->f = cutFace;
				he_mid->flip->f = cutFace;

			}
			he = he->next;
		}while(he != f.he);
	}
#ifdef _DEBUG
	cout<<"start weaving"<<endl;
#endif
	//start weaving
	for (face_t f: cur_mesh->faces()) {
		he_t* he = f.he;

		do {
			he_t* he_nxt = he->next;
			//find top piece
			he_t* top_piece = control_edges[he->refid];
			face_t* cutFace = top_piece->f->isCutFace? top_piece->f: top_piece->flip->f;
			//find top piece control points
			QVector3D cur_v_up, cur_v_down;

			//flip he if needed
			if (ori_v_id[he->refid] != he->v->refid) {
				top_piece = top_piece->flip;
				cur_v_up = control_points_p[he->refid].second;
				cur_v_down = control_points_p[he->refid].first;

			}else {
				cur_v_up = control_points_n[he->refid].first;
				cur_v_down = control_points_n[he->refid].second;
			}

			//move inwards the vpos
			QVector3D offset = he_nxt->computeNormal()*depth;

			//create next bottom piece
			QVector3D nxt_v_up, nxt_v_down;

			//get middle edge as control edges
			QVector3D mid_up = bot_control_points_mid[he_nxt->refid].first - offset;
			QVector3D mid_down = bot_control_points_mid[he_nxt->refid].second - offset;


			//flip controls if needed
			if (ori_v_id[he_nxt->refid] != he_nxt->v->refid) {
				QVector3D tmp = mid_up;
				mid_up = mid_down;
				mid_down = tmp;

				nxt_v_up = bot_control_points_n[he_nxt->refid].second - offset;
				nxt_v_down = bot_control_points_n[he_nxt->refid].first - offset;

			}else {
				nxt_v_up = bot_control_points_p[he_nxt->refid].first - offset;
				nxt_v_down = bot_control_points_p[he_nxt->refid].second - offset;
			}

			vert_t* vmid_up = new vert_t(mid_up);
			vert_t* vmid_down = new vert_t(mid_down);
			vmid_up->refid = he_nxt->flip->v->refid;
			vmid_down->refid = he_nxt->v->refid;
			verts_new.push_back(*vmid_up);
			verts_new.push_back(*vmid_down);

			he_t* he_mid = HDS_Mesh::insertEdge(vmid_up, vmid_down);
			he_mid->refid = he_nxt->refid;
			he_mid->f = cutFace;
			he_mid->flip->f = cutFace;
			hes_new.push_back(*he_mid);
			he_mid->setCutEdge(true);

			vector<QVector3D> vpos = {cur_v_up, cur_v_down, nxt_v_up, nxt_v_down};

			addBridger(top_piece, he_mid->flip, vpos);

			he = he->next;
		}while (he != f.he);
	}

}

void MeshWeaver::weaveMesh(HDS_Mesh *mesh)
{
	initiate();
	cur_mesh = mesh;
	planeHeight = size;

	if (!isBilinear) {
		weaveLinearScaledPiece();
	}else {
		weaveBilinearScaledPiece();
	}
	updateNewMesh();
}
