#pragma once
#ifndef __BBox__
#define __BBox__

#include "common.h"
#include <QVector3D>

// Bounding Box Class
class BBox3
{
public:
	enum PLANE
	{
		XY_PLANE,
		YZ_PLAZE,
		XZ_PLANE
	};
	QVector3D pMin, pMax;

	BBox3();
	BBox3(float val);
	BBox3(const QVector3D& p);
	BBox3(const QVector3D& p1, const QVector3D& p2);
	~BBox3();

	void printInfo() const;

	const QVector3D getMidPoint() const;
	const QVector3D getDiagnal() const;
	bool overlaps(const BBox3& box) const;
	bool overlapon(const BBox3& box, PLANE over_plane = XY_PLANE) const;
	void expand(double delta);
	void Union(const QVector3D& p);
	void Union(const BBox3& box);
	friend BBox3 Union(const BBox3& box, const QVector3D& p);
	friend BBox3 Union(const BBox3& box1, const BBox3& box2);

	int maxExtent() const;
	double surfaceArea() const;
};

#endif
