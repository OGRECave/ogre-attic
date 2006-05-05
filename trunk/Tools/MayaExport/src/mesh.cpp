#include "mesh.h"
#include <maya/MFnMatrixData.h>

namespace OgreMayaExporter
{
	/***** Class Mesh *****/
	// constructor
	Mesh::Mesh(const MString& name)
	{
		m_name = name;
		m_numTriangles = 0;
		m_pSkeleton = NULL;
		m_sharedGeom.vertices.clear();
		m_sharedGeom.dagMap.clear();
		m_vertexClips.clear();
	}

	// destructor
	Mesh::~Mesh()
	{
		clear();
	}

	// clear data
	void Mesh::clear()
	{
		m_name = "";
		m_numTriangles = 0;
		for (int i=0; i<m_submeshes.size(); i++)
			delete m_submeshes[i];
		m_sharedGeom.vertices.clear();
		m_sharedGeom.dagMap.clear();
		m_vertexClips.clear();
		m_uvsets.clear();
		m_submeshes.clear();
		if (m_pSkeleton)
			delete m_pSkeleton;
		m_pSkeleton = NULL;
	}

	// get pointer to linked skeleton
	Skeleton* Mesh::getSkeleton()
	{
		return m_pSkeleton;
	}

	/*******************************************************************************
	 *                    Load mesh data from a Maya node                          *
	 *******************************************************************************/
	MStatus Mesh::load(const MDagPath& meshDag,ParamList &params)
	{
		int i,j,k;
		MStatus stat;
		// Check that given DagPath corresponds to a mesh node
		if (!meshDag.hasFn(MFn::kMesh))
			return MS::kFailure;

		// Initialise temporary variables
		newvertices.clear();
		newweights.clear();
		newjointIds.clear();
		newuvsets.clear();
		newpoints.clear();
		newnormals.clear();
		newnumJoints = 0;
		opposite = false;
		shaders.clear();
		shaderPolygonMapping.clear();
		polygonSets.clear();
		pSkinCluster = NULL;

		// Get mesh uvsets
		stat = getUVSets(meshDag);		
		if (stat != MS::kSuccess)
		{
			std::cout << "Error retrieving uvsets for current mesh\n";
			std::cout.flush();
		}
		// Get linked skin cluster
		stat = getSkinCluster(meshDag,params);
		if (stat != MS::kSuccess)
		{
			std::cout << "Error retrieving skin cluster linked to current mesh\n";
			std::cout.flush();
		}
		// Get connected shaders
		stat = getShaders(meshDag);
		if (stat != MS::kSuccess)
		{
			std::cout << "Error getting shaders connected to current mesh\n";
			std::cout.flush();
		}
		// Get vertex data
		stat = getVertices(meshDag,params);
		if (stat != MS::kSuccess)
		{
			std::cout << "Error retrieving vertex data for current mesh\n";
			std::cout.flush();
		}
		// Get vertex bone weights
		if (pSkinCluster)
		{
			getVertexBoneWeights(meshDag);
			if (stat != MS::kSuccess)
			{
				std::cout << "Error retrieving veretex bone assignements for current mesh\n";
				std::cout.flush();
			}
		}
		// Get faces data
		stat = getFaces(meshDag,params);
		if (stat != MS::kSuccess)
		{
			std::cout << "Error retrieving faces data for current mesh\n";
			std::cout.flush();
		}
		// Build shared geometry
		if (params.useSharedGeom)
		{
			stat = buildSharedGeometry(meshDag,params);
			if (stat != MS::kSuccess)
			{
				std::cout << "Error building shared geometry for current mesh\n";
				std::cout.flush();
			}
		}
		// Create submeshes (a different submesh for every different shader linked to the mesh)
		stat = createSubmeshes(meshDag,params);
		if (stat != MS::kSuccess)
		{
			std::cout << "Error creating submeshes for current mesh\n";
			std::cout.flush();
		}

		// Free up memory
		newvertices.clear();
		newweights.clear();
		newjointIds.clear();
		newpoints.clear();
		newnormals.clear();
		newuvsets.clear();
		shaders.clear();
		shaderPolygonMapping.clear();
		polygonSets.clear();
		if (pSkinCluster)
			delete pSkinCluster;

		return MS::kSuccess;
	}


