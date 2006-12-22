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

#ifndef __NDS_LexiExporter_IntermediateMaterial__
#define __NDS_LexiExporter_IntermediateMaterial__

//
struct STextureMapInfo
{
	Ogre::String m_sFilename;
	Ogre::String m_sMapType;
	unsigned int m_iCoordSet;

	Ogre::TextureUnitState::TextureAddressingMode m_AdressingMode;

	float		m_fAmount;
	float		m_fOffset[2];
	float		m_fScale[2];
	float		m_fAngle;
	bool		m_bAlpha;

	bool isNull( void ) 
	{ 
		if(m_sFilename.empty())
			return true;
		else 
			return false;
	}

};

class CIntermediateMaterial {

	public:

		CIntermediateMaterial( Ogre::String name );
		CIntermediateMaterial(const CIntermediateMaterial& other);
		~CIntermediateMaterial( void );

		bool	UsesSameTextureMaps( CIntermediateMaterial* pIMat );

		void	AddTextureMap( Ogre::String identifier, STextureMapInfo texInfo );

		// Gets
		Ogre::String	GetName( void );
		Ogre::String	GetParentName( void );

		const Ogre::ColourValue& GetAmbientColor( void ) const;	
		const Ogre::ColourValue& GetDiffuseColor( void ) const;
		const Ogre::ColourValue& GetSpecularColor( void ) const;
		const Ogre::ColourValue& GetEmissiveColor( void ) const;

		float	GetSpecularLevel( void ) const;
		float	GetGlossiness( void ) const;
		float	GetOpacity( void ) const;

		bool 	Get2Sided( void ) const;
		bool 	GetWired( void ) const;
		bool 	GetFaceted( void ) const;

		short&	GetMask( void );
		const std::map< Ogre::String, STextureMapInfo >& GetTextureMaps( void );
		STextureMapInfo GetTextureMapInfoFromName( Ogre::String name );

		// Sets
		void	SetParentName( Ogre::String parentName );

		void	SetAmbientColor( const Ogre::Vector4& color );
		void	SetAmbientColor( const Ogre::Vector3& color );
		void	SetAmbientColor( const Ogre::ColourValue& color );
		void	SetAmbientColor( float r, float g, float b, float a=1.0f );

		void	SetDiffuseColor( const Ogre::Vector4& color );
		void	SetDiffuseColor( const Ogre::Vector3& color );
		void	SetDiffuseColor( float r, float g, float b, float a=1.0f );

		void	SetSpecularColor( const Ogre::Vector4& color );
		void	SetSpecularColor( const Ogre::Vector3& color );
		void	SetSpecularColor( float r, float g, float b, float a=1.0f );

		void	SetEmissiveColor( const Ogre::Vector4& color );
		void	SetEmissiveColor( const Ogre::Vector3& color );
		void	SetEmissiveColor( float r, float g, float b, float a=1.0f );

		void	SetSpecularLevel( float level );
		void	SetGlosiness( float glossiness );
		void	SetOpacity( float opacity );

		void 	Set2Sided( bool b2Sided );
		void 	SetWired( bool bWired );
		void 	SetFaceted( bool bFaceted );


	private:

		// Name
		Ogre::String m_sName;

		// Parent name
		Ogre::String m_sParentName;

		// Ambient color
		Ogre::ColourValue m_vAmbientColor;

		// Diffuse color
		Ogre::ColourValue m_vDiffuseColor;

		// Specular color
		Ogre::ColourValue m_vSpecularColor;

		// Emissive color
		Ogre::ColourValue m_vEmissiveColor;

		// Specular level
		float m_fSpecularLevel;

		// Glossiness
		float m_fGlossiness;

		// Opacity
		float m_fOpacity;

		// Emissive Strength
		float m_fEmmisiveStrength;

		// 2-Sided
		bool m_b2Sided;

		// Wire Framed
		bool m_bWire;

		// Faceted
		bool m_bFaceted;

		// Texture map
		std::map< Ogre::String, STextureMapInfo > m_lTextureMaps;

		// Mask used for determining materials with same texture usage
		short m_Mask;

};

//

#endif // __NDS_LexiExporter_IntermediateMaterial__