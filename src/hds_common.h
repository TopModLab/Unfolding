#pragma once
#include "common.h"
#include <QString>
#include "mathutils.hpp"

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
	static int assignRef_ID(hdsid_t id, hdsid_t type);
	
	static QString ref_ID2String(hdsid_t refid);

};
