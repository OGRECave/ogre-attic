/*
	Lwo2MeshWriter based on the MilkShape exporter
	Dennis Verbeek (dennis.verbeek@chello.nl)

	doExportSkeleton is unfinished
*/

#pragma once
#include "lwObject.h"
#include "Ogre.h"

using namespace Ogre;

enum Parameters
{
	InfoOnly,
	PrintVMaps,
	UseSharedVertexData,
	UseSeparateLayers,
	GenerateLOD,
	UseFixedMethod,
	ExportMaterials,
	ExportSkeleton,
	HasNormals,
	MakeNewSubMesh,
	LinearCopy
};

#define NUMFLAGS 11

class Lwo2MeshWriter
{
public:	
	bool writeLwo2Mesh(lwObject *nobject, char *ndest);
private:
	void createSingletons(void);
	void destroySingletons(void);
	void createStatusArrays(void);
	void destroyStatusArrays(void);

	void countPointsPolys(void);
	void prepLwObject(void);

	void doExportMaterials(void);

	Skeleton *doExportSkeleton(const String &skelName, int layer);

	VertexData *setupVertexData(unsigned short vertexCount, VertexData *oldVertexData = 0, bool deleteOldVertexData = true);
	void copyPoints(int surfaceIndex, unsigned long polygontype, vpoints &sourcepoints, vpoints &destpoints);
	void copyPolygons(int surfaceIndex, unsigned long polygontype, vpolygons &sourcepolygons, vpolygons &destpolygons);
	void copyDataToVertexData(vpoints &points,
		vpolygons &polygons,
		vvmaps &vmaps,
		IndexData *indexData,
		VertexData *vertexData,
		unsigned short vertexDataOffset = 0);

	inline int getPointIndex(lwPoint *point, vpoints &points);
	inline void getTextureVMaps(vtextures &textures, vvmaps &svmaps, vvmaps &dvmaps);

	inline String makeLayerFileName(char* dest, unsigned int l, char *layername);
	inline String makeMaterialFileName(char* dest);

	char *dest;
	lwObject *object;
	Mesh* ogreMesh;
	
	unsigned int nLayers;
	unsigned int nSurfaces;
	
	unsigned int numPolygons;
	unsigned int *numLayerPolygons;
	unsigned int *numLayerSurfacePolygons;
	unsigned int *numSurfacePolygons;
	
	unsigned int vertexCount;
	unsigned int *numLayerVertices;
	unsigned int *numLayerSurfaceVertices;
	unsigned int *numSurfaceVertices;
};
