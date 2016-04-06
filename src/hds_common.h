#pragma once
#include "common.h"
#include <QString>
#include <QVector3D>

using hdsid_t = uint32_t;

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

	HDS_Common() : flag(0) {}

	static int assignRef_ID(hdsid_t id, hdsid_t type);
	
	static QString ref_ID2String(hdsid_t refid);

	virtual uint16_t getFlag() const = 0;
	virtual void setFlag(uint16_t flgComp);
	virtual void unsetFlag(uint16_t flgComp);
	virtual void reverseFlag(uint16_t flgComp);

protected:
	uint16_t flag;

};

class HDS_Vertex;
class HDS_HalfEdge;
class HDS_Face;
