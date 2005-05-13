
#include "transferMesh.h"
#include <maya/MRenderUtil.h>
#include <process.h>

/***** Class TransferMesh *****/
// constructor
TransferMesh::TransferMesh()
{
	mName = "";
	mLoaded = false;
	bool mUse32bitIndexes = false;
	pMaterial = NULL;
	pSkinCluster = NULL;
	uvsets.clear();
	faces.clear();
	points.clear();
	vertices.clear();
	normals.clear();
}

// destructor
TransferMesh::~TransferMesh()
{
	if (pMaterial)
		delete pMaterial;
	if (pSkinCluster)
		delete pSkinCluster;
}

// load mesh data from a maya Fn
MStatus TransferMesh::load(MDagPath& meshDag,ParamList &params,TransferSkeleton* pSkeleton)
{
	MPlug plug;
	MPlugArray srcplugarray;
	MStatus stat;	
	std::vector<MFloatArray> weights;
	std::vector<MIntArray> jointIds;
	unsigned int numJoints;
	bool different;
	bool foundMaterial = false;
	bool foundSkinCluster = false;
	MVector normal;
	MColor color;
	int vtxIdx;
	int nrmIdx;
	int idx;
	int numFaces;
	int i,j,k;

	// GATHER MESH DATA
	MFnMesh mesh(meshDag);

	// Get mesh name
	mName = mesh.partialPathName();

	// Get mesh material
	// 1st: get connected shading groups
	MObjectArray shaders;
	MIntArray indices;
	stat = mesh.getConnectedShaders(0,shaders,indices);
	if (MS::kSuccess != stat)
	{
		MGlobal::displayInfo("Error getting connected shaders");
		return MS::kFailure;
	}
	// 2nd: get material connected to desired shading group
	MFnDependencyNode shadingGroup(shaders[0]);
	plug = shadingGroup.findPlug("surfaceShader");
	plug.connectedTo(srcplugarray,true,false,&stat);
	for (i=0; i<srcplugarray.length() && !foundMaterial; i++)
	{
		if (srcplugarray[i].node().hasFn(MFn::kLambert))
		{
			pMaterial = new MFnLambertShader(srcplugarray[i].node());
			foundMaterial = true;
		}
	}
	MString msg = "Found material: ";
	msg += pMaterial->name();
	MGlobal::displayInfo(msg);
	
	// Get uv texture coordinate sets' names
	uvsets.clear();
	if (mesh.numUVSets() > 0)
	{
		stat = mesh.getUVSetNames(uvsets);
		if (MS::kSuccess != stat)
		{
			MGlobal::displayInfo("Error retrieving UV sets names\n");
			return MS::kFailure;
		}
	}

	// Get number of polygons
	numFaces = mesh.numPolygons(&stat);
	if (MS::kSuccess != stat)
	{
		MGlobal::displayInfo("Error retrieving number of polygons\n");
		return MS::kFailure;
	}
	
	// Get faces data
	//initialise variables
	faces.resize(numFaces);
	vertices.resize(mesh.numVertices());
	weights.resize(mesh.numVertices());
	jointIds.resize(mesh.numVertices());
	//set flag for use of 32 bit indexes
	if (numFaces < 65000 && vertices.size() < 65000)
		mUse32bitIndexes = false;
	else
		mUse32bitIndexes = true;
	// prepare vertex table
	for (i=0; i<vertices.size(); i++)
		vertices[i].next = -2;
	//get vertex positions from mesh data
	if (params.exportWorldCoords)
		mesh.getPoints(points,MSpace::kWorld);
	else
		mesh.getPoints(points,MSpace::kTransform);
	//get list of normals from mesh data
	if (params.exportWorldCoords)
		mesh.getNormals(normals,MSpace::kWorld);
	else
		mesh.getNormals(normals,MSpace::kTransform);
	//get list of vertex weights
	//1st: get the associated skin cluster (if present)
	plug = mesh.findPlug("inMesh");
	plug.connectedTo(srcplugarray,true,false,&stat);
	for (i=0; i<srcplugarray.length() && !foundSkinCluster; i++)
	{
		if (srcplugarray[i].node().hasFn(MFn::kSkinClusterFilter))
		{
			pSkinCluster = new MFnSkinCluster(srcplugarray[i].node());
			foundSkinCluster = true;
		}
	}
	//2nd get the weights
	if (foundSkinCluster && pSkinCluster)
	{
		msg = "Found skin cluster: ";
		msg += pSkinCluster->name();
		MGlobal::displayInfo(msg);
		MItGeometry iterGeom(meshDag);
		for (i=0; !iterGeom.isDone(); iterGeom.next(), i++)
		{
			MObject component = iterGeom.component();
			MFloatArray vertexWeights;
			stat=pSkinCluster->getWeights(meshDag,component,vertexWeights,numJoints);
			weights[i]=vertexWeights;
			if (MS::kSuccess != stat)
			{
				MGlobal::displayInfo("Error retrieving vba");
			}
			// get ids for the joints
			if (pSkeleton)
			{
				MDagPathArray influenceObjs;
				pSkinCluster->influenceObjects(influenceObjs,&stat);
				jointIds[i].setLength(weights[i].length());
				for (j=0; j<influenceObjs.length(); j++)
				{
					bool foundJoint = false;
					for (k=0; k<pSkeleton->joints.size() && !foundJoint; k++)
					{
						if (influenceObjs[j].partialPathName() == pSkeleton->joints[k].name)
						{
							foundJoint=true;
							jointIds[i][j] = pSkeleton->joints[k].id;
						}
					}
				}
			}
		}
	}
	// create an iterator to go through mesh polygons
	MItMeshPolygon faceIter(mesh.object(),&stat);
	if (MS::kSuccess != stat)
	{
		MGlobal::displayInfo("Error accessing mesh polygons");
		return MS::kFailure;
	}
	// iterate over mesh polygons (we suppose the mesh has been triangulated)
	for (; !faceIter.isDone(); faceIter.next())
	{
		for (i=0; i<3; i++)
		{
			different = true;
			vtxIdx = faceIter.vertexIndex(i);
			nrmIdx = faceIter.normalIndex(i);
			
			// get vertex colour
			stat = faceIter.getColor(color,i);
			if (MS::kSuccess != stat)
			{
				MGlobal::displayInfo("Error retrieving vertex colour");
				return MS::kFailure;
			}
			
			if (vertices[vtxIdx].next == -2)	// first time we encounter a vertex in this position
			{
				// save vertex position
				points[vtxIdx].cartesianize();
				vertices[vtxIdx].pointIdx = vtxIdx;
				// save vertex normal
				vertices[vtxIdx].normalIdx = nrmIdx;
				// save vertex colour
				vertices[vtxIdx].r = color.r;
				vertices[vtxIdx].g = color.g;
				vertices[vtxIdx].b = color.b;
				vertices[vtxIdx].a = color.a;
				// save vertex texture coordinates
				vertices[vtxIdx].u.resize(uvsets.length());
				vertices[vtxIdx].v.resize(uvsets.length());
				// save vbas
				vertices[vtxIdx].vba.resize(weights[vtxIdx].length());
				for (int j=0; j<weights[vtxIdx].length(); j++)
				{
					vertices[vtxIdx].vba[j] = (weights[vtxIdx])[j];
				}
				// save joint ids
				vertices[vtxIdx].jointIds.resize(jointIds[vtxIdx].length());
				for (j=0; j<jointIds[vtxIdx].length(); j++)
				{
					vertices[vtxIdx].jointIds[j] = (jointIds[vtxIdx])[j];
				}
				// save uv sets data
				for (j=0; j<uvsets.length(); j++)
				{
					float2 uv;
					stat = faceIter.getUV(i,uv,&uvsets[j]);
					if (MS::kSuccess != stat)
					{
						msg = "Warning, no uv assigned to vertex for uvset ";
						msg += uvsets[j];
						MGlobal::displayInfo(msg);
						uv[0] = 0;
						uv[1] = 0;
					}
					vertices[vtxIdx].u[j] = uv[0];
					vertices[vtxIdx].v[j] = (-1)*(uv[1]-1);
				}
				// save vertex index in face info
				faces[faceIter.index()].v[i] = vtxIdx;
				// update value of index to next vertex info (-1 means nothing next)
				vertices[vtxIdx].next = -1;
			}
			else	// already found at least 1 vertex in this position
			{
				// check if a vertex with same attributes has been saved already
				for (k=vtxIdx; k!=-1 && different; k=vertices[k].next)
				{
					different = false;

					if (params.exportVertNorm)
					{
						MFloatVector n1 = normals[vertices[k].normalIdx];
						MFloatVector n2 = normals[nrmIdx];
						if (n1.x!=n2.x || n1.y!=n2.y || n1.z!=n2.z)
							different = true;
					}
					
					if ((params.exportVertCol) &&
						(vertices[k].r!=color.r || vertices[k].g!=color.g || vertices[k].b!= color.b || vertices[k].a!=color.a))
					{
						different = true;
					}
					
					if (params.exportTexCoord)
					{
						for (int j=0; j<uvsets.length(); j++)
						{
							float2 uv;
							stat = faceIter.getUV(i,uv,&uvsets[j]);
							if (MS::kSuccess != stat)
							{
								uv[0] = 0;
								uv[1] = 0;
							}
							uv[1] = (-1)*(uv[1]-1);
							if (vertices[k].u[j]!=uv[0] || vertices[k].v[j]!=uv[1])
								different = true;
						}
					}

					idx = k;
				}
				// if no identical vertex has been saved, then save the vertex info
				if (different)
				{
					vertex vtx;
					// save vertex position
					vtx.pointIdx = vtxIdx;
					// save vertex normal
					vtx.normalIdx = nrmIdx;
					// save vertex colour
					vtx.r = color.r;
					vtx.g = color.g;
					vtx.b = color.b;
					vtx.a = color.a;
					// save vertex vba
					vtx.vba.resize(weights[vtxIdx].length());
					for (j=0; j<weights[vtxIdx].length(); j++)
					{
						vtx.vba[j] = (weights[vtxIdx])[j];
					}
					// save joint ids
					vtx.jointIds.resize(jointIds[vtxIdx].length());
					for (j=0; j<jointIds[vtxIdx].length(); j++)
					{
						vtx.jointIds[j] = (jointIds[vtxIdx])[j];
					}
					// save vertex texture coordinates
					vtx.u.resize(uvsets.length());
					vtx.v.resize(uvsets.length());
					for (j=0; j<uvsets.length(); j++)
					{
						float2 uv;
						stat = faceIter.getUV(i,uv,&uvsets[j]);
						if (MS::kSuccess != stat)
						{
							msg = "Warning, no uv assigned to vertex for uvset ";
							msg += uvsets[j];
							MGlobal::displayInfo(msg);
							uv[0] = 0;
							uv[1] = 0;
						}
						vtx.u[j] = uv[0];
						vtx.v[j] = (-1)*(uv[1]-1);
					}
					vtx.next = -1;
					vertices.push_back(vtx);
					// save vertex index in face info
					faces[faceIter.index()].v[i] = vertices.size()-1;
					vertices[idx].next = vertices.size()-1;
				}
				else
				{
					faces[faceIter.index()].v[i] = idx;
				}
			}			
		}
	}
	return MS::kSuccess;
}


