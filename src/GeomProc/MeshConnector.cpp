#include "GeomProc/meshmanager.h"
#include "MeshFactory/meshcutter.h"
#include "MeshFactory/MeshUnfolder.h"
#include "meshsmoother.h"
#include "MeshFactory/MeshExtender.h"
#include "MeshFactory/meshhollower.h"
#include "MeshIterator.h"
#include "GeomProc/MeshConnector.h"
#include "Utils/utils.h"

#if USE_REEB_GRAPH
#include <vtkPolyDataToReebGraphFilter.h>
#include <vtkDirectedGraph.h>
#include <vtkReebGraph.h>
#include <vtkDoubleArray.h>
#include <vtkCellArray.h>
#include <vtkVertexListIterator.h>
#include <vtkEdgeListIterator.h>
#include <vtkGraphEdge.h>
#include <vtkDataSetAttributes.h>
#include <vtkVariantArray.h>
#endif

//////////////////////////////////////////////////////////////////////////
// SVG formats
// 
// Text:	left-bottom corner as orginal position
//			rotation without position means rotating around origin point.
// Rim/Ring: 
//			
//////////////////////////////////////////////////////////////////////////
auto printText = [&](FILE* file,
	Float x, Float y, Float angle, Float str_wd, const QString &text)
{
	fprintf(file, SVG_TEXT, x, y, angle, x, y,
		ConnectorPanel::fontSize,
		ConnectorPanel::fontfamily.family().toUtf8().constData(),
		str_wd, text.toUtf8().data());
};

MeshConnector::MeshConnector()
{
}

void MeshConnector::exportQuadEdgePiece(FILE* fp,
	const mesh_t* unfolded_mesh, const confMap &conf)
{
#ifdef USE_LEGACY_FACTORY
	/************************************************************************/
	/* Scalors                                                              */
	/************************************************************************/
	Float str_wd = conf.at("strokeWd");
	Float he_scale = conf.at("scale");
	//Float uncut_len = ConvertToPt((int)UNIT_TYPE::INCH, 0.1);
	//int matConnLen = static_cast<int>(conf.at("connectMat"));
	int etchSegCount = static_cast<int>(conf.at("etchSeg"));
	double etchSegWidth = conf.at("etchSegWidth");
	//int cn_t = static_cast<int>(conf.at("connector"));
	//int unit_type = static_cast<int>(conf.at("pinUnit"));
	Float uncut_len = ConvertToPt(
		static_cast<int>(conf.at("matConnUnit")),
		conf.at("matConnLen"));
	Float pin_radius = ConvertToPt(
		static_cast<int>(conf.at("pinUnit")), conf.at("pinSize")) * 0.5;
	int pinholecount_type = static_cast<int>(conf.at("pinCount"));
	int score_type = static_cast<int>(conf.at("scoreType"));
	cstchar* score_text;
	if (score_type == 0)
	{
		score_text = SVG_LINE;
	}
	else
	{
		Float score_len = conf.at("dashLen");
		Float score_gap = conf.at("dashGap");
	}

	QVector2D size_vec = unfolded_mesh->bound->getDiagnal().toVector2D();

	//SVG file head
	// define the size of export graph
	fprintf(fp, SVG_HEAD,
		static_cast<int>(size_vec.x() * he_scale) + 50,
		static_cast<int>(size_vec.y() * he_scale) + 50);

	int printFaceID(0), printCircleID(0);
	// Go through each piece
	for (auto piece : unfolded_mesh->pieceSet)
	{
		vector<face_t *> cutfaces;

		vector<QVector2D> printBorderEdgePts;//Edges on the boundary
		vector<QVector2D> printEdgePtsCarves;
		vector<QVector2D> printPinholes;

		vector<QVector2D> printTextPos;
		floats_t printTextRot;
		vector<QString> printTextIfo;
		unordered_map<int, QVector2D> printTextRecord;

		vector<QVector2D> printOrientLabel;

		vector<QVector2D> printEtchEdges;
		unordered_set<he_t*> visitedEtchEdges;

		// Group current piece
		fprintf(fp, "<g>\n");
		for (auto fid : piece)
		{
			const face_t *curFace = &unfolded_mesh->faceSet[fid];
			auto he = curFace->he;
			auto curHE = he;
			// Cut layer
			if (curFace->isCutFace)
			{
				do
				{
					printBorderEdgePts.push_back(curHE->v->pos.toVector2D() * he_scale);
					curHE = curHE->next;
				} while (curHE != he);
			}
			// Etch layer
			else if (curFace->isBridger)
			{
				// Write points of each edge
				do
				{
					if (!curHE->isCutEdge
						&& visitedEtchEdges.find(curHE) == visitedEtchEdges.end())
					{
						printEtchEdges.push_back(curHE->v->pos.toVector2D() * he_scale);
						printEtchEdges.push_back(curHE->next()->v->pos.toVector2D() * he_scale);
						visitedEtchEdges.insert(curHE);
						visitedEtchEdges.insert(curHE->flip);
					}
					curHE = curHE->next;
				} while (curHE != he);
			}
			else
			{
				// Add pinholes
				vector<he_t*> cutedges;
				he_t* refedge;
				// Find ref edge, which is the boundary of shared face
				// Find cut edge, which refers to conner of the flap
				do
				{
					if (!curHE->isCutEdge)
					{
						refedge = curHE;
					}
					else if (curHE->prev()->isCutEdge)
					{
						cutedges.push_back(curHE);
					}
					curHE = curHE->next;
				} while (curHE != he);
				for (auto cut_he : cutedges)
				{
					vert_t* targetV;
					vert_t* targetNextV;
					he_t* targHE;
					if (cut_he->next == refedge)
					{
						targHE = refedge;
						targetV = refedge->v;
						targetNextV = refedge->flip()->v;
					}
					else
					{
						targHE = refedge->flip;
						targetV = refedge->flip()->v;
						targetNextV = refedge->v;
					}
					QVector2D targetVPos = targetV->pos.toVector2D();
					Float tpin = MeshHollower::refMapPointer->at(targHE->index);
					QVector2D targPos = targetVPos * (1 - tpin)
						+ targetNextV->pos.toVector2D() * tpin;
					QVector2D startPos = cut_he->v->pos.toVector2D();
					QVector2D dirPin = targPos - startPos;

					// 1 Pinhole
					if (pinholecount_type == 0)
					{
						printPinholes.push_back((startPos + dirPin * 0.5) * he_scale);
						// Add orientation label
						if (!cut_he->next()->isCutEdge)
						{
							printOrientLabel.push_back((startPos + dirPin * 0.75) * he_scale);
						}
					}
					// 2 or 4 Pinhole
					else
					{
						int pin_seg = max(3, min(2, static_cast<int>(
							dirPin.length() * he_scale / pin_radius / 4)) + 1);
						if (pin_seg > 1)
						{
							for (int pin_i = 1; pin_i < pin_seg; pin_i++)
							{
								printPinholes.push_back((startPos + dirPin * pin_i / pin_seg) * he_scale);
							}
						}

						// If 4 pinholes
						if (pinholecount_type == 2 || pin_seg < 2)
						{
							targPos += startPos - targetVPos;
							startPos = targetVPos;
							dirPin = targPos - startPos;

							pin_seg = max(3, min(2, static_cast<int>(
								dirPin.length() * he_scale / pin_radius / 4)) + 1);
							for (int pin_i = 1; pin_i < pin_seg; pin_i++)
							{
								printPinholes.push_back((startPos + dirPin * pin_i / pin_seg) * he_scale);
							}
						}
						// Add orientation label
						if (!cut_he->next()->isCutEdge)
						{
							printOrientLabel.push_back((startPos + dirPin * 0.5) * he_scale);
						}
					}
					
					// Add labels for pinholes
					auto res = printTextRecord.find(cut_he->v->refid);
					if (res != printTextRecord.end())
					{
						QVector2D midPos = (res->second + startPos + dirPin * 0.5) * 0.5;
						QVector2D midDir = midPos - res->second;
						printTextPos.push_back(midPos * he_scale);
						printTextRot.push_back(RadianToDegree(atan2(midDir.y(), midDir.x())));
						printTextIfo.push_back(HDS_Common::ref_ID2String(res->first));
						printTextRecord.erase(res);
					}
					else
					{
						printTextRecord.insert(make_pair(cut_he->v->refid, startPos + dirPin * 0.5));
					}
				}
				// Add labels for face
				QVector2D faceDir = (refedge->v->pos - refedge->flip()->v->pos).toVector2D();
				printTextPos.push_back(curFace->center().toVector2D() * he_scale);
				printTextRot.push_back(RadianToDegree(atan2(faceDir.y(), faceDir.x())));
				printTextIfo.push_back(HDS_Common::ref_ID2String(curFace->refid));
				
			}
		}

		/************************************************************************/
		/* Print Text                                                           */
		/************************************************************************/
		for (int i = 0; i < printTextPos.size(); i++)
		{
			auto pos = printTextPos[i];
			auto rot = printTextRot[i];
			auto ifo = printTextIfo[i];
			
			printText(fp, pos.x(), pos.y(), rot, str_wd, ifo);
		}
		/************************************************************************/
		/* Write out circles                                                    */
		/************************************************************************/
		for (auto pinpos : printPinholes)
		{
			fprintf(fp, SVG_CIRCLE, printCircleID++,
				pinpos.x(), pinpos.y(), pin_radius, str_wd);
		}
		/************************************************************************/
		/* Print Orientation Label                                              */
		/************************************************************************/
		for (auto labpos : printOrientLabel)
		{
            fprintf(fp, SVG_LABEL, labpos.x(), labpos.y(), str_wd,
				ConnectorPanel::fontfamily.family().toUtf8().constData());
		}

		/************************************************************************/
		/* Write out edge for cut                                               */
		/************************************************************************/
		// To open polyline
		if (uncut_len == 0)// if cut as open polyline
		{
			writeCutLayer(fp, printBorderEdgePts,
				str_wd, 1, printFaceID++);
		}
		// Cut as polygon
		else
		{
			auto endpos = printBorderEdgePts.back();
			auto lastDir = printBorderEdgePts.front() - endpos;
			endpos += lastDir * max(1 - uncut_len / lastDir.length(), 0.0);

			printBorderEdgePts.push_back(endpos);

			writeCutLayer(fp, printBorderEdgePts,
				str_wd, 0, printFaceID++);
		}
		/************************************************************************/
		/* Write out edge for etch                                              */
		/************************************************************************/
		wrtieEtchLayer(fp, printEtchEdges, str_wd, etchSegCount, etchSegWidth);
	}
	/************************************************************************/
	/* End of SVG File End                                                  */
	/************************************************************************/
	fprintf(fp, "</svg>");
#endif
}

