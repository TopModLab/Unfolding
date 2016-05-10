#include "meshrimface.h"
#include "common.h"


float MeshRimFace::planeWidth = 0.2f;
float MeshRimFace::planeHeight = 0.2f;
float MeshRimFace::flapWidth = 0.1f;
float MeshRimFace::pivotPosition = 0.0f;
int MeshRimFace::shapeType = 0;

bool MeshRimFace::isHalf = true;
bool MeshRimFace::isQuadratic = true;
bool MeshRimFace::onEdge = true;
bool MeshRimFace::smoothEdge = false;
bool MeshRimFace::addConnector = false;
bool MeshRimFace::avoidIntersect = false;


/*project face centers outwards*/
void MeshRimFace::projectFaceCenter(vert_t* v, he_t* he, QVector3D &vn_pos, QVector3D &vp_pos)
{
	QVector3D n = he->computeNormal();

	//get face_n face_p center
	QVector3D fn_center = he->f->center();
	QVector3D fp_center = he->flip->f->center();

	//get projected centers
	//cout<<fn_center.distanceToPlane(v->pos,n)<<endl;
	vn_pos = fn_center - n * fn_center.distanceToPlane(v->pos,n);
	vp_pos = fp_center - n * fp_center.distanceToPlane(v->pos,n);
}

/*project edge vertices inwards*/

void MeshRimFace::projectEdgeVertices(vert_t* v, he_t* he, QVector3D &up_pos, QVector3D &down_pos)
{
	QVector3D n = he->computeNormal();

	//get face_n face_p center
	QVector3D fn_center = he->f->center();

	up_pos = v->pos - n * v->pos.distanceToPlane(fn_center,n);
	down_pos = v->he->flip->v->pos - n * v->he->flip->v->pos.distanceToPlane(fn_center,n);
}

void MeshRimFace::computePlaneCornerOnEdge(vert_t* v, he_t* he, vector<QVector3D> &vpos) {

	vert_t* v1 = he->flip->v;

	//get middle point of the edge
	QVector3D v_mid = (v->pos + v1->pos)/2;

	//get projected centers
	//cout<<fn_center.distanceToPlane(v->pos,n)<<endl;
	QVector3D vn_max, vp_max;
	projectFaceCenter(v, he, vn_max, vp_max);

	QVector3D vn_mid = planeWidth * vn_max + (1- planeWidth) * v_mid;
	QVector3D vp_mid = planeWidth * vp_max + (1- planeWidth) * v_mid;

	QVector3D vn_i_pos = ( planeHeight * v->pos + (1 - planeHeight) * vn_mid);
	QVector3D vp_i_pos = ( planeHeight * v->pos + (1 - planeHeight) * vp_mid);
	QVector3D vn_o_pos, vp_o_pos;
	if (isHalf) {
		vn_o_pos = vn_mid;
		vp_o_pos = vp_mid;
	}else {
		if (avoidIntersect) {
			//mirror
			vn_o_pos = ( planeHeight * v1->pos + (1 - planeHeight) * vn_mid);
			vp_o_pos = ( planeHeight * v1->pos + (1 - planeHeight) * vp_mid);
		}else {

			vn_o_pos = 2* vn_mid - vn_i_pos;
			vp_o_pos = 2* vp_mid - vp_i_pos;
		}
	}

	vpos.push_back(vn_i_pos);
	vpos.push_back(vn_o_pos);
	vpos.push_back(vp_i_pos);
	vpos.push_back(vp_o_pos);

}

