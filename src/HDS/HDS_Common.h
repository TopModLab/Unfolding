#pragma once
#include "Utils/common.h"
#include "Utils/mathutils.h"
#include <QString>
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
};

inline QString HDS_Common::ref_ID2String(hdsid_t refid)
{
	int type = refid & 3;
#if 0
	QString ret;
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
#endif

	return QString::number(refid >> 2);
}
struct HDSBinHeader
{
    uint32_t vertCount, heCount, faceCount;
    uint32_t pieceCount;
    uint16_t process_type;
    bool     bound_exist;
    HDSBinHeader() {}
    HDSBinHeader(uint32_t vc, uint32_t hc, uint32_t fc,
                 uint32_t pc, uint16_t pt, bool be)
                : vertCount(vc), heCount(hc), faceCount(fc)
                , pieceCount(pc), process_type(pt), bound_exist(be) {}
};
class HDS_Vertex;
class HDS_HalfEdge;
class HDS_Face;
class HDS_Mesh;
