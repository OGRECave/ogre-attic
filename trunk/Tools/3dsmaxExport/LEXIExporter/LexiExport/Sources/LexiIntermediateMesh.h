/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Mark Folkenberg,
Bo Krohn

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
-----------------------------------------------------------------------------
*/

#ifndef __NDS_LexiExporter_IntermediateMesh__
#define __NDS_LexiExporter_IntermediateMesh__

#include "LexiIntermediateObject.h"
//

class CIntermediateMesh : public Ogre::MovableObject, public CIntermediateObject {

	friend class CIntermediateBuilder;

	private:

		typedef struct {

			unsigned int m_iTime;
			CMeshArray* m_pArray;

		} MeshArray;

		typedef struct {

			Ogre::String	m_sPoseName;
			unsigned int	m_iTime;
			bool			m_bOptimize;

		} PoseData;

		typedef struct {

			PoseData		m_poseData;
			Ogre::String	m_sAnimName;
			unsigned int	m_iStartFrame;
			unsigned int	m_iEndFrame;
			float			m_fSampleRate;

		} PoseAnimData;

	private:

		void BuildMaterialList();
//		void BuildSubmeshIndexMaps( void );
		std::map< CIntermediateMaterial*, std::map< unsigned int, unsigned int> > m_lSubmeshIndexMap;

		unsigned int m_iNumTriangles;
		CTriangleArray m_Triangles;

		typedef std::map<std::string, MeshArray*> ArrayMap;
		ArrayMap m_Arrays;

		fastvector<CIntermediateMaterial*> m_Materials;

		CIntermediateSkeleton* m_pISkeleton;

		std::map< Ogre::String ,PoseData > m_lPoseList;
		std::map< Ogre::String, PoseAnimData > m_lPoseAnims;

		unsigned int* m_pIndexTable;
		unsigned int* m_pPickIndexTable;
		unsigned int m_iIndexCount;

	protected:

//		unsigned int m_iNodeID;
		bool	m_bIsCollapsed;

		//

		CIntermediateMesh(unsigned int iNumTriangles, unsigned int iNodeID);

		void ForceCreateArray(const char* pszName);

		//

        virtual const Ogre::String& getMovableType(void) const;
        virtual const Ogre::AxisAlignedBox& getBoundingBox(void) const;
		virtual Ogre::Real getBoundingRadius(void) const;
        virtual void _updateRenderQueue(Ogre::RenderQueue* queue);

	public:

		// Destructor
		~CIntermediateMesh();

		//
		//  Triangle

		// Get number of triangles
		unsigned int GetNumTriangles() const;

		// Get reference to triangle
		CTriangle& GetTriangle(unsigned int iIndex);
		const CTriangle& GetTriangle(unsigned int iIndex) const;

		// Get reference to entire triangle array
		CTriangleArray& GetTriangles();
		const CTriangleArray& GetTriangles() const;

		// Get triangle indices using a specific material
		void GetTrianglesUsingMaterial(CIntermediateMaterial* pMaterial, std::vector<unsigned int>& Triangles) const;

		//
		//  Array

		// Get array
		CMeshArray* GetArray(const char* pszName, TimeValue iTime);

		bool IsCollapsed( void );

		//
		//  Material

		// Get number of materials
		unsigned int GetNumMaterials() const;

		// Get material
		CIntermediateMaterial* GetMaterial(unsigned int iIndex) const;

		//
		// Skeleton
		void SetSkeleton( CIntermediateSkeleton* pSkel );
		CIntermediateSkeleton* GetSkeleton( void );

		//
		// Pose
		bool AddPose(Ogre::String name, unsigned int frame, bool bOptimize);
		bool HasPoseData( void );
		unsigned int GetPoseCount( void );
		bool GetPose(unsigned int index, Ogre::String& poseName, unsigned int& frameRef, bool& optimize);

		bool AddPoseAnimation(const char* pszAnimName, unsigned int iStartFrame, unsigned int iEndFrame, float fRate, bool bOptimize);
		unsigned int GetPoseAnimCount( void );
		bool GetPoseAnimation(unsigned int index, Ogre::String& poseName, unsigned int& iStartFrame, unsigned int& iEndFrame, float& fRate, bool& bOptimize);

		
		//
		//  Utilities

		// Reindex vertices

		void Reindex(const fastvector<CMeshArray*>& ArrayList);
		void PostReindex(const fastvector<CMeshArray*>& ArrayList);
		unsigned int GetReindexedIndex( unsigned int iOldIndex );

		//// Submesh helper Maps
		//unsigned int GetSubmeshMappedIndex( CIntermediateMaterial* pMat, unsigned int oldIndex );
		//unsigned int GetSubmeshMappedIndexCount( CIntermediateMaterial* pMat );
		//bool GetSubmeshMappedIndexMap( CIntermediateMaterial* pMat, std::map< unsigned int, unsigned int>& returnMap );
		//unsigned int GetSubmeshMappedUniqueIndexCount( CIntermediateMaterial* pMat );


};

//

#endif // __NDS_LexiExporter_IntermediateMesh__