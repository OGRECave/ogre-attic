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

CIntermediateBone::CIntermediateBone(const char* pszName)
{
	m_sName = pszName;
	m_pParent = NULL;
	m_iIndex = 0;
	m_Handle = 0;
	m_ParentHandle = 0;

	m_bindingPos = Ogre::Vector3::ZERO;
	m_bindingOrientation = Ogre::Quaternion::IDENTITY;
	m_bindingScale = Ogre::Vector3::ZERO;

	m_lAnimations.clear();
}

CIntermediateBone::~CIntermediateBone()
{
	clear();
}

//

void CIntermediateBone::clear()
{
	m_sName.clear();

	unsigned int iCount = m_Bones.size();
	for(unsigned int x = 0; x < iCount; x++)
	{
		CIntermediateBone* pBone = m_Bones[x];
		delete pBone;
	}
	m_Bones.clear();

	std::map< Ogre::String, CAnimationData* >::iterator iter = m_lAnimations.begin();
	while(iter != m_lAnimations.end())
	{
		delete (*iter).second;
		iter++;
	}
	m_lAnimations.clear();

	//m_Positions.clear();
	//m_Orientations.clear();
	//m_Scales.clear();
	//m_pParent = NULL;
}

//

std::string CIntermediateBone::GetName() const
{
	return m_sName;
}

void CIntermediateBone::SetHandle( ULONG handle )
{
	m_Handle = handle;
}

ULONG CIntermediateBone::GetHandle( void )
{
	return m_Handle;
}

void CIntermediateBone::SetParentHandle( ULONG handle )
{
	m_ParentHandle = handle;
}

ULONG CIntermediateBone::GetParentHandle( void )
{
	return m_ParentHandle;
}

void CIntermediateBone::SetIndex( unsigned int index )
{
	m_iIndex = index;
}

unsigned int CIntermediateBone::GetIndex( void )
{
	return m_iIndex;
}

//

unsigned int CIntermediateBone::GetBoneCount() const
{
	return m_Bones.size();
}

CIntermediateBone* CIntermediateBone::GetBone(unsigned int iBone)
{
	return m_Bones[iBone];
}

const CIntermediateBone* CIntermediateBone::GetBone(unsigned int iBone) const
{
	return m_Bones[iBone];
}

void CIntermediateBone::AddBone(CIntermediateBone* pBone)
{
	m_Bones.push_back(pBone);
	pBone->SetParent( this );
}

void CIntermediateBone::SetParent( CIntermediateBone* parent )
{
	m_pParent = parent;
}

CIntermediateBone* CIntermediateBone::GetParent( void )
{
	return m_pParent;
}

//

unsigned int CIntermediateBone::GetFrameCount( Ogre::String animationName ) const
{
	std::map< Ogre::String, CAnimationData* >::const_iterator iter = m_lAnimations.find(animationName);

	if(iter == m_lAnimations.end())
		return false;	// Animation with that name doesn´t exist

	return (*iter).second->GetNrFrames();
}

bool CIntermediateBone::CreateAnimation( const CAnimationSetting animSetting )
{
	std::map< Ogre::String, CAnimationData* >::iterator iter = m_lAnimations.find(animSetting.m_sAnimName);

	if(iter != m_lAnimations.end())
		return false;	// Animation with that name already exists

	CAnimationData* newAnim = new CAnimationData();//animSetting.m_iEndFrame - animSetting.m_iStartFrame);
	newAnim->SetOptimize(animSetting.m_bOptimize);

	m_lAnimations.insert( std::pair< Ogre::String, CAnimationData* >(animSetting.m_sAnimName, newAnim) );
	return true;
}


bool CIntermediateBone::GetFrame( Ogre::String animName, unsigned int iFrame, float& fTimeInSecs, Ogre::Vector3& vPos, Ogre::Quaternion& qOri, Ogre::Vector3& vScale) const
{
	std::map< Ogre::String, CAnimationData* >::const_iterator iter = m_lAnimations.find(animName);
	if(iter == m_lAnimations.end())
		return false;

	//if(iFrame > (*iter).second->GetNrFrames() )
	//	return false;

	return (*iter).second->GetFrame(iFrame, fTimeInSecs, vPos, qOri, vScale);

	//fTimeInSecs = (*iter).second->m_lTimes[iFrame];
	//vPos = (*iter).second->m_lPositions[iFrame];
	//qOri = (*iter).second->m_lOrientations[iFrame];
	//vScale = (*iter).second->m_lScales[iFrame];
	//return true;
}

bool CIntermediateBone::AddFrame( const std::string animName, unsigned int frameNr, float timeInSecs, const Ogre::Vector3& vPos, const Ogre::Quaternion& qOri, const Ogre::Vector3& vScale)
{
	std::map< Ogre::String, CAnimationData* >::const_iterator iter = m_lAnimations.find(animName);
	if(iter == m_lAnimations.end())
		return false;

	// add only if it appends
	if(frameNr != (*iter).second->GetNrFrames() )
		return false;

	(*iter).second->AddFrame(timeInSecs,vPos,qOri,vScale);

	//(*iter).second->m_lTimes.push_back(timeInSecs);
	//(*iter).second->m_lPositions.push_back(vPos);
	//(*iter).second->m_lOrientations.push_back(qOri);
	//(*iter).second->m_lScales.push_back(vScale);

	return true;
}

void CIntermediateBone::SetBindingPose(const Ogre::Vector3& vPos, const Ogre::Quaternion& qOri, const Ogre::Vector3& vScale)
{
	m_bindingPos = vPos;
	m_bindingOrientation = qOri;
	m_bindingScale = vScale;
}

void CIntermediateBone::GetBindingPose(Ogre::Vector3& vPos, Ogre::Quaternion& qOri, Ogre::Vector3& vScale) const
{
	vPos = m_bindingPos;
	qOri = m_bindingOrientation;
	vScale = m_bindingScale;
}

std::map< Ogre::String, CAnimationData* > CIntermediateBone::GetAnimations( void )
{
	return m_lAnimations;
}

//