// write submesh description to xml file
MStatus TransferMesh::writeToXML(ParamList& params)
{
	MStatus stat;
	if (params.exportMesh)
	{
		stat = writeMesh(params);
		if(MS::kSuccess != stat)
		{
			MGlobal::displayInfo("Error writing mesh info");
			return MS::kFailure;
		}
	}
	if (params.exportMaterial)
	{
		stat = writeMaterial(params);
		if(MS::kSuccess != stat)
		{
			MGlobal::displayInfo("Error writing mesh info");
			return MS::kFailure;
		}
	}
	return MS::kSuccess;
}

// write mesh description
MStatus TransferMesh::writeMesh(ParamList &params)
{
	int i,j;
	// Get material name
	MString matPrefix;
	MGlobal::executeCommand("textField -query -text ExportMaterialPrefix",matPrefix);
	MString matName = matPrefix;
	if (matName != "")
		matName += "/";
	matName += pMaterial->name();
	// Start mesh description
	params.outMesh << "\t\t<submesh ";
	params.outMesh << "material=\"" << matName.asChar() << "\" ";
	if (!mUse32bitIndexes)
		params.outMesh << "usesharedvertices=\"false\" use32bitindexes=\"false\" operationtype=\"triangle_list\">\n";
	else
		params.outMesh << "usesharedvertices=\"false\" use32bitindexes=\"true\" operationtype=\"triangle_list\">\n";
	
	// Write mesh polygons
	params.outMesh << "\t\t\t<faces count=\"" << faces.size() << "\">\n";
	for (i=0; i<faces.size(); i++)
	{
		params.outMesh << "\t\t\t\t<face v1=\"" << faces[i].v[0] << "\" v2=\"" << faces[i].v[1] << "\" "
			<< "v3=\"" << faces[i].v[2] << "\"/>\n";
	}
	params.outMesh << "\t\t\t</faces>\n";
	
	// Write mesh geometry
	params.outMesh << "\t\t\t<geometry vertexcount=\"" << vertices.size() << "\">\n";
	params.outMesh << "\t\t\t\t<vertexbuffer positions=\"true\" normals=";
	if (params.exportVertNorm)
		params.outMesh << "\"true\"";
	else 
		params.outMesh << "\"false\"";
	params.outMesh << " colours_diffuse=";
	if (params.exportVertCol)
		params.outMesh << "\"true\"";
	else
		params.outMesh << "\"false\"";
	params.outMesh << " colours_specular=\"false\" texture_coords=\"";
	if (params.exportTexCoord)
		params.outMesh << uvsets.length() << "\">\n";
	else
		params.outMesh << 0 << "\">\n";
	//write vertex data
	for (i=0; i<vertices.size(); i++)
	{
		params.outMesh << "\t\t\t\t\t<vertex>\n";
		params.outMesh << "\t\t\t\t\t\t<position x=\"" << points[vertices[i].pointIdx].x << "\" y=\"" 
			<< points[vertices[i].pointIdx].y << "\" " << "z=\"" << points[vertices[i].pointIdx].z << "\"/>\n";
		if (params.exportVertNorm)
		{
			params.outMesh << "\t\t\t\t\t\t<normal x=\"" << normals[vertices[i].normalIdx].x << "\" y=\"" 
				<< normals[vertices[i].normalIdx].y << "\" " << "z=\"" << normals[vertices[i].normalIdx].z << "\"/>\n";
		}
		if (params.exportVertCol)
		{
			float r,g,b,a;
			if (params.exportVertColWhite)
			{
				r = g = b = a = 1.0f;
			}
			else
			{
				r = vertices[i].r;
				g = vertices[i].g;
				b = vertices[i].b;
				a = vertices[i].a;
			}
			
			params.outMesh << "\t\t\t\t\t\t<colour_diffuse value=\"" << r << " " << g 
				<< " " << b << " " << a << "\"/>\n";
		}
		if (params.exportTexCoord)
		{
			for (int j=0; j<uvsets.length(); j++)
			{
				params.outMesh << "\t\t\t\t\t\t<texcoord u=\"" << vertices[i].u[j] << "\" v=\"" << 
					vertices[i].v[j] << "\"/>\n";
			}
		}
		params.outMesh << "\t\t\t\t\t</vertex>\n";
	}
	//end vertex data
	params.outMesh << "\t\t\t\t</vertexbuffer>\n";
	//end geometry description
	params.outMesh << "\t\t\t</geometry>\n";

	// Write bone assignments
	if (params.exportVBA)
	{
		params.outMesh << "\t\t\t<boneassignments>\n";
		for (i=0; i<vertices.size(); i++)
		{
			for (j=0; j<vertices[i].vba.size(); j++)
			{
				if (vertices[i].vba[j] > 0.001)
				{
					params.outMesh << "\t\t\t\t<vertexboneassignment vertexindex=\"" << i 
						<< "\" boneindex=\"" << vertices[i].jointIds[j] << "\" weight=\"" << vertices[i].vba[j] <<"\"/>\n";
				}
			}
		}
		params.outMesh << "\t\t\t</boneassignments>\n";
	}
	
	// End mesh description
	params.outMesh << "\t\t</submesh>\n";
	
	return MS::kSuccess;
}