	/*******************************************************************************
	 *                    Load mesh animations from Maya                           *
	 *******************************************************************************/
	MStatus Mesh::loadAnims(ParamList& params)
	{
		MStatus stat;
		int i;
		// save current time for later restore
		MTime curTime = MAnimControl::currentTime();
		std::cout << "Loading vertex animations...\n";
		std::cout.flush();
		// clear animations data
		m_vertexClips.clear();
		// load the requested clips
		for (i=0; i<params.vertClipList.size(); i++)
		{
			stat = loadClip(params.vertClipList[i].name,params.vertClipList[i].start,
				params.vertClipList[i].stop,params.vertClipList[i].rate,params);
			if (stat == MS::kSuccess)
			{
				std::cout << "Clip successfully loaded\n";
				std::cout.flush();
			}
			else
			{
				std::cout << "Failed loading clip\n";
				std::cout.flush();
			}
		}
		//restore current time
		MAnimControl::setCurrentTime(curTime);
		return MS::kSuccess;
	}



/******************** Methods to parse geometry data from Maya ************************/
	// Get uvsets info from the maya mesh
	MStatus Mesh::getUVSets(const MDagPath& meshDag)
	{
		MFnMesh mesh(meshDag);
<<<<<<< mesh.cpp
		int i;
		MStatus stat;
		// Get uv texture coordinate sets' names
=======
		//initialise variables
		std::vector<MFloatArray> weights;
		std::vector<MIntArray> jointIds;
		unsigned int numJoints = 0;
		std::vector<vertexInfo> vertices;
		MPointArray points;
		MFloatVectorArray normals;
		vertices.resize(mesh.numVertices());
		weights.resize(mesh.numVertices());
		jointIds.resize(mesh.numVertices());
		// get uv texture coordinate sets' names
		MStringArray uvsets;
>>>>>>> 1.7
		if (mesh.numUVSets() > 0)
		{
			stat = mesh.getUVSetNames(newuvsets);
			if (MS::kSuccess != stat)
			{
				std::cout << "Error retrieving UV sets names\n";
				std::cout.flush();
				return MS::kFailure;
			}
		}
<<<<<<< mesh.cpp
		// Save uvsets info
		for (i=m_uvsets.size(); i<newuvsets.length(); i++)
=======
		//Save uvsets info
		for (i=m_uvsets.size(); i<uvsets.length(); i++)
>>>>>>> 1.7
		{
			uvset uv;
			uv.size = 2;
			m_uvsets.push_back(uv);
		}
		return MS::kSuccess;
	}


<<<<<<< mesh.cpp
	// Get skin cluster info from the maya mesh
	MStatus Mesh::getSkinCluster(const MDagPath &meshDag, ParamList &params)
	{
		MStatus stat;
		MFnMesh mesh(meshDag);
		pSkinCluster = NULL;
=======
		// load the connected skeleton (if present)
		MFnSkinCluster* pSkinCluster = NULL;
		MFnMesh* pInputMesh = NULL;
>>>>>>> 1.7
		if (params.exportVBA || params.exportSkeleton)
		{
			// get connected skin clusters (if present)
			MItDependencyNodes kDepNodeIt( MFn::kSkinClusterFilter );            
			for( ;!kDepNodeIt.isDone() && !pSkinCluster; kDepNodeIt.next()) 
			{            
				MObject kObject = kDepNodeIt.item();
<<<<<<< mesh.cpp
				pSkinCluster = new MFnSkinCluster(kObject);
				unsigned int uiNumGeometries = pSkinCluster->numOutputConnections();
				unsigned int uiGeometry;
				for(uiGeometry = 0; uiGeometry < uiNumGeometries; ++uiGeometry ) 
=======
				pSkinCluster = new MFnSkinCluster(kObject);
				unsigned int uiNumGeometries = pSkinCluster->numOutputConnections();
				unsigned int uiGeometry;
				for(uiGeometry = 0; uiGeometry < uiNumGeometries && !pInputMesh; ++uiGeometry ) 
>>>>>>> 1.7
				{
					unsigned int uiIndex = pSkinCluster->indexForOutputConnection(uiGeometry);
					MObject kOutputObject = pSkinCluster->outputShapeAtIndex(uiIndex);
					if(kOutputObject == mesh.object()) 
					{
						std::cout << "Found skin cluster " << pSkinCluster->name().asChar() << " for mesh " 
							<< mesh.name().asChar() << "\n"; 
						std::cout.flush();
					}	
					else
					{
						delete pSkinCluster;
						pSkinCluster = NULL;
					}
				}
			}
<<<<<<< mesh.cpp
			if (pSkinCluster)
=======

			if (pSkinCluster)
>>>>>>> 1.7
			{
				// load the skeleton
				std::cout << "Loading skeleton data...\n";
				std::cout.flush();
				if (!m_pSkeleton)
					m_pSkeleton = new Skeleton();
				stat = m_pSkeleton->load(pSkinCluster,params);
				if (MS::kSuccess != stat)
				{
					std::cout << "Error loading skeleton data\n";
					std::cout.flush();
				}
				else
				{
					std::cout << "OK\n";
					std::cout.flush();
				}
			}
		}
		return MS::kSuccess;
	}


	// Get connected shaders
	MStatus Mesh::getShaders(const MDagPath& meshDag)
	{
		MStatus stat;
		MFnMesh mesh(meshDag);
		stat = mesh.getConnectedShaders(0,shaders,shaderPolygonMapping);
		std::cout.flush();
		if (MS::kSuccess != stat)
		{
			std::cout << "Error getting connected shaders\n";
			std::cout.flush();
			return MS::kFailure;
		}
		std::cout << "Found " << shaders.length() << " connected shaders\n";
		std::cout.flush();
		if (shaders.length() <= 0)
		{
			std::cout << "No connected shaders, skipping mesh\n";
			std::cout.flush();
			return MS::kFailure;
		}
		// create a series of arrays of faces for each different submesh
		polygonSets.clear();
		polygonSets.resize(shaders.length());
		return MS::kSuccess;
	}


	// Get vertex data
	MStatus Mesh::getVertices(const MDagPath &meshDag, OgreMayaExporter::ParamList &params)
	{
		int i;
		MFnMesh mesh(meshDag);
		// prepare vertex table
<<<<<<< mesh.cpp
		newvertices.resize(mesh.numVertices());
		newweights.resize(mesh.numVertices());
		newjointIds.resize(mesh.numVertices());
		for (i=0; i<newvertices.size(); i++)
			newvertices[i].next = -2;
		//get vertex positions from mesh
		if (params.exportWorldCoords || (pSkinCluster && params.exportSkeleton))
			mesh.getPoints(newpoints,MSpace::kWorld);
=======
		for (i=0; i<vertices.size(); i++)
			vertices[i].next = -2;
		//get vertex positions from mesh
		if (params.exportWorldCoords || (pSkinCluster && params.exportSkeleton))
			mesh.getPoints(points,MSpace::kWorld);
>>>>>>> 1.7
		else
			mesh.getPoints(newpoints,MSpace::kTransform);
		//get list of normals from mesh data
		if (params.exportWorldCoords)
			mesh.getNormals(newnormals,MSpace::kWorld);
		else
<<<<<<< mesh.cpp
			mesh.getNormals(newnormals,MSpace::kTransform);
		//check the "opposite" attribute to see if we have to flip normals
		mesh.findPlug("opposite",true).getValue(opposite);
		return MS::kSuccess;
	}


