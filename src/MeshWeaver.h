#pragma once
#ifndef MESHWEAVER_H
#define MESHWEAVER_H

#include "meshrimface.h"

class MeshWeaver : public MeshRimFace
{
public:
	static void configWeaveMesh(std::map<QString, float> config);

	static void weaveMesh(HDS_Mesh* mesh);
	static void weaveLinearScaledPiece();
	static void weaveBilinearScaledPiece();

private:
	static float size;
	static float depth;
	static float roundness;
	static bool isBilinear;

};

inline void
MeshWeaver::configWeaveMesh(std::map<QString, float> config) {
	isBilinear = config["scaleBilinear"];
	size = config["thickness"];
	depth = config["depth"];
	roundness = config["roundness"];
}

#endif // MESHWEAVER_H