// write material description
MStatus TransferMesh::writeMaterial(ParamList &params)
{
	MStatus stat;
	// Check if material has already been written (no duplicate materals allowed)
	bool written = false;
	for (int i=0; i<params.writtenMaterials.length(); i++)
	{
		if (params.writtenMaterials[i] == pMaterial->name().asChar())
			written = true;
	}
	if (!written)
	{
		MaterialType type = MT_LAMBERT;
		MFnPhongShader* pPhong;
		MFnBlinnShader* pBlinn;
		bool hasTexture = false;
		bool multiTextured = false;
		bool isTransparent = false;
		int lightingOff = 0;
		MPlugArray colorSrcPlugs;
		MPlugArray texSrcPlugs;
		MPlugArray placetexSrcPlugs;
		MColor ambientColor;
		MColor diffuseColor;
		MColor specularColor;
		MColor emissiveColor;

		// GET MATERIAL DATA

		// Check material type
		if (pMaterial->object().hasFn(MFn::kPhong))
			type = MT_PHONG;
		else if (pMaterial->object().hasFn(MFn::kBlinn))
			type = MT_BLINN;
				
		// Check if material is textured
		pMaterial->findPlug("color").connectedTo(colorSrcPlugs,true,false);
		for (int i=0; i<colorSrcPlugs.length(); i++)
		{
			if (colorSrcPlugs[i].node().hasFn(MFn::kFileTexture))
			{
				hasTexture = true;
				continue;
			}
			else if (colorSrcPlugs[i].node().hasFn(MFn::kLayeredTexture))
			{
				hasTexture = true;
				multiTextured = true;
				continue;
			}
		}
		
		// Check if material is transparent
		if (pMaterial->findPlug("transparency").isConnected() || pMaterial->transparency().r>0.0f)
			isTransparent = true;
		
		// Get material colours
		//diffuse colour
		if (hasTexture)
			diffuseColor = MColor(1.0,1.0,1.0,1.0);
		else
		{
			diffuseColor = pMaterial->color();
			diffuseColor.a = 1.0 - pMaterial->transparency().r;
		}
		//ambient colour
		ambientColor = diffuseColor;
		//emissive colour
		emissiveColor = pMaterial->incandescence();
		//specular colour
		switch(type)
		{
		case MT_PHONG:
			pPhong = new MFnPhongShader(pMaterial->object());
			specularColor = pPhong->specularColor();
			specularColor.a = pPhong->cosPower();
			delete pPhong;
			break;
		case MT_BLINN:
			pBlinn = new MFnBlinnShader(pMaterial->object());
			specularColor = pBlinn->specularColor();
			specularColor.a = 20.0;
			delete pBlinn;
			break;
		default:
			specularColor = MColor(0.0,0.0,0.0,0.0);
		}
		// Check if we have to export with lighting off option
		MGlobal::executeCommand("checkBox -query -value MatLightingOff",lightingOff);
		// Start material description
		MString matPrefix;
		MGlobal::executeCommand("textField -query -text ExportMaterialPrefix",matPrefix);
		MString matName = matPrefix;
		if (matName != "")
			matName += "/";
		matName += pMaterial->name();
		params.outMaterial << "material " << matName.asChar() << "\n";
		params.outMaterial << "{\n";
		params.outMaterial << "\ttechnique\n";
		params.outMaterial << "\t{\n";
		
		// Start rendering pass description
		params.outMaterial << "\t\tpass\n";
		params.outMaterial << "\t\t{\n";
		//set lighting off option if requested
		if (lightingOff)
			params.outMaterial << "\t\t\tlighting off\n\n";
		//ambient colour
		params.outMaterial << "\t\t\tambient " << ambientColor.r << " " << ambientColor.g << " " << ambientColor.b
			<< " " << ambientColor.a << "\n";
		//diffuse colour
		params.outMaterial << "\t\t\tdiffuse " << diffuseColor.r << " " << diffuseColor.g << " " << diffuseColor.b
			<< " " << diffuseColor.a << "\n";
		//specular colour
		params.outMaterial << "\t\t\tspecular " << specularColor.r << " " << specularColor.g << " " << specularColor.b
			<< " " << specularColor.a << "\n";
		//emissive colour
		params.outMaterial << "\t\t\temissive " << emissiveColor.r << " " << emissiveColor.g << " " 
			<< emissiveColor.b << "\n";
		//if material is transparent set blend mode and turn off depth_writing
		if (isTransparent)
		{
			params.outMaterial << "\n\t\t\tscene_blend alpha_blend\n";
			params.outMaterial << "\t\t\tdepth_write off\n";
		}
		
		// Write texture units
		if (hasTexture)
		{
			// Translate multiple textures if material is multitextured
			if (multiTextured)
			{
				// Get layered texture node
				MFnDependencyNode* pLayeredTexNode;
				pMaterial->findPlug("color").connectedTo(colorSrcPlugs,true,false);
				for (i=0; i<colorSrcPlugs.length(); i++)
				{
					if (colorSrcPlugs[i].node().hasFn(MFn::kLayeredTexture))
					{
						pLayeredTexNode = new MFnDependencyNode(colorSrcPlugs[i].node());
						continue;
					}
				}
				
				// Get inputs to layered texture
				MPlug inputsPlug = pLayeredTexNode->findPlug("inputs");
				
				// Scan inputs and export textures
				for (i=inputsPlug.numElements()-1; i>=0; i--)
				{
					MFnDependencyNode* pTextureNode = NULL;
					TexOpType texOp;
					// Search for a connected texture
					inputsPlug[i].child(0).connectedTo(colorSrcPlugs,true,false);
					for (int j=0; j<colorSrcPlugs.length(); j++)
					{
						if (colorSrcPlugs[j].node().hasFn(MFn::kFileTexture))
						{
							pTextureNode = new MFnDependencyNode(colorSrcPlugs[j].node());
							continue;
						}
					}
					
					// Get blend mode
					short bm;
					inputsPlug[i].child(2).getValue(bm);
					switch(bm)
					{
					/*				
					case 0:
						texOp = TOT_REPLACE;
						break;
					case 1:
						texOp = TOT_ALPHABLEND;
						break;
						*/				
					case 4:
						texOp = TOT_ADD;
						break;
					case 6:
						texOp = TOT_MODULATE;
						break;
					default:
						texOp = TOT_MODULATE;
					}
					
					// Translate the texture if it was found
					if (pTextureNode)
					{
						stat = writeTexture(*pTextureNode,texOp,params);
						delete pTextureNode;
						if (MS::kSuccess != stat)
						{
							MGlobal::displayInfo("Error writing layered texture");
							delete pLayeredTexNode;
							return MS::kFailure;
						}
					}
				}
				
				if (pLayeredTexNode)
					delete pLayeredTexNode;
			}
			// Else translate the single texture
			else
			{
				// Get texture node
				MFnDependencyNode* pTextureNode = NULL;
				pMaterial->findPlug("color").connectedTo(colorSrcPlugs,true,false);
				for (i=0; i<colorSrcPlugs.length(); i++)
				{
					if (colorSrcPlugs[i].node().hasFn(MFn::kFileTexture))
					{
						pTextureNode = new MFnDependencyNode(colorSrcPlugs[i].node());
						continue;
					}
				}
				if (pTextureNode)
				{
					stat = writeTexture(*pTextureNode,TOT_MODULATE,params);
					delete pTextureNode;
					if (MS::kSuccess != stat)
					{
						MGlobal::displayInfo("Error writing texture");
						return MS::kFailure;
					}
				}
			}
		}
		
		// End rendering pass description
		params.outMaterial << "\t\t}\n";
		
		// End material description
		params.outMaterial << "\t}\n";
		params.outMaterial << "}\n\n";
		
		// Append material name to list of written materials
		params.writtenMaterials.append(pMaterial->name());
	}
	
	return MS::kSuccess;
}


