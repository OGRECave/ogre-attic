/*
	Lwo2MeshWriter based on the MilkShape exporter
	Dennis Verbeek (dennis.verbeek@chello.nl)

	doExportSkeleton is unfinished
*/

#pragma once
#include "lwObject.h"
#include "Ogre.h"

using namespace Ogre;

#define NUMFLAGS 9

enum Parameters
{
	InfoOnly,
	PrintVMaps,
	UseSharedGeometry,
	UseSeparateLayers,
	ExportMaterials,
	ExportSkeleton,
	HasNormals,
	MakeNewSubMesh,
	LinearCopy
};

class Lwo2MeshWriter
{
public:
	Lwo2MeshWriter()
	{
		createSingletons();
	}
	
	~Lwo2MeshWriter()
	{
		destroySingletons();
	}
	
	bool writeLwo2Mesh(lwObject *nobject, char *ndest, bool nflags[]);
private:
	void createSingletons(void);
	void destroySingletons(void);
	void createStatusArrays(void);
	void destroyStatusArrays(void);

	void countPointsPolys(void);
	void prepLwObject(void);

	void doExportMaterials(void);

	Skeleton *doExportSkeleton(const String &skelName, int layer);

	unsigned short setupGeometry(GeometryData *geometry, unsigned short numVertices);
	void copyPoints(int surfaceIndex, unsigned long polygontype, vpoints &sourcepoints, vpoints &destpoints);
	void copyPolygons(int surfaceIndex, unsigned long polygontype, vpolygons &sourcepolygons, vpolygons &destpolygons);
	void copyDataToGeometry(vpoints &points,
		vpolygons &polygons,
		vvmaps &vmaps,
		SubMesh *ogreSubMesh,
		GeometryData *geometry,
		unsigned short geometryOffset);

	inline int getPointIndex(lwPoint *point, vpoints &points);
	inline void getTextureVMaps(vtextures &textures, vvmaps &svmaps, vvmaps &dvmaps);

	inline String makeLayerFileName(char* dest, unsigned int l, char *layername);

	char *dest;
	lwObject *object;
	Mesh* ogreMesh;
	bool *flags;
	
	unsigned int nLayers;
	unsigned int nSurfaces;
	
	unsigned int numPolygons;
	unsigned int *numLayerPolygons;
	unsigned int *numLayerSurfacePolygons;
	unsigned int *numSurfacePolygons;
	
	unsigned int numVertices;
	unsigned int *numLayerVertices;
	unsigned int *numLayerSurfaceVertices;
	unsigned int *numSurfaceVertices;

	LogManager *pLogMgr;
	MaterialManager *pMatMgr;
	SkeletonManager *pSkelMgr;
};
