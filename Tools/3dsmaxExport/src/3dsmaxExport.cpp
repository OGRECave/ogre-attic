
// 3dsmaxExport.cpp
//
// John Martin, 2002
//
// Entry point for Ogre 3dsmax exporter. Provides three maxscript functions
// to export static, keyframe animated, and skeletal animated meshes.

#include "3dsmaxExport.h"


// The maxscript "ogreExportStaticMesh" function.
Value* ogre_export_static_mesh_cf(Value** arg_list, int count)
{
	// init singleton ogre services
    Ogre::MaterialManager matMgr;
	Ogre::LogManager logMgr;

	// start a log file
	logMgr.createLog("3dsmaxOgreExporter.log");
	logMgr.logMessage("OGRE 3dsmax Exporter Log");

	Ogre::String msg;

	// quick sanity check
	check_arg_count(matoTest, 3, count);

	// create some names to access maxscript struct members
	Value* vertbufElementName = Name::intern("vertbuf");
	Value* facesElementName = Name::intern("faces");
	Value* materialElementName = Name::intern("material");
	Value* textureNameElementName = Name::intern("texture");
	Value* vertElementName = Name::intern("vert");
	Value* normalElementName = Name::intern("normal");
	Value* tvertElementName = Name::intern("tvert");
	Value* materialNameElementName = Name::intern("name");
	Value* ambientColorElementName = Name::intern("ambientColor");
	Value* diffuseColorElementName = Name::intern("diffuseColor");
	Value* specularColorElementName = Name::intern("specularColor");
	Value* selfIllumColorElementName = Name::intern("selfIllumColor");
	Value* shininessElementName = Name::intern("shininess");
	Value* diffuseMapElementName = Name::intern("diffuseMap");

	// grab the name
	char* meshName = arg_list[0]->to_string();

	// grab the output filename
	char* outputFilename = arg_list[2]->to_string();

	// grab the submesh array
	Array* subMeshs = static_cast<Array*>(arg_list[1]);

	// Note: at the moment we're throwing everything into seperate
	// submeshs. In time we'll make this configurable.

	Ogre::Mesh* ogreMesh = new Ogre::Mesh(meshName);

	logMgr.logMessage("Only exporting submeshes in this version.");
	msg = ""; msg << "Found " << subMeshs->size << " submeshs.";
	logMgr.logMessage(msg);

	// iterate through each submesh
	for (int i = 0; i < subMeshs->size; i++)
	{
		msg = ""; msg << "Exporting submesh " << i << ".";
		logMgr.logMessage(msg);

		// grab the subMesh
		Struct* subMesh = static_cast<Struct*>((*subMeshs)[i]);

		logMgr.logMessage("Setting up the submesh.");

		// grab each part of the subMesh struct
		int vertbufElementIndex = subMesh->definition->members->get(vertbufElementName)->to_int();
		int facesElementIndex = subMesh->definition->members->get(facesElementName)->to_int();
		int materialElementIndex = subMesh->definition->members->get(materialElementName)->to_int();

		Array* vertbuf = static_cast<Array*>(subMesh->member_data[vertbufElementIndex]);
		Array* faces = static_cast<Array*>(subMesh->member_data[facesElementIndex]);
		Struct* material = static_cast<Struct*>(subMesh->member_data[materialElementIndex]);

		Ogre::SubMesh* ogreSubMesh = ogreMesh->createSubMesh();

		// set up submesh geometry
		ogreSubMesh->geometry.numTexCoords = 1;
		ogreSubMesh->geometry.numTexCoordDimensions[0] = 2;
		ogreSubMesh->geometry.hasNormals = true;
		ogreSubMesh->geometry.hasColours = false;
		ogreSubMesh->geometry.vertexStride = 0;
		ogreSubMesh->geometry.texCoordStride[0]	= 0;
		ogreSubMesh->geometry.normalStride = 0;
		ogreSubMesh->useSharedVertices = false;
		ogreSubMesh->useTriStrips =	false;

		ogreSubMesh->geometry.numVertices =	vertbuf->size;
		ogreSubMesh->geometry.pVertices	= new Ogre::Real[ogreSubMesh->geometry.numVertices * 3];
		ogreSubMesh->geometry.pNormals = new Ogre::Real[ogreSubMesh->geometry.numVertices *	3];
		ogreSubMesh->geometry.pTexCoords[0]	= new Ogre::Real[ogreSubMesh->geometry.numVertices * 2];

		logMgr.logMessage("Copying vertex data.");

		// iterate over each vertex
		for (int j = 0; j < ogreSubMesh->geometry.numVertices; j++)
		{
			// dig the data out
			Struct* vertexInfo = static_cast<Struct*>((*vertbuf)[j]);

			int vertElementIndex = vertexInfo->definition->members->get(vertElementName)->to_int();
			int normalElementIndex = vertexInfo->definition->members->get(normalElementName)->to_int();
			int tvertElementIndex = vertexInfo->definition->members->get(tvertElementName)->to_int();

			Point3 vert = vertexInfo->member_data[vertElementIndex]->to_point3();
			Point3 norm = vertexInfo->member_data[normalElementIndex]->to_point3();
			Point3 tvert = vertexInfo->member_data[tvertElementIndex]->to_point3();

			// copy verts
			ogreSubMesh->geometry.pVertices[j * 3] = vert.x;
			ogreSubMesh->geometry.pVertices[(j * 3) + 1] = vert.y;
			ogreSubMesh->geometry.pVertices[(j * 3) + 2] = vert.z;

			// copy normals
			ogreSubMesh->geometry.pNormals[j * 3]= norm.x;
			ogreSubMesh->geometry.pNormals[(j * 3) + 1] = norm.y;
			ogreSubMesh->geometry.pNormals[(j * 3) + 2] = norm.z;

			// copy texture coords
			ogreSubMesh->geometry.pTexCoords[0][j * 2] = tvert.x;
			ogreSubMesh->geometry.pTexCoords[0][(j * 2) + 1] = 1 - tvert.y;
		}

		logMgr.logMessage("Copying face data.");

		// allocate some space for faces
		ogreSubMesh->numFaces = faces->size;
		ogreSubMesh->faceVertexIndices = new unsigned short[ogreSubMesh->numFaces * 3];

		// iterate through the faces
		for	(j = 0;	j <	ogreSubMesh->numFaces; j++)
		{
			Point3 face = (*faces)[j]->to_point3();

			ogreSubMesh->faceVertexIndices[(j * 3)] = static_cast<unsigned short>(face.x);
			ogreSubMesh->faceVertexIndices[(j * 3) + 1] = static_cast<unsigned short>(face.y);
			ogreSubMesh->faceVertexIndices[(j * 3) + 2] = static_cast<unsigned short>(face.z);
		}

		logMgr.logMessage("Setting up material.");

		// grab the indexes for material struct elements
		int materialNameElementIndex = material->definition->members->get(materialNameElementName)->to_int();
		int ambientColorElementIndex = material->definition->members->get(ambientColorElementName)->to_int();
		int diffuseColorElementIndex = material->definition->members->get(diffuseColorElementName)->to_int();
		int specularColorElementIndex = material->definition->members->get(specularColorElementName)->to_int();
		int selfIllumColorElementIndex = material->definition->members->get(selfIllumColorElementName)->to_int();
		int shininessElementIndex = material->definition->members->get(shininessElementName)->to_int();
		int diffuseMapElementIndex = material->definition->members->get(diffuseMapElementName)->to_int();

		char* materialName = material->member_data[materialNameElementIndex]->to_string();
		AColor ambientColor = material->member_data[ambientColorElementIndex]->to_acolor();
		AColor diffuseColor = material->member_data[diffuseColorElementIndex]->to_acolor();
		AColor specularColor = material->member_data[specularColorElementIndex]->to_acolor();
		AColor selfIllumColor = material->member_data[selfIllumColorElementIndex]->to_acolor();
		float shininess = material->member_data[shininessElementIndex]->to_float();
		char* diffuseMap = material->member_data[diffuseMapElementIndex]->to_string();

		// create the material and fill it up
		Ogre::Material* ogreMat = (Ogre::Material*)matMgr.createDeferred(materialName);
		
		logMgr.logMessage("Copying material properties.");

		ogreMat->setAmbient(ambientColor.r, ambientColor.g, ambientColor.b);
		ogreMat->setDiffuse(diffuseColor.r, diffuseColor.g, diffuseColor.b);
		ogreMat->setSpecular(specularColor.r, specularColor.g, specularColor.b);
		ogreMat->setSelfIllumination(selfIllumColor.r, selfIllumColor.g, selfIllumColor.b);
		ogreMat->setShininess(shininess);

		// set the diffuse texture map
		ogreMat->addTextureLayer(diffuseMap);

		msg = ""; msg << "Assigning material \"" << materialName << "\" to submesh.";
		logMgr.logMessage(msg);

		// assign the material to the submesh
		ogreSubMesh->setMaterialName(materialName);
	}

	logMgr.logMessage("Writing the mesh file.");

	Ogre::MeshSerializer serializer;
	serializer.exportMesh(ogreMesh, outputFilename, true);

	msg = ""; msg << "Export to " << outputFilename <<  " completed successfully.";

	// clean up and go home
	delete ogreMesh;
	return &true_value;
}

// The maxscript "ogreExportAnimMesh" function.
Value *ogre_export_anim_mesh_cf(Value **arg_list, int count)
{
	// Vertex tweening is not supported by Ogre.
	return &false_value;
}

// The maxscript "ogreExportSkelMesh" function.
Value *ogre_export_skel_mesh_cf(Value **arg_list, int count)
{
	// Not implemented yet.
	return &false_value;
}