void MeshConnector::exportWingedEdgePiece(FILE* fp,
	const mesh_t* unfolded_mesh, const confMap &conf)
{
#ifdef USE_LEGACY_FACTORY
	/************************************************************************/
	/* Scalors                                                              */
	/************************************************************************/
	Float he_offset = 10;
	Float str_wd = conf.at("strokeWd");
	Float he_scale = conf.at("scale");
	Float wid_conn = conf.at("width");
	Float len_conn = conf.at("length");
	int unit_type = static_cast<int>(conf.at("pinUnit"));
	Float pin_radius = ConvertToPt(unit_type,
		conf.at("pinSize")) * 0.5;
	Float scale = MeshHollower::flapSize;
	Float shift = (MeshHollower::shiftAmount + 1) * 0.5;
	int pinnum = shift > 0.8 ? 2 : 1;

	Float circle_offset = 3;
	QVector2D size_vec = unfolded_mesh->bound->getDiagnal().toVector2D();

	//SVG file head
	// define the size of export graph
	fprintf(fp, SVG_HEAD,
		static_cast<int>(size_vec.x() * he_scale),
		static_cast<int>(size_vec.y() * he_scale));

	int printFaceID(0), printCircleID(0);


	for (auto piece : unfolded_mesh->pieceSet)
	{
		vector<face_t *> cutfaces;

		vector<QVector2D> printBorderEdgePts;//Edges on the boundary
		vector<QVector2D> printEdgePtsCarves;
		vector<QVector2D> printPinholes;

		vector<QVector2D> printTextPos;
		floats_t printTextRot;
		vector<QString> printTextIfo;
		unordered_map<int, QVector2D> printTextRecord;

		vector<QVector2D> printEtchEdges;
		unordered_set<he_t*> visitedEtchEdges;

		// Group current piece
		fprintf(fp, "<g>\n");
		for (auto fid : piece)
		{
			auto curFace = &unfolded_mesh->faceSet[fid];
			auto he = curFace->he;
			auto curHE = he;
			// Cut layer
			if (curFace->isCutFace)
			{
				do
				{
					printBorderEdgePts.push_back(curHE->v->pos.toVector2D());
					curHE = curHE->next;
				} while (curHE != he);
			}
			// Pinholes
			else if (!curFace->isBridger)
			{
				// Add pinholes
				vector<he_t*> cutedges;
				
				vector<QVector2D> flapPos;
				size_t offset = 0;
				do
				{
					if (!curHE->isCutEdge)
					{
						offset = flapPos.size();
					}
					
					flapPos.push_back(curHE->v->pos.toVector2D());
					
					curHE = curHE->next;
				} while (curHE != he);

				// Pinhole on main flap edge
				auto p0 = (flapPos[0] + flapPos[5]) * 0.5;
				auto p1 = (flapPos[1] + flapPos[4]) * 0.5;
				printPinholes.push_back((p0 + p1 * 2.0) / 3.0);
				printPinholes.push_back((p0 * 2.0 + p1) / 3.0);

				// Pinhole on extended flaps
				switch (flapPos.size())
				{
				case 6:
				{
					/*********************/
					/*             /|    */
					/*     _______|*|    */
					/*    /_________|    */
					/*********************/
					// Add pinholes
					//printPinholes.push_back(
					//	0.25 * (flapPos[1] + flapPos[2] + flapPos[3] + flapPos[4]).toVector2D());
					auto d1 = (flapPos[2] + flapPos[3]) * 0.5 - p1;
					printPinholes.push_back(p1 + d1 * 2.0 / 3.0);
					printPinholes.push_back(p1 + d1 / 3.0);
					break;
				}
				case 8:
				{
					/*********************/
					/*     /|      /|    */
					/*    |*|_____|*|    */
					/*    |_________|    */
					/*********************/

					auto d0 = ((flapPos[6] + flapPos[7]) * 0.5 - p0) / (3.0 * (1 - shift));
					auto d1 = ((flapPos[2] + flapPos[3]) * 0.5 - p1) / (3.0 * shift);

					if (shift > 0.7)
					{
						for (int i = 1; i < pinnum + 1; i++)
						{
							printPinholes.push_back(p1 + d1 * i);
						}
					}
					else if (shift < 0.4)
					{
						for (int i = 1; i < pinnum + 1; i++)
						{
							printPinholes.push_back(p0 + d0 * i);
						}
					}
					else
					{
						printPinholes.push_back(p0 + d0);
						printPinholes.push_back(p1 + d1);
					}
					break;
				}
				default:
					break;
				}

				// Add labels for pinholes
				
				// Add labels for pinholes
				// Label for v0
				auto res = printTextRecord.find(curHE->v->refid);
				if (res != printTextRecord.end())
				{
					QVector2D midPos = (res->second + p0) * 0.5;
					QVector2D midDir = midPos - res->second;
					printTextPos.push_back(midPos);
					printTextRot.push_back(RadianToDegree(atan2(midDir.y(), midDir.x())));
					printTextIfo.push_back(HDS_Common::ref_ID2String(res->first));
					printTextRecord.erase(res);
				}
				else
				{
					printTextRecord.insert(make_pair(curHE->v->refid, p0));
				}
				// Label for v1
				res = printTextRecord.find(curHE->next()->v->refid);
				if (res != printTextRecord.end())
				{
					QVector2D midPos = (res->second + p1) * 0.5;
					QVector2D midDir = midPos - res->second;
					printTextPos.push_back(midPos);
					printTextRot.push_back(RadianToDegree(atan2(midDir.y(), midDir.x())));
					printTextIfo.push_back(HDS_Common::ref_ID2String(res->first));
					printTextRecord.erase(res);
				}
				else
				{
					printTextRecord.insert(make_pair(curHE->next()->v->refid, p1));
				}

				// add face label
				QVector2D dir = flapPos[0] - flapPos[1];
				printTextPos.push_back(0.5 * (flapPos[0] + flapPos[1]));
				printTextRot.push_back(RadianToDegree(atan2(dir.y(), dir.x())));
				printTextIfo.push_back(HDS_Common::ref_ID2String(curFace->refid));
			}
			// Etch layer
			else
			{
				// Write points of each edge
				do
				{
					if (!curHE->isCutEdge
						&& visitedEtchEdges.find(curHE)== visitedEtchEdges.end())
					{
						printEtchEdges.push_back(curHE->v->pos.toVector2D());
						printEtchEdges.push_back(curHE->next()->v->pos.toVector2D());
						visitedEtchEdges.insert(curHE);
						visitedEtchEdges.insert(curHE->flip);
					}
					curHE = curHE->next;
				} while (curHE != he);
			}
		}

		/************************************************************************/
		/* Print Text                                                           */
		/************************************************************************/
		for (int i = 0; i < printTextPos.size(); i++)
		{
			auto pos = printTextPos[i];
			printText(fp, pos.x() * he_scale, pos.y() * he_scale,
				printTextRot[i], str_wd, printTextIfo[i]);

		}
		/************************************************************************/
		/* Write out pinholes                                                   */
		/************************************************************************/
		for (auto pinpos : printPinholes)
		{
			fprintf(fp, SVG_CIRCLE, printCircleID++,
				pinpos.x() * he_scale, pinpos.y() * he_scale,
				pin_radius, str_wd);
		}
		/************************************************************************/
		/* Write out edge for cut                                               */
		/************************************************************************/
		fprintf(fp, "\t<polygon id=\"%d\" points=\"", printFaceID++);
		for (int isec = 0; isec < printBorderEdgePts.size(); isec++)
		{
			fprintf(fp, "%f,%f ",
				printBorderEdgePts[isec].x() * he_scale,
				printBorderEdgePts[isec].y() * he_scale);
		}
		fprintf(fp, "\" style=\"fill:none;stroke:blue;stroke-width:%lf\" />\n", str_wd);
		/************************************************************************/
		/* Write out edge for etch                                               */
		/************************************************************************/
		for (int isec = 0; isec < printEtchEdges.size(); isec+=2)
		{
			fprintf(fp, SVG_LINE, isec / 2,
				printEtchEdges[isec].x() * he_scale,
				printEtchEdges[isec].y() * he_scale,
				printEtchEdges[isec + 1].x() * he_scale,
				printEtchEdges[isec + 1].y() * he_scale,
				"yellow", str_wd);
		}

		// End of group
		fprintf(fp, "</g>\n");//set a new group for inner lines
	}
	/************************************************************************/
	/* End of SVG File End                                                  */
	/************************************************************************/
	fprintf(fp, "</svg>");
#endif
}

