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

CIntermediateSkeleton::CIntermediateSkeleton()
{
}

CIntermediateSkeleton::~CIntermediateSkeleton()
{
	clear();
}

//

void CIntermediateSkeleton::clear()
{
	unsigned int iCount = m_BoneList.size();
	for(unsigned int x = 0; x < iCount; x++)
	{
		CIntermediateBone* pBone = m_BoneList[x];
		delete pBone;
	}
	m_BoneList.clear();
	m_RootBoneList.clear();
	m_BoneNameList.clear();
}

//

void CIntermediateSkeleton::SetFPS(float fFPS)
{
	m_fFPS = fFPS;
}

float CIntermediateSkeleton::GetFPS() const
{
	return m_fFPS;
}

//

unsigned int CIntermediateSkeleton::GetBoneCount() const
{
	return m_BoneList.size();
}

CIntermediateBone* CIntermediateSkeleton::GetBone(unsigned int iBone)
{
	return m_BoneList[iBone];
}

const CIntermediateBone* CIntermediateSkeleton::GetBone(unsigned int iBone) const
{
	return m_BoneList[iBone];
}

std::string CIntermediateSkeleton::GetBoneName(unsigned int iBone) const
{
	return m_BoneList[iBone]->GetName();
}

CIntermediateBone* CIntermediateSkeleton::GetBoneByName( const char* pszName ) const
{
	std::map<std::string, CIntermediateBone*>::const_iterator iter = m_BoneNameList.find( pszName );
	if(iter != m_BoneNameList.end())
		return iter->second;
	else
		return NULL;
}

CIntermediateBone* CIntermediateSkeleton::GetBoneByIndex( int index ) const
{
	std::map<int, CIntermediateBone*>::const_iterator iter = m_IndexedBoneList.find( index );
	if(iter != m_IndexedBoneList.end())
		return iter->second;
	else
		return NULL;
}

void CIntermediateSkeleton::AddBone(CIntermediateBone* pBone, const char* pszName)
{
	m_BoneList.push_back(pBone);
	//m_BoneNameList.push_back(pszName);
	m_BoneNameList.insert( std::pair<std::string, CIntermediateBone*>(pBone->GetName(),pBone) );
	m_BoneHandles.insert( std::pair<ULONG, CIntermediateBone*>(pBone->GetHandle(), pBone));
}

bool CIntermediateSkeleton::AssembleBones( void )
{
	if(m_BoneList.empty())
		PopulateBoneHandleMap();
	if(m_BoneList.empty())
		return false; // still nothing to process

	std::vector<CIntermediateBone*>::iterator iter = m_BoneList.begin();

	while (iter != m_BoneList.end())
	{
		RecursiveAssembleBones(*iter);
		iter++;
	}

	return true;
}

bool CIntermediateSkeleton::RecursiveAssembleBones( CIntermediateBone* pIBone )
{
	ULONG parentHandle = pIBone->GetParentHandle();

	CIntermediateBone* parent = FindBone( parentHandle );
	if(parent)
		parent->AddBone( pIBone );

	return true;
//	for (int i = 0; i < pIBone->GetBoneCount(); i++)
//	{
//		RecursiveAssembleBones(pIBone->GetBone(i));
//	}
}

void CIntermediateSkeleton::PopulateBoneHandleMap( void )
{
	std::vector<CIntermediateBone*>::iterator iter = m_BoneList.begin();

	while (iter != m_BoneList.end())
	{
		RecursivePopulateBoneHandleMap(*iter);
		iter++;
	}
}

void CIntermediateSkeleton::BuildIndexedBoneList( void )
{
	std::vector<CIntermediateBone*>::iterator iter = m_BoneList.begin();

	while (iter != m_BoneList.end())
	{
		m_IndexedBoneList.insert( std::pair<int, CIntermediateBone*>( (*iter)->GetIndex(), *iter));
		iter++;
	}
}

void CIntermediateSkeleton::RecursivePopulateBoneHandleMap( CIntermediateBone* pIBone )
{
	m_BoneHandles.insert( std::pair<ULONG, CIntermediateBone*>(pIBone->GetHandle(), pIBone));

	for (int i = 0; i < pIBone->GetBoneCount(); i++)
	{
		RecursivePopulateBoneHandleMap(pIBone->GetBone(i));
	}
}

CIntermediateBone* CIntermediateSkeleton::FindBone( ULONG handle )
{
	std::map<ULONG,CIntermediateBone*>::iterator iter = m_BoneHandles.find( handle );

	if(iter != m_BoneHandles.end())
		return iter->second;

	return NULL;
}

int CIntermediateSkeleton::GetNrOfAssignmentsOnVertex( int idx )
{
	std::map< int, std::vector<SVertexBoneData> >::iterator iter = m_VertexBoneData.find( idx );

	if( iter != m_VertexBoneData.end() )
	{
		return iter->second.size();
	}

	return 0;
}

