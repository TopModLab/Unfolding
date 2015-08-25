#pragma once
#ifndef __BBox__
#define __BBox__

#include "common.h"
#include <QVector2D>
#include <QVector3D>

// Bounding Box Class
template <typename vec_t>
class BBox
{
public:
	vec_t pMin, pMax;

	BBox();
	BBox(const vec_t& p);
	BBox(const vec_t& p1, const vec_t& p2);
	~BBox();

	void printInfo() const;

	const vec_t getMidPoint() const;
	/*bool intersectP(const Ray& inRay) const;*/
	bool overlaps(const BBox& box) const;
	void expand(double delta);
	void Union(const vec_t& p);
	void Union(const BBox& box);
	friend BBox Union(const BBox& box, const vec_t& p);
	friend BBox Union(const BBox& box1, const BBox& box2);

	int maxExtent() const;
	double surfaceArea() const;
};
typedef BBox<QVector2D> BBox2D;
typedef BBox<QVector3D> BBox3D;
//template class BBox < QVector2D > ;
//template class BBox < QVector3D > ;
#endif
