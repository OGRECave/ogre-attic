#include "vector3.h"
#include "lwo2mesh.h"
#include "Ogre.h"
#include "OgreMesh.h"
#include "OgreStringConverter.h"

#define POLYLIMIT 0x5555
#define POINTLIMIT 0x5555

void Lwo2MeshWriter::doExportMaterials()
{
	char
		drive[ _MAX_DRIVE ],
		dir[ _MAX_DIR ],
		node[ _MAX_FNAME ],
		ext[ _MAX_EXT ],
		texname [128];
	
    for (unsigned int i = 0; i < object->surfaces.size(); ++i)
    {		
        lwSurface *surface = object->surfaces[i];
        // Create deferred material so no load
		
		Ogre::Material* ogreMat = (Ogre::Material*)pMatMgr->getByName(surface->name);
		if (!ogreMat)
			ogreMat = (Ogre::Material*)pMatMgr->createDeferred(surface->name);
		
        ogreMat->setAmbient
		(
			surface->color.rgb[0],
			surface->color.rgb[1],
			surface->color.rgb[2]
		);

        ogreMat->setDiffuse
		(
			surface->diffuse.val * surface->color.rgb[0],
			surface->diffuse.val * surface->color.rgb[1],
			surface->diffuse.val * surface->color.rgb[2]
		);

        ogreMat->setSpecular
		(
			surface->specularity.val * surface->color.rgb[0],
			surface->specularity.val * surface->color.rgb[1],
			surface->specularity.val * surface->color.rgb[2]
		);

        ogreMat->setShininess(surface->glossiness.val);

		ogreMat->setSelfIllumination
		(
			surface->luminosity.val * surface->color.rgb[0],
			surface->luminosity.val * surface->color.rgb[1],
			surface->luminosity.val * surface->color.rgb[2]
		);
		
		for (unsigned int j = 0; j < surface->color.textures.size(); j++)
		{
			lwTexture *tex = surface->color.textures[j];
			int cindex = tex->param.imap->cindex;
			lwClip *clip = object->lwFindClip(cindex);
			
			if (clip)
			{
				_splitpath( clip->source.still->name, drive, dir, node, ext );
				_makepath( texname, 0, 0, node, ext );
				
				ogreMat->addTextureLayer(texname);
			}
		}
    }
}

Skeleton *Lwo2MeshWriter::doExportSkeleton(const String &skelName, int l)
{
	vpolygons bones;
	bones.clear();
	bones.reserve(256);

	vpoints bonepoints;
	bonepoints.clear();
	bonepoints.reserve(512);

	if (l == -1)
	{
		for (l = 0; l < object->layers.size(); ++l)
		{
			copyPoints(-1, ID_BONE, object->layers[l]->points, bonepoints);
			copyPolygons(-1, ID_BONE, object->layers[l]->polygons, bones);
		}
	}
	else
	{
		copyPoints(-1, ID_BONE, object->layers[l]->points, bonepoints);
		copyPolygons(-1, ID_BONE, object->layers[l]->polygons, bones);
	}

    if (!bones.size()) return NULL; // no bones means no skeleton

    Skeleton *ogreskel = new Skeleton(skelName);

    unsigned int i;
    // Create all the bones in turn
    for (i = 0; i < bones.size(); ++i)
    {
        lwPolygon* bone = bones[i];
		if (bone->vertices.size() != 2) continue; // a bone has only 2 sides

        Bone* ogreBone = ogreskel->createBone("Bone");

		Ogre::Vector3 bonePos(bone->vertices[0]->point->x, bone->vertices[0]->point->y, bone->vertices[0]->point->z);

        ogreBone->setPosition(bonePos);
        // Hmm, Milkshape has chosen a Euler angle representation of orientation which is not smart
        // Rotation Matrix or Quaternion would have been the smarter choice
        // Might we have Gimbal lock here? What order are these 3 angles supposed to be applied?
        // Grr, we'll try our best anyway...
        Ogre::Quaternion qx, qy, qz, qfinal;
/*
        qx.FromAngleAxis(msBoneRot[0], Ogre::Vector3::UNIT_X);
        qy.FromAngleAxis(msBoneRot[1], Ogre::Vector3::UNIT_Y);
        qz.FromAngleAxis(msBoneRot[2], Ogre::Vector3::UNIT_Z);
*/
		// Assume rotate by x then y then z
        qfinal = qz * qy * qx;
        ogreBone->setOrientation(qfinal);
    }
/*
    for (i = 0; i < numBones; ++i)
    {
        msBone* bone = msModel_GetBoneAt(pModel, i);

        if (strlen(bone->szParentName) == 0)
        {
        }
        else
        {
            Ogre::Bone* ogrechild = ogreskel->getBone(bone->szName);
            Ogre::Bone* ogreparent = ogreskel->getBone(bone->szParentName);

            if (ogrechild == 0)
            {
                continue;
            }
            if (ogreparent == 0)
            {
                continue;
            }
            // Make child
            ogreparent->addChild(ogrechild);
        }


    }

    // Create the Animation(s)
    doExportAnimations(pModel, ogreskel);

    // Create skeleton serializer & export
    SkeletonSerializer serializer;
    serializer.exportSkeleton(ogreskel, szFile);

    ogreMesh->_notifySkeleton(ogreskel);

    return ogreskel;
*/
	delete ogreskel;
	return NULL;
}