void MeshConnector::exportGESPiece(FILE* fp,
	const mesh_t* unfolded_mesh, const confMap &conf)
{
#ifdef USE_LEGACY_FACTORY
	/************************************************************************/
	/* Scalors                                                              */
	/************************************************************************/
	Float he_offset = 10;
	Float str_wd = conf.at("strokeWd");
	Float he_scale = conf.at("scale");
	Float wid_conn = conf.at("width");
	Float len_conn = conf.at("length");
	Float pin_radius = conf.at("pinSize");
	double uncut_len = ConvertToPt(
		static_cast<int>(conf.at("matConnUnit")),
		conf.at("matConnLen"));


	Float circle_offset = 3;
	QVector2D size_vec = unfolded_mesh->bound->getDiagnal().toVector2D();

	//SVG file head
	// define the size of export graph
	fprintf(fp, SVG_HEAD,
		static_cast<int>(size_vec.x() * he_scale),
		static_cast<int>(size_vec.y() * he_scale));

	int printFaceID(0), printCircleID(0);
	for (auto piece : unfolded_mesh->pieceSet)
	{
		vector<face_t *> cutfaces;

		vector<QVector2D> printBorderEdgePts;//Edges on the boundary
		vector<QVector2D> printEdgePtsCarves;
		vector<QVector2D> printPinholes;

		vector<QVector2D> printEtchEdges;
		unordered_set<he_t*> visitedEtchEdges;

		// Group current piece
		fprintf(fp, "<g>\n");
		for (auto fid : piece)
		{
			auto curFace = &unfolded_mesh->faceSet[fid];
			auto he = curFace->he;
			auto curHE = he;
			// Cut layer
			if (curFace->isCutFace)
			{
				do
				{
					printBorderEdgePts.push_back(curHE->v->pos.toVector2D() * he_scale);
					curHE = curHE->next;
				} while (curHE != he);
			}
			else if (!curFace->isBridger)
			{
				QVector2D centerPt = curFace->center().toVector2D();
				do
				{
					printPinholes.push_back(
						(curHE->v->pos.toVector2D() + centerPt) * 0.5 * he_scale);
					curHE = curHE->next;
				} while (curHE != he);
			}
			// Etch layer
			else
			{
				// Write points of each edge
				do
				{
					if (!curHE->isCutEdge
						&& visitedEtchEdges.find(curHE) == visitedEtchEdges.end())
					{
						printEtchEdges.push_back(
							curHE->v->pos.toVector2D() * he_scale);
						printEtchEdges.push_back(
							curHE->next->v->pos.toVector2D() * he_scale);
						visitedEtchEdges.insert(curHE);
						visitedEtchEdges.insert(curHE->flip);
					}
					curHE = curHE->next;
				} while (curHE != he);
			}
		}
		/************************************************************************/
		/* Write out circles                                                    */
		/************************************************************************/
		for (auto circlepos : printPinholes)
		{
			fprintf(fp, SVG_CIRCLE, printCircleID++,
				circlepos.x(), circlepos.y(),
				pin_radius, str_wd);
		}

		/************************************************************************/
		/* Write out edge for cut                                               */
		/************************************************************************/
		// To open polyline
		if (uncut_len == 0)// if cut as open polyline
		{
			writeCutLayer(fp, printBorderEdgePts,
				str_wd, 1, printFaceID++);
		}
		// Cut as polygon
		else
		{
			auto endpos = printBorderEdgePts.back();
			auto lastDir = printBorderEdgePts.front() - endpos;
			endpos += lastDir * max(1 - uncut_len / lastDir.length(), 0.0);

			printBorderEdgePts.push_back(endpos);

			writeCutLayer(fp, printBorderEdgePts,
				str_wd, 0, printFaceID++);
		}
		/************************************************************************/
		/* Write out edge for etch                                              */
		/************************************************************************/
		wrtieEtchLayer(fp, printEtchEdges, str_wd, 0);

		// End of group
		fprintf(fp, "</g>\n");
	}
	/************************************************************************/
	/* End of SVG File End                                                  */
	/************************************************************************/
	fprintf(fp, "</svg>");
#endif
}

