#include "hds_connector.h"

size_t HDS_Connector::uid = 0;

int HDS_Connector::shape = 0;
double HDS_Connector::scale = 0;
double HDS_Connector::curv = 0;
int HDS_Connector::nSamples = 1;
double HDS_Connector::cp = 0;
int HDS_Connector::opening = 0;

void HDS_Connector::setConnector(std::map<QString, double> config)
{
	shape = (int)config["shape"]; //0 bezier 1 original 2 flat
	scale = config["size"];
	curv = config["curv"];
	nSamples = (int)config["samples"];
	cp = config["cp"];
	opening = (int)config["opening"];

}
HDS_Connector::HDS_Connector(HDS_HalfEdge* he, HDS_HalfEdge* hef)
{
	this->he = he;
	this->hef = hef;


	for (int n = 0; n < nSamples; n++){
		HDS_Face * nf = new HDS_Face;
		/// fix the new face
		nf->index = HDS_Face::assignIndex();
		nf->isConnector = true;

		if (n == 0) {
			nf->he = he;

		}

		if (n > 1) {
			//init two new vertices
			//determine vertices positions
			vector<HDS_Vertex*> vertices = calculateBezierCurve(n);

			//init a pair of half edges from these vertices
			HDS_HalfEdge* newhe = new HDS_HalfEdge;
			HDS_HalfEdge* newhe_flip = new HDS_HalfEdge;

			newhe_flip->flip = newhe;
			newhe->flip = newhe_flip;

			newhe->index = HDS_HalfEdge::assignIndex();
			newhe_flip->index = HDS_HalfEdge::assignIndex();
			newhe->v = vertices.front();
			newhe_flip->v = vertices.back();

			//link adjacent faces to half edges
			newhe->f = faces.at(n-1);
			newhe_flip->f = nf;
			nf->he = newhe_flip;


			//link adjacent edges
			newhe->next = faces.at(n-1)->he;
			newhe->prev = faces.at(n-1)->he;
			faces.at(n-1)->he->next = newhe;
			faces.at(n-1)->he->prev = newhe;

			for(auto v : vertices) {
				v->he = newhe;
				v->index = HDS_Vertex::assignIndex();
			}
			verts.insert(verts.end(),vertices.begin(),vertices.end());
			hes.push_back(newhe);
			hes.push_back(newhe_flip);

		}
		faces.push_back(nf);
	}
	he->f = faces.front();
	hef->f = faces.back();

	//link original half edge
	he->next = hes.empty()? hef:hes.front();
	he->prev = hes.empty()? hef:hes.front();
	hef->next = hes.empty()? he:hes.back();
	hef->prev = hes.empty()? he:hes.back();

	//nf->isFlap = hef->f->isCutFace || he->f->isCutFace;

}
vector<HDS_Vertex*> HDS_Connector::calculateBezierCurve(int index)
{
	///bezier curve calculator
	///
	///           _________p0'
	///         p0      c2/__\
	///      c1/__\      /    \
	///       /    \    p1'    p2'
	///      p1    p2
	///

	//init vector size with (sample size - 1)*2
	vector<HDS_Vertex*> vertices((nSamples - 1)*2);

	//get he vertices position as refernce for p0 and p0'
	QVector3D hev1 = he->v->pos;
	QVector3D hev2 = hef->v->pos;

	//p1 p1'and p2 p2' are corners of incident faces



	//get curve point c1 based on #index

	//get c2 based on converging point position

	//init two vertices based on c1 c2 position

	return vertices;

}

HDS_Connector::~HDS_Connector()
{

}

