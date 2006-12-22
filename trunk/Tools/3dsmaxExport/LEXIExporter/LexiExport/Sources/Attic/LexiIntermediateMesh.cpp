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

#include "LexiStdAfx.h"
#include "LexiIntermediateAPI.h"

//

const Ogre::String& CIntermediateMesh::getMovableType(void) const
{
	static Ogre::String sTypeName = "CIntermediateMesh";
	return sTypeName;
}

const Ogre::AxisAlignedBox& CIntermediateMesh::getBoundingBox(void) const
{
	static Ogre::AxisAlignedBox aab;
	return aab;
}

Ogre::Real CIntermediateMesh::getBoundingRadius(void) const
{
	return 0.0f;
}

void CIntermediateMesh::_updateRenderQueue(Ogre::RenderQueue* queue)
{
}

///////////////////////////////////////////////////////////

CIntermediateMesh::CIntermediateMesh(unsigned int iNumTriangles, unsigned int iNodeID) : MovableObject(GetNodeFromID(iNodeID)->GetName())
{
	m_iNodeID = iNodeID;

	m_Triangles.Create(iNumTriangles);
	m_iNumTriangles = iNumTriangles;
	m_pISkeleton = NULL;
}

CIntermediateMesh::~CIntermediateMesh()
{
	for(ArrayMap::iterator it = m_Arrays.begin(); it != m_Arrays.end(); it++)
	{
		MeshArray* pArray = it->second;
		delete pArray->m_pArray;
		delete pArray;
	}
	m_Arrays.clear();
}

//

unsigned int CIntermediateMesh::GetNumTriangles() const
{
	return m_iNumTriangles;
}

//

CTriangle& CIntermediateMesh::GetTriangle(unsigned int iIndex)
{
	return m_Triangles[iIndex];
}

const CTriangle& CIntermediateMesh::GetTriangle(unsigned int iIndex) const
{
	return m_Triangles[iIndex];
}

//

CTriangleArray& CIntermediateMesh::GetTriangles()
{
	return m_Triangles;
}

const CTriangleArray& CIntermediateMesh::GetTriangles() const
{
	return m_Triangles;
}

void CIntermediateMesh::SetSkeleton( CIntermediateSkeleton* pSkel )
{
	m_pISkeleton = pSkel;
}

CIntermediateSkeleton* CIntermediateMesh::GetSkeleton( void )
{
	return m_pISkeleton;
}

//

void CIntermediateMesh::ForceCreateArray(const char* pszName)
{
	unsigned int iNumVertices = m_iNumTriangles * 3;

	CMeshArray* pArray = NULL;
	if(!stricmp(pszName, "position") || !stricmp(pszName, "normal"))
	{
		pArray = new CVec3Array(iNumVertices);
	}
	else if(!stricmp(pszName, "diffuse") || !stricmp(pszName, "specular"))
	{
		pArray = new CVec4Array(iNumVertices);
	}
	else
	{
		char temp[16];

		for(unsigned int x = 1; x <= 99; x++)
		{
			sprintf(temp, "uv%i", x);
			if(!stricmp(pszName, temp))
			{
				pArray = new CVec2Array(iNumVertices);
				break;
			}
		}
	}

	if(pArray) pArray->Zero();

	MeshArray* pMA = new MeshArray;
	pMA->m_iTime = 0;
	pMA->m_pArray = pArray;
	m_Arrays.insert(ArrayMap::value_type(pszName, pMA));
}

//

CMeshArray* CIntermediateMesh::GetArray(const char* pszName, TimeValue iTime)
{
	ArrayMap::iterator it = m_Arrays.find(pszName);
	if(it != m_Arrays.end())
	{
		const MeshArray* pMA = it->second;
		if(pMA->m_iTime == iTime) return pMA->m_pArray;

		delete pMA->m_pArray;
		delete pMA;
		m_Arrays.erase(it);
	}

	CMeshArray* pArray = CIntermediateBuilder::Get()->BuildMeshArray(m_iNodeID, pszName, iTime);

	MeshArray* pMA = new MeshArray;
	pMA->m_iTime = iTime;
	pMA->m_pArray = pArray;
	m_Arrays.insert(ArrayMap::value_type(pszName, pMA));

	return pArray;
}

//

