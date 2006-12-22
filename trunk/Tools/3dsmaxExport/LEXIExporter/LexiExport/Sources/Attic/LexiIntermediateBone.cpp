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

	m_Positions.clear();
	m_Orientations.clear();
	m_Scales.clear();
	m_pParent = NULL;
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

unsigned int CIntermediateBone::GetFrameCount() const
{
	return m_Positions.size();
}

void CIntermediateBone::GetFrame(unsigned int iFrame, float& fTimeInSecs, Ogre::Vector3& vPos, Ogre::Quaternion& qOri, Ogre::Vector3& vScale) const
{
	fTimeInSecs = m_Times[iFrame];
	vPos = m_Positions[iFrame];
	qOri = m_Orientations[iFrame];
	vScale = m_Scales[iFrame];
}

void CIntermediateBone::AddFrame( float timeInSecs, const Ogre::Vector3& vPos, const Ogre::Quaternion& qOri, const Ogre::Vector3& vScale)
{
	m_Times.push_back(timeInSecs);
	m_Positions.push_back(vPos);
	m_Orientations.push_back(qOri);
	m_Scales.push_back(vScale);
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

//