void MeshConnector::exportGRSPiece(FILE *fp, const MeshConnector::mesh_t *unfolded_mesh, const confMap &conf)
{

}

void MeshConnector::exportRegularPiece(FILE* fp,
	const mesh_t* unfolded_mesh, const confMap &conf)
{
#ifdef USE_LEGACY_FACTORY
	auto faces = unfolded_mesh->faces();
	unordered_set<const HDS_Mesh::face_t*> cutfaces;// , infaces;
	for (auto &face : faces)
	{
		if (face.isCutFace)
		{
			cutfaces.insert(&face);
		}
	}
	/************************************************************************/
	/* Scalors                                                              */
	/************************************************************************/
	Float he_offset = 10;
	Float str_wd = conf.at("strokeWd");
	Float he_scale = conf.at("scale");
	Float wid_conn = conf.at("width");
	Float len_conn = conf.at("length");
	int cn_t = static_cast<int>(conf.at("connector"));
	
	QVector2D size_vec = unfolded_mesh->bound->getDiagnal().toVector2D();

	//SVG file head
	// define the size of export graph
	fprintf(fp, SVG_HEAD,
		static_cast<int>(size_vec.x() * he_scale),
		static_cast<int>(size_vec.y() * he_scale));

	int printFaceID(0), printCircleID(0);
	/*for (auto piece : unfolded_mesh->pieceSet)
	{
		vector<face_t *> cutfaces;

		vector<QVector2D> printBorderEdgePts;//Edges on the boundary
		vector<QVector2D> printEdgePtsCarves;
		vector<QVector2D> printPinholes;

		// Group current piece
		fprintf(SVG_File, "<g>\n");
		for (auto fid : piece)
		{
			face_t *curFace = unfolded_mesh->faceMap[fid];
			auto he = curFace->he;
			auto curHE = he;
			if (curFace->isCutFace)
			{
				do
				{
					printBorderEdgePts.push_back(curHE->v->pos.toVector2D());
					curHE = curHE->next;
				} while (curHE != he);
			}
			else
			{
			}
		}
	}*/

	for (auto face : cutfaces)
	{
		HDS_HalfEdge *he = face->he;
		HDS_HalfEdge *curHE = he;

		//////////////////////////////////////////////////////////////////////////
		unordered_set<HDS_HalfEdge*> cutedges;
		unordered_set<HDS_HalfEdge*> cutTwinEdges;
		vector<QVector2D*> printEdgePts;
		vector<QVector2D*> printEdgePtsCarves;

		// Connection label
		vector<QVector2D> printTextPos;
		floats_t printTextRot;
		vector<QString> printTextIfo;
		do
		{
			cutedges.insert(curHE);
			curHE = curHE->next;
		} while (curHE != he);

		QVector2D *Pthis = new QVector2D(curHE->v->pos.toVector2D() * he_scale);
		/*QVector2D *Pthis = new QVector2D((QVector3D::dotProduct(curHE->v->pos, nx) + he_offset)*he_scale,
			(QVector3D::dotProduct(curHE->v->pos, ny) + he_offset)*he_scale);*/
		do
		{
			QVector3D faceCenter = curHE->flip()->f->center();
			QVector2D Pc(faceCenter.toVector2D() * he_scale);

			QVector2D *Pnext = new QVector2D(curHE->next()->v->pos.toVector2D() * he_scale);

			printEdgePts.push_back(Pthis);

			//
			// T: normalized vector representing current edge direction
			// d: vector from current point to face center
			// a: vector projected from d onto T
			// n: normalized normal of the current edge
			QVector2D T = (*Pnext - *Pthis).normalized();
			QVector2D d = (Pc - *Pthis);
			QVector2D a = QVector2D::dotProduct(d, T) * T;
			QVector2D n = (a - d).normalized();

			QVector2D Pn = *Pthis + a;
			QVector2D Psc = Pn + n * wid_conn * 0.6f;

			switch (cn_t)
			{
			case SIMPLE_CONNECTOR:
			{
				if (cutedges.find(curHE) != cutedges.end())
				{
					//calculate 
					/*QVector2D T = (*Pnext - *Pthis).normalized();
					QVector2D d = (Pc - *Pthis);
					QVector2D a = QVector2D::dotProduct(d, T) * T;
					QVector2D n = (a - d).normalized();

					QVector2D Pn = *Pthis + a;
					QVector2D Psc = Pn + n * wid_conn;*/

					//draw connector

					QVector2D *Pnst = new QVector2D(Psc - len_conn * T);
					QVector2D *Pnsn = new QVector2D(Psc + len_conn * T);

					printEdgePts.push_back(Pnst);
					printEdgePts.push_back(Pnsn);

					cutedges.erase(curHE);
					//cutedges.erase(curHE->flip()->cutTwin->flip);

				}
				else
				{
					//draw receiver
				}
				break;
			}
			case INSERT_CONNECTOR:
			{
				//calculate 
				/*QVector2D T = (*Pnext - *Pthis).normalized();
				QVector2D d = (Pc - *Pthis);
				QVector2D a = QVector2D::dotProduct(d, T) * T;
				QVector2D n = (a - d).normalized();

				QVector2D Pn = *Pthis + a;
				QVector2D Psc = Pn + n * wid_conn;*/

				QVector2D *Pst = new QVector2D(Pn - len_conn * T);
				QVector2D *Psn = new QVector2D(Pn + len_conn * T);
				QVector2D *Pnst = new QVector2D(Psc - len_conn * T);
				QVector2D *Pnsn = new QVector2D(Psc + len_conn * T);

				QVector2D *Pcvt = new QVector2D(Pn - 0.5 * len_conn * T);
				QVector2D *Pcvn = new QVector2D(Pn + 0.5 * len_conn * T);

				printEdgePts.push_back(Pst);
				printEdgePts.push_back(Pnst);
				printEdgePts.push_back(Pnsn);
				printEdgePts.push_back(Psn);

				if (cutedges.find(curHE) != cutedges.end())
				{
					//draw connector
					printEdgePtsCarves.push_back(Pst);
					printEdgePtsCarves.push_back(Pcvt);
					printEdgePtsCarves.push_back(Pcvn);
					printEdgePtsCarves.push_back(Psn);

					//cutedges.erase(curHE);
					//cutedges.erase(curHE->flip()->cutTwin->flip);
				}
				else
				{
					//draw receiver
					printEdgePtsCarves.push_back(Pcvt);
					printEdgePtsCarves.push_back(Pcvn);
				}
				break;
			}
			case GEAR_CONNECTOR:
			{
				//calculate 
				/*QVector2D T = (*Pnext - *Pthis).normalized();
				QVector2D d = (Pc - *Pthis);
				QVector2D a = QVector2D::dotProduct(d, T) * T;
				QVector2D n = (a - d).normalized();

				QVector2D Pn = *Pthis + a;
				QVector2D Psc = Pn + n * wid_conn;*/

				//division number
				// Gear count
				int ndiv = 8;
				//connector segment length
				Float len_seg = Pthis->distanceToPoint(*Pnext) / ndiv * 0.5f;

				QVector2D *Pst = Pthis;
				for (int i = 0; i < ndiv; i++)
				{
					if (i > 0)
					{
						printEdgePts.push_back(Pst);
					}
					QVector2D seg_T = T * len_seg;
					QVector2D *Pnst = new QVector2D(*Pst + n * len_seg);
					QVector2D *Pnsn = new QVector2D(*Pnst + seg_T);
					QVector2D *Psn = new QVector2D(*Pst + seg_T);

					Pst = new QVector2D(*Psn + seg_T);

					printEdgePts.push_back(Pnst);
					printEdgePts.push_back(Pnsn);
					printEdgePts.push_back(Psn);

				}
				break;
			}
			case SAW_CONNECTOR:
			{
				//calculate 
				/*QVector2D T = (*Pnext - *Pthis).normalized();
				QVector2D d = (Pc - *Pthis);
				QVector2D a = QVector2D::dotProduct(d, T) * T;
				QVector2D n = (a - d).normalized();

				QVector2D Pn = *Pthis + a;
				QVector2D Psc = Pn + n * wid_conn * 0.6;*/

				QVector2D *Pst = new QVector2D(Pn - len_conn * T * 0.5);
				QVector2D *Psn = new QVector2D(Pn + len_conn * T * 0.5);
				QVector2D *Pnst = new QVector2D(Psc - len_conn * T * 0.5);
				QVector2D *Pnsn = new QVector2D(Psc + len_conn * T * 0.5);
				QVector2D *Pnn = new QVector2D(Pn);

				printEdgePts.push_back(Pst);
				printEdgePts.push_back(Pnst);
				printEdgePts.push_back(Pnsn);
				printEdgePts.push_back(Psn);

				printEdgePtsCarves.push_back(Pnn);
				printEdgePtsCarves.push_back(Psn);

				break;
			}
			case ADVSAW_CONNECTOR:
			{
				//calculate 
				Float edge_len = Pthis->distanceToPoint(*Pnext);
				/*QVector2D T = (*Pnext - *Pthis).normalized();
				QVector2D d = (Pc - *Pthis);
				QVector2D a = QVector2D::dotProduct(d, T) * T;
				QVector2D n = (a - d).normalized();*/

				QVector2D Pn = *Pthis + T * edge_len * 0.5;
				QVector2D Psc = Pn + n * wid_conn * 1.5;

				//connector segment length				
				Float edgeConn_len = edge_len * 0.5f;

				QVector2D *Pst = new QVector2D(Pn - edgeConn_len * T * 0.5);
				QVector2D *Psn = new QVector2D(Pn + edgeConn_len * T * 0.1);
				QVector2D *Pnst = new QVector2D(Psc - edgeConn_len * T * 0.25);
				QVector2D *Pnsn = new QVector2D(Psc);
				QVector2D *Pnn = new QVector2D(Pn);

				printEdgePts.push_back(Pst);
				printEdgePts.push_back(Pnst);
				printEdgePts.push_back(Pnsn);
				printEdgePts.push_back(Psn);

				printEdgePtsCarves.push_back(Pnn);
				printEdgePtsCarves.push_back(Psn);

				break;
			}
			default:
				break;
			}

			// add text
			printTextPos.push_back(Pn - n * wid_conn * 0.6);
			printTextRot.push_back(RadianToDegree(atan2(T.y(), T.x())));
			printTextIfo.push_back(HDS_Common::ref_ID2String(curHE->refid));

			printEdgePts.push_back(Pnext);

			Pthis = Pnext;
			curHE = curHE->next;
		} while (curHE != he);

		/************************************************************************/
		/* Print Text                                                           */
		/************************************************************************/
		for (int i = 0; i < printTextPos.size(); i++)
		{
			auto pos = printTextPos[i];
			printText(fp, pos.x(), pos.y(),
				printTextRot[i], str_wd, printTextIfo[i]);

		}
		//////////////////////////////////////////////////////////////////////////
		fprintf(fp,
			"<g>\n" \
			"\t<polygon id=\"%d\" points=\"",
			face->index);
		for (int i = 0; i < printEdgePts.size(); i++)
		{
			fprintf(fp, "%f,%f ", printEdgePts[i]->x(), printEdgePts[i]->y());
		}
		fprintf(fp, "%f,%f\" style=\"fill:none;stroke:blue;stroke-width:%lf\" />\n",
			printEdgePts[0]->x(), printEdgePts[0]->y(), str_wd);
		//print carve edges
		if (printEdgePtsCarves.size())
		{

			for (int i = 0; i < printEdgePtsCarves.size(); i += 2)
			{
				fprintf(fp, "\t<polyline id=\"%d\" points=\"%f,%f %f,%f\" " \
					"style=\"fill:none;stroke:blue;stroke-width:%lf\" />\n",
					i,
					printEdgePtsCarves[i]->x(), printEdgePtsCarves[i]->y(),
					printEdgePtsCarves[i + 1]->x(), printEdgePtsCarves[i + 1]->y(), str_wd);

			}
		}

		// draw connected faces
		auto neighbourFaces = face->connectedFaces();
		for (auto face : neighbourFaces)
		{
			HDS_HalfEdge *he = face->he;
			HDS_HalfEdge *curHE = he;
			fprintf(fp, "\t<polygon id=\"%d\" points=\"", face->index);

			do {
				fprintf(fp, "%f,%f ",
					curHE->v->pos.x() * he_scale, curHE->v->pos.y() * he_scale);
				curHE = curHE->next;
			} while (curHE != he);
			fprintf(fp, "%f,%f\" style=\"fill:none;stroke:yellow;stroke-width:%lf\" />\n",
				he->v->pos.x() * he_scale, he->v->pos.y() * he_scale, str_wd);
		}
		//////////////////////////////////////////////////////////////////////////
		fprintf(fp, "</g>\n");//set a new group for inner lines
	}
	fprintf(fp,
		/*//"</g>\n"\*/
		"</svg>");
	fclose(fp);
	cout << "SVG file saved successfully!" << endl;
#endif
}

