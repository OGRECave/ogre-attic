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

#include "LexiIntermediateAPI.h"
#include "LexiOgreSkeletonCompiler.h"

class COgreMeshCompiler
{
public:

	COgreMeshCompiler( CIntermediateMesh* pIntermediateMesh, const CDDObject* pConfig, CExportProgressDlg* pProgressDlg);
	virtual ~COgreMeshCompiler();

	bool			WriteOgreMesh( const Ogre::String& sFilename, bool bXMLexport=false );

	Ogre::MeshPtr	GetOgreMesh( void );

protected:

	void	InitializeOgreComponents( void );

	void	CreateOgreMesh( CIntermediateMesh* pIntermediateMesh );
	void	CreateSubMeshes( CIntermediateMesh* pIntermediateMesh );
	void	CreateBuffers( CIntermediateMesh* pIntermediateMesh );
	void	CreateIndexBuffer( CIntermediateMesh* pIntermediateMesh );
	void	CreateVertexBuffer( CIntermediateMesh* pIntermediateMesh );
	void	CreateNormalBuffer( CIntermediateMesh* pIntermediateMesh );
	void	CreateDiffuseBuffer( CIntermediateMesh* pIntermediateMesh );
	void	CreateTexCoordBuffer( CIntermediateMesh* pIntermediateMesh );
	void	CreatePoseBuffers( CIntermediateMesh* pIntermediateMesh );
	void	CreateMeshBounds( void );

	void	SetBoneAssignments( const CTriangle& face, CIntermediateMesh* pIntermediateMesh, Ogre::SubMesh* pSubMesh=NULL );

private:

	void	ReadConfig( const CDDObject* pConfig );
	void	ReindexIntermediateBuffers( CIntermediateMesh* pIntermediateMesh );
	void	OptimizeVertexDeclaration( void );
	CMeshArray* ExtractSubMeshData_Vec3( CMeshArray* pMeshArray, CIntermediateMesh* pIMesh, CIntermediateMaterial* pIMat, std::map< unsigned int, unsigned int>& lIndexMap );
	CMeshArray* ExtractSubMeshData_Vec4( CMeshArray* pMeshArray, CIntermediateMesh* pIMesh, CIntermediateMaterial* pIMat, std::map< unsigned int, unsigned int>& lIndexMap );
	CMeshArray* ExtractSubMeshDataFromIndexMap_Vec4( CMeshArray* pMeshArray, std::map< unsigned int, unsigned int>& lIndexMap );
	CMeshArray* ExtractSubMeshDataFromIndexMap_Vec3( CMeshArray* pMeshArray, std::map< unsigned int, unsigned int>& lIndexMap );
	CMeshArray* ExtractSubMeshDataFromIndexMap_Vec2( CMeshArray* pMeshArray, std::map< unsigned int, unsigned int>& lIndexMap );
	

	unsigned int CreatePose( int** poseIndex, CIntermediateMesh* pIntermediateMesh, Ogre::String name, unsigned int iFrame, bool bOptimize );
	unsigned int m_iNrPoses;

	Ogre::MeshPtr			m_pOgreMesh;
	Ogre::Real				m_MaxSquaredLength;
	Ogre::AxisAlignedBox	m_AABB;
	int						m_iBind;

	bool 	m_bReindex;
	bool 	m_bExportNormals;
	bool 	m_bExportColours;
	bool 	m_bExportTexUVs;
	bool 	m_bExportSkeleton;
	bool	m_bPoseAnimation;
	bool	m_bOptimizeBuffers;
	bool	m_bExportEdges;
	bool	m_bExportTangents;
	bool	m_bUseSharedGeometry;

	CExportProgressDlg* m_pProgressDlg;

	COgreSkeletonCompiler* m_pSkeletonCompiler;

	Ogre::HardwareIndexBuffer::IndexType		m_IndexBitType;
	std::map<CIntermediateMaterial*, Ogre::SubMesh*>	m_lMaterialSubMeshMap;
	std::map<Ogre::SubMesh*, std::map<unsigned int, unsigned int> >	m_lSubMeshIndexReMap; // old index -> newIndex

	Ogre::VertexDeclaration* m_pVertexDecl;
	unsigned int m_iNrVerts;
};
