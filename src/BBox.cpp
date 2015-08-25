#include "BBox.h"

template <typename vec_t>
BBox::BBox()
{
	pMin = vec_t(INFINITY, INFINITY, INFINITY);
	pMax = vec_t(-INFINITY, -INFINITY, -INFINITY);
}

template <typename vec_t>
BBox::BBox(const vec_t& p) :pMin(p), pMax(p)
{
}
template <typename vec_t>
BBox::BBox(const vec_t& p1, const vec_t& p2)
{
	pMin = vec_t(min(p1.x, p2.x), min(p1.y, p2.y), min(p1.z, p2.z));
	pMax = vec_t(max(p1.x, p2.x), max(p1.y, p2.y), max(p1.z, p2.z));
}
template <typename vec_t>
BBox::~BBox()
{
}
template <typename vec_t>
const vec_t BBox::getMidPoint() const
{
	return (pMax + pMin) / 2.0;
}
template <typename vec_t>
void BBox::expand(double delta)
{
	pMin -= vec_t(delta, delta, delta);
	pMax += vec_t(delta, delta, delta);
}
/*
bool BBox::intersectP(const Ray& inRay, double *hitt0, double *hitt1) const
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
template <typename vec_t>
bool BBox::overlaps(const BBox& box) const
{
	bool over = true;
	for (int i = 0; i < 3; i++)
	{
		over &= (pMax[i] >= box.pMin[i]) && (pMin[i] <= box.pMax[i]);
	}
	return over;
}
template <typename vec_t>
void BBox::Union(const vec_t& p)
{
	for (int i = 0; i < 3; i++)
	{
		this->pMin[i] = min(this->pMin[i], p[i]);
		this->pMax[i] = max(this->pMax[i], p[i]);
	}
}
template <typename vec_t>
void BBox::Union(const BBox& box)
{
	for (int i = 0; i < 3; i++)
	{
		this->pMin[i] = min(this->pMin[i], box.pMin[i]);
		this->pMax[i] = max(this->pMax[i], box.pMax[i]);
	}
}
template <typename vec_t>
BBox Union(const BBox& box, const vec_t& p)
{
	BBox ret = box;
	for (int i = 0; i < 3; i++)
	{
		ret.pMin[i] = min(box.pMin[i], p[i]);
		ret.pMax[i] = max(box.pMax[i], p[i]);
	}
	return ret;
}
template <typename vec_t>
BBox Union(const BBox& box1, const BBox& box2)
{
	BBox ret;
	for (int i = 0; i < 3; i++)
	{
		ret.pMin[i] = min(box1.pMin[i], box2.pMin[i]);
		ret.pMax[i] = max(box1.pMax[i], box2.pMax[i]);
	}
	return ret;
}
template <typename vec_t>
int BBox::maxExtent() const
{
	vec_t diag = pMax - pMin;
	if (diag.x > diag.y && diag.x > diag.z)
	{
		return 0;
	}
	else if (diag.y > diag.z)
	{
		return 1;
	}
	else
	{
		return 2;
	}
}
template <typename vec_t>
double BBox::surfaceArea() const
{
	vec_t d = pMax - pMin;
	return 2.0 * (d[0] * d[1] + d[0] * d[2] + d[1] * d[2]);
}
template <typename vec_t>
void BBox::printInfo() const
{
	/*std::cout << "min point:"; pMin.printInfo();
	std::cout << "max point:"; pMax.printInfo();
	std::cout << "size:"; (pMax - pMin).printInfo();*/
}