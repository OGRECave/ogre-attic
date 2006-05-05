#ifndef _MESH_H
#define _MESH_H

#include "submesh.h"
#include "skeleton.h"
#include "mayaExportLayer.h"

namespace OgreMayaExporter
{
	/***** structures to store shared geometry *****/
	typedef struct dagInfotag
	{
		long offset;
		long numVertices;
		MDagPath dagPath;
	} dagInfo;

	typedef struct sharedGeometrytag
	{
		std::vector<vertex> vertices;
		std::vector<dagInfo> dagMap;
	} sharedGeometry;



	/***** Class Mesh *****/
	class Mesh
	{
	public:
		//constructor
		Mesh(const MString& name = "");
		//destructor
		~Mesh();
		//clear data
		void clear();
		//get pointer to linked skeleton
		Skeleton* getSkeleton();
		//load mesh data from a maya Fn
		MStatus load(const MDagPath& meshDag,ParamList &params);
		//load vertex animations
		MStatus loadAnims(ParamList &params);
		//write mesh data to maya XML
		MStatus writeXML(ParamList &params);	

	protected:
		//get uvsets info from the maya mesh
		MStatus getUVSets(const MDagPath& meshDag);
		//get skin cluster info from the maya mesh
		MStatus getSkinCluster(const MDagPath& meshDag,ParamList& params); 
		//get connected shaders
		MStatus getShaders(const MDagPath& meshDag);
		//get vertex data
		MStatus getVertices(const MDagPath& meshDag,ParamList& params);
		//get vertex bone assignements
		MStatus getVertexBoneWeights(const MDagPath& meshDag);
		//get faces data
		MStatus getFaces(const MDagPath& meshDag,ParamList& params);
		//build shared geometry
		MStatus buildSharedGeometry(const MDagPath& meshDag,ParamList& params);
		//create submeshes
		MStatus createSubmeshes(const MDagPath& meshDag,ParamList& params);
		//load a vertex animation clip
		MStatus loadClip(MString& clipName,float start,float stop,float rate,ParamList& params);
		//load a vertex animation track for the whole mesh
		MStatus loadMeshTrack(Animation& a,std::vector<float>& times,ParamList& params);
		//load all submesh animation tracks (one for each submesh)
		MStatus loadSubmeshTracks(Animation& a,std::vector<float>& times,ParamList& params);
		//load a keyframe for the whole mesh
		MStatus loadKeyframe(Track& t,float time,ParamList& params);

		//internal members
		MString m_name;
		long m_numTriangles;
		std::vector<uvset> m_uvsets;
		std::vector<Submesh*> m_submeshes;
		Skeleton* m_pSkeleton;
		sharedGeometry m_sharedGeom;
		std::vector<Animation> m_vertexClips;
		//temporary members (existing only during translation from maya mesh)
		std::vector<vertexInfo> newvertices;
		std::vector<MFloatArray> newweights;
		std::vector<MIntArray> newjointIds;
		MPointArray newpoints;
		MFloatVectorArray newnormals;
		MStringArray newuvsets;
		unsigned int newnumJoints;
		MFnSkinCluster* pSkinCluster;
		MObjectArray shaders;
		MIntArray shaderPolygonMapping;
		std::vector<faceArray> polygonSets;
		bool opposite;
	};

}; // end of namespace

#endif