void MeshConnector::exportFBWalkPiece(FILE* fp,
	const mesh_t* unfolded_mesh, const confMap &conf)
{
#ifdef USE_LEGACY_FACTORY
	/************************************************************************/
	/* Scalors                                                              */
	/************************************************************************/
	Float he_offset = 10;
	Float str_wd = conf.at("strokeWd");
	Float he_scale = conf.at("scale");
	Float wid_conn = conf.at("width");
	Float len_conn = conf.at("length");
	Float pin_radius = conf.at("pinSize");
	int cn_t = static_cast<int>(conf.at("connector"));

	Float circle_offset = 3;
	QVector2D size_vec = unfolded_mesh->bound->getDiagnal().toVector2D();

	//SVG file head
	// define the size of export graph
	fprintf(fp, SVG_HEAD,
		static_cast<int>(size_vec.x() * he_scale),
		static_cast<int>(size_vec.y() * he_scale));

	int printFaceID(0), printPinholeID(0);

	// Pinhole numbers on each piece
	int n_pinhole;// = cn_t == ARCH_CONNECTOR
	switch (cn_t)
	{
	case ARCH_CONNECTOR:
		n_pinhole = 9;
		break;
	case RING_CONNECTOR:
		n_pinhole = 12;
		break;
	default:
		break;
	}

	// Start to record each piece
	for (auto piece : unfolded_mesh->pieceSet)
	{
		vector<face_t *> cutfaces;

		QVector2D centerPt; // piece center position
		vector<QVector2D> printRimPts;//Edges on the boundary
		vector<QVector2D> printPinholes;
		Float outerR, innerR, innerPinR, outerPinR, offRad, offPinRad;

		// Group current piece
		fprintf(fp, "<g>\n");
		for (auto fid : piece)
		{
			auto curFace = &unfolded_mesh->faceSet[fid];
			
			if (curFace->isCutFace)
			{
				auto he = curFace->he;
				auto curHE = he;

				centerPt = curFace->center().toVector2D() * he_scale;
				Float edgeLen = (curHE->v->pos - curHE->next()->v->pos).length() * he_scale;

				outerR = edgeLen * 2.0 / 3.0;
				innerR = outerR - wid_conn * 2.0 / 3.0 * sqrt(2);
				innerPinR = innerR + (outerR - innerR) / 3.0;
				outerPinR = outerR - (outerR - innerR) / 3.0;
                offRad = M_PI * 30.0 / 180.0;
				offPinRad = offRad * 0.5;
				QVector2D endDir(cos(offRad),sin(offRad));

				QVector2D v0 = centerPt + QVector2D(0, outerR);
				QVector2D v1 = centerPt + endDir * outerR;
				QVector2D v2 = centerPt + endDir * innerR;
				QVector2D v3 = centerPt + QVector2D(0, innerR);

				printRimPts.push_back(v0);
				printRimPts.push_back(v1);
				printRimPts.push_back(v2);
				printRimPts.push_back(v3);

				// Pinhole positions

				for (int i = 0; i < n_pinhole; i++)
				{
					Float rad = i * offRad + offPinRad;
					QVector2D pinDir(cos(rad), -sin(rad));
					printPinholes.push_back(centerPt + innerPinR * pinDir);
					printPinholes.push_back(centerPt + outerPinR * pinDir);
				}

			}
		}
		/************************************************************************/
		/* Write pinholes                                                    */
		/************************************************************************/
		for (auto pinpos : printPinholes)
		{
			fprintf(fp, SVG_CIRCLE, printPinholeID++, 1.0,
				pinpos.x(), pinpos.y(), str_wd);
		}

		/************************************************************************/
		/* Write out edge for cut                                               */
		/************************************************************************/
		// graph is upside down, rotation direction should be flipped
		switch (cn_t)
		{
		case ARCH_CONNECTOR:
			fprintf(fp,
				SVG_ARCH,
				printFaceID++,
				printRimPts[0].x(), printRimPts[0].y(),
				outerR, outerR, printRimPts[1].x(), printRimPts[1].y(),
				printRimPts[2].x(), printRimPts[2].y(),
				innerR, innerR, printRimPts[3].x(), printRimPts[3].y(), str_wd);
			break;
		case RING_CONNECTOR:
			fprintf(fp, // Outer circle
				"\t<circle id=\"Circle%d\" cx=\"%f\" cy=\"%f\" r=\"%lf\" " \
				"style=\"stroke:blue;stroke-width:%lf;fill:none\" />\n",
				printFaceID, centerPt.x(), centerPt.y(), outerR, str_wd);
			fprintf(fp, // Inner circle
				"\t<circle id=\"Circle%d\" cx=\"%f\" cy=\"%f\" r=\"%lf\" " \
				"style=\"stroke:blue;stroke-width:%lf;fill:none\" />\n",
				printFaceID, centerPt.x(), centerPt.y(), innerR, str_wd);
			fprintf(fp, // Carve edge
				SVG_LINE,
				printFaceID++,
				printRimPts[1].x(), printRimPts[1].y(),
				printRimPts[2].x(), printRimPts[2].y(),
				"blue", str_wd);
			break;
		default:
			break;
		}
		

		
		fprintf(fp, "</g>\n");//set a new group for inner lines
	}
	/************************************************************************/
	/* End of SVG File End                                                  */
	/************************************************************************/
	fprintf(fp, "</svg>");
#endif
}