unsigned short Lwo2MeshWriter::setupGeometry(GeometryData *geometry, unsigned short numVertices)
{
	unsigned short oldNumvertices = geometry->numVertices;
	
	Real* pVertices;
	Real* pNormals;
	Real* pTexCoords[OGRE_MAX_TEXTURE_COORD_SETS];
	
	if (oldNumvertices)
	{
		pVertices = geometry->pVertices;
		if (geometry->hasNormals)
			pNormals = geometry->pNormals;
		pTexCoords[0] = geometry->pTexCoords[0];		
		
		numVertices += geometry->numVertices;
	}
	
	geometry->numVertices = numVertices;
	geometry->pVertices = new Ogre::Real[geometry->numVertices * 3];
	geometry->hasNormals = flags[HasNormals];
	if (geometry->hasNormals)
		geometry->pNormals = new Ogre::Real[geometry->numVertices * 3];
	else
		geometry->pNormals = 0;
	
	geometry->pTexCoords[0] = new Ogre::Real[geometry->numVertices * 2];
	
	if (oldNumvertices)
	{
		memcpy(geometry->pVertices, pVertices, oldNumvertices * 3 * 4);
		if (geometry->hasNormals)
			memcpy(geometry->pNormals, pNormals, oldNumvertices * 3 * 4);
		memcpy(geometry->pTexCoords[0], pTexCoords[0], oldNumvertices * 2 * 4);
		
		delete []pVertices;
		if (geometry->hasNormals)
			delete []pNormals;
		delete []pTexCoords[0];
	}
	
	return oldNumvertices;
}

void Lwo2MeshWriter::copyPoints(int surfaceIndex, unsigned long polygontype, vpoints &sourcepoints, vpoints &destpoints)
{
	for (unsigned int i = 0; i < sourcepoints.size(); i++)
	{
		lwPoint *point = sourcepoints[i];
		
		for (unsigned int j = 0; j < point->polygons.size(); j++)
		{
			lwPolygon *polygon = point->polygons[j];
			if (polygon->type == polygontype)
				if (surfaceIndex == -1 || surfaceIndex == polygon->surfidx)
				{
					destpoints.push_back(point);
					break;
				}
		}
	}
}

void Lwo2MeshWriter::copyPolygons(int surfaceIndex, unsigned long polygontype, vpolygons &sourcepolygons, vpolygons &destpolygons)
{
	for (unsigned int i = 0; i < sourcepolygons.size(); i++)
	{
		lwPolygon *polygon = sourcepolygons[i];
		if (polygon->type == polygontype)
			if (surfaceIndex == -1 || surfaceIndex == polygon->surfidx)
				destpolygons.push_back(polygon);
	}
}