// write a single texture description
MStatus TransferMesh::writeTexture(MFnDependencyNode& tex,TexOpType texOp,ParamList &params)
{
	 MString absFilename,filename,outputDir,command;
	 int uvcoord = 0;
	 int i,j,k;

	 // Get texture filename
//	 tex.findPlug("fileTextureName").getValue(filename);
	 MRenderUtil::exactFileTextureName(tex.object(),absFilename);
 	 filename = absFilename.substring(absFilename.rindex('/')+1,absFilename.length()-1);
	 command = "toNativePath(\"";
	 command += absFilename;
	 command += "\")";
	 MGlobal::executeCommand(command,absFilename);
	 // Check if we want to copy textures
	 int copyTex = 0;
	 MGlobal::executeCommand("checkBox -query -value CopyTextures",copyTex);
	 if (copyTex)
	 {
		 // Get output dir
		 MGlobal::executeCommand("textField -query -text OutputDirectory",outputDir);
		 if (outputDir == "")
			 outputDir = ".";
		 // Copy file texture to output dir
		 command = "copy \"";
		 command += absFilename;
		 command += "\" \"";
		 command += outputDir;
		 command += "\"";
		 system(command.asChar());
	 }
	 // Get connections to uvCoord attribute of texture node
	 MPlugArray texSrcPlugs;
	 tex.findPlug("uvCoord").connectedTo(texSrcPlugs,true,false);
	 // Get place2dtexture node
	 MFnDependencyNode* pPlace2dTexNode = NULL;
	 for (j=0; j<texSrcPlugs.length(); j++)
	 {
		 if (texSrcPlugs[j].node().hasFn(MFn::kPlace2dTexture))
		 {
			 pPlace2dTexNode = new MFnDependencyNode(texSrcPlugs[j].node());
			 continue;
		 }
	 }
	 // Get uvChooser node
	 MFnDependencyNode* pUvChooserNode = NULL;
	 if (pPlace2dTexNode)
	 {
		 MPlugArray placetexSrcPlugs;
		 pPlace2dTexNode->findPlug("uvCoord").connectedTo(placetexSrcPlugs,true,false);
		 for (j=0; j<placetexSrcPlugs.length(); j++)
		 {
			 if (placetexSrcPlugs[j].node().hasFn(MFn::kUvChooser))
			 {
				 pUvChooserNode = new MFnDependencyNode(placetexSrcPlugs[j].node());
				 continue;
			 }
		 }
	 }
	 // Get uvset index
	 if (pUvChooserNode)
	 {
		 bool foundMesh = false;
		 bool foundUvset = false;
		 MPlug uvsetsPlug = pUvChooserNode->findPlug("uvSets");
		 MPlugArray uvsetsSrcPlugs;
		 for (i=0; i<uvsetsPlug.evaluateNumElements() && !foundMesh; i++)
		 {
			 uvsetsPlug[i].connectedTo(uvsetsSrcPlugs,true,false);
			 for (j=0; j<uvsetsSrcPlugs.length() && !foundMesh; j++)
			 {
				 if (uvsetsSrcPlugs[j].node().hasFn(MFn::kMesh))
				 {
					 MFnMesh tempMesh(uvsetsSrcPlugs[j].node());
					 if (tempMesh.partialPathName() == mName)
					 {
						 MString uvsetname;
						 uvsetsSrcPlugs[j].getValue(uvsetname);
						 for (k=0; k<uvsets.length() && !foundUvset; k++)
						 {
							 if (uvsets[k] == uvsetname)
							 {
								 uvcoord = k;
								 foundUvset = true;
							 }
						 }
						 foundMesh = true;
					 }
				 }
			 }
		 }
	 }
	 
	 // Start texture description
	 params.outMaterial << "\n\t\t\ttexture_unit\n";
	 params.outMaterial << "\t\t\t{\n";
	 params.outMaterial << "\t\t\t\ttexture " << filename.asChar() << "\n";
	 params.outMaterial << "\t\t\t\ttex_coord_set " << uvcoord << "\n";
	 switch (texOp)
	 {
	 case TOT_MODULATE:
		 params.outMaterial << "\t\t\t\tcolour_op modulate\n";
		 break;
	 case TOT_REPLACE:
		 params.outMaterial << "\t\t\t\tcolour_op replace\n";
		 break;
	 case TOT_ADD:
		 params.outMaterial << "\t\t\t\tcolour_op add\n";
		 break;
	 case TOT_ALPHABLEND:
		 params.outMaterial << "\t\t\t\tcolour_op alpha_blend\n";
		 break;
	 default:
		 params.outMaterial << "\t\t\t\tcolour_op replace\n";
	 }

	 // End texture description
	 params.outMaterial << "\t\t\t}\n";

	 // Free memory
	 if (pUvChooserNode)
		 delete pUvChooserNode;
	 if (pPlace2dTexNode)
		 delete pPlace2dTexNode;

	return MS::kSuccess;
}