void MeshConnector::exportWovenPiece(FILE* fp,
	const mesh_t* unfolded_mesh, const confMap &conf)
{
#ifdef USE_LEGACY_FACTORY
	/************************************************************************/
	/* Scalors                                                              */
	/************************************************************************/
	Float he_offset = 10;
	Float str_wd = conf.at("strokeWd");
	Float he_scale = conf.at("scale");
	Float wid_conn = conf.at("width");
	Float len_conn = conf.at("length");
	Float pin_radius = conf.at("pinSize");
	double uncut_len = ConvertToPt(
		static_cast<int>(conf.at("matConnUnit")),
		conf.at("matConnLen"));
	int etchSegCount = static_cast<int>(conf.at("etchSeg"));
	double etchSegWidth = conf.at("etchSegWidth");
	int cn_t = static_cast<int>(conf.at("connector"));

	Float circle_offset = 3;
	QVector2D size_vec = unfolded_mesh->bound->getDiagnal().toVector2D();

	//SVG file head
	// define the size of export graph
	fprintf(fp, SVG_HEAD,
		static_cast<int>(size_vec.x() * he_scale),
		static_cast<int>(size_vec.y() * he_scale));

	switch (cn_t)
	{
	case WOVEN_HOLE_CONNECTOR:
		break;
	default:
		break;
	}
	int printFaceID(0), printCircleID(0);
	
	// for pieces
	for (auto piece : unfolded_mesh->pieceSet)
	{
		vector<QVector2D> printBorderEdgePts;//Edges on the boundary
		vector<QVector2D> printEdgePtsCarves;
		vector<QVector2D> printPinholes;

		vector<QVector2D> printEtchEdges;
		unordered_set<he_t*> visitedEtchEdges;

		// Label data
		vector<QVector2D> printTextPos;
		floats_t printTextRot;
		vector<QString> printTextIfo;
		//unordered_map<int32_t, QVector2D> printTextRecord;
		
		// Group current piece
		fprintf(fp, "<g>\n");
		for (auto fid : piece)
		{
			face_t* curFace = unfolded_mesh->faceMap.at(fid);
			auto he = curFace->he;
			auto curHE = he;
			// Cut layer
			if (curFace->isCutFace)
			{
				do
				{
					printBorderEdgePts.push_back(curHE->v->pos.toVector2D() * he_scale);
					curHE = curHE->next;
				} while (curHE != he);
			}
			// Etch layer
			else if (curFace->isJoint)
			{
				//QVector2D centerPt = curFace->center().toVector2D();
				printPinholes.push_back(curFace->center().toVector2D() * he_scale);
				
				// Add labels for pinhole
				printTextPos.push_back(printPinholes.back() * he_scale);
				printTextRot.push_back(0);
				printTextIfo.push_back(HDS_Common::ref_ID2String(curFace->refid));
			}
			else // Etch
			{
				// Write points of each edge
				do
				{
					if (!curHE->isCutEdge
						&& visitedEtchEdges.find(curHE) == visitedEtchEdges.end())
					{
						printEtchEdges.push_back(curHE->v->pos.toVector2D() * he_scale);
						printEtchEdges.push_back(curHE->next->v->pos.toVector2D() * he_scale);
						visitedEtchEdges.insert(curHE);
						visitedEtchEdges.insert(curHE->flip);
					}
					curHE = curHE->next;
				} while (curHE != he);
			}			
		}
		/************************************************************************/
		/* Print Text                                                           */
		/************************************************************************/
		for (int i = 0; i < printTextPos.size(); i++)
		{
			auto pos = printTextPos[i];
			printText(fp, pos.x(), pos.y(),
				printTextRot[i], str_wd, printTextIfo[i]);
		}
		/************************************************************************/
		/* Write out circles                                                    */
		/************************************************************************/
		for (auto circlepos : printPinholes)
		{
			fprintf(fp, SVG_CIRCLE, printCircleID++,
				circlepos.x() , circlepos.y() ,
				pin_radius, str_wd);
		}

		/************************************************************************/
		/* Write out edge for cut                                               */
		/************************************************************************/
		// To open polyline
		if (uncut_len == 0)// if cut as open polyline
		{
			writeCutLayer(fp, printBorderEdgePts,
				str_wd, 1, printFaceID++);
		}
		// Cut as polygon
		else
		{
			auto endpos = printBorderEdgePts.back();
			auto lastDir = printBorderEdgePts.front() - endpos;
			endpos += lastDir * max(1 - uncut_len / lastDir.length(), 0.0);

			printBorderEdgePts.push_back(endpos);

			writeCutLayer(fp, printBorderEdgePts,
				str_wd, 0, printFaceID++);
		}
		/************************************************************************/
		/* Write out edge for etch                                               */
		/************************************************************************/
		wrtieEtchLayer(fp, printEtchEdges, str_wd, etchSegCount, etchSegWidth);
	}

	/************************************************************************/
	/* End of SVG File End                                                  */
	/************************************************************************/
	fprintf(fp, "</svg>");
#endif
}

