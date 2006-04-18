#include "submesh.h"

namespace OgreMayaExporter
{
	/***** Class Submesh *****/
	// constructor
	Submesh::Submesh(const MString& name)
	{
		clear();
		m_name = name;
	}

	// destructor
	Submesh::~Submesh()
	{
		clear();
	}

	// clear data
	void Submesh::clear()
	{
		m_name = "";
		m_numTriangles = 0;
		m_pMaterial = NULL;
		m_vertices.clear();
		m_faces.clear();
		m_uvsets.clear();
		m_use32bitIndexes = false;
	}

	// return number of triangles composing the mesh
	long Submesh::numTriangles()
	{
		return m_numTriangles;
	}

	// return number of vertices composing the mesh
	long Submesh::numVertices()
	{
		return m_numVertices;
	}

	// return submesh name
	MString& Submesh::name()
	{
		return m_name;
	}

/***** load data *****/
	MStatus Submesh::loadMaterial(MObject& shader,MStringArray& uvsets,ParamList& params)
	{
		int i;
		MPlug plug;
		MPlugArray srcplugarray;
		bool foundShader = false;
		MStatus stat;
		MFnLambertShader* pShader;
		//get shader from shading group
		MFnDependencyNode shadingGroup(shader);
		plug = shadingGroup.findPlug("surfaceShader");
		plug.connectedTo(srcplugarray,true,false,&stat);
		for (i=0; i<srcplugarray.length() && !foundShader; i++)
		{
			if (srcplugarray[i].node().hasFn(MFn::kLambert))
			{
				pShader = new MFnLambertShader(srcplugarray[i].node());
				foundShader = true;
			}
		}
		if (foundShader)
		{
			std::cout << "Found material: " << pShader->name().asChar() << "\n";
			std::cout.flush();
			//check if this material has already been created
			//fix material name, adding the requested prefix
			MString tmpStr = params.matPrefix;
			if (tmpStr != "")
				tmpStr += "/";
			tmpStr += pShader->name();
			MStringArray tmpStrArray;
			tmpStr.split(':',tmpStrArray);
			MString name = "";
			for (i=0; i<tmpStrArray.length(); i++)
			{
				name += tmpStrArray[i];
				if (i < tmpStrArray.length()-1)
					name += "_";
			}
			Material* pMaterial = MaterialSet::getSingleton().getMaterial(name);
			//if the material has already been created, update the pointer
			if (pMaterial)
				m_pMaterial = pMaterial;
			//else create it and add it to the material set
			else
			{
				pMaterial = new Material();
				pMaterial->load(pShader,uvsets,params);
				m_pMaterial = pMaterial;
				MaterialSet::getSingleton().addMaterial(pMaterial);
			}
			//delete temporary shader
			delete pShader;
		}
		else
		{
			std::cout << "Unsupported material, replacing with default lambert\n";
			std::cout.flush();
			m_pMaterial = MaterialSet::getSingleton().getDefaultMaterial();
		}
		
		//loading complete
		return MS::kSuccess;
	}

