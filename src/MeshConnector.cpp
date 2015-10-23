#include "meshmanager.h"
#include "meshcutter.h"
#include "meshunfolder.h"
#include "meshsmoother.h"
#include "MeshExtender.h"
#include "meshhollower.h"
#include "MeshIterator.h"
#include "MeshConnector.h"

#include "ConnectorSelectionPanel.h"

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

const char SVG_HEAD[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n" \
"<svg width=\"%d\" height=\"%d\" xmlns=\"http://www.w3.org/2000/svg\">\n";

MeshConnector::MeshConnector()
{
}


MeshConnector::~MeshConnector()
{
}

void MeshConnector::exportHollowPiece(mesh_t* unfolded_mesh,
	const char* filename, int mode)
{
	if (unfolded_mesh == nullptr)
	{
		//assert();
		return;
	}
	ConnectorType cn_t = SIMPLE_CONNECTOR;
	FILE *SVG_File;
	errno_t err = fopen_s(&SVG_File, filename, "w");
	if (err)
	{
		printf("Can't write to file %s!\n", filename);
		return;
	}
	int size_x(360), size_y(360);
	//printf("Type in SVG file size: ");
	//err = scanf_s("%d%d", &size_x, &size_y);

	//SVG file head
	fprintf(SVG_File, SVG_HEAD,	size_x, size_y);// define the size of export graph

	/************************************************************************/
	/* for cut layer                                                        */
	/************************************************************************/

	double he_offset(10), he_scale(40), wid_conn(10), len_conn(10);
	double circle_offset = 3;
	//err = scanf_s("%lf", &circle_offset);

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
				do
				{
					// Find flipped corner
					if (!curHE->flip->f->isBridger
						&& !curHE->next->flip->f->isBridger
						&& !curHE->next->next->flip->f->isBridger)
					{
						QVector2D Pc = curHE->flip->f->center().toVector2D() ;

						QVector2D v0 = curHE->v->pos.toVector2D();
						QVector2D v1 = curHE->next->v->pos.toVector2D();
						QVector2D v2 = curHE->next->next->v->pos.toVector2D();
						QVector2D v3 = curHE->next->next->next->v->pos.toVector2D();

						QVector2D d0 = (v3 - v0).normalized();
						QVector2D d1 = v0 - v1;
						QVector2D d2 = v1 - v2;
						QVector2D d3 = v3 - v2;

						QVector2D dirPin0 = d1 + d1.length() * d0;
						QVector2D dirPin1 = d3 - d3.length() * d0;

						/*QVector2D d1 = ((v0 - v1).normalized()
							+ (v2 - v1).normalized()).normalized();
						/ *QVector2D d2 = ((v1 - v2).normalized()
							+ (v3 - v2).normalized()).normalized();* /*/
						//if (d1.lengthSquared() != 0)
						{
							printPinholes.push_back(v1 + dirPin0 * 2.0 / 3.0);
							printPinholes.push_back(v1 + dirPin0 / 3.0);
							printPinholes.push_back(v2 + dirPin1 * 2.0 / 3.0);
							printPinholes.push_back(v2 + dirPin1 / 3.0);
						}
						/*QVector2D v0 = curHE->prev->v->pos.toVector2D() * he_scale;
						QVector2D v1 = curHE->v->pos.toVector2D() * he_scale;
						QVector2D v2 = curHE->next->v->pos.toVector2D() * he_scale;
						QVector2D v3 = curHE->next->next->v->pos.toVector2D() * he_scale;

						QVector2D d1 = ((v0 - v1).normalized()
							+ (v2 - v1).normalized()).normalized();
						QVector2D d2 = ((v1 - v2).normalized()
							+ (v3 - v2).normalized()).normalized();
						if (d1.lengthSquared() != 0 && d2.lengthSquared() != 0)
						{
							printPinholes.push_back(v1 + d1 * circle_offset);
							printPinholes.push_back(v1 + d1 * circle_offset * 2);
							printPinholes.push_back(v2 + d2 * circle_offset);
							printPinholes.push_back(v2 + d2 * circle_offset * 2);
						}*/
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
		/* Write out circles                                                    */
		/************************************************************************/
		for (auto circlepos : printPinholes)
		{
			fprintf(SVG_File, "\t<circle id=\"Circle%d\" cx=\"%f\" cy=\"%f\" r=\"0.5\" " \
				"style=\"stroke:black;stroke-width:0.1;fill:none\" />\n",
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

void MeshConnector::exportRegularPiece(mesh_t* unfolded_mesh,
	const char* filename, int mode)
{

	enum ConnectorType
	{
		SIMPLE_CONNECTOR,
		INSERT_CONNECTOR,
		GEAR_CONNECTOR,
		SAW_CONNECTOR,
		ADVSAW_CONNECTOR
	};
	ConnectorType cn_t = SIMPLE_CONNECTOR;
	FILE *SVG_File;
	errno_t err = fopen_s(&SVG_File, filename, "w");
	if (err)
	{
		printf("Can't write to files!\n");
		return;
	}
	int size_x(360), size_y(360);
	//printf("Type in SVG file size: ");
	//err = scanf_s("%d%d", &size_x, &size_y);

	unordered_set<HDS_Mesh::face_t*> faces = unfolded_mesh->faces();
	unordered_set<HDS_Mesh::face_t*> cutfaces;// , infaces;
	for (auto face : faces)
	{
		if (face->isCutFace)
		{
			cutfaces.insert(face);
		}
	}
	//SVG file head
	fprintf(SVG_File, SVG_HEAD, size_x, size_y);// define the size of export graph
	//for cut layer
	//QVector3D nx, ny;
	double he_offset(10), he_scale(20), wid_conn(10), len_conn(10);

	for (auto face : cutfaces)
	{
		HDS_HalfEdge *he = face->he;
		HDS_HalfEdge *curHE = he;

		//////////////////////////////////////////////////////////////////////////
		unordered_set<HDS_HalfEdge*> cutedges;
		unordered_set<HDS_HalfEdge*> cutTwinEdges;
		vector<QVector2D*> printEdgePts;
		vector<QVector2D*> printEdgePtsCarves;
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
			switch (cn_t)
			{
			case SIMPLE_CONNECTOR:
			{
				if (cutedges.find(curHE) != cutedges.end())
				{
					//calculate 
					QVector2D T = (*Pnext - *Pthis).normalized();
					QVector2D d = (Pc - *Pthis);
					QVector2D a = QVector2D::dotProduct(d, T) * T;
					QVector2D n = (a - d).normalized();

					QVector2D Pn = *Pthis + a;
					QVector2D Psc = Pn + n * wid_conn;

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
				QVector2D T = (*Pnext - *Pthis).normalized();
				QVector2D d = (Pc - *Pthis);
				QVector2D a = QVector2D::dotProduct(d, T) * T;
				QVector2D n = (a - d).normalized();

				QVector2D Pn = *Pthis + a;
				QVector2D Psc = Pn + n * wid_conn;

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
				QVector2D T = (*Pnext - *Pthis).normalized();
				QVector2D d = (Pc - *Pthis);
				QVector2D a = QVector2D::dotProduct(d, T) * T;
				QVector2D n = (a - d).normalized();

				QVector2D Pn = *Pthis + a;
				QVector2D Psc = Pn + n * wid_conn;

				//division number
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
				QVector2D T = (*Pnext - *Pthis).normalized();
				QVector2D d = (Pc - *Pthis);
				QVector2D a = QVector2D::dotProduct(d, T) * T;
				QVector2D n = (a - d).normalized();

				QVector2D Pn = *Pthis + a;
				QVector2D Psc = Pn + n * wid_conn * 0.6;

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
				QVector2D T = (*Pnext - *Pthis).normalized();
				QVector2D d = (Pc - *Pthis);
				QVector2D a = QVector2D::dotProduct(d, T) * T;
				QVector2D n = (a - d).normalized();

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

			printEdgePts.push_back(Pnext);

			Pthis = Pnext;
			curHE = curHE->next;
		} while (curHE != he);

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
			//double he_offset(10), he_scale(20);
			HDS_HalfEdge *he = face->he;
			HDS_HalfEdge *curHE = he;
			fprintf(SVG_File, "\t<polygon id=\"%d\" points=\"", face->index);

			do {
				//fprintf(SVG_File, "%f,%f ",
				//		(QVector3D::dotProduct(curHE->v->pos, nx) + he_offset) * he_scale,
				//		(QVector3D::dotProduct(curHE->v->pos, ny) + he_offset) * he_scale);
				fprintf(SVG_File, "%f,%f ",
					curHE->v->pos.x() * he_scale, curHE->v->pos.y() * he_scale);
				curHE = curHE->next;
			} while (curHE != he);
			/*fprintf(SVG_File, "%f,%f\" style=\"fill:none;stroke:yellow;stroke-width:0.8\" />\n",
				(QVector3D::dotProduct(he->v->pos, nx) + he_offset) * he_scale,
				(QVector3D::dotProduct(he->v->pos, ny) + he_offset) * he_scale);*/
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
void MeshConnector::exportRimmedPiece(mesh_t* unfolded_mesh, const char* filename, int mode)
{
	if (unfolded_mesh == nullptr)
	{
		//assert();
		return;
	}
	FILE *SVG_File;
	errno_t err = fopen_s(&SVG_File, filename, "w");
	if (err)
	{
		printf("Can't write to file %s!\n", filename);
		return;
	}
	int size_x(360), size_y(360);

	//SVG file head
	fprintf(SVG_File, SVG_HEAD, size_x, size_y);// define the size of export graph

	/************************************************************************/
	/* for cut layer                                                        */
	/************************************************************************/

	double he_offset(10), he_scale(20), wid_conn(10), len_conn(10);
	double circle_offset = 3;
	//err = scanf_s("%lf", &circle_offset);

	int printFaceID(0), printPinholeID(0);
	for (auto piece : unfolded_mesh->pieceSet)
	{
		vector<face_t *> cutfaces;

		vector<QVector2D> printRimPts;//Edges on the boundary
		vector<QVector2D> printPinholes;
		double outerR, innerR, innerPinR, outerPinR, offRad;

		// Group current piece
		fprintf(SVG_File, "<g opacity=\"0.8\">\n");
		for (auto fid : piece)
		{
			face_t *curFace = unfolded_mesh->faceMap[fid];
			
			if (curFace->isCutFace)
			{
				auto he = curFace->he;
				auto curHE = he;

				auto fc = curFace->center().toVector2D() * he_scale;
				double edgeLen = (curHE->v->pos - curHE->next->v->pos).length() * he_scale;

				outerR = edgeLen * 2.0 / 3.0;
				innerR = outerR - wid_conn * 2.0 / 3.0 * sqrt(2);
				innerPinR = innerR + (outerR - innerR) / 3.0;
				outerPinR = outerR - (outerR - innerR) / 3.0;
                offRad = M_PI * 30.0 / 180.0;
				QVector2D endDir(cos(offRad),sin(offRad));

				QVector2D v0 = fc + QVector2D(0, outerR);
				QVector2D v1 = fc + endDir * outerR;
				QVector2D v2 = fc + endDir * innerR;
				QVector2D v3 = fc + QVector2D(0, innerR);

				printRimPts.push_back(v0);
				printRimPts.push_back(v1);
				printRimPts.push_back(v2);
				printRimPts.push_back(v3);

				// Pinhole positions
				for (int i = 0; i < 9; i++)
				{
					QVector2D pinDir(cos(i * offRad), -sin(i * offRad));
					printPinholes.push_back(fc + innerPinR * pinDir);
					printPinholes.push_back(fc + outerPinR * pinDir);
				}

			}
		}
		/************************************************************************/
		/* Write pinholes                                                    */
		/************************************************************************/
		for (auto pinpos : printPinholes)
		{
			fprintf(SVG_File, "\t<circle id=\"Circle%d\" cx=\"%f\" cy=\"%f\" r=\"0.8\" " \
				"style=\"stroke:black;stroke-width:0.1;fill:none\" />\n",
				printPinholeID++, pinpos.x(), pinpos.y());
		}

		/************************************************************************/
		/* Write out edge for cut                                               */
		/************************************************************************/
		// graph is upside down, rotation direction should be flipped
		fprintf(SVG_File,
			"<path d=\"M %lf %lf " \
			"A %lf %lf, 0, 1, 1, %lf %lf " \
			"L %lf %lf " \
			"A %lf %lf, 0, 1, 0, %lf %lf " \
			"Z\" style=\"fill:none;stroke:blue;stroke-width:0.1\" />\n",
			printRimPts[0].x(), printRimPts[0].y(),
			outerR, outerR, printRimPts[1].x(), printRimPts[1].y(),
			printRimPts[2].x(), printRimPts[2].y(),
			innerR, innerR, printRimPts[3].x(), printRimPts[3].y());

		
		fprintf(SVG_File, "</g>\n");//set a new group for inner lines
	}
	/************************************************************************/
	/* End of SVG File End                                                  */
	/************************************************************************/
	fprintf(SVG_File, "</svg>");
	fclose(SVG_File);
	/*fprintf(SVG_File,
		"<path d=\"M p1x p1y " \
		"A R R, 0, 1, 0, p2x p2y " \
		"L p3x p3y " \
		"A r r, 0, 1, 1, p4x p4y " \
		"Z\" />");*/

	cout << "SVG file saved successfully!" << endl;
}

void MeshConnector::generateConnector(mesh_t *unfolded_mesh)
{
	ConnectorSelectionPanel *connPanel = new ConnectorSelectionPanel;
	connPanel->exec();
	
	const char* filename = connPanel->getFilename().toUtf8().constData();
	switch (unfolded_mesh->processType)
	{
	case HDS_Mesh::HOLLOWED_PROC:
		exportHollowPiece(unfolded_mesh, filename);
		break;
	case HDS_Mesh::RIMMED_PROC:
		exportRimmedPiece(unfolded_mesh, filename);
		break;
	default:
		exportRegularPiece(unfolded_mesh, filename);
		break;
	}
	delete connPanel;
}
