/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2007 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef _OgreTangentSpaceCalc_H_
#define _OgreTangentSpaceCalc_H_

#include "OgrePrerequisites.h"

namespace Ogre
{

	/** Class for calculating a tangent space basis.
	*/
	class _OgreExport TangentSpaceCalc
	{
	public:
		TangentSpaceCalc();
		virtual ~TangentSpaceCalc();

		/** List of triangles that are affected by index remapping (split vertices).
			For details of the remappings, see VertexSplits.
		*/
		typedef std::list<size_t> TriangleRemapList;
		typedef std::pair<size_t, size_t> VertexSplit;

		typedef std::list<VertexSplit> VertexSplits;

		/// The result of having built a tangent space basis
		struct Result
		{
			/// The vertex data containing tangents
			VertexData* vdata;
			/// The index data, which may have been modified
			IndexData* idata;
			/** A list of vertex indices which were split off into new vertices
				because of mirroring. First item in each pair is the source vertex 
				index, the secon value is the split vertex index.
			*/
			VertexSplits vertexSplits;
			/** A list of triangles which were re-indexed. You can use this if you have other
				triangle-based data which you will need to alter to match. */
			TriangleRemapList trianglesRemapped;
		};

		
		/** Build a tangent space basis from the provided data.
		@param v_in The incoming vertex data, not modified
		@param i_in The incoming index data, not modified
		@param opType The type of render operation being used, only valid options
			are OT_TRIANGLE_LIST, OT_TRIANGLE_STRIP and OT_TRIANGLE_FAN
		@param targetSemantic The semantic to store the tangents in. Defaults to 
			the explicit tangent binding, but note that this is only usable on more
			modern hardware (Shader Model 2), so if you need portability with older
			cards you should change this to a texture coordinate binding instead.
        @param sourceTexCoordSet The texture coordinate index which should be used as the source
            of 2D texture coordinates, with which to calculate the tangents.
        @param index The element index, ie the texture coordinate set which should be used to store the 3D
            coordinates representing a tangent vector per vertex, if targetSemantic is 
			VES_TEXTURE_COORDINATES. If this already exists, it will be overwritten.
		@returns
			A structure containing the results of the tangent space build. Vertex data
			will always be modified but it's also possible that the index data
			could be adjusted. This happens when mirroring is used on a mesh, which
			causes the tangent space to be inverted on opposite sides of an edge.
			This is discontinuous, therefore the vertices have to be split along
			this edge, resulting in new vertices.
		*/
		Result build(const VertexData* v_in, const IndexData* i_in, 
			RenderOperation::OperationType opType, 
			VertexElementSemantic targetSemantic = VES_TANGENT,
			unsigned short sourceTexCoordSet = 0, unsigned short index = 1);


	protected:

		struct VertexInfo
		{
			Vector3 pos;
			Vector3 norm;
			Vector2 uv;
			Vector3 tangent;
			Vector3 binormal;
			// Which way the tangent space is oriented (+1 / -1) (set on first time found)
			int parity;
			// What index the opposite parity vertex copy is at (-1 if not created yet)
			long oppositeParityIndex;

			VertexInfo() : tangent(Vector3::ZERO), binormal(Vector3::ZERO), 
				parity(0), oppositeParityIndex(-1) {}
		};
		typedef std::vector<VertexInfo> VertexInfoArray;
		VertexInfoArray mVertexArray;

		void organiseTangentsBuffer(VertexData *vertexData,
			VertexElementSemantic targetSemantic, 
			unsigned short sourceTexCoordSet, unsigned short index);

		void populateVertexArray(const VertexData* v_in, unsigned short sourceTexCoordSet);
		void processFaces(const IndexData* i_in, RenderOperation::OperationType opType, 
			Result& result);
		/// Calculate face tangent space, U and V are weighted by UV area, N is normalised
		void calculateFaceTangentSpace(const size_t* vertInd, Vector3& tsU, Vector3& tsV, Vector3& tsN);
		Real calculateAngleWeight(size_t v0, size_t v1, size_t v2);
		int calculateParity(const Vector3& u, const Vector3& v, const Vector3& n);
		void addFaceTangentSpaceToVertices(size_t faceIndex, size_t *localVertInd, 
			const Vector3& faceTsU, const Vector3& faceTsV, const Vector3& faceNorm, Result& result);
		void normaliseVertices();

	};

}



#endif