void CIntermediateSkeleton::NormalizeVertexAssignments( void )
{
	std::map< int, std::vector<SVertexBoneData> >::iterator iter = m_VertexBoneData.begin();

	// normalize
	while( iter != m_VertexBoneData.end() )
	{
		std::vector<SVertexBoneData>::iterator it = iter->second.begin();
		std::vector<SVertexBoneData>::iterator iend = iter->second.end();

		float fTotal=0;
		for(; it != iend; ++it) {
			SVertexBoneData& vertexBoneData = (*it);
			fTotal+=vertexBoneData.weight;		
		}

		if(fTotal!=0) {
			it = iter->second.begin();
			for(; it != iend; ++it) {
				SVertexBoneData& vertexBoneData = (*it);
				vertexBoneData.weight/=fTotal;
			}	
		}
		iter++;
	}
}

bool CIntermediateSkeleton::GetVertexData( int idx, int assignmentNr, SVertexBoneData& returnVal )
{
	std::map< int, std::vector<SVertexBoneData> >::iterator iter = m_VertexBoneData.find( idx );

	if( iter != m_VertexBoneData.end() )
	{
		returnVal.boneIndex = iter->second[assignmentNr].boneIndex;
		returnVal.weight = iter->second[assignmentNr].weight;
		return true;
	}

	return false;
}

bool CIntermediateSkeleton::AddVertexData( int idx, SVertexBoneData vertexData )
{
	std::map< int, std::vector<SVertexBoneData> >::iterator iter = m_VertexBoneData.find( idx );

	if( iter != m_VertexBoneData.end() )
	{
		iter->second.push_back(vertexData);
		return true;
	}
	else
	{
		std::vector<SVertexBoneData> newlist;
		newlist.push_back(vertexData);
		m_VertexBoneData.insert( std::pair<int, std::vector<SVertexBoneData>>(idx,newlist) );
		return true;
	}

	return false;
}

bool CIntermediateSkeleton::PrepareReindexChange( int oldIndex, int newIndex )
{
	std::map< int, std::vector<SVertexBoneData> >::iterator iter = m_VertexBoneData.find( oldIndex );

	if( iter != m_VertexBoneData.end() )
	{
		std::map< int, std::vector<SVertexBoneData> >::iterator iter2 = m_ReindexVertexBoneData.find( newIndex );
		if( iter2 != m_ReindexVertexBoneData.end() )
		{
			// we already have a record on this index.. so we appends
			while(!iter->second.empty())
			{
				iter2->second.push_back(iter->second.back());
				iter->second.pop_back();
			}
			return true;
		}
		else
		{
			m_ReindexVertexBoneData.insert( std::pair<int, std::vector<SVertexBoneData>>(newIndex,iter->second) );
			return true;
		}
	}

	return false;
}

bool CIntermediateSkeleton::ApplyReindexChanges( void )
{
	m_VertexBoneData.clear();
	m_VertexBoneData = m_ReindexVertexBoneData;
	m_ReindexVertexBoneData.clear();
	return true;
}

void CIntermediateSkeleton::MarkBoneAsRoot( CIntermediateBone* pIBone )
{
	m_RootBoneList.push_back( pIBone );
}

const std::vector<CIntermediateBone*>& CIntermediateSkeleton::GetRootBones( void ) const
{
	return m_RootBoneList;
}


std::string CIntermediateSkeleton::ToString( void )
{
	std::stringstream str;
	str << "\n";
	print( str, m_RootBoneList.front() );

	print( str, m_RootBoneList.front(), 0 ); // obsolete since multiple animation support added

	return str.str();
}

void CIntermediateSkeleton::print( std::stringstream& output, CIntermediateBone* pBone, int indent)
{
	for (int i=0; i <= indent; i++)
		output << "\t";

	output << pBone->GetName() << "(" << pBone->GetHandle() << ")" << "[" << pBone->GetIndex() << "]" << "\n";

	for (int j = 0; j < pBone->GetBoneCount(); j++)
		print(output, pBone->GetBone(j), indent+1);
}

void CIntermediateSkeleton::printAnimationData( std::stringstream& output, CIntermediateBone* pBone, int indent)
{
	for (int i=0; i <= indent; i++)
		output << "\t";

	output << pBone->GetName() << "(" << pBone->GetHandle() << ")" << "[" << pBone->GetIndex() << "]" << "\n";
	
	//Ogre::Vector3 vPos, vScale;
	//float fTimeInSecs;
	//Ogre::Quaternion qOri;

	//for (int j =0; j < pBone->GetFrameCount(); j++)
	//{
	//	pBone->GetFrame(j, fTimeInSecs, vPos, qOri, vScale);

	//	for (int i=0; i <= indent; i++) output << "\t";
	//	output << "Frame: " << j << "\t" << " time:" << Ogre::StringConverter::toString(fTimeInSecs) 
	//		<< "\t" << "(" << Ogre::StringConverter::toString(vPos) << ")" 
	//		<< "\t" << "(" << Ogre::StringConverter::toString(qOri) << ")"
	//		<< "\t" << "(" << Ogre::StringConverter::toString(vScale) << ")" << "\n";
	//}
	//output << "\t" << "\n";

	//for (int j = 0; j < pBone->GetBoneCount(); j++)
	//	printAnimationData(output, pBone->GetBone(j), indent+1);
}


//
