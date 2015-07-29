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
void HDS_Connector::setOriginalPositions()
{
	QVector3D corner1, corner0, center, mid;
	//find corner1 pos that corresponds to he->v
	center = he->flip->f->center();
	corner1 = center + (he->v->pos - center)/scale;
	//find corner0 pos that corresponds to he->flip->v
	corner0 = center + (he->flip->v->pos - center)/scale;
	//get mid position
	mid = (corner1 + corner0) /2.0;
	//calculate p00 p01
	p01 = mid + scale * (corner1 - mid);
	p00 = mid + scale * (corner0 - mid);

}

HDS_Connector::HDS_Connector(HDS_HalfEdge* he, HDS_HalfEdge* hef)
{

	this->he = he;
	this->hef = hef;
	setOriginalPositions();


	//push back all internal edges
	for (int i = 0; i < nSamples-1; i++) {
		HDS_Vertex* vs = new HDS_Vertex;
		HDS_Vertex* ve = new HDS_Vertex;
		vs->index = HDS_Vertex::assignIndex();
		ve->index = HDS_Vertex::assignIndex();
		//test for original connectors
		vs->pos = p00;
		ve->pos = p01;

		HDS_HalfEdge* he_new = new HDS_HalfEdge;
		HDS_HalfEdge* he_new_flip = new HDS_HalfEdge;
		he_new->index = HDS_HalfEdge::assignIndex();
		he_new_flip->index = HDS_HalfEdge::assignIndex();
		he_new->setFlip(he_new_flip);

		vs->he = he_new;
		ve->he = he_new_flip;

		he_new->v = vs;
		he_new_flip->v = ve;

		//connect edge loop
		he_new->prev = he_new_flip;
		he_new->next = he_new_flip;
		he_new_flip->prev = he_new;
		he_new_flip->next = he_new;

		hes.push_back(he_new);
		hes.push_back(he_new_flip);
		verts.push_back(vs);
		verts.push_back(ve);
	}


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