void MeshConnector::writeCutLayer(
	FILE* SVG_File, const vector<QVector2D> &cut,
	Float str_wd, int cuttype, int id)
{
	switch (cuttype)
	{
	case 0: // To open polyline, connect with material
		fprintf(SVG_File, SVG_LINE, id,
			cut.back().x(), cut.back().y(),
			cut.front().x(), cut.front().y(),
			"yellow", str_wd);
		fprintf(SVG_File, "\t<polyline id=\"%d\" points=\"", id);
		break;
	case 1: // To closed polygon, disconnect from material
		fprintf(SVG_File, "\t<polygon id=\"%d\" points=\"", id);
		break;
	default:
		break;
	}

	for (int isec = 0; isec < cut.size(); isec++)
	{
		fprintf(SVG_File, "%f,%f ", cut[isec].x(), cut[isec].y());
	}
	
	fprintf(SVG_File, "\" style=\"fill:none;stroke:cyan;stroke-width:%lf\" />\n", str_wd);
}

void MeshConnector::wrtieEtchLayer(
	FILE* SVG_File, const vector<QVector2D> &etch,
	Float str_wd, int seg, Float segLen)
{
	double halfSegLen = ConvertToPt((int)UNIT_TYPE::INCH, segLen) * 0.5f;
	for (int isec = 0; isec < etch.size(); isec += 2)
	{
		float edgeNormX = etch[isec + 1].y() - etch[isec].y();
		float edgeNormY = etch[isec].x() - etch[isec + 1].x();
		for (int iseg = 0; iseg <= seg; iseg++)
		{
			Float offset = (iseg * 2 - seg) * halfSegLen;
			fprintf(SVG_File, SVG_LINE, isec / 2,
				etch[isec].x() + offset * edgeNormX,
				etch[isec].y() + offset * edgeNormY,
				etch[isec + 1].x() + offset * edgeNormX,
				etch[isec + 1].y() + offset * edgeNormY,
				"yellow", str_wd);
		}
	}
	fprintf(SVG_File, "</g>\n");//set a new group for inner lines
}