void MeshRimFace::computePlaneCornerOnFace(vert_t* v, he_t* he, vector<QVector3D> &vmid, vector<QVector3D> &vpos) {

	//get center of the face
	QVector3D top = v->pos;
	QVector3D center = he->f->center();
	vert_t* vn = he->prev->v;
	vert_t* vp = he->flip->v;

	QVector3D bot;
	Utils::LineLineIntersect(v->pos, center,vn->pos, vp->pos, &bot);

	QVector3D v_up = ( planeHeight * top + (1 - planeHeight) * center);
	QVector3D v_down;
	if (isHalf) {
		v_down = ( planeHeight * bot + (1 - planeHeight) * center);
	}else {
		//mirror up vector based on center point
		v_down = 2* center - v_up;
	}

	vmid.push_back(v_up);
	vmid.push_back(v_down);


	//get vector vp to vn
	QVector3D cross = (vp->pos - vn->pos);
	cross.normalize();

	//get v_up_prev, v_down_prev, v_up_next, v_down_next boundaries on edge
	QVector3D v_up_prev_max;
	Utils::LineLineIntersect(v->pos, vp->pos, v_up, v_up - cross, &v_up_prev_max);
	QVector3D v_down_prev_max;
	if (isHalf)
		Utils::LineLineIntersect(v->pos, vp->pos, v_down, v_down - cross, &v_down_prev_max);
	else
		v_down_prev_max = v_up_prev_max - (v_up - v_down);

	QVector3D v_up_next_max;
	Utils::LineLineIntersect(v->pos, vn->pos, v_up, v_up + cross, &v_up_next_max);
	QVector3D v_down_next_max;
	if (isHalf)
		Utils::LineLineIntersect(v->pos, vn->pos, v_down, v_down + cross, &v_down_next_max);
	else
		v_down_next_max = v_up_next_max - (v_up - v_down);

	//get v_up_prev, v_down_prev, v_up_next, v_down_next
	QVector3D v_up_prev = ( planeWidth * v_up_prev_max + (1 - planeWidth) * v_up);
	QVector3D v_down_prev = ( planeWidth * v_down_prev_max + (1 - planeWidth) * v_down);
	QVector3D v_up_next = ( planeWidth * v_up + (1 - planeWidth) * v_up_next_max);
	QVector3D v_down_next = ( planeWidth * v_down + (1 - planeWidth) * v_down_next_max);


	vpos.push_back(v_up_prev);
	vpos.push_back(v_down_prev);
	vpos.push_back(v_up_next);
	vpos.push_back(v_down_next);
}

void MeshRimFace::computeDiamondCornerOnFace(he_t* he, vector<QVector3D> &vpos)
{
	//planeHeight as thickness scale factor
	face_t* f = he->f;
	f->setScaleFactor(planeHeight);
	he_t* curHE = he;
	do {
		vpos.push_back(f->scaleCorner(curHE->v));
		curHE = curHE->next;
	}while(curHE != he);
}

void MeshRimFace::computeDiamondCornerOnEdge(he_t* he, vector<QVector3D> &vpos, QVector3D &vn_max, QVector3D &vp_max)
{
	//get face centers
	projectFaceCenter(he->v, he, vn_max, vp_max);

	//scale down those corners
	QVector3D c = (he->v->pos + he->flip->v->pos)/2;
	//scale v
	vpos.push_back(c + planeHeight * (he->v->pos - c));
	vpos.push_back(c + planeHeight * (vp_max - c));
	vpos.push_back(c + planeHeight * (he->flip->v->pos - c));
	vpos.push_back(c + planeHeight * (vn_max - c));

}

