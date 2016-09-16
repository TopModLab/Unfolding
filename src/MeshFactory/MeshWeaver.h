#pragma once
#ifndef MESHWEAVER_H
#define MESHWEAVER_H

#include "meshrimface.h"

class MeshWeaver : public MeshRimFace
{
public:
	static void configWeaveMesh(const confMap &config);

	static void weaveMesh(HDS_Mesh* mesh);
	static void weaveLinearScaledPiece();
	static void weaveBilinearScaledPiece();

private:
	static float size;
	static float depth;
	static float roundness;
	static float pivot;
	static float flapSize;
	static bool isBilinear;
	static bool isCone;

};

inline void
MeshWeaver::configWeaveMesh(const confMap &config) {
	isCone = config.at("shapeCone");
	isBilinear = config.at("scaleBilinear");
	size = config.at("thickness");
	depth = config.at("depth");
	roundness = config.at("roundness");
	pivot = config.at("pivot");
	flapSize = config.at("flap");
}

#endif // MESHWEAVER_H