void Lwo2MeshWriter::copyDataToGeometry(vpoints &points,
										vpolygons &polygons,
										vvmaps &vmaps,
										SubMesh *ogreSubMesh,
										GeometryData *geometry,
										unsigned short geometryOffset)
{
	lwVMap *vmap = 0;
	unsigned int ni;
	
	for (unsigned int p = 0; p < polygons.size(); p++)
	{
		lwPolygon *polygon = polygons[p];
		
		if (polygon->vertices.size() != 3) continue; // only copy triangles;

		for (unsigned int v = 0; v < polygon->vertices.size(); v++)
		{
			lwVertex *vertex = polygon->vertices[v];
			lwPoint *point = vertex->point;
			unsigned short i = getPointIndex(point, points);
			ogreSubMesh->faceVertexIndices[p*3 + v] = geometryOffset + i;
			
			ni = (geometryOffset + i) * 3;
			
			geometry->pVertices[ni] = vertex->point->x;
			geometry->pVertices[ni + 1] = vertex->point->y;
			geometry->pVertices[ni + 2] = vertex->point->z;
			
			if (geometry->hasNormals)
			{
				geometry->pNormals[ni] = vertex->normal.x;
				geometry->pNormals[ni + 1] = vertex->normal.y;
				geometry->pNormals[ni + 2] = vertex->normal.z;
			}
			
			bool found = false;
			
			ni = (geometryOffset + i) * 2;
			
			for (unsigned int v = 0; v < point->vmaps.size(); v++)
			{
				for (unsigned int vr = 0; vr < vmaps.size(); vr++)
				{
					vmap = vmaps[vr];
					if (point->vmaps[v].vmap == vmap)
					{
						int n = point->vmaps[v].index;
						
						geometry->pTexCoords[0][ni] = vmap->val[n][0];
						geometry->pTexCoords[0][ni + 1] = vmap->val[n][1];
						found = true;
						break;
					}
				}
				if (found) break;
			}
		}
	}
	
}

void Lwo2MeshWriter::prepLwObject(void)
{
	unsigned int l, p;
	
	for (l = 0; l < object->layers.size(); l++)
	{
		lwLayer *layer = object->layers[l];
		
#ifdef _DEBUG
		cout << "Triangulating layer " << l << ", Polygons before: " << layer->polygons.size();
#endif
		layer->triangulatePolygons();
#ifdef _DEBUG
		cout << ", Polygons after: " << layer->polygons.size() << endl;
#endif
		
		// mirror x-coord for Ogre;
		for (p = 0; p < layer->points.size(); p++)
		{
			layer->points[p]->x *= -1.0f;
			layer->points[p]->polygons.clear();
		}
		
		for ( p = 0; p < layer->polygons.size(); p++ )
		{
			lwPolygon *polygon = layer->polygons[ p ];
			for (unsigned int j = 0; j < polygon->vertices.size(); j++ )
				polygon->vertices[ j ]->point->polygons.push_back(polygon);
		}	
		
		for (p = 0; p < layer->polygons.size(); p++)
			layer->polygons[p]->flip();
		
		layer->calculatePolygonNormals();
		layer->calculateVertexNormals();
	}
}

inline int Lwo2MeshWriter::getPointIndex(lwPoint *point, vpoints &points)
{
	for (unsigned int i = 0; i < points.size(); ++i)
		if (points[i] == point) return i;
		
	return -1;
}

inline String Lwo2MeshWriter::makeLayerFileName(char* dest, unsigned int l, char *layername)
{
	char
		drive[ _MAX_DRIVE ],
		dir[ _MAX_DIR ],
		node[ _MAX_FNAME ],
		ext[ _MAX_EXT ];

	_splitpath( dest, drive, dir, node, ext );

	String LayerFileName;

	LayerFileName += drive;
	LayerFileName += dir;
	LayerFileName += node;

	if (layername)
	{
		LayerFileName += ".";
		LayerFileName += layername;
	}
	else
	{
		LayerFileName += ".layer" + StringConverter::toString(l);
	}

	LayerFileName += ext;

	const char *test = LayerFileName.c_str();

	return LayerFileName;
}