void MeshRimFace::rimMeshV(HDS_Mesh *mesh)
{
	initiate();
	cur_mesh = mesh;


	unordered_map <hdsid_t, he_t*> ori_map = ori_mesh->hesMap();


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
			vert_t* v1 = he->flip->v;
			QVector3D v_mid = (v->pos + v1->pos)/2;
			vector<QVector3D> vpos;
			if (onEdge) {
				if (shapeType == 0) {
					computePlaneCornerOnEdge(v, he, vpos);
					if (isQuadratic) {
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
					}else {
						control_points_n.push_back(vpos[0]);
						control_points_n.push_back(vpos[1]);
						control_points_p.push_back(vpos[2]);
						control_points_p.push_back(vpos[3]);

						vert_t *vmid_i, *vmid_o;
						if (smoothEdge) {
							vmid_i = new vert_t((vpos[0]+vpos[2])/2.0);
							vmid_o = new vert_t((vpos[1]+vpos[3])/2.0);
						}else {
							vmid_i = new vert_t( planeHeight * v->pos + (1 - planeHeight) * v_mid);
							if (isHalf) {
								vmid_o = new vert_t(v_mid);
							}else {
								vmid_o = new vert_t(-planeHeight * v->pos + (1 + planeHeight) * v_mid);
							}
						}
						verts_new.push_back(vmid_i);
						verts_new.push_back(vmid_o);
						he_t* he_mid = HDS_Mesh::insertEdge(vmid_i, vmid_o);
						he_mid->refid = he->refid;
						hes_new.push_back(he_mid);
						control_edges.push_back(he_mid);
					}
				}else if(shapeType == 1) {
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

					face_t* newFace = createFace(vertices, cutFace);
					newFace->refid = he->refid;
					faces_new.push_back(newFace);
					pieces.push_back(newFace);

					if (!isQuadratic) {
						//calculate control points
						QVector3D vp_up, vp_down, vn_up, vn_down;
						Utils::LineLineIntersect(vpos[0], vpos[3], he->v->pos, vp_max, &vp_up);
						Utils::LineLineIntersect(vpos[1], vpos[2], he->v->pos, vp_max, &vp_down);
						Utils::LineLineIntersect(vpos[0], vpos[1], he->v->pos, vn_max, &vn_up);
						Utils::LineLineIntersect(vpos[3], vpos[2], he->v->pos, vn_max, &vn_down);

						control_points_n.push_back(vn_up* planeWidth + (1- planeWidth)* vpos[0]);
						control_points_n.push_back(vn_down* planeWidth + (1- planeWidth)* vpos[3]);
						control_points_p.push_back(vp_up* planeWidth + (1- planeWidth)* vpos[0]);
						control_points_p.push_back(vp_down* planeWidth + (1- planeWidth)*vpos[1]);
					}
				}else if (shapeType == 2) {

				}
			}
			else {
				if (shapeType == 0) {
					vector<QVector3D> vmid;
					computePlaneCornerOnFace(v,he,vmid, vpos);
					if (isQuadratic) {

						vert_t* vp_i = new vert_t(vpos[0]);
						vert_t* vp_o = new vert_t(vpos[1]);
						vert_t* vn_i = new vert_t(vpos[2]);
						vert_t* vn_o = new vert_t(vpos[3]);

						vn_i->refid = v->refid;
						vn_o->refid = he->prev->v->refid;
						vp_i->refid = v->refid;
						vp_o->refid = v1->refid;

						vector<vert_t*> vertices;
						vertices.push_back(vp_i);
						vertices.push_back(vp_o);
						vertices.push_back(vn_o);
						vertices.push_back(vn_i);

						verts_new.insert(verts_new.end(), vertices.begin(), vertices.end());

						face_t* newFace = createFace(vertices, cutFace);
						newFace->he->refid = he->refid;
						faces_new.push_back(newFace);
						pieces.push_back(newFace);
					}else {
						vert_t* vup = new vert_t(vmid[0]);
						vert_t* vdown = new vert_t(vmid[1]);
						verts_new.push_back(vup);
						verts_new.push_back(vdown);
						he_t* he_mid = HDS_Mesh::insertEdge(vup, vdown);
						he_mid->refid = he->refid;
						hes_new.push_back(he_mid);
						control_edges.push_back(he_mid);

						control_points_p.push_back(vpos[0]);
						control_points_p.push_back(vpos[1]);
						control_points_n.push_back(vpos[2]);
						control_points_n.push_back(vpos[3]);


					}
				}else if(shapeType == 1) {
					//is diamond shape piece
					computeDiamondCornerOnFace(he,vpos);
					he_t* ori_he = he;
					vector<vert_t*> vertices;
					for (QVector3D pos: vpos) {
						vert_t* vertex = new vert_t(pos);
						vertices.push_back(vertex);
						vertex->refid = ori_he->v->refid;
						ori_he = ori_he->next;
					}

					verts_new.insert(verts_new.end(), vertices.begin(), vertices.end());

					face_t* newFace = createFace(vertices, cutFace);
					newFace->refid = he->f->refid;
					faces_new.push_back(newFace);
					pieces.push_back(newFace);

					if (!isQuadratic) {
						//calculate control points
						QVector3D vp_up, vp_down, vn_up, vn_down;
						Utils::LineLineIntersect(vpos[0], vpos[3], he->v->pos, he->next->v->pos, &vp_up);
						Utils::LineLineIntersect(vpos[1], vpos[2], he->v->pos, he->next->v->pos, &vp_down);
						Utils::LineLineIntersect(vpos[0], vpos[1], he->v->pos, he->prev->v->pos, &vn_up);
						Utils::LineLineIntersect(vpos[3], vpos[2], he->v->pos, he->prev->v->pos, &vn_down);


						control_points_n.push_back(vn_up* planeWidth + (1- planeWidth)* vpos[0]);
						control_points_n.push_back(vn_down* planeWidth + (1- planeWidth)* vpos[3]);
						control_points_p.push_back(vp_up* planeWidth + (1- planeWidth)* vpos[0]);
						control_points_p.push_back(vp_down* planeWidth + (1- planeWidth)*vpos[1]);
					}

				}else if (shapeType == 2) {

				}
			}

		}
		///connect pieces

		float angleSum = 0;
		if (isQuadratic || shapeType == 1) {

			if(shapeType == 1 && !isQuadratic) {
				control_points_n.push_back(control_points_n[0]);
				control_points_n.push_back(control_points_n[1]);
				control_points_n.erase(control_points_n.begin(), control_points_n.begin()+2);
			}

			// quadratic rect piece
			// diamond piece
			for(int i = 0; i < pieces.size(); i++) {
				face_t* curFace = pieces[i];
				face_t* nextFace;

				//check for negative curvature vertices,
				//if angleSum exceeds 360 degree,
				//cut it, duplicate current piece and start a new cutFace
				if (angleSum > 1* M_PI) {
					cutFace = new face_t;
					cutFace->isCutFace = true;
					faces_new.push_back(cutFace);
					curFace = duplicateFace(pieces[i], cutFace);

					angleSum = 0;
				}

				//when it comes to final piece, duplicate the first piece
				if(i < pieces.size() - 1)
					nextFace = pieces[i+1];
				else {
					//duplicate pieces[0]
					nextFace = duplicateFace(pieces[0], cutFace);
				}
				if (onEdge) {
					//calculate angle between pieces' original he
					he_t* curHE = ori_map[(curFace->refid)>>2];
					he_t* nxtHE = ori_map[(nextFace->refid)>>2];
					QVector3D curHE_v = curHE->flip->v->pos - curHE->v->pos;
					QVector3D nxtHE_v = nxtHE->flip->v->pos - nxtHE->v->pos;
					double param = QVector3D::dotProduct(curHE_v, nxtHE_v);
					double angle = acos (param);
					angleSum += angle;
				}else {
					he_t* HE =  ori_map[(nextFace->he->refid)>>2];
					angleSum += HE->angle;
				}

				//update nextFace 's cutFace
				if (nextFace->he->flip->f != cutFace) {
					assignCutFace(nextFace, cutFace);
				}


				//get curFace he1
				he_t* he1 = curFace->he;
				//get nextFace he2
				he_t* he2;
				if(shapeType == 0)
					he2 = nextFace->he->next->next;
				else
					he2 = nextFace->he->prev;
				he1->setCutEdge(false);
				he2->setCutEdge(false);

				vector<QVector3D> vpos;

				if (isQuadratic) {
					vector<vert_t*> curCorners, nxtCorners;
					curCorners = curFace->corners();
					nxtCorners = nextFace->corners();

					if (shapeType == 1) {
						nxtCorners.push_back(nxtCorners[0]);
						nxtCorners.erase(nxtCorners.begin());
					}
					QVector3D v1;
					Utils::LineLineIntersect(nxtCorners[0]->pos, nxtCorners[3]->pos, curCorners[3]->pos, curCorners[0]->pos, &v1);
					QVector3D v2;
					Utils::LineLineIntersect(nxtCorners[1]->pos, nxtCorners[2]->pos, curCorners[2]->pos, curCorners[1]->pos, &v2);

					vpos.push_back(v1);
					vpos.push_back(v2);

				}else {
					//cubic diamond shape

					vpos = {control_points_n[2*i], control_points_n[2*i+1],
							control_points_p[2*i], control_points_p[2*i+1]};

				}
				addBridger(he2->flip, he1->flip, vpos);

			}

		}
		else {
			//cubic rect shape
			control_points_n.push_back(control_points_n[0]);
			control_points_n.push_back(control_points_n[1]);
			control_points_n.erase(control_points_n.begin(), control_points_n.begin()+2);

			for (int index = 0; index < control_edges.size(); index++) {
				he_t* curEdge = control_edges[index];
				he_t* nxtEdge;

				if (angleSum > 1.6* M_PI) {
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

				if (onEdge){
					//calculate angle between pieces' original he
					he_t* curHE = ori_map[(curEdge->refid)>>2];
					he_t* nxtHE = ori_map[(nxtEdge->refid)>>2];
					QVector3D curHE_v = curHE->flip->v->pos - curHE->v->pos;
					QVector3D nxtHE_v = nxtHE->flip->v->pos - nxtHE->v->pos;
					double param = QVector3D::dotProduct(curHE_v, nxtHE_v);
					double angle = acos (param);
					angleSum += angle;
				}else {
					he_t* HE =  ori_map[(nxtEdge->refid)>>2];
					angleSum += HE->angle;
				}


				vector<QVector3D> vpos;
				vpos = {control_points_p[2*index+1], control_points_p[2*index],
						control_points_n[2*index+1], control_points_n[2*index]};

				curEdge->f = cutFace;
				curEdge->flip->f = cutFace;
				nxtEdge->f = cutFace;
				nxtEdge->flip->f = cutFace;
				addBridger(curEdge->flip, nxtEdge, vpos);

			}

		}

	}
	//cur_mesh->processType = HDS_Mesh::RIMMED_PROC;
	updateNewMesh();
}

