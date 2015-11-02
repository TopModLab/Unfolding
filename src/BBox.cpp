#include "BBox.h"


BBox3::BBox3()
{
	float max_val = std::numeric_limits<float>::max();
	float min_val = std::numeric_limits<float>::lowest();
	pMin = QVector3D(max_val, max_val, max_val);
	pMax = QVector3D(min_val, min_val, min_val);
}
BBox3::BBox3(const QVector3D& p) :pMin(p), pMax(p)
{
}
BBox3::BBox3(const QVector3D& p1, const QVector3D& p2)
{
	pMin = QVector3D(min(p1.x(), p2.x()), min(p1.y(), p2.y()), min(p1.z(), p2.z()));
	pMax = QVector3D(max(p1.x(), p2.x()), max(p1.y(), p2.y()), max(p1.z(), p2.z()));
}

BBox3::BBox3(float val)
	: pMin(val, val, val), pMax(val, val, val)
{
}

BBox3::~BBox3()
{
}
const QVector3D BBox3::getMidPoint() const
{
	return (pMax + pMin) * 0.5;
}

const QVector3D BBox3::getDiagnal() const
{
	return pMax - pMin;
}

void BBox3::expand(double delta)
{
	pMin -= QVector3D(delta, delta, delta);
	pMax += QVector3D(delta, delta, delta);
}
/*
bool BBox3::intersectP(const Ray& inRay, double *hitt0, double *hitt1) const
{
	double t0 = inRay.tmin, t1 = inRay.tmax;
	//double t0 = 0, t1 = INFINITY;
	for (int i = 0; i < 3; i++)
	{
		double tNear = (pMin[i] - inRay.pos[i]) / inRay.dir[i];
		double tFar = (pMax[i] - inRay.pos[i]) / inRay.dir[i];
		if (tNear > tFar)
		{
			swap(tNear, tFar);
		}
		t0 = tNear > t0 ? tNear : t0;
		t1 = tFar < t1 ? tFar : t1;
		if (t0 > t1)
		{
			return false;
		}
	}
	if (hitt0)
	{
		*hitt0 = t0;
	}
	if (hitt1)
	{
		*hitt1 = t1;
	}
	return true;
}*/
bool BBox3::overlaps(const BBox3& box) const
{
	bool over = true;
	for (int i = 0; i < 3; i++)
	{
		over &= (pMax[i] >= box.pMin[i]) && (pMin[i] <= box.pMax[i]);
	}
	return over;
}
bool BBox3::overlapon(const BBox3& box, PLANE over_plane) const
{
	return (pMax[over_plane] >= box.pMin[over_plane])
		&& (pMin[over_plane] <= box.pMax[over_plane]);
}
void BBox3::Union(const QVector3D& p)
{
	for (int i = 0; i < 3; i++)
	{
		this->pMin[i] = min(this->pMin[i], p[i]);
		this->pMax[i] = max(this->pMax[i], p[i]);
	}
}
void BBox3::Union(const BBox3& box)
{
	for (int i = 0; i < 3; i++)
	{
		this->pMin[i] = min(this->pMin[i], box.pMin[i]);
		this->pMax[i] = max(this->pMax[i], box.pMax[i]);
	}
}
BBox3 Union(const BBox3& box, const QVector3D& p)
{
	BBox3 ret = box;
	for (int i = 0; i < 3; i++)
	{
		ret.pMin[i] = min(box.pMin[i], p[i]);
		ret.pMax[i] = max(box.pMax[i], p[i]);
	}
	return ret;
}
BBox3 Union(const BBox3& box1, const BBox3& box2)
{
	BBox3 ret;
	for (int i = 0; i < 3; i++)
	{
		ret.pMin[i] = min(box1.pMin[i], box2.pMin[i]);
		ret.pMax[i] = max(box1.pMax[i], box2.pMax[i]);
	}
	return ret;
}
int BBox3::maxExtent() const
{
	QVector3D diag = pMax - pMin;
	if (diag.x() > diag.y() && diag.x() > diag.z())
	{
		return 0;
	}
	else if (diag.y() > diag.z())
	{
		return 1;
	}
	else
	{
		return 2;
	}
}
double BBox3::surfaceArea() const
{
	QVector3D d = pMax - pMin;
	return 2.0 * (d[0] * d[1] + d[0] * d[2] + d[1] * d[2]);
}
void BBox3::printInfo() const
{
	/*std::cout << "min point:"; pMin.printInfo();
	std::cout << "max point:"; pMax.printInfo();
	std::cout << "size:"; (pMax - pMin).printInfo();*/
}