inline void Lwo2MeshWriter::getTextureVMaps(vtextures &textures, vvmaps &svmaps, vvmaps &dvmaps)
{
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		lwTexture *texture = textures[i];
		
		if (texture->type == ID_IMAP && texture->param.imap)
		{
			char *mapname = texture->param.imap->vmap_name;
			if (mapname)
				for (unsigned int v = 0; v < svmaps.size(); v++)
				{
					lwVMap *vmap = svmaps[v];
					if (strcmp(mapname, vmap->name) == 0) dvmaps.push_back(vmap);
				}
		}
	}
	return;
}

bool Lwo2MeshWriter::writeLwo2Mesh(lwObject *nobject, char *ndest, bool nflags[])
{
	object = nobject;
	dest = ndest;
	flags = nflags;
	
	if (!object) return false;
	if (!object->layers.size()) return false;
	
	pLogMgr->createLog("Lwo2MeshWriter.log");
	
	prepLwObject();
	
	vpoints points;
	vpolygons polygons;
	vvmaps vmaps;
	
	MeshSerializer meshserializer;

	if (flags[ExportMaterials])
		doExportMaterials();

	unsigned int ml = object->layers.size();

	bool SeparateLayers = flags[UseSeparateLayers] && ml > 1;

	if (!SeparateLayers) ogreMesh = new Mesh(ndest);

	for( unsigned int ol = 0; ol < ml; ++ol )
	{
		if (SeparateLayers)
		{
			if (!object->layers[ol]->polygons.size())
				continue;
			else
				ogreMesh = new Mesh(ndest);
		}
		
		for (unsigned int s = 0; s < object->surfaces.size(); s++)
		{
			lwSurface *surface = object->surfaces[s];
			
			points.clear();
			polygons.clear();
			vmaps.clear();
			
			unsigned int l = ol;

			for( unsigned int il = 0; il < ml; ++il )
			{
				if (!SeparateLayers) l = il;

				copyPoints(s, ID_FACE, object->layers[l]->points, points);
				copyPolygons(s, ID_FACE, object->layers[l]->polygons, polygons);
				getTextureVMaps(surface->color.textures, object->layers[l]->vmaps, vmaps);

				if (SeparateLayers)	break;
			}

			if (!polygons.size()) continue;				
			
			SubMesh *ogreSubMesh = ogreMesh->createSubMesh();
			ogreSubMesh->numFaces = polygons.size();
			ogreSubMesh->faceVertexIndices = new unsigned short[ogreSubMesh->numFaces * 3];
			ogreSubMesh->setMaterialName(surface->name);
			
			ogreSubMesh->useSharedVertices = flags[UseSharedGeometry] && points.size() < POINTLIMIT;
			GeometryData* geometry = ogreSubMesh->useSharedVertices ? &ogreMesh->sharedGeometry : &ogreSubMesh->geometry;
			copyDataToGeometry(points, polygons, vmaps, ogreSubMesh, geometry, setupGeometry(geometry, points.size()));
		}
		
		String fname = SeparateLayers ? makeLayerFileName(dest, ol, object->layers[ol]->name) : dest;

		Skeleton *skeleton = 0;

		if (flags[ExportSkeleton])
			if (SeparateLayers)
				skeleton = doExportSkeleton(fname, ol);
			else
				if (!ol) skeleton = doExportSkeleton(fname, -1);
		
		try
		{
			meshserializer.exportMesh(ogreMesh, fname, flags[ExportMaterials]);
		}
		catch (...)
		{
			cout << "Could not export to file: " << fname << endl;
		}

		ogreMesh->unload();

		delete ogreMesh;
		if (flags[ExportSkeleton] && skeleton) delete skeleton;

		if (!SeparateLayers) break;
	}

	return true;
}
