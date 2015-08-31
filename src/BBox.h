#pragma once
#ifndef __BBox__
#define __BBox__

#include "common.h"
#include <QVector3D>

// Bounding Box Class
class BBox3
{
public:
	enum AXIS
	{
		X_AXIS,
		Y_AXIS,
		Z_AXIS
	};
	QVector3D pMin, pMax;

	BBox3();
	BBox3(const QVector3D& p);
	BBox3(const QVector3D& p1, const QVector3D& p2);
	~BBox3();

	void printInfo() const;

	const QVector3D getMidPoint() const;
	/*bool intersectP(const Ray& inRay) const;*/
	bool overlaps(const BBox3& box) const;
	bool overlapon(const BBox3& box, AXIS axis = X_AXIS) const;
	void expand(double delta);
	void Union(const QVector3D& p);
	void Union(const BBox3& box);
	friend BBox3 Union(const BBox3& box, const QVector3D& p);
	friend BBox3 Union(const BBox3& box1, const BBox3& box2);

	int maxExtent() const;
	double surfaceArea() const;
};

#endif
