#include "meshmanager.h"
#include "meshcutter.h"
#include "meshunfolder.h"
#include "meshsmoother.h"
#include "MeshExtender.h"
#include "meshhollower.h"
#include "MeshIterator.h"
#include "MeshConnector.h"

#include "ConnectorPanel.h"

#include "utils.hpp"

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


const char SVG_HEAD[] =		"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n" \
							"<svg width=\"%d\" height=\"%d\" xmlns=\"http://www.w3.org/2000/svg\">\n";
const char SVG_CIRCLE[] = 	"\t<circle id=\"Circle%d\" cx=\"%f\" cy=\"%f\" r=\"1.0\" " \
							"style=\"stroke:blue;stroke-width:0.1;fill:none\" />\n";
const char SVG_LINE[] = 	"<line id=\"Line%d\" x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\" " \
							"style=\"fill:none;stroke:blue;stroke-width:0.1\" />\n";
const char SVG_TEXT[] = 	"\t<text x=\"%lf\" y=\"%lf\" fill=\"green\" " \
							"transform=\"rotate(%lf %lf,%lf)\" " \
							"style=\"font-size:3;stroke:green;stroke-width:0.1;fill:none;\" >" \
							"%s</text>\n";
const char SVG_ARCH[] = 	"<path id=\"Rim%d\" d=\"M %lf %lf " \
							"A %lf %lf, 0, 1, 1, %lf %lf " \
							"L %lf %lf " \
							"A %lf %lf, 0, 1, 0, %lf %lf " \
							"Z\" style=\"fill:none;stroke:blue;stroke-width:0.1\" />\n";
							/*
							"<path d=\"M p1x p1y " \
							"A R R, 0, 1, 0, p2x p2y " \
							"L p3x p3y " \
							"A r r, 0, 1, 1, p4x p4y " \
							"Z\" />");
							*/
void printText(FILE* file, double x, double y, double angle, const QString &text)
{
	fprintf(file, SVG_TEXT, x, y, angle, x, y, text.toUtf8().data());
}

//////////////////////////////////////////////////////////////////////////
// SVG formats
// 
// Text:	left-bottom corner as orginal position
//			rotation without position means rotating around origin point.
// Rim/Ring: 
//			
//////////////////////////////////////////////////////////////////////////

MeshConnector::MeshConnector()
{
}


MeshConnector::~MeshConnector()
{
}