bool MeshConnector::genConnector(
	const mesh_t* unfolded_mesh,
	const QString &filename, const confMap &conf)
{
	if (unfolded_mesh == nullptr) return false;

	FILE *fp = fopen(filename.toUtf8(), "w");
	if (!fp) return false;

	switch (unfolded_mesh->processType)
	{
	case HDS_Mesh::HALFEDGE_PROC:
		exportRegularPiece(fp, unfolded_mesh, conf);
		break;
	case HDS_Mesh::QUAD_PROC:
		exportQuadEdgePiece(fp, unfolded_mesh, conf);
		break;
	case HDS_Mesh::WINGED_PROC:
		// new proc
		exportWingedEdgePiece(fp, unfolded_mesh, conf);
		break;
	case HDS_Mesh::GES_PROC:
		exportGESPiece(fp, unfolded_mesh, conf);
		break;
	case HDS_Mesh::GRS_PROC:
		exportGRSPiece(fp, unfolded_mesh, conf);//TBD, need connector on bridger
		fclose(fp);
		return false;
		break;
	case HDS_Mesh::FBWALK_PROC:
		//exportRimmedPiece(fp, unfolded_mesh, conf);
		fclose(fp);
		return false;
		break;
	case HDS_Mesh::WOVEN_PROC:
		exportWovenPiece(fp, unfolded_mesh, conf);
		break;
	default:
		break;
	}

	fclose(fp);
	return true;
}