	MStatus Submesh::load(std::vector<face>& faces, std::vector<vertexInfo>& vertInfo, MPointArray& points, 
		MFloatVectorArray& normals, MStringArray& texcoordsets,ParamList& params,bool opposite)
	{
		int i,j,k;
		std::cout << "Loading submesh associated to material: " << m_pMaterial->name().asChar() << "...";
		std::cout.flush();
		//save uvsets info
		for (i=m_uvsets.size(); i<texcoordsets.length(); i++)
		{
			uvset uv;
			uv.size = 2;
			m_uvsets.push_back(uv);
		}
		//iterate over faces array, to retrieve vertices info
		for (i=0; i<faces.size(); i++)
		{
			face newFace;
			// if we are using shared geometry, indexes refer to the vertex buffer of the whole mesh
			if (params.useSharedGeom)
			{
				if(opposite)
				{	// reverse order of face vertices for correct culling
					newFace.v[0] = faces[i].v[2];
					newFace.v[1] = faces[i].v[1];
					newFace.v[2] = faces[i].v[0];
				}
				else
				{
					newFace.v[0] = faces[i].v[0];
					newFace.v[1] = faces[i].v[1];
					newFace.v[2] = faces[i].v[2];
				}
			}
			// otherwise we create a vertex buffer for this submesh
			else
			{	// faces are triangles, so retrieve index of the three vertices
				for (j=0; j<3; j++)
				{
					vertex v;
					vertexInfo vInfo = vertInfo[faces[i].v[j]];
					// save vertex coordinates (rescale to desired length unit)
					MPoint point = points[vInfo.pointIdx];
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
					// add newly created vertex to vertex list
					m_vertices.push_back(v);
					if (opposite)	// reverse order of face vertices to get correct culling
						newFace.v[2-j] = m_vertices.size() - 1;
					else
						newFace.v[j] = m_vertices.size() - 1;
				}
			}
			m_faces.push_back(newFace);
		}
		// set use32bitIndexes flag
		if (params.useSharedGeom || (m_vertices.size() > 65535) || (m_faces.size() > 65535))
			m_use32bitIndexes = true;
		else
			m_use32bitIndexes = false;
		std::cout << "DONE\n";
		std::cout.flush();
		return MS::kSuccess;
	}

/***** write data *****/
	MStatus Submesh::writeXML(ParamList &params)
	{
		int i;
		// Start submesh description
		params.outMesh << "\t\t<submesh ";
		// Write material name
		params.outMesh << "material=\"" << m_pMaterial->name().asChar() << "\" ";
		// Write use32bitIndexes flag
		params.outMesh << "use32bitindexes=\"";
		if (m_use32bitIndexes)
			params.outMesh << "true";
		else
			params.outMesh << "false";
		params.outMesh << "\" ";
		// Write use32bitIndexes flag
		params.outMesh << "usesharedvertices=\"";
		if (params.useSharedGeom)
			params.outMesh << "true";
		else
			params.outMesh << "false";
		params.outMesh << "\" ";
		// Write operation type flag
		params.outMesh << "operationtype=\"triangle_list\">\n";

		// Write submesh polygons
		params.outMesh << "\t\t\t<faces count=\"" << m_faces.size() << "\">\n";
		for (i=0; i<m_faces.size(); i++)
		{
			params.outMesh << "\t\t\t\t<face v1=\"" << m_faces[i].v[0] << "\" v2=\"" << m_faces[i].v[1] << "\" "
				<< "v3=\"" << m_faces[i].v[2] << "\"/>\n";
		}
		params.outMesh << "\t\t\t</faces>\n";

		// Write mesh geometry
		if (!params.useSharedGeom)
		{
			params.outMesh << "\t\t\t<geometry vertexcount=\"" << m_vertices.size() << "\">\n";
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
				params.outMesh << m_uvsets.size() << "\">\n";
			else
				params.outMesh << 0 << "\">\n";
			//write vertex data
			for (i=0; i<m_vertices.size(); i++)
			{
				params.outMesh << "\t\t\t\t\t<vertex>\n";
				//write vertex position
				params.outMesh << "\t\t\t\t\t\t<position x=\"" << m_vertices[i].x << "\" y=\"" 
					<< m_vertices[i].y << "\" " << "z=\"" << m_vertices[i].z << "\"/>\n";
				//write vertex normal
				if (params.exportVertNorm)
				{
					params.outMesh << "\t\t\t\t\t\t<normal x=\"" << m_vertices[i].n.x << "\" y=\"" 
						<< m_vertices[i].n.y << "\" " << "z=\"" << m_vertices[i].n.z << "\"/>\n";
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
						r = m_vertices[i].r;
						g = m_vertices[i].g;
						b = m_vertices[i].b;
						a = m_vertices[i].a;
					}

					params.outMesh << "\t\t\t\t\t\t<colour_diffuse value=\"" << r << " " << g 
						<< " " << b << " " << a << "\"/>\n";
				}//write vertex texture coordinates
				if (params.exportTexCoord)
				{
					for (int j=0; j<m_uvsets.size(); j++)
					{
						params.outMesh << "\t\t\t\t\t\t<texcoord u=\"" << m_vertices[i].texcoords[j].u << "\" v=\"" << 
							m_vertices[i].texcoords[j].v << "\"/>\n";
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
				for (i=0; i<m_vertices.size(); i++)
				{
					for (int j=0; j<m_vertices[i].vbas.size(); j++)
					{
						if (m_vertices[i].vbas[j].weight > 0.001)
						{
							params.outMesh << "\t\t\t\t<vertexboneassignment vertexindex=\"" << i 
								<< "\" boneindex=\"" << m_vertices[i].vbas[j].jointIdx << "\" weight=\"" 
								<< m_vertices[i].vbas[j].weight <<"\"/>\n";
						}
					}
				}
				params.outMesh << "\t\t\t</boneassignments>\n";
			}
		}
		// End submesh description
		params.outMesh << "\t\t</submesh>\n";

		return MS::kSuccess;
	}


}; //end of namespace