void MeshConnector::exportHollowPiece(mesh_t* unfolded_mesh, const char* filename,
	const confMap& conf, int cn_t)
{
	if (unfolded_mesh == nullptr)
	{
		//assert();
		return;
	}
	//ConnectorType cn_t = SIMPLE_CONNECTOR;
	FILE *SVG_File;
	errno_t err = fopen_s(&SVG_File, filename, "w");
	if (err)
	{
		printf("Can't write to file %s!\n", filename);
		return;
	}

	/************************************************************************/
	/* Scalors                                                              */
	/************************************************************************/
	double he_offset(10);
	double he_scale = conf.find(ConnectorConf::SCALE)->second;
	double wid_conn = conf.find(ConnectorConf::WIDTH)->second;
	double len_conn = conf.find(ConnectorConf::LENGTH)->second; 
	
	double circle_offset = 3;
	QVector2D size_vec = unfolded_mesh->bound->getDiagnal().toVector2D();

	//SVG file head
	// define the size of export graph
	fprintf(SVG_File, SVG_HEAD,
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
		vector<double> printTextRot;
		vector<QString> printTextIfo;

		// Group current piece
		fprintf(SVG_File, "<g opacity=\"0.8\">\n");
		for (auto fid : piece)
		{
			face_t *curFace = unfolded_mesh->faceMap[fid];
			auto he = curFace->he;
			auto curHE = he;
			if (curFace->isCutFace)
			{
				vector<QVector2D> connCorners;
				do
				{
					// Find flipped corner
					if (!curHE->flip->f->isBridger &&
						!curHE->next->flip->f->isBridger)// &&
						//!curHE->next->next->flip->f->isBridger)
					{
						QVector2D Pc = curHE->flip->f->center().toVector2D() ;

						QVector2D v0 = curHE->v->pos.toVector2D();
						QVector2D v1 = curHE->next->v->pos.toVector2D();
						QVector2D v2 = curHE->next->next->v->pos.toVector2D();
						
						QVector2D d1 = v0 - v1;
						QVector2D d2 = v2 - v1;

						double len_1 = d1.length();
						double len_2 = d2.length();
						
						QVector2D dirPin;
						if (len_1 < len_2)
						{
							dirPin = d1 + len_1 * d2.normalized();
						}
						else
						{
							dirPin = d2 + len_2 * d1.normalized();
						}

						printPinholes.push_back(v1 + dirPin * 2.0 / 3.0);
						printPinholes.push_back(v1 + dirPin / 3.0);

						printTextPos.push_back(v1 + dirPin * 0.5);
						printTextRot.push_back(Radian2Degree(atan2(dirPin.y(), dirPin.x())));
						printTextIfo.push_back(HDS_Common::ref_ID2String(curHE->next->v->refid));
					}
					printBorderEdgePts.push_back(curHE->v->pos.toVector2D());
					curHE = curHE->next;
				} while (curHE != he);
			}
			else
			{
				fprintf(SVG_File, "\t<polygon id=\"%d\" points=\"", printFaceID++);
				// Write points of each edge
				do
				{
					fprintf(SVG_File, "%f,%f ",
						curHE->v->pos.x() * he_scale,
						curHE->v->pos.y() * he_scale);
					curHE = curHE->next;
				} while (curHE != he);
				// Close face loop
				fprintf(SVG_File,
					"%f,%f\" style=\"fill:none;stroke:yellow;stroke-width:0.1\" />\n",
					curHE->v->pos.x() * he_scale,
					curHE->v->pos.y() * he_scale);
				curHE = curHE->next;
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
			printText(SVG_File, pos.x() * he_scale, pos.y() * he_scale, rot, ifo);
			
		}
		/************************************************************************/
		/* Write out circles                                                    */
		/************************************************************************/
		for (auto circlepos : printPinholes)
		{
			fprintf(SVG_File, "\t<circle id=\"Circle%d\" cx=\"%f\" cy=\"%f\" r=\"0.5\" " \
				"style=\"stroke:blue;stroke-width:0.1;fill:none\" />\n",
				printCircleID++, circlepos.x() * he_scale, circlepos.y() * he_scale);
		}

		/************************************************************************/
		/* Write out edge for cut                                               */
		/************************************************************************/
		fprintf(SVG_File, "\t<polygon id=\"%d\" points=\"", printFaceID++);
		for (int isec = 0; isec < printBorderEdgePts.size(); isec++)
		{
			fprintf(SVG_File, "%f,%f ",
				printBorderEdgePts[isec].x() * he_scale,
				printBorderEdgePts[isec].y() * he_scale);
		}
		fprintf(SVG_File, "\" style=\"fill:none;stroke:blue;stroke-width:0.8\" />\n");
		/*fprintf(SVG_File, "%f,%f\" style=\"fill:none;stroke:blue;stroke-width:0.8\" />\n",
		printBorderEdgePts[0].x(), printBorderEdgePts[0].y());*/
		fprintf(SVG_File, "</g>\n");//set a new group for inner lines
	}
	/************************************************************************/
	/* End of SVG File End                                                  */
	/************************************************************************/
	fprintf(SVG_File, "</svg>");
	fclose(SVG_File);
	printf("SVG file %s saved successfully!\n", filename);
}

void MeshConnector::exportBindPiece(mesh_t* unfolded_mesh, const char* filename, const confMap& conf, int cn_t /*= HOLLOW_CONNECTOR*/)
{
	if (unfolded_mesh == nullptr)
	{
		//assert();
		return;
	}
	//ConnectorType cn_t = SIMPLE_CONNECTOR;
	FILE *SVG_File;
	errno_t err = fopen_s(&SVG_File, filename, "w");
	if (err)
	{
		printf("Can't write to file %s!\n", filename);
		return;
	}

	/************************************************************************/
	/* Scalors                                                              */
	/************************************************************************/
	double he_offset(10);
	double he_scale = conf.find(ConnectorConf::SCALE)->second;
	double wid_conn = conf.find(ConnectorConf::WIDTH)->second;
	double len_conn = conf.find(ConnectorConf::LENGTH)->second;

	double circle_offset = 3;
	QVector2D size_vec = unfolded_mesh->bound->getDiagnal().toVector2D();

	//SVG file head
	// define the size of export graph
	fprintf(SVG_File, SVG_HEAD,
		static_cast<int>(size_vec.x() * he_scale),
		static_cast<int>(size_vec.y() * he_scale));

	int printFaceID(0), printCircleID(0);
	for (auto piece : unfolded_mesh->pieceSet)
	{
		vector<face_t *> cutfaces;

		vector<QVector2D> printBorderEdgePts;//Edges on the boundary
		vector<QVector2D> printEdgePtsCarves;
		vector<QVector2D> printPinholes;

		// Group current piece
		fprintf(SVG_File, "<g opacity=\"0.8\">\n");
		for (auto fid : piece)
		{
			face_t *curFace = unfolded_mesh->faceMap[fid];
			auto he = curFace->he;
			auto curHE = he;
			if (curFace->isCutFace)
			{
				vector<QVector2D> connCorners;
				do
				{
					printBorderEdgePts.push_back(curHE->v->pos.toVector2D());
					curHE = curHE->next;
				} while (curHE != he);
			}
			else
			{
				if (!curFace->isBridger)
				{
					QVector2D centerPt = curFace->center().toVector2D();
					do
					{
						printPinholes.push_back((curHE->v->pos.toVector2D() + centerPt) * 0.5);
						curHE = curHE->next;
					} while (curHE != he);
				}
				fprintf(SVG_File, "\t<polygon id=\"%d\" points=\"", printFaceID++);
				// Write points of each edge
				do
				{
					fprintf(SVG_File, "%f,%f ",
						curHE->v->pos.x() * he_scale,
						curHE->v->pos.y() * he_scale);
					curHE = curHE->next;
				} while (curHE != he);
				// Close face loop
				fprintf(SVG_File,
					"%f,%f\" style=\"fill:none;stroke:yellow;stroke-width:0.1\" />\n",
					curHE->v->pos.x() * he_scale,
					curHE->v->pos.y() * he_scale);
				curHE = curHE->next;
			}

		}
		/************************************************************************/
		/* Write out circles                                                    */
		/************************************************************************/
		for (auto circlepos : printPinholes)
		{
			fprintf(SVG_File, SVG_CIRCLE, printCircleID++, circlepos.x() * he_scale, circlepos.y() * he_scale);
		}

		/************************************************************************/
		/* Write out edge for cut                                               */
		/************************************************************************/
		fprintf(SVG_File, "\t<polygon id=\"%d\" points=\"", printFaceID++);
		for (int isec = 0; isec < printBorderEdgePts.size(); isec++)
		{
			fprintf(SVG_File, "%f,%f ",
				printBorderEdgePts[isec].x() * he_scale,
				printBorderEdgePts[isec].y() * he_scale);
		}
		fprintf(SVG_File, "\" style=\"fill:none;stroke:blue;stroke-width:0.8\" />\n");
		/*fprintf(SVG_File, "%f,%f\" style=\"fill:none;stroke:blue;stroke-width:0.8\" />\n",
		printBorderEdgePts[0].x(), printBorderEdgePts[0].y());*/
		fprintf(SVG_File, "</g>\n");//set a new group for inner lines
	}
	/************************************************************************/
	/* End of SVG File End                                                  */
	/************************************************************************/
	fprintf(SVG_File, "</svg>");
	fclose(SVG_File);
	printf("SVG file %s saved successfully!\n", filename);
}

void MeshConnector::exportRegularPiece(mesh_t* unfolded_mesh, const char* filename,
	const confMap& conf, int cn_t)
{
	//ConnectorType cn_t = SIMPLE_CONNECTOR;
	FILE *SVG_File;
	errno_t err = fopen_s(&SVG_File, filename, "w");
	if (err)
	{
		printf("Can't write to files!\n");
		return;
	}

	unordered_set<HDS_Mesh::face_t*> faces = unfolded_mesh->faces();
	unordered_set<HDS_Mesh::face_t*> cutfaces;// , infaces;
	for (auto face : faces)
	{
		if (face->isCutFace)
		{
			cutfaces.insert(face);
		}
	}
	/************************************************************************/
	/* Scalors                                                              */
	/************************************************************************/
	double he_offset(10);
	double he_scale = conf.find(ConnectorConf::SCALE)->second;
	double wid_conn = conf.find(ConnectorConf::WIDTH)->second;
	double len_conn = conf.find(ConnectorConf::LENGTH)->second; 
	
	QVector2D size_vec = unfolded_mesh->bound->getDiagnal().toVector2D();

	//SVG file head
	// define the size of export graph
	fprintf(SVG_File, SVG_HEAD,
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
		fprintf(SVG_File, "<g opacity=\"0.8\">\n");
		for (auto fid : piece)
		{
			face_t *curFace = unfolded_mesh->faceMap[fid];
			auto he = curFace->he;
			auto curHE = he;
			if (curFace->isCutFace)
			{
				vector<QVector2D> connCorners;
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
		vector<double> printTextRot;
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
			QVector3D faceCenter = curHE->flip->f->center();
			QVector2D Pc(faceCenter.toVector2D() * he_scale);

			QVector2D *Pnext = new QVector2D(curHE->next->v->pos.toVector2D() * he_scale);

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
			QVector2D Psc = Pn + n * wid_conn * 0.6;

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
					//cutedges.erase(curHE->flip->cutTwin->flip);

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
					//cutedges.erase(curHE->flip->cutTwin->flip);
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
				double len_seg = Pthis->distanceToPoint(*Pnext) / ndiv * 0.5;

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
				double edge_len = Pthis->distanceToPoint(*Pnext);
				/*QVector2D T = (*Pnext - *Pthis).normalized();
				QVector2D d = (Pc - *Pthis);
				QVector2D a = QVector2D::dotProduct(d, T) * T;
				QVector2D n = (a - d).normalized();*/

				QVector2D Pn = *Pthis + T * edge_len * 0.5;
				QVector2D Psc = Pn + n * wid_conn * 1.5;

				//connector segment length				
				double edgeConn_len = edge_len * 0.5;

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
			printTextRot.push_back(Radian2Degree(atan2(T.y(), T.x())));
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
			auto rot = printTextRot[i];
			auto ifo = printTextIfo[i];
			printText(SVG_File, pos.x(), pos.y(), rot, ifo);

		}
		//////////////////////////////////////////////////////////////////////////
		fprintf(SVG_File,
			"<g opacity=\"0.8\">\n" \
			"\t<polygon id=\"%d\" points=\"",
			face->index);
		for (int i = 0; i < printEdgePts.size(); i++)
		{
			fprintf(SVG_File, "%f,%f ", printEdgePts[i]->x(), printEdgePts[i]->y());
		}
		fprintf(SVG_File, "%f,%f\" style=\"fill:none;stroke:blue;stroke-width:0.8\" />\n",
			printEdgePts[0]->x(), printEdgePts[0]->y());
		//print carve edges
		if (printEdgePtsCarves.size())
		{

			for (int i = 0; i < printEdgePtsCarves.size(); i += 2)
			{
				fprintf(SVG_File, "\t<polyline id=\"%d\" points=\"%f,%f %f,%f\" " \
					"style=\"fill:none;stroke:blue;stroke-width:0.8\" />\n",
					i,
					printEdgePtsCarves[i]->x(), printEdgePtsCarves[i]->y(),
					printEdgePtsCarves[i + 1]->x(), printEdgePtsCarves[i + 1]->y());

			}
		}

		/// draw connected faces
		set<HDS_Face*> neighbourFaces = face->connectedFaces();
		for (auto face : neighbourFaces)
		{
			HDS_HalfEdge *he = face->he;
			HDS_HalfEdge *curHE = he;
			fprintf(SVG_File, "\t<polygon id=\"%d\" points=\"", face->index);

			do {
				fprintf(SVG_File, "%f,%f ",
					curHE->v->pos.x() * he_scale, curHE->v->pos.y() * he_scale);
				curHE = curHE->next;
			} while (curHE != he);
			fprintf(SVG_File, "%f,%f\" style=\"fill:none;stroke:yellow;stroke-width:0.8\" />\n",
				he->v->pos.x() * he_scale, he->v->pos.y() * he_scale);
		}
		//////////////////////////////////////////////////////////////////////////
		fprintf(SVG_File, "</g>\n");//set a new group for inner lines
	}
	fprintf(SVG_File,
		/*//"</g>\n"\*/
		"</svg>");
	fclose(SVG_File);
	cout << "SVG file saved successfully!" << endl;
}

void MeshConnector::exportRimmedPiece(mesh_t* unfolded_mesh, const char* filename,
	const confMap& conf, int cn_t)
{
	assert(unfolded_mesh != nullptr);
	/*if (unfolded_mesh == nullptr)
	{
		return;
	}*/
	FILE *SVG_File;
	errno_t err = fopen_s(&SVG_File, filename, "w");
	if (err)// Unable to save to disk
	{
		printf("Can't write to file %s!\n", filename);
		return;
	}
	/************************************************************************/
	/* Scalors                                                              */
	/************************************************************************/
	double he_offset(10);
	double he_scale = conf.find(ConnectorConf::SCALE)->second;
	double wid_conn = conf.find(ConnectorConf::WIDTH)->second;
	double len_conn = conf.find(ConnectorConf::LENGTH)->second;

	double circle_offset = 3;
	QVector2D size_vec = unfolded_mesh->bound->getDiagnal().toVector2D();

	//SVG file head
	// define the size of export graph
	fprintf(SVG_File, SVG_HEAD,
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
		double outerR, innerR, innerPinR, outerPinR, offRad, offPinRad;

		// Group current piece
		fprintf(SVG_File, "<g opacity=\"0.8\">\n");
		for (auto fid : piece)
		{
			face_t *curFace = unfolded_mesh->faceMap[fid];
			
			if (curFace->isCutFace)
			{
				auto he = curFace->he;
				auto curHE = he;

				centerPt = curFace->center().toVector2D() * he_scale;
				double edgeLen = (curHE->v->pos - curHE->next->v->pos).length() * he_scale;

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
					double rad = i * offRad + offPinRad;
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
			fprintf(SVG_File, SVG_CIRCLE, printPinholeID++, pinpos.x(), pinpos.y());
		}

		/************************************************************************/
		/* Write out edge for cut                                               */
		/************************************************************************/
		// graph is upside down, rotation direction should be flipped
		switch (cn_t)
		{
		case ARCH_CONNECTOR:
			fprintf(SVG_File,
				SVG_ARCH,
				printFaceID++,
				printRimPts[0].x(), printRimPts[0].y(),
				outerR, outerR, printRimPts[1].x(), printRimPts[1].y(),
				printRimPts[2].x(), printRimPts[2].y(),
				innerR, innerR, printRimPts[3].x(), printRimPts[3].y());
			break;
		case RING_CONNECTOR:
			fprintf(SVG_File, // Outer circle
				"\t<circle id=\"Circle%d\" cx=\"%f\" cy=\"%f\" r=\"%lf\" " \
				"style=\"stroke:blue;stroke-width:0.1;fill:none\" />\n",
				printFaceID, centerPt.x(), centerPt.y(), outerR);
			fprintf(SVG_File, // Inner circle
				"\t<circle id=\"Circle%d\" cx=\"%f\" cy=\"%f\" r=\"%lf\" " \
				"style=\"stroke:blue;stroke-width:0.1;fill:none\" />\n",
				printFaceID, centerPt.x(), centerPt.y(), innerR);
			fprintf(SVG_File, // Carve edge
				SVG_LINE,
				printFaceID++,
				printRimPts[1].x(), printRimPts[1].y(),
				printRimPts[2].x(), printRimPts[2].y());
			break;
		default:
			break;
		}
		

		
		fprintf(SVG_File, "</g>\n");//set a new group for inner lines
	}
	/************************************************************************/
	/* End of SVG File End                                                  */
	/************************************************************************/
	fprintf(SVG_File, "</svg>");
	fclose(SVG_File);
	
	cout << "SVG file saved successfully!" << endl;
}

void MeshConnector::generateConnector(mesh_t *unfolded_mesh)
{
	ConnectorPanel *connPanel = new ConnectorPanel(unfolded_mesh->processType);
	connPanel->exec();
	
	QString filename = connPanel->getFilename();
	//double scale = connPanel->getScale();
	int cn_t = connPanel->getConnectorType();
	auto conf = connPanel->getConfiguration();
	switch (unfolded_mesh->processType)
	{
	case HDS_Mesh::REGULAR_PROC:
		exportRegularPiece(unfolded_mesh, filename.toUtf8(), conf, cn_t);
		break;
	case HDS_Mesh::HOLLOWED_PROC:
		exportHollowPiece(unfolded_mesh, filename.toUtf8(), conf, cn_t);
		break;
	case HDS_Mesh::BINDED_PROC:
		exportBindPiece(unfolded_mesh, filename.toUtf8(), conf, cn_t);
		break;
	case HDS_Mesh::RIMMED_PROC:
		exportRimmedPiece(unfolded_mesh, filename.toUtf8(), conf, cn_t);
		break;
	default:
		break;
	}
	delete connPanel;
}
