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

CIntermediateMaterial::CIntermediateMaterial( Ogre::String name )
{
	REGISTER_MODULE("Intermediate Material")

	m_sName = name;
	m_Mask = 0;
	m_b2Sided = false;
	m_bWire = false;
	m_bFaceted = false;
}

CIntermediateMaterial::CIntermediateMaterial(const CIntermediateMaterial& other)
{
	REGISTER_MODULE("Intermediate Material")

	m_sName = other.m_sName;
	m_sParentName = other.m_sParentName;
	m_vAmbientColor = other.m_vAmbientColor;
	m_vDiffuseColor = other.m_vDiffuseColor;
	m_vSpecularColor = other.m_vSpecularColor;
	m_fSpecularLevel = other.m_fSpecularLevel;
	m_fGlossiness = other.m_fGlossiness;
	m_fOpacity = other.m_fOpacity;
	m_b2Sided = other.m_b2Sided;
	m_bWire = other.m_bWire;
	m_bFaceted = other.m_bFaceted;
	m_lTextureMaps = other.m_lTextureMaps;
	m_Mask = other.m_Mask;
}

CIntermediateMaterial::~CIntermediateMaterial( void )
{
	UNREGISTER_MODULE
}

void CIntermediateMaterial::AddTextureMap( Ogre::String identifier, STextureMapInfo texInfo)
{
	std::map< Ogre::String, STextureMapInfo >::iterator it = m_lTextureMaps.find( identifier );
	if (it != m_lTextureMaps.end())
		return;

	std::pair< Ogre::String, STextureMapInfo > entry(identifier, texInfo);
	m_lTextureMaps.insert( entry );
}

short& CIntermediateMaterial::GetMask( void )
{
	return m_Mask;
}

const std::map< Ogre::String, STextureMapInfo >& CIntermediateMaterial::GetTextureMaps( void )
{
	return m_lTextureMaps;
}

STextureMapInfo CIntermediateMaterial::GetTextureMapInfoFromName( Ogre::String name )
{
	std::map< Ogre::String, STextureMapInfo >::iterator it = m_lTextureMaps.find( name );
	if ( it == 	m_lTextureMaps.end() )
		return STextureMapInfo();
	else
		return it->second;
}

bool CIntermediateMaterial::UsesSameTextureMaps( CIntermediateMaterial* pIMat )
{
	short& mask2 = pIMat->GetMask();

	if ( (m_Mask^mask2) == 0 )
		return true;
	else
		return false;
}


//////////////////////////////////////////////////////////////////////////
// Gets
//////////////////////////////////////////////////////////////////////////

Ogre::String CIntermediateMaterial::GetName( void )
{
	return m_sName;
}

Ogre::String CIntermediateMaterial::GetParentName( void )
{
	return m_sParentName;
}

const Ogre::ColourValue& CIntermediateMaterial::GetAmbientColor( void ) const
{
	return m_vAmbientColor;
}

const Ogre::ColourValue& CIntermediateMaterial::GetDiffuseColor( void ) const
{
	return m_vDiffuseColor;
}

const Ogre::ColourValue& CIntermediateMaterial::GetSpecularColor( void ) const
{
	return m_vSpecularColor;
}

const Ogre::ColourValue& CIntermediateMaterial::GetEmissiveColor( void ) const
{
	return m_vEmissiveColor;
}

float CIntermediateMaterial::GetSpecularLevel( void ) const
{
	return m_fSpecularLevel;
}

float CIntermediateMaterial::GetGlossiness( void ) const
{
	return m_fGlossiness;
}

float CIntermediateMaterial::GetOpacity( void ) const
{
	return m_fOpacity;
}

bool CIntermediateMaterial::Get2Sided( void ) const
{
	return m_b2Sided;
}

bool CIntermediateMaterial::GetWired( void ) const
{
	return m_bWire;
}

bool CIntermediateMaterial::GetFaceted( void ) const
{
	return m_bFaceted;
}

//////////////////////////////////////////////////////////////////////////
// Sets
//////////////////////////////////////////////////////////////////////////

void CIntermediateMaterial::SetParentName( Ogre::String parentName )
{
	m_sParentName = parentName;
}

void CIntermediateMaterial::SetAmbientColor( const Ogre::Vector4& color )
{
	m_vAmbientColor = Ogre::ColourValue(color.x,color.y, color.z, color.w);
}

void CIntermediateMaterial::SetAmbientColor( const Ogre::Vector3& color )
{
	m_vAmbientColor = Ogre::ColourValue(color.x, color.y, color.x, 1.0f);
}

void CIntermediateMaterial::SetAmbientColor( float r, float g, float b, float a )
{
	m_vAmbientColor = Ogre::ColourValue(r,g,b,a);
}

void CIntermediateMaterial::SetDiffuseColor( const Ogre::Vector4& color )
{
	m_vDiffuseColor = Ogre::ColourValue(color.x,color.y, color.z, color.w);
}

void CIntermediateMaterial::SetDiffuseColor( const Ogre::Vector3& color )
{
	m_vDiffuseColor = Ogre::ColourValue(color.x, color.y, color.x, 1.0f);
}

void CIntermediateMaterial::SetDiffuseColor( float r, float g, float b, float a )
{
	m_vDiffuseColor = Ogre::ColourValue(r,g,b,a);
}

void CIntermediateMaterial::SetSpecularColor( const Ogre::Vector4& color )
{
	m_vSpecularColor = Ogre::ColourValue(color.x,color.y, color.z, color.w);;
}

void CIntermediateMaterial::SetSpecularColor( const Ogre::Vector3& color )
{
	m_vSpecularColor = Ogre::ColourValue(color.x, color.y, color.x, 1.0f);
}

void CIntermediateMaterial::SetSpecularColor( float r, float g, float b, float a )
{
	m_vSpecularColor = Ogre::ColourValue(r,g,b,a);
}

void CIntermediateMaterial::SetEmissiveColor( const Ogre::Vector4& color )
{
	m_vEmissiveColor = Ogre::ColourValue(color.x,color.y, color.z, color.w);;
}

void CIntermediateMaterial::SetEmissiveColor( const Ogre::Vector3& color )
{
	m_vEmissiveColor = Ogre::ColourValue(color.x, color.y, color.x, 1.0f);
}

void CIntermediateMaterial::SetEmissiveColor( float r, float g, float b, float a )
{
	m_vEmissiveColor = Ogre::ColourValue(r,g,b,a);
}

void CIntermediateMaterial::SetSpecularLevel( float level )
{
	m_fSpecularLevel = level;
}

void CIntermediateMaterial::SetGlosiness( float glossiness )
{
	m_fGlossiness = glossiness;
}

void CIntermediateMaterial::SetOpacity( float opacity )
{
	m_fOpacity = opacity;
}

void CIntermediateMaterial::Set2Sided( bool b2Sided )
{
	m_b2Sided = b2Sided;
}

void CIntermediateMaterial::SetWired( bool bWired )
{
	m_bWire = bWired;
}

void CIntermediateMaterial::SetFaceted( bool bFaceted )
{
	m_bFaceted = bFaceted;
}

