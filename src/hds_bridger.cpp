#include "hds_bridger.h"

size_t HDS_Bridger::uid = 0;

int HDS_Bridger::shape = 0;
double HDS_Bridger::scale = 0.8;
double HDS_Bridger::curv = 0.5;
int HDS_Bridger::nSamples = 3;
double HDS_Bridger::cp = 0;
int HDS_Bridger::opening = 0;

void HDS_Bridger::setBridger(std::map<QString, double> config)
{
	shape = (int)config["shape"]; //0 bezier 1 original 2 flat
	scale = config["size"];
	curv = config["curv"];
	nSamples = (int)config["samples"];
	cp = config["cp"];
	opening = (int)config["opening"];

}

void HDS_Bridger::setScale(double size)
{
	scale = size;
}

void HDS_Bridger::setOriginalPositions()
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

	p10 = he->flip->v->pos;
	p11 = he->v->pos;
	p20 = hef->v->pos;
	p21 = hef->flip->v->pos;

	if (shape != 2) {
		bezierPos_front = calculateBezierCurve(p10, p00, p20);
		bezierPos_back = calculateBezierCurve(p11, p01, p21);
	}
}

HDS_Bridger::HDS_Bridger(HDS_HalfEdge* he, HDS_HalfEdge* hef)
{

	this->he = he;
	this->hef = hef;
	setOriginalPositions();


	if (shape != 2) { // not flat
	//push back all internal edges
	for (int i = 0; i < nSamples-1; i++) {
		HDS_Vertex* vs = new HDS_Vertex;
		HDS_Vertex* ve = new HDS_Vertex;
		vs->index = HDS_Vertex::assignIndex();
		ve->index = HDS_Vertex::assignIndex();
		vs->pos = bezierPos_front[i];
		ve->pos = bezierPos_back[i];

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


}


QVector3D getPt( QVector3D n1 , QVector3D n2 , float perc )
{
	QVector3D diff = n2 - n1;

	return n1 + ( diff * perc );
}


/////recursive function on bezier curve
//vector<QVector3D> HDS_Bridger::calculateBezierCurve(QVector3D p0, QVector3D p1, QVector3D p2, int iteration)
//{
//	cout<<"bezier curve recursion, iter = "<<iteration<<endl;
//	///bezier curve calculator
//	///
//	///           _________p0'
//	///         p0      c2/__\
//	///      c1/__\      /    \
//	///       /    \    p1'    p2'
//	///      p1    p2
//	///
// ///
// ///
//	vector<QVector3D> pos;

//	iteration++;
//	QVector3D p01_new = p0;
//	QVector3D p11_new = ( p0 + p1 )/2.0;
//	QVector3D p21_new = (p0 + 2.0*p1 + p2)/4.0;

//	QVector3D p02_new = p21;
//	QVector3D p12_new = ( p1 + p2 )/2.0;
//	QVector3D p22_new = p2;

//	if (iteration == nSamples - 2 ) {
//		pos.push_back(p11_new);
//		pos.push_back(p12_new);
//	}else {
//		vector<QVector3D> r_pos;
//		pos = calculateBezierCurve(p01_new, p11_new, p21_new, iteration);
//		r_pos = calculateBezierCurve(p02_new, p12_new, p22_new, iteration);
//		pos.insert( pos.end(), r_pos.begin(), r_pos.end() );
//	}
//	return pos;
//}

vector<QVector3D> HDS_Bridger::calculateBezierCurve(QVector3D p0, QVector3D p1, QVector3D p2)
{
	vector<QVector3D> pos;
	if (shape == 1)
	{
		pos.push_back(p1);
	}else {

		for( float i = 1.0/(float)nSamples ; i < 1 ; i += 1.0/(float)nSamples )
		{
			QVector3D pa = getPt( p0 , p1 , i );
			QVector3D pb = getPt( p1 , p2 , i );

			QVector3D p = getPt( pa , pb , i );
			pos.push_back(p);
		}
	}
	return pos;
}

HDS_Bridger::~HDS_Bridger()
{

}