void MeshRimFace::rimMeshF(HDS_Mesh *mesh)
{
	initiate();
	cur_mesh = mesh;

	for (face_t* f: cur_mesh->faces()) {
		he_t* he = f->he;

		vector<he_t*> control_edges_n;
		vector<he_t*> control_edges_p;

		vector<QVector3D> control_points_n;
		vector<QVector3D> control_points_p;

		face_t* cutFace = new face_t;
		cutFace->isCutFace = true;
		faces_new.push_back(cutFace);

		do {

			//get projection plane
			QVector3D vu_max = he->v->pos;
			QVector3D vd_max = he->flip->v->pos;
			QVector3D vp_max, vn_max;
			projectFaceCenter(he->v, he, vn_max, vp_max);

			//get center point //flap pivot
			QVector3D flapPivot;
			Utils::LineLineIntersect(vu_max, vd_max, vp_max, vn_max, &flapPivot);

			//flap control points
			QVector3D flap_in = Utils::Lerp(flapPivot, vn_max, flapWidth);
			QVector3D flap_out = Utils::Lerp(flapPivot, vp_max, flapWidth);

			//construct flap edge //WARNING::refid not assigned
			vert_t* vflap_in = new vert_t(flap_in);
			vert_t* vflap_out = new vert_t(flap_out);
			verts_new.push_back(vflap_in);
			verts_new.push_back(vflap_out);
			he_t* flap_he = HDS_Mesh::insertEdge(vflap_in, vflap_out);
			hes_new.push_back(flap_he);
			flap_he->f = cutFace;
			flap_he->flip->f = cutFace;

			//get planar piece height pivot
			QVector3D pieceHeightPivot = Utils::Lerp(flap_in, vn_max, 0.5);

			QVector3D pieceHeight_in = Utils::Lerp(pieceHeightPivot, vn_max, planeWidth);
			QVector3D pieceHeight_out = Utils::Lerp(pieceHeightPivot, flap_in, planeWidth);


			QVector3D v_max;
			for (int i = 0; i < 2; i++) {
				if (i == 0)
					v_max = vd_max;
				else
					v_max = vu_max;

				//get planar piece width center //planar width pivot
				QVector3D pieceWidthPivot = Utils::Lerp(vn_max, v_max, 0.5);

				//set the ratio of planar piece
				//pieceWidth_in and out are control points for bridger
				QVector3D pieceWidth_in = Utils::Lerp(pieceWidthPivot, vn_max, planeHeight);
				QVector3D pieceWidth_out = Utils::Lerp(pieceWidthPivot, v_max, planeHeight);


				//get control points for the flap piece
				QVector3D flap_in_ctl, flap_out_ctl;
				Utils::LineLineIntersect(flap_in, v_max, vp_max, pieceWidth_in, &flap_in_ctl);
				Utils::LineLineIntersect(flap_out, v_max, vp_max, pieceWidth_out, &flap_out_ctl);

				//get planar piece vertices postion
				QVector3D vpos_0, vpos_1, vpos_2, vpos_3;
				Utils::LineLineIntersect(vp_max, pieceWidth_out, v_max, pieceHeight_in, &vpos_0);
				Utils::LineLineIntersect(vp_max, pieceWidth_in, v_max, pieceHeight_in, &vpos_1);
				Utils::LineLineIntersect(vp_max, pieceWidth_in, v_max, pieceHeight_out, &vpos_2);
				Utils::LineLineIntersect(vp_max, pieceWidth_out, v_max, pieceHeight_in, &vpos_3);

				//construct planar piece //WARNING: refid not assigned
				vert_t* v0 = new vert_t(vpos_0);
				vert_t* v1 = new vert_t(vpos_1);
				vert_t* v2 = new vert_t(vpos_2);
				vert_t* v3 = new vert_t(vpos_3);

				vector<vert_t*> vertices;
				if (i == 0) {
					vertices.push_back(v0);
					vertices.push_back(v1);
					vertices.push_back(v2);
					vertices.push_back(v3);
				}else {
					vertices.push_back(v1);
					vertices.push_back(v0);
					vertices.push_back(v3);
					vertices.push_back(v2);
				}
				verts_new.insert(verts_new.end(), vertices.begin(), vertices.end());

				face_t* newFace = createFace(vertices, cutFace);
				newFace->refid = he->refid;
				faces_new.push_back(newFace);

				//connect to flap
				if (i == 0) {
					vector<QVector3D> vpos = {flap_in_ctl, flap_out_ctl};
					addBridger(flap_he, newFace->he->next->next->flip, vpos);
					control_edges_n.push_back(newFace->he->flip);
					control_points_n.push_back(pieceWidth_in);
					control_points_n.push_back(pieceWidth_out);
				}else {
					vector<QVector3D> vpos = {flap_out_ctl, flap_in_ctl};
					addBridger(flap_he->flip, newFace->he->next->next->flip, vpos);
					control_edges_p.push_back(newFace->he->flip);
					control_points_p.push_back(pieceWidth_out);
					control_points_p.push_back(pieceWidth_in);
				}

			}
			he = he->next;
		}while(he != f->he);

		//connect pieces

		//shift control p by one
		control_points_p.push_back(control_points_p[0]);
		control_points_p.push_back(control_points_p[1]);
		control_points_p.erase(control_points_p.begin(), control_points_p.begin()+2);
		control_edges_p.push_back(control_edges_p[0]);
		control_edges_p.erase(control_edges_p.begin());


		int edgeCount = control_edges_n.size();
		for (int index = 0; index < edgeCount; index++) {
			vector<QVector3D> vpos;
			vpos = {control_points_n[2*index], control_points_n[2*index+1],
					control_points_p[2*index+1], control_points_p[2*index]};
			addBridger(control_edges_n[index], control_edges_p[index], vpos);

			he = he->next;
		}
	}
	updateNewMesh();
}
