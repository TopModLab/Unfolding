#pragma once
#include "Utils/common.h"
#include <QString>
#include "Utils/mathutils.h"
#include <QVector3D>

using hdsid_t = int32_t;
const static hdsid_t sInvalidHDS = -1;

class HDS_Common
{
public:
	enum ReferenceID_type
	{
		ORIGIN = 0,
		FROM_VERTEX = 1,
		FROM_EDGE = 2,
		FROM_FACE = 3
	};

	static int assignRef_ID(hdsid_t id, hdsid_t type) { return (id << 2) + type; }
	
	static QString ref_ID2String(hdsid_t refid);

	virtual uint16_t getFlag() const = 0;
};

inline QString HDS_Common::ref_ID2String(hdsid_t refid)
{
	QString ret;
	int type = refid & 3;
	switch (type)
	{
	case ORIGIN:
		ret = "R";
		break;
	case FROM_VERTEX:
		ret = "V";
		break;
	case FROM_EDGE:
		ret = "E";
		break;
	case FROM_FACE:
		ret = "F";
		break;
	default:
		break;
	}

	return QString::number(refid >> 2);
}

class HDS_Vertex;
class HDS_HalfEdge;
class HDS_Face;
