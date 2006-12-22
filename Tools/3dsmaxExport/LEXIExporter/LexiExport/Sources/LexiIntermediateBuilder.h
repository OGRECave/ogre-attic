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

#ifndef __NDS_LexiExporter_IntermediateBuilder__
#define __NDS_LexiExporter_IntermediateBuilder__

//

class CIntermediateBuilder {

	friend class CIntermediateMesh;

	private:

		Ogre::SceneNode* CreateHierarchy(unsigned int iNodeID, Ogre::SceneNode* pParent, bool bRecursive, bool bHidden);
		CIntermediateMesh* CreateMesh(unsigned int iNodeID);

		int m_iBoneIndex;
		bool FindSkinModifier(INode* nodePtr, Modifier** pSkinMod,ISkin** pSkin, ISkinContextData** pSkinContext);
		void CreateIntermediateBonePool(CIntermediateSkeleton* pSkel, ISkin* pSkin);
		void ConnectLinkedBones(CIntermediateSkeleton* pSkel);
		void BuildSkeleton( CIntermediateSkeleton* pSkel );
		void AddFrame( CIntermediateSkeleton* pSkel, unsigned int meshNodeID, unsigned int iStartFrame, unsigned int iEndFrame, unsigned int iSampleRate);
		void SetBindingPose( CIntermediateSkeleton* pISkel, unsigned int meshNodeID, unsigned int frame );

		void CountTriangles(Ogre::SceneNode* pNode, unsigned int& iNumTriangles) const;
		void CollapseHierarchy(CIntermediateMesh* pMesh, unsigned int& iTriangleOffset, unsigned int& iVertexOffset, Ogre::SceneNode* pNode, const std::list<std::string>& Arrays, const Ogre::Matrix4& mat) const;

		static void Clamp( Point3& inVec, float threshold );
		static void Rotate90DegreesAroundX( Point3& inVec );

		bool			m_bExportSkeleton;
		int				m_iAnimStart;
		int				m_iAnimEnd;
		float			m_fSampleRate;
		Ogre::String	m_sAnimationName;
		float			m_fAnimTotalLength;

	protected:

		CMeshArray* BuildMeshArray(unsigned int iNodeID, const char* pszTypeName, TimeValue iTime);

		// Build and track Intermediate Materials
		CIntermediateMaterial* CreateMaterial( Mtl* pMaxMaterial );

		void RegisterMaps( CIntermediateMaterial* pIMat, Mtl* pMaxMaterial ) ;

	public:

		// Constructor/Destructor
		CIntermediateBuilder();
		~CIntermediateBuilder();

		// Get pointer to intermediate builder
		static CIntermediateBuilder* Get();

		// Clear everything (cached materials, etc.)
		void Clear();

		void SetConfig( CDDObject* pConfig );

		// Get built hierarchy
		Ogre::SceneNode* CreateHierarchy(unsigned int iNodeID, bool bRecursive, bool bHidden);

		// Collapse hierarchy
		Ogre::SceneNode* CollapseHierarchy(Ogre::SceneNode* pHierarchy, const std::list<std::string>& Arrays, const char* pszName) const;

		// Get the list of active intermediate materials
		bool GetMaterials( std::map<Ogre::String, CIntermediateMaterial*>& materialMap ) const;

		Ogre::String GetAnimationName( void );
		float GetAnimationLength( void );

		// Material Map
		std::map<Ogre::String, CIntermediateMaterial*>	m_lMaterials;

};

//

#endif // __NDS_LexiExporter_IntermediateBuilder__
