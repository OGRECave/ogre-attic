/*
===============================================================================
This source file is part of the Ogre-Maya Tools.
Distributed as part of Ogre (Object-oriented Graphics Rendering Engine).
Copyright © 2003 Fifty1 Software Inc. 

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free 
Software Foundation; either version 2 of the License, or (at your option) any 
later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more 
details.

You should have received a copy of the GNU Lesser General Public License along 
with this program; if not, write to the Free Software Foundation, Inc., 
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
===============================================================================
*/
#ifndef _OGREMAYA_MESH_H_
#define _OGREMAYA_MESH_H_

#include "OgreMayaCommon.h"

#include <maya/MGlobal.h>
#include <maya/MFloatArray.h>
#include <maya/MPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MColorArray.h>
#include <maya/MObjectArray.h>
#include <maya/MFnMesh.h>
#include <maya/MStatus.h>
#include <maya/MItMeshPolygon.h>

#include <fstream.h>

#include <string>
#include <list>
#include <vector>

namespace OgreMaya {

    typedef float Real;


    struct Vector3 {
        Real x,y,z;
    };

    struct ColourValue {
        Real r,g,b,a;
    };

	//	===========================================================================
	/** \struct		MeshUV
		Simple structure with single set of UV coordinates for a single vertex.
	*/	
	//	===========================================================================
	struct MeshVertexUV
	{
		Real u;
		Real v;

		MeshVertexUV() {
			u = (Real)0.0;
			v = (Real)0.0;
		}
	};
	typedef std::list<MeshVertexUV> MeshVertexUVList;
	

	//	===========================================================================
	/** \struct		MeshMayaUVSet
		Structure that holds UVs for all vertex-faces for a single UV set.
	*/	
	//	===========================================================================
	struct MeshMayaUVSet {
		MFloatArray	uArray;
		MFloatArray	vArray;
		MString		sName;
	};
	typedef std::list<MeshMayaUVSet> MeshMayaUVSetList;


    //	===========================================================================
	/** \struct		MeshMayaUVSet
		Structure that holds UVs for all vertex-faces for a single UV set.
	*/	
	//	===========================================================================
	struct VertexBoneAssigment {
        int vertexId;
        int boneId;
		float weight;
	};
	typedef std::list<VertexBoneAssigment> VertexBoneAssigmentList;


	//	===========================================================================
	/** \struct		MeshMayaGeometry
		Structure that holds all data for a single Maya mesh.
	*/	
	//	===========================================================================
	struct MeshMayaGeometry {
		MString                  Name;
		MString                  MaterialName;
		MPointArray			     Vertices;
		MFloatVectorArray	     FaceVertexNormals;	 // face-vertex normals
		MColorArray			     FaceVertexColours;
		MIntArray			     TriangleVertexIds;	 // face-relative ids
		MIntArray			     TrianglePolygonIds; // polygon number for each triangle
		MeshMayaUVSetList	     UVSets;
        std::vector<MFloatArray> Weights;
	};

	//	===========================================================================
	/** \struct		MeshFaceVertex
		Structure that defines a face-vertex.
	*/	
	//	===========================================================================
	struct MeshFaceVertex {
		Vector3       vecPosition;
		Vector3       vecNormal;
		ColourValue   colour;
		MeshVertexUVList	listUV;

        VertexBoneAssigmentList boneAssigments;

		bool isAlmostEqual(const MeshFaceVertex *other, 
			               const bool bNormal, const bool bColour, const bool bUV,
			               const Real fTolPos, const Real fTolNorm) const
		{
            // TODO
            /*
			bool bAlmostEqual = true;
			bAlmostEqual &= ((vecPosition - other->vecPosition).length() < fTolPos);
			if (bNormal) bAlmostEqual &= ((vecNormal - other->vecNormal).length() < fTolNorm);
			if (bColour) bAlmostEqual &= (colour == other->colour);
			if (bUV) {
				bAlmostEqual &= (listUV.size() == other->listUV.size());
				MeshVertexUVList::const_iterator iterUVthis, iterUVother;
				iterUVthis = listUV.begin();
				iterUVother = other->listUV.begin();
				while (iterUVthis != listUV.end()) {
					bAlmostEqual &= (iterUVthis->u == iterUVother->u);
					bAlmostEqual &= (iterUVthis->v == iterUVother->v);

					++iterUVthis;
					++iterUVother;
				}
			}
			return bAlmostEqual;
            */
            return false;
		}
	};
	typedef std::vector<MeshFaceVertex> MeshFaceVertexVector;


	//	===========================================================================
	/** \struct		MeshTriFace
		Structure that defines a triangular face as a set of 3 indices into an
		arrray of MeshFaceVertex'es.
	*/	
	//	===========================================================================
	struct MeshTriFace {
		unsigned long index0;
		unsigned long index1;
		unsigned long index2;
	};
	typedef std::list<MeshTriFace> MeshTriFaceList;


	//	===========================================================================
	/** \class		MeshGenerator
		\author		John Van Vliet, Fifty1 Software Inc.
		\version	1.0
		\date		June 2003

		Generates an Ogre mesh from a Maya scene. The complete Maya scene is 
		represented by a single Ogre Mesh, and Maya meshes are represented by 
		Ogre SubMeshes.
	*/	
	//	===========================================================================
	class MeshGenerator	{
	public:

		/// Standard constructor.
		MeshGenerator();
		
		/// Destructor.
		virtual ~MeshGenerator();

		/// Export the complete Maya scene (called by OgreMaya.mll or OgreMaya.exe).
		bool exportAll();

		/// Export selected parts of the Maya scene (called by OgreMaya.mll).
		bool exportSelection();

	protected:
		/// Required for OptionParser interface.
		//bool _validateOptions();

		/// Return visibility of a DAG node.
		bool _isVisible(MFnDagNode &fnDag, MStatus &status);
		
		/// Process a Maya polyMesh.
		MStatus _processPolyMesh(ofstream& out, const MDagPath dagPath);
		MStatus _queryMayaGeometry(MFnMesh &fnMesh, MeshMayaGeometry &rGeom);
		MStatus _parseMayaGeometry(MFnMesh &fnMesh,
			                       MeshMayaGeometry &MayaGeometry, 
			                       MeshFaceVertexVector &FaceVertices, 
								   MeshTriFaceList &TriFaces);
		void _optimizeMesh(MeshFaceVertexVector &FaceVertices, MeshTriFaceList &TriFaces);

		void _convertObjectToFace(MItMeshPolygon &iterPoly, MIntArray &objIndices, MIntArray &faceIndices);

	};

} // namespace OgreMaya

#endif