unsigned int CIntermediateMesh::GetNumMaterials() const
{
	return m_Materials.size();
}

CIntermediateMaterial* CIntermediateMesh::GetMaterial(unsigned int iIndex) const
{
	return m_Materials[iIndex];
}

//

static fastvector<CMeshArray*> _CmpList;
static unsigned int _iCmpListSize;

class vertexcmp {

	public:

		bool operator () (const unsigned int& a, const unsigned int& b) const
		{
			for(unsigned int x = 0; x < _iCmpListSize; x++)
			{
				CMeshArray* pArray = _CmpList[x];
				int r = memcmp(pArray->Data(a), pArray->Data(b), pArray->ElementSize());
				if(r < 0) return true;
				else if(r > 0) return false;
			}

			return false;
		}

};

typedef std::map<unsigned int, unsigned int, vertexcmp> vertexmap;

//

void CIntermediateMesh::Reindex(const fastvector<CMeshArray*>& ArrayList)
{
	_iCmpListSize = ArrayList.size();
	if(!_iCmpListSize) return;

	unsigned int iNumVertices = ArrayList[0]->Size();
	if(!iNumVertices) return;

	_CmpList = ArrayList;

	unsigned int* pIndexTable = new unsigned int[iNumVertices];
	unsigned int* pPickIndexTable = new unsigned int[iNumVertices];

	//

	vertexmap crcmap;
	unsigned int iIndexCount = 0;

	for(unsigned int x = 0; x < iNumVertices; x++)
	{
		vertexmap::iterator it = crcmap.find(x);
		if(it != crcmap.end())
		{
			pIndexTable[x] = it->second;
		}
		else
		{
			pIndexTable[x] = iIndexCount;
			pPickIndexTable[iIndexCount++] = x;
			crcmap.insert(vertexmap::value_type(x, crcmap.size()));
		}
	}

	//

	for(unsigned int x = 0; x < _iCmpListSize; x++)
	{
		CMeshArray* pArray = _CmpList[x];

		unsigned int iElemSize = pArray->ElementSize();
		unsigned char* pTarget = new unsigned char[iIndexCount * iElemSize];

		for(unsigned int y = 0; y < iIndexCount; y++)
		{
			memcpy(pTarget + (y * iElemSize), pArray->Data(pPickIndexTable[y]), iElemSize);
		}

		pArray->Create(iIndexCount, pTarget);
		delete []pTarget;
	}

	//

	for(unsigned int x = 0; x < m_iNumTriangles; x++)
	{
		CTriangle& tri = m_Triangles[x];
		unsigned int& t1 = tri.m_Vertices[0];
		unsigned int& t2 = tri.m_Vertices[1];
		unsigned int& t3 = tri.m_Vertices[2];

		if(m_pISkeleton) m_pISkeleton->PrepareReindexChange(t1, pIndexTable[t1]);
		t1 = pIndexTable[t1];

		if(m_pISkeleton) m_pISkeleton->PrepareReindexChange(t2, pIndexTable[t2]);
		t2 = pIndexTable[t2];

		if(m_pISkeleton) m_pISkeleton->PrepareReindexChange(t3, pIndexTable[t3]);
		t3 = pIndexTable[t3];
	}

	//

	delete []pIndexTable;
	delete []pPickIndexTable;

	if(m_pISkeleton) m_pISkeleton->ApplyReindexChanges();
}

//

void CIntermediateMesh::GetTrianglesUsingMaterial(CIntermediateMaterial* pMaterial, std::vector<unsigned int>& Triangles) const
{
	Triangles.clear();

	for(unsigned int x = 0; x < m_iNumTriangles; x++)
	{
		const CTriangle& tri = m_Triangles[x];
		if(tri.m_pMaterial == pMaterial) Triangles.push_back(x);
	} 
}

void CIntermediateMesh::BuildMaterialList( void )
{
	m_Materials.clear();

	for(unsigned int x = 0; x < m_iNumTriangles; x++)
	{
		CIntermediateMaterial* pMat = m_Triangles[x].m_pMaterial;

		bool bMatExists = false;
		for ( int i =0; i < m_Materials.size(); i++)
		{
			if(m_Materials[i] == pMat)
				bMatExists = true;
		}
		if(!bMatExists)
			m_Materials.push_back(pMat);
	} 
}

//