	// Get vertex bone assignements
	MStatus Mesh::getVertexBoneWeights(const MDagPath& meshDag)
	{
		int i,j,k;
		MStatus stat;
		std::cout << "Get vbas\n";
		std::cout.flush();
		MItGeometry iterGeom(meshDag);
		for (i=0; !iterGeom.isDone(); iterGeom.next(), i++)
		{
			MObject component = iterGeom.component();
			MFloatArray vertexWeights;
			stat=pSkinCluster->getWeights(meshDag,component,vertexWeights,newnumJoints);
			//normalize vertex weights
			int widx;
			MFloatArray oldw = vertexWeights;
			// first, truncate the weights to given precision
			long weightSum = 0;
			for (widx=0; widx < vertexWeights.length(); widx++)
			{
				long w = (long) (((float)vertexWeights[widx]) / ((float)PRECISION));
				vertexWeights[widx] = w;
				weightSum += w;
			}
			// then divide by the sum of the weights to add up to 1 
			// (if there is at least one weight > 0)
			if (weightSum > 0)
			{
				float newSum = 0;
				for (widx=0; widx < vertexWeights.length(); widx++)
				{
					long w = (long) ((float)vertexWeights[widx] / ((float)PRECISION));
					w = (long) (((float)w) / ((float)weightSum));
					vertexWeights[widx] = (float) (((long)w) * ((float)PRECISION));
					newSum += vertexWeights[widx];
				}
				if (newSum < 1.0f)
					vertexWeights[vertexWeights.length()-1] += PRECISION;
			}
			// else set all weights to 0
			else
			{
				for (widx=0; widx < vertexWeights.length(); widx++)
					vertexWeights[widx] = 0;
			}
			// save the normalized weights
			newweights[i]=vertexWeights;
			if (MS::kSuccess != stat)
			{
				std::cout << "Error retrieving vertex weights\n";
				std::cout.flush();
			}
			// get ids for the joints
			if (m_pSkeleton)
			{
				MDagPathArray influenceObjs;
				pSkinCluster->influenceObjects(influenceObjs,&stat);
=======
			mesh.getNormals(normals,MSpace::kTransform);
		//get list of vertex weights
		if (pSkinCluster)
		{
			std::cout << "Get vbas\n";
			std::cout.flush();
			MItGeometry iterGeom(meshDag);
			std::ofstream out;
			out.open("C:\\test.txt");
			for (i=0; !iterGeom.isDone(); iterGeom.next(), i++)
			{
				MObject component = iterGeom.component();
				MFloatArray vertexWeights;
				stat=pSkinCluster->getWeights(meshDag,component,vertexWeights,numJoints);
				//normalize vertex weights
				int widx;
				MFloatArray oldw = vertexWeights;
				// first, truncate the weights to given precision
				long weightSum = 0;
				for (widx=0; widx < vertexWeights.length(); widx++)
				{
					long w = (long) (((float)vertexWeights[widx]) / ((float)PRECISION));
					vertexWeights[widx] = w;
					weightSum += w;
				}
				// then divide by the sum of the weights to add up to 1 
				// (if there is at least one weight > 0)
				if (weightSum > 0)
				{
					float newSum = 0;
					for (widx=0; widx < vertexWeights.length(); widx++)
					{
						long w = (long) ((float)vertexWeights[widx] / ((float)PRECISION));
						w = (long) (((float)w) / ((float)weightSum));
						vertexWeights[widx] = (float) (((long)w) * ((float)PRECISION));
						newSum += vertexWeights[widx];
					}
					if (newSum < 1.0f)
						vertexWeights[vertexWeights.length()-1] += PRECISION;
				}
				// else set all weights to 0
				else
				{
					for (widx=0; widx < vertexWeights.length(); widx++)
						vertexWeights[widx] = 0;
				}
				// save the normalized weights
				weights[i]=vertexWeights;
				for (widx=0; widx < vertexWeights.length(); widx++)
				{
					out << oldw[widx] << "\t" << vertexWeights[widx] << "\n";
				}
>>>>>>> 1.7
				if (MS::kSuccess != stat)
				{
<<<<<<< mesh.cpp
					std::cout << "Error retrieving influence objects for given skin cluster\n";
					std::cout.flush();
=======
					std::cout << "Error retrieving vertex weights\n";
					std::cout.flush();
>>>>>>> 1.7
				}
				newjointIds[i].setLength(newweights[i].length());
				for (j=0; j<influenceObjs.length(); j++)
				{
<<<<<<< mesh.cpp
					bool foundJoint = false;
					for (k=0; k<m_pSkeleton->getJoints().size() && !foundJoint; k++)
=======
					MDagPathArray influenceObjs;
					pSkinCluster->influenceObjects(influenceObjs,&stat);
					if (MS::kSuccess != stat)
>>>>>>> 1.7
					{
<<<<<<< mesh.cpp
						if (influenceObjs[j].partialPathName() == m_pSkeleton->getJoints()[k].name)
=======
						std::cout << "Error retrieving influence objects for given skin cluster\n";
						std::cout.flush();
					}
					jointIds[i].setLength(weights[i].length());
					for (j=0; j<influenceObjs.length(); j++)
					{
						bool foundJoint = false;
						for (k=0; k<m_pSkeleton->getJoints().size() && !foundJoint; k++)
>>>>>>> 1.7
						{
<<<<<<< mesh.cpp
							foundJoint=true;
							newjointIds[i][j] = m_pSkeleton->getJoints()[k].id;
=======
							if (influenceObjs[j].fullPathName() == m_pSkeleton->getJoints()[k].name)
							{
								foundJoint=true;
								jointIds[i][j] = m_pSkeleton->getJoints()[k].id;
							}
>>>>>>> 1.7
						}
					}
				}
			}
			out.close();
		}
		return MS::kSuccess;
	}


	// Get faces data
	MStatus Mesh::getFaces(const MDagPath &meshDag, ParamList &params)
	{
		int i,j,k;
		MStatus stat;
		MFnMesh mesh(meshDag);
		// create an iterator to go through mesh polygons
		if (mesh.numPolygons() > 0)
		{
			std::cout << "Iterate over mesh polygons\n";
			std::cout.flush();
			MItMeshPolygon faceIter(mesh.object(),&stat);
			if (MS::kSuccess != stat)
			{
				std::cout << "Error accessing mesh polygons\n";
				std::cout.flush();
				return MS::kFailure;
			}
			std::cout << "num polygons = " << mesh.numPolygons() << "\n";
			std::cout.flush();
			// iterate over mesh polygons
			for (; !faceIter.isDone(); faceIter.next())
			{
				int numTris=0;
				int iTris;
				bool different;
				int vtxIdx, nrmIdx;
				faceIter.numTriangles(numTris);
				// for every triangle composing current polygon extract triangle info
				for (iTris=0; iTris<numTris; iTris++)
				{
					MPointArray triPoints;
					MIntArray tempTriVertexIdx,triVertexIdx;
					int idx;
					// create a new face to store triangle info
					face newFace;
					// extract triangle vertex indices
					faceIter.getTriangle(iTris,triPoints,tempTriVertexIdx);
					// convert indices to face-relative indices
					MIntArray polyIndices;
					faceIter.getVertices(polyIndices);
					unsigned int iPoly, iObj;
					for (iObj=0; iObj < tempTriVertexIdx.length(); ++iObj)
					{
						// iPoly is face-relative vertex index
						for (iPoly=0; iPoly < polyIndices.length(); ++iPoly)
						{
							if (tempTriVertexIdx[iObj] == polyIndices[iPoly]) 
							{
								triVertexIdx.append(iPoly);
								break;
							}
						}
					}
					// iterate over triangle's vertices
					for (i=0; i<3; i++)
					{
						different = true;
						vtxIdx = faceIter.vertexIndex(triVertexIdx[i],&stat);
						if (stat != MS::kSuccess)
						{
							std::cout << "Could not access vertex position\n";
							std::cout.flush();
						}
						nrmIdx = faceIter.normalIndex(triVertexIdx[i],&stat);
						if (stat != MS::kSuccess)
						{
							std::cout << "Could not access vertex normal\n";
							std::cout.flush();
						}

						// get vertex color
						MColor color;
						if (faceIter.hasColor(triVertexIdx[i]))
						{
							stat = faceIter.getColor(color,triVertexIdx[i]);
							if (MS::kSuccess != stat)
							{
								color = MColor(1,1,1,1);
							}
							if (color.r > 1)
								color.r = 1;
							else if (color.r < PRECISION)
								color.r = 0;
							if (color.g > 1)
								color.g = 1;
							else if (color.g < PRECISION)
								color.g = 0;
							if (color.b > 1)
								color.b = 1;
							else if (color.b < PRECISION)
								color.b = 0;
							if (color.a > 1)
								color.a = 1;
							else if (color.a < PRECISION)
								color.a = 0;
						}
						else
						{
							color = MColor(1,1,1,1);
						}
						if (newvertices[vtxIdx].next == -2)	// first time we encounter a vertex in this position
						{
							// save vertex position
							newpoints[vtxIdx].cartesianize();
							newvertices[vtxIdx].pointIdx = vtxIdx;
							// save vertex normal
							newvertices[vtxIdx].normalIdx = nrmIdx;
							// save vertex colour
							newvertices[vtxIdx].r = color.r;
							newvertices[vtxIdx].g = color.g;
							newvertices[vtxIdx].b = color.b;
							newvertices[vtxIdx].a = color.a;
							// save vertex texture coordinates
							newvertices[vtxIdx].u.resize(newuvsets.length());
							newvertices[vtxIdx].v.resize(newuvsets.length());
							// save vbas
<<<<<<< mesh.cpp
							newvertices[vtxIdx].vba.resize(newweights[vtxIdx].length());
							for (j=0; j<newweights[vtxIdx].length(); j++)
=======
							vertices[vtxIdx].vba.resize(weights[vtxIdx].length());
							for (j=0; j<weights[vtxIdx].length(); j++)
>>>>>>> 1.7
							{
								newvertices[vtxIdx].vba[j] = (newweights[vtxIdx])[j];
							}
							// save joint ids
<<<<<<< mesh.cpp
							newvertices[vtxIdx].jointIds.resize(newjointIds[vtxIdx].length());
							for (j=0; j<newjointIds[vtxIdx].length(); j++)
=======
							vertices[vtxIdx].jointIds.resize(jointIds[vtxIdx].length());
							for (j=0; j<jointIds[vtxIdx].length(); j++)
>>>>>>> 1.7
							{
								newvertices[vtxIdx].jointIds[j] = (newjointIds[vtxIdx])[j];
							}
							// save uv sets data
<<<<<<< mesh.cpp
							for (j=0; j<newuvsets.length(); j++)
=======
							for (j=0; j<uvsets.length(); j++)
>>>>>>> 1.7
							{
								float2 uv;
								stat = faceIter.getUV(triVertexIdx[i],uv,&newuvsets[j]);
								if (MS::kSuccess != stat)
								{
									uv[0] = 0;
									uv[1] = 0;
								}
								newvertices[vtxIdx].u[j] = uv[0];
								newvertices[vtxIdx].v[j] = (-1)*(uv[1]-1);
							}
							// save vertex index in face info
							newFace.v[i] = m_sharedGeom.vertices.size() + vtxIdx;
							// update value of index to next vertex info (-1 means nothing next)
							newvertices[vtxIdx].next = -1;
						}
						else	// already found at least 1 vertex in this position
						{
							// check if a vertex with same attributes has been saved already
<<<<<<< mesh.cpp
							for (k=vtxIdx; k!=-1 && different; k=newvertices[k].next)
=======
							for (k=vtxIdx; k!=-1 && different; k=vertices[k].next)
>>>>>>> 1.7
							{
								different = false;

								if (params.exportVertNorm)
								{
									MFloatVector n1 = newnormals[newvertices[k].normalIdx];
									MFloatVector n2 = newnormals[nrmIdx];
									if (n1.x!=n2.x || n1.y!=n2.y || n1.z!=n2.z)
									{
										different = true;
									}
								}

								if ((params.exportVertCol) &&
									(newvertices[k].r!=color.r || newvertices[k].g!=color.g || newvertices[k].b!= color.b || newvertices[k].a!=color.a))
								{
									different = true;
								}

								if (params.exportTexCoord)
								{
<<<<<<< mesh.cpp
									for (j=0; j<newuvsets.length(); j++)
=======
									for (j=0; j<uvsets.length(); j++)
>>>>>>> 1.7
									{
										float2 uv;
										stat = faceIter.getUV(triVertexIdx[i],uv,&newuvsets[j]);
										if (MS::kSuccess != stat)
										{
											uv[0] = 0;
											uv[1] = 0;
										}
										uv[1] = (-1)*(uv[1]-1);
										if (newvertices[k].u[j]!=uv[0] || newvertices[k].v[j]!=uv[1])
										{
											different = true;
										}
									}
								}

								idx = k;
							}
							// if no identical vertex has been saved, then save the vertex info
							if (different)
							{
								vertexInfo vtx;
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
<<<<<<< mesh.cpp
								vtx.vba.resize(newweights[vtxIdx].length());
								for (j=0; j<newweights[vtxIdx].length(); j++)
=======
								vtx.vba.resize(weights[vtxIdx].length());
								for (j=0; j<weights[vtxIdx].length(); j++)
>>>>>>> 1.7
								{
									vtx.vba[j] = (newweights[vtxIdx])[j];
								}
								// save joint ids
								vtx.jointIds.resize(newjointIds[vtxIdx].length());
								for (j=0; j<newjointIds[vtxIdx].length(); j++)
								{
									vtx.jointIds[j] = (newjointIds[vtxIdx])[j];
								}
								// save vertex texture coordinates
								vtx.u.resize(newuvsets.length());
								vtx.v.resize(newuvsets.length());
								for (j=0; j<newuvsets.length(); j++)
								{
									float2 uv;
									stat = faceIter.getUV(triVertexIdx[i],uv,&newuvsets[j]);
									if (MS::kSuccess != stat)
									{
										uv[0] = 0;
										uv[1] = 0;
									}
									if (fabs(uv[0]) < PRECISION)
										uv[0] = 0;
									if (fabs(uv[1]) < PRECISION)
										uv[1] = 0;
									vtx.u[j] = uv[0];
									vtx.v[j] = (-1)*(uv[1]-1);
								}
								vtx.next = -1;
								newvertices.push_back(vtx);
								// save vertex index in face info
								newFace.v[i] = m_sharedGeom.vertices.size() + newvertices.size()-1;
								newvertices[idx].next = newvertices.size()-1;
							}
							else
							{
								newFace.v[i] = m_sharedGeom.vertices.size() + idx;
							}
						}
					} // end iteration of triangle vertices
					// add face info to the array corresponding to the submesh it belongs
					// skip faces with no shaders assigned
					if (shaderPolygonMapping[faceIter.index()] >= 0)
						polygonSets[shaderPolygonMapping[faceIter.index()]].push_back(newFace);
				} // end iteration of triangles
			}
		}
		std::cout << "done reading mesh triangles\n";
<<<<<<< mesh.cpp
		std::cout.flush();
		return MS::kSuccess;
	}

=======
		std::cout.flush();
>>>>>>> 1.7

	// Build shared geometry
	MStatus Mesh::buildSharedGeometry(const MDagPath &meshDag,ParamList& params)
	{
		int i,k;
		std::cout << "Create list of shared vertices\n";
		std::cout.flush();
		// save a new entry in the shared geometry map: we associate the index of the first 
		// vertex we're loading with the dag path from which it has been read
		dagInfo di;
		di.offset = m_sharedGeom.vertices.size();
		di.dagPath = meshDag;
		// load shared vertices
		for (i=0; i<newvertices.size(); i++)
		{
<<<<<<< mesh.cpp
			vertex v;
			vertexInfo vInfo = newvertices[i];
			// save vertex coordinates (rescale to desired length unit)
			MPoint point = newpoints[vInfo.pointIdx] * params.lum;
			if (fabs(point.x) < PRECISION)
				point.x = 0;
			if (fabs(point.y) < PRECISION)
				point.y = 0;
			if (fabs(point.z) < PRECISION)
				point.z = 0;
			v.x = point.x;
			v.y = point.y;
			v.z = point.z;
			// save vertex normal
			MFloatVector normal = newnormals[vInfo.normalIdx];
			if (fabs(normal.x) < PRECISION)
				normal.x = 0;
			if (fabs(normal.y) < PRECISION)
				normal.y = 0;
			if (fabs(normal.z) < PRECISION)
				normal.z = 0;
			if (opposite)
			{
				v.n.x = -normal.x;
				v.n.y = -normal.y;
				v.n.z = -normal.z;
			}
			else
=======
			std::cout << "Create list of shared vertices\n";
			std::cout.flush();
			for (i=0; i<vertices.size(); i++)
>>>>>>> 1.7
			{
<<<<<<< mesh.cpp
				v.n.x = normal.x;
				v.n.y = normal.y;
				v.n.z = normal.z;
			}
			v.n.normalize();
			// save vertex color
			v.r = vInfo.r;
			v.g = vInfo.g;
			v.b = vInfo.b;
			v.a = vInfo.a;
			// save vertex bone assignements
			for (k=0; k<vInfo.vba.size(); k++)
			{
				vba newVba;
				newVba.jointIdx = vInfo.jointIds[k];
				newVba.weight = vInfo.vba[k];
				v.vbas.push_back(newVba);
			}
			// save texture coordinates
			for (k=0; k<vInfo.u.size(); k++)
			{
				texcoords newTexCoords;
				newTexCoords.u = vInfo.u[k];
				newTexCoords.v = vInfo.v[k];
				newTexCoords.w = 0;
				v.texcoords.push_back(newTexCoords);
=======
				vertex v;
				vertexInfo vInfo = vertices[i];
				// save vertex coordinates (rescale to desired length unit)
				MPoint point = points[vInfo.pointIdx] * params.lum;
				if (fabs(point.x) < PRECISION)
					point.x = 0;
				if (fabs(point.y) < PRECISION)
					point.y = 0;
				if (fabs(point.z) < PRECISION)
					point.z = 0;
				v.x = point.x * params.lum;
				v.y = point.y * params.lum;
				v.z = point.z * params.lum;
				// save vertex normal
				MFloatVector normal = normals[vInfo.normalIdx];
				if (fabs(normal.x) < PRECISION)
					normal.x = 0;
				if (fabs(normal.y) < PRECISION)
					normal.y = 0;
				if (fabs(normal.z) < PRECISION)
					normal.z = 0;
				if (opposite)
				{
					v.n.x = -normal.x;
					v.n.y = -normal.y;
					v.n.z = -normal.z;
				}
				else
				{
					v.n.x = normal.x;
					v.n.y = normal.y;
					v.n.z = normal.z;
				}
				v.n.normalize();
				// save vertex color
				v.r = vInfo.r;
				v.g = vInfo.g;
				v.b = vInfo.b;
				v.a = vInfo.a;
				// save vertex bone assignements
				for (k=0; k<vInfo.vba.size(); k++)
				{
					vba newVba;
					newVba.jointIdx = vInfo.jointIds[k];
					newVba.weight = vInfo.vba[k];
					v.vbas.push_back(newVba);
				}
				// save texture coordinates
				for (k=0; k<vInfo.u.size(); k++)
				{
					texcoords newTexCoords;
					newTexCoords.u = vInfo.u[k];
					newTexCoords.v = vInfo.v[k];
					newTexCoords.w = 0;
					v.texcoords.push_back(newTexCoords);
				}
				// add newly created vertex to vertices list
				m_vertices.push_back(v);
>>>>>>> 1.7
			}
<<<<<<< mesh.cpp
			// save vertex index in maya mesh, to retrieve future positions of the same vertex
			v.index = vInfo.pointIdx;
			// add newly created vertex to vertices list
			m_sharedGeom.vertices.push_back(v);
=======
			std::cout << "done creating vertices list\n";
			std::cout.flush();
>>>>>>> 1.7
		}
		// save number of vertices referring to this mesh dag in the dag path map
		di.numVertices = m_sharedGeom.vertices.size() - di.offset;
		m_sharedGeom.dagMap.push_back(di);
		std::cout << "done creating vertices list\n";
		std::cout.flush();
		return MS::kSuccess;
	}


	// Create submeshes
	MStatus Mesh::createSubmeshes(const MDagPath& meshDag,ParamList& params)
	{
		int i;
		MStatus stat;
		MFnMesh mesh(meshDag);
		for (i=0; i<shaders.length(); i++)
		{
			// check if the submesh has at least 1 triangle
			if (polygonSets[i].size() > 0)
			{
				//create new submesh
				Submesh* pSubmesh = new Submesh();

				//load linked shader
				stat = pSubmesh->loadMaterial(shaders[i],newuvsets,params);
				if (stat != MS::kSuccess)
				{
					MFnDependencyNode shadingGroup(shaders[i]);
					std::cout << "Error loading submesh linked to shader " << shadingGroup.name().asChar() << "\n";
					std::cout.flush();
					return MS::kFailure;
				}

				//load vertex and face data
				stat = pSubmesh->load(meshDag,polygonSets[i],newvertices,newpoints,newnormals,newuvsets,params,opposite);

				//add submesh to current mesh
				m_submeshes.push_back(pSubmesh);
				//update number of triangles composing the mesh
				m_numTriangles += pSubmesh->numTriangles();
			}
		}
<<<<<<< mesh.cpp
		return MS::kSuccess;
	}
=======
		
		if (pSkinCluster)
			delete pSkinCluster;
		if (pInputMesh)
			delete pInputMesh;
>>>>>>> 1.7





/******************** Methods to read vertex animations from Maya ************************/
	//load a vertex animation clip
	MStatus Mesh::loadClip(MString& clipName,float start,float stop,float rate,ParamList& params)
	{
		MStatus stat;
		MString msg;
		std::vector<float> times;
		// display clip name
		std::cout << "clip \"" << clipName.asChar() << "\"\n";
		std::cout.flush();
		// calculate times from clip sample rate
		times.clear();
		for (float t=start; t<stop; t+=rate)
			times.push_back(t);
		times.push_back(stop);
		// get animation length
		float length=0;
		if (times.size() >= 0)
			length = times[times.size()-1] - times[0];
		if (length < 0)
		{
			std::cout << "invalid time range for the clip, we skip it\n";
			std::cout.flush();
			return MS::kFailure;
		}
		// create a new animation
		Animation a;
		a.m_name = clipName;
		a.m_length = length;
		a.m_tracks.clear();
		// if we're using shared geometry, create a single animation track for the whole mesh
		if (params.useSharedGeom)
		{
			// load the animation track
			stat = loadMeshTrack(a,times,params);
			if (stat != MS::kSuccess)
			{
				std::cout << "Error loading mesh vertex animation\n";
				std::cout.flush();
			}
		}
		// else creae a different animation track for each submesh
		else
		{
			// load all tracks (one for each submesh)
			stat = loadSubmeshTracks(a,times,params);
			if (stat != MS::kSuccess)
			{
				std::cout << "Error loading submeshes vertex animation\n";
				std::cout.flush();
				return MS::kFailure;
			}
		}
		// add newly created animation to animations list
		m_vertexClips.push_back(a);
		// display info
		std::cout << "length: " << a.m_length << "\n";
		std::cout << "num keyframes: " << a.m_tracks[0].m_vertexKeyframes.size() << "\n";
		std::cout.flush();
		// clip successfully loaded
		return MS::kSuccess;
	}


	//load an animation track for the whole mesh (using shared geometry)
	MStatus Mesh::loadMeshTrack(Animation& a,std::vector<float> &times, OgreMayaExporter::ParamList &params)
	{
		int i;
		MStatus stat;
		// create a new track
		Track t;
		t.m_type = TT_MORPH;
		t.m_target = T_MESH;
		t.m_vertexKeyframes.clear();
		// get keyframes at given times
		for (i=0; i<times.size(); i++)
		{
			//set time to wanted sample time
			MAnimControl::setCurrentTime(MTime(times[i],MTime::kSeconds));
			//load a keyframe for the mesh at current time
			stat = loadKeyframe(t,times[i]-times[0],params);
			if (stat != MS::kSuccess)
			{
				std::cout << "Error reading animation keyframe at time: " << times[i] << "\n";
				std::cout.flush();
			}
		}
		// add track to given animation
		a.addTrack(t);
		// track sucessfully loaded
		return MS::kSuccess;
	}


	//load all submesh animation tracks (one for each submesh)
	MStatus Mesh::loadSubmeshTracks(Animation& a,std::vector<float> &times, OgreMayaExporter::ParamList &params)
	{
		int i,j;
		MStatus stat;
		// create a new track for each submesh
		std::vector<Track> tracks;
		for (i=0; i<m_submeshes.size(); i++)
		{
			Track t;
			t.m_type = TT_MORPH;
			t.m_target = T_SUBMESH;
			t.m_index = i;
			t.m_vertexKeyframes.clear();
			tracks.push_back(t);
		}
		// get keyframes at given times
		for (i=0; i<times.size(); i++)
		{
			//set time to wanted sample time
			MAnimControl::setCurrentTime(MTime(times[i],MTime::kSeconds));
			//load a keyframe for each submesh at current time
			for (j=0; j<m_submeshes.size(); j++)
			{
				stat = m_submeshes[j]->loadKeyframe(tracks[j],times[i]-times[0],params);
				if (stat != MS::kSuccess)
				{
					std::cout << "Error reading animation keyframe at time: " << times[i] << " for submesh: " << j << "\n";
					std::cout.flush();
				}
			}
		}
		// add tracks to given animation
		for (i=0; i< tracks.size(); i++)
			a.addTrack(tracks[i]);
		// track sucessfully loaded
		return MS::kSuccess;
		return MS::kSuccess;
	}


	// Load a keyframe for the whole mesh
	MStatus Mesh::loadKeyframe(Track& t,float time,ParamList& params)
	{
		int i,j;
		// create a new keyframe
		vertexKeyframe k;
		// set keyframe time
		k.time = time;
		for (i=0; i<m_sharedGeom.dagMap.size(); i++)
		{
			// get the mesh Fn
			dagInfo di = m_sharedGeom.dagMap[i];
			MFnMesh mesh(di.dagPath);
			// get vertex positions
			MFloatPointArray points;
			if (params.exportWorldCoords)
				mesh.getPoints(points,MSpace::kWorld);
			else
				mesh.getPoints(points,MSpace::kObject);
			// calculate vertex offsets
			for (j=0; j<di.numVertices; j++)
			{
				vertexPosition pos;
				vertex v = m_sharedGeom.vertices[di.offset+j];
				pos.x = points[v.index].x;
				pos.y = points[v.index].y;
				pos.z = points[v.index].z;
				if (fabs(pos.x) < PRECISION)
					pos.x = 0;
				if (fabs(pos.y) < PRECISION)
					pos.y = 0;
				if (fabs(pos.z) < PRECISION)
					pos.z = 0;
				k.positions.push_back(pos);
			}
		}
		// add keyframe to given track
		t.addVertexKeyframe(k);
		// keyframe successfully loaded
		return MS::kSuccess;
	}

/*********************************** Write mesh data to XML file **************************************/
	// Write mesh data to Ogre XML
	MStatus Mesh::writeXML(ParamList &params)
	{
<<<<<<< mesh.cpp
		int i,j,k,w;
=======
		int i,j;
>>>>>>> 1.7
		MStatus stat;
		// start mesh description
		params.outMesh << "<mesh>\n";
		// write shared geometry (if used)
		if (params.useSharedGeom)
		{
			params.outMesh << "\t<sharedgeometry vertexcount=\"" << m_sharedGeom.vertices.size() << "\">\n";
			params.outMesh << "\t\t<vertexbuffer positions=\"true\" normals=";
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
				params.outMesh << m_uvsets.size() << "\">\n";
			else
				params.outMesh << 0 << "\">\n";
			// write vertex data
<<<<<<< mesh.cpp
			for (i=0; i < m_sharedGeom.vertices.size(); i++)
=======
			for (i=0; i < m_vertices.size(); i++)
>>>>>>> 1.7
			{
				params.outMesh << "\t\t\t<vertex>\n";
				//write vertex position
				params.outMesh << "\t\t\t\t<position x=\"" << m_sharedGeom.vertices[i].x << "\" y=\"" 
					<< m_sharedGeom.vertices[i].y << "\" " << "z=\"" << m_sharedGeom.vertices[i].z << "\"/>\n";
				//write vertex normal
				if (params.exportVertNorm)
				{
					params.outMesh << "\t\t\t\t<normal x=\"" << m_sharedGeom.vertices[i].n.x << "\" y=\"" 
						<< m_sharedGeom.vertices[i].n.y << "\" " << "z=\"" << m_sharedGeom.vertices[i].n.z << "\"/>\n";
				}
				//write vertex colour
				if (params.exportVertCol)
				{
					float r,g,b,a;
					if (params.exportVertColWhite)
					{
						r = g = b = a = 1.0f;
					}
					else
					{
						r = m_sharedGeom.vertices[i].r;
						g = m_sharedGeom.vertices[i].g;
						b = m_sharedGeom.vertices[i].b;
						a = m_sharedGeom.vertices[i].a;
					}

					params.outMesh << "\t\t\t\t<colour_diffuse value=\"" << r << " " << g 
						<< " " << b << " " << a << "\"/>\n";
				}//write vertex texture coordinates
				if (params.exportTexCoord)
				{
					for (j=0; j<m_uvsets.size(); j++)
					{
						if (j < m_sharedGeom.vertices[i].texcoords.size())
						{
							params.outMesh << "\t\t\t\t<texcoord u=\"" << m_sharedGeom.vertices[i].texcoords[j].u << "\" v=\"" << 
								m_sharedGeom.vertices[i].texcoords[j].v << "\"/>\n";
						}
						else
						{
							params.outMesh << "\t\t\t\t<texcoord u=\"0\" v=\"0\"/>\n";
						}
					}
				}
				params.outMesh << "\t\t\t</vertex>\n";
			}
			params.outMesh << "\t\t</vertexbuffer>\n";
			params.outMesh << "\t</sharedgeometry>\n";
		}
		// write submeshes data
		params.outMesh << "\t<submeshes>\n";
		for (i=0; i < m_submeshes.size(); i++)
		{
			stat = m_submeshes[i]->writeXML(params);
			if (MS::kSuccess != stat)
			{
				std::cout << "Error writing submesh " << m_submeshes[i]->name().asChar() << ", aborting operation\n";
				return MS::kFailure;
			}
		}
		params.outMesh << "\t</submeshes>\n";
		// write skeleton link
		if (params.exportSkeleton && m_pSkeleton)
		{
			int ri = params.skeletonFilename.rindex('\\');
			int end = params.skeletonFilename.length() - 1;
			MString filename = params.skeletonFilename.substring(ri+1,end);
			if (filename.substring(filename.length()-4,filename.length()-1) == MString(".xml")
				&& filename.length() >= 5)
				filename = filename.substring(0,filename.length()-5);
			params.outMesh << "\t<skeletonlink name=\"" <<  filename.asChar() << "\"/>\n";
		}
		// Write shared geometry bone assignments
		if (params.useSharedGeom && params.exportVBA)
		{
			params.outMesh << "\t<boneassignments>\n";
<<<<<<< mesh.cpp
			for (i=0; i<m_sharedGeom.vertices.size(); i++)
=======
			for (i=0; i<m_vertices.size(); i++)
>>>>>>> 1.7
			{
<<<<<<< mesh.cpp
				for (j=0; j<m_sharedGeom.vertices[i].vbas.size(); j++)
=======
				for (j=0; j<m_vertices[i].vbas.size(); j++)
>>>>>>> 1.7
				{
<<<<<<< mesh.cpp
					if (m_sharedGeom.vertices[i].vbas[j].weight >= PRECISION)
=======
					if (m_vertices[i].vbas[j].weight >= PRECISION)
>>>>>>> 1.7
					{
						params.outMesh << "\t\t<vertexboneassignment vertexindex=\"" << i 
							<< "\" boneindex=\"" << m_sharedGeom.vertices[i].vbas[j].jointIdx << "\" weight=\"" 
							<< m_sharedGeom.vertices[i].vbas[j].weight <<"\"/>\n";
					}
				}
			}
			params.outMesh << "\t</boneassignments>\n";
		}
		// write submesh names
		params.outMesh << "\t<submeshnames>\n";
		for (i=0; i<m_submeshes.size(); i++)
		{
			if (m_submeshes[i]->name() != "")
				params.outMesh << "\t\t<submeshname name=\"" << m_submeshes[i]->name().asChar() << "\" index=\"" << i << "\"/>\n";
		}
		params.outMesh << "\t</submeshnames>\n";
		// write mesh vertex animations
		params.outMesh << "\t<animations>\n";
		for (i=0; i<m_vertexClips.size(); i++)
		{
			// write animation info
			params.outMesh << "\t\t<animation name=\"" << m_vertexClips[i].m_name.asChar() << "\" length=\"" << 
				m_vertexClips[i].m_length << "\">\n";
			// write tracks
			params.outMesh << "\t\t\t<tracks>\n";
			// cycle through tracks
			for (j=0; j<m_vertexClips[i].m_tracks.size(); j++)
			{
				Track t = m_vertexClips[i].m_tracks[j];
				// write track target
				params.outMesh << "\t\t\t\t<track target=\"";
				if (t.m_target == T_MESH)
					params.outMesh << "mesh\" ";
				else if (t.m_target == T_SUBMESH)
					params.outMesh << "submesh\" index=\"" << t.m_index << "\" ";
				//write track type
				params.outMesh << "type=\"";
				if (t.m_type == TT_MORPH)
					params.outMesh << "morph";
				else if (t.m_type == TT_POSE)
					params.outMesh << "pose";
				params.outMesh << "\">\n";
				// write track keyframes
				params.outMesh << "\t\t\t\t\t<keyframes>\n";
				for (k=0; k<t.m_vertexKeyframes.size(); k++)
				{
					// time
					params.outMesh << "\t\t\t\t\t\t<keyframe time=\"" << t.m_vertexKeyframes[k].time << "\">\n";

					// positions
					for (w=0; w<t.m_vertexKeyframes[k].positions.size(); w++)
					{
						params.outMesh << "\t\t\t\t\t\t\t<position x=\"" << t.m_vertexKeyframes[k].positions[w].x << "\" y=\"" <<
							t.m_vertexKeyframes[k].positions[w].y << "\" z=\"" << t.m_vertexKeyframes[k].positions[w].z << "\"/>\n";
					}
					params.outMesh << "\t\t\t\t\t\t</keyframe>\n";
				}
				params.outMesh << "\t\t\t\t\t</keyframes>\n";
				params.outMesh << "\t\t\t\t</track>\n";
			}
			// end tracks description
			params.outMesh << "\t\t\t</tracks>\n";
			// end animation description
			params.outMesh << "\t\t</animation>\n";
		}
		params.outMesh << "\t</animations>\n";
		// end mesh description
		params.outMesh << "</mesh>\n";

		return MS::kSuccess;
	}




}; //end of namespace
