/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2003 The OGRE Team
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
-----------------------------------------------------------------------------
*/
#include "OgreD3D9HardwareOcclusionQuery.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreException.h"

namespace Ogre {

int D3D9HardwareOcclusionQuery::m_Skip = 0;

/**
  * This is a class that is the DirectX9 implementation of 
  * hardware occlusion testing.
  *
  * @author Lee Sandberg
  */

/**
  * Default object constructor
  * 
  */
D3D9HardwareOcclusionQuery::D3D9HardwareOcclusionQuery( IDirect3DDevice9* pD3DDevice ) 
{ 
	m_pD3DDevice = pD3DDevice;
	m_uintPixelCount = 0; 
	m_SkipCounter = 0;
	m_bHWOcclusionSupport = false;

	HRESULT hr = m_pD3DDevice->CreateQuery(D3DQUERYTYPE_OCCLUSION, &m_pD3DQuery);
	if ( hr != D3D_OK ) 
	{
		//Except(hr, "D3D9HardwareOcclusionQuery couldn't create hardware occlusion query object.", 
        //        "D3D9HardwareOcclusionQuery::D3D9HardwareOcclusionQuery");
		m_bHWOcclusionSupport = false;
	}
	else 
	{
		m_bHWOcclusionSupport = true;
	}
}


/**
  * Object destructor
  */
D3D9HardwareOcclusionQuery::~D3D9HardwareOcclusionQuery() 
{ 
	SAFE_RELEASE( m_pD3DQuery ); 
}

//------------------------------------------------------------------
// Occlusion query functions (see base class documentation for this)
//--
void D3D9HardwareOcclusionQuery::beginOcclusionQuery() 
{ 
	if( m_bHWOcclusionSupport )		// Make it fail silently if hardware occlusion isn't supported
	{
		if  ( m_SkipCounter ==  m_Skip ) { m_SkipCounter = 0; };		// Counter starts at 0 again at m_Skip 
		
		if ( m_SkipCounter == 0 && m_uintPixelCount != 0 )	// New or none visable objects must allways be tested but visable objects can be skiped
		{
			m_pD3DQuery->Issue(D3DISSUE_BEGIN); 
		}
	}
}
	
void D3D9HardwareOcclusionQuery::endOcclusionQuery() 
{ 
	if( m_bHWOcclusionSupport )	// Make it fail silently if hardware occlusion isn't supported
	{
		if (m_SkipCounter == 0 && m_uintPixelCount != 0 ) // New or none visable objects must allways be tested but visable objects can be skiped
		{
			m_pD3DQuery->Issue(D3DISSUE_END); 
		}
		m_SkipCounter++;								 // The skip counter is increased 
	}
}

//------------------------------------------------------------------
// This version of pullOcclusionQuery cases the DX9 API/Driver to flush all commands to the 3D card
// to allow a fast result from the query, but at the cost of poorer batching of API calls to the card.
// Note: OpenGL dosn't use this flag at all so the application running OpenGL won't display any different behaviour.
//--
bool D3D9HardwareOcclusionQuery::pullOcclusionQuery( unsigned int* NumOfFragments) 
{
	HRESULT hr;
	
	if( m_bHWOcclusionSupport )	// Make it fail silently if hardware occlusion isn't supported
	{
		hr = m_pD3DQuery->GetData( NumOfFragments, sizeof( NumOfFragments ), D3DGETDATA_FLUSH );

		if ( hr != S_OK ) 
			{
				return false; 
			}
			else 
			{ 
				m_uintPixelCount = *NumOfFragments; 
				return true; 
			}
	}
	else
	{
		m_uintPixelCount = 100000; // Fails quitlly if hardware occlusion is not supported - every object is visable
		return true;
	}
}

//------------------------------------------------------------------
// This version of pullOcclusionQuery cases the DX9 API/Driver to not flush all commands to the 3D card
// to allow a fast result from the query, but the batching of API calls to the card will be normal. 
// But the query wont be processed until the card recives the query in the nexr batch.
// Note: OpenGL dosn't use this flag at all so the application running OpenGL won't display any different behaviour.
//--
bool D3D9HardwareOcclusionQuery::pullOcclusionQuery( unsigned int* NumOfFragments, const HW_OCCLUSIONQUERY flag  ) 
{
	HRESULT hr;

	// TO DO: USE lockOpts= D3D9Mappings::get(options); instead of RS_OCCLUSIONQUERY enum

	if( m_bHWOcclusionSupport )	// Make it fail silently if hardware occlusion isn't supported
	{
	
		switch( flag )
		{
			case HWOCCLUSIONQUERY_FLUSH : 	
				hr = m_pD3DQuery->GetData( NumOfFragments, sizeof( NumOfFragments ), D3DGETDATA_FLUSH );
			break;

			case HWOCCLUSIONQUERY_NOFLUSH :	
				hr = m_pD3DQuery->GetData( NumOfFragments, sizeof( NumOfFragments ), 0 );
			break;
		};



		if ( hr != S_OK ) 
			{
				return false; 
			}
			else 
			{ 
				m_uintPixelCount = *NumOfFragments; 
				return true; 
			}
	}
	else 
	{
		m_uintPixelCount = 100000; // Fails quitlly if hardware occlusion is not supported - every object is visable
		return true;
	}
}




}
