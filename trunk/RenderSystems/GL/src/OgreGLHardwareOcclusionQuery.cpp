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

#include "OgreGLHardwareOcclusionQuery.h"
#include "OgreException.h"

namespace Ogre {

int GLHardwareOcclusionQuery::m_Skip = 0;

/**
  * This is a class that is the base class of the query class for 
  * hardware occlusion testing.
  *
  * @author Lee Sandberg email: lee@abcmedia.se
  */

/* Functions used;

glGenOcclusionQueriesNV_ptr( 1, m_uintQuery );
glDeleteOcclusionQueriesNV_ptr( 1, &m_uintQuery[0] );
glBeginOcclusionQueryNV_ptr( m_uintQuery[0] );
glEndOcclusionQueryNV_ptr();
glGetOcclusionQueryuivNV_ptr( m_uintQuery[0], GL_PIXEL_COUNT_NV, NumOfFragments );

  TO DO: change this to the new ARB functions...

*/


/**
  * Default object constructor
  * 
  */
GLHardwareOcclusionQuery::GLHardwareOcclusionQuery() 
{ 
	m_uintPixelCount = 0; 
	m_SkipCounter = 0;

	// Check for hardware occlusion support
    if( glDeleteOcclusionQueriesNV_ptr != 0 )	// This is a hack to see if hw occlusion is supported. pointer is 0 if it's not supported.
    {
		m_bHWOcclusionSupport = true;
	}
	else
	{
		m_bHWOcclusionSupport = false;
	}

	if( m_bHWOcclusionSupport )
	{
		glGenOcclusionQueriesNV_ptr( 1, m_uintQuery );	
	}
}


/**
  * Object destructor
  */
GLHardwareOcclusionQuery::~GLHardwareOcclusionQuery() 
{ 
	if( m_bHWOcclusionSupport )
	{
		glDeleteOcclusionQueriesNV_ptr( 1, &m_uintQuery[0] );  
	}	
}

//------------------------------------------------------------------
// Occlusion query functions (see base class documentation for this)
//--
void GLHardwareOcclusionQuery::beginOcclusionQuery() 
{ 
	if( m_bHWOcclusionSupport )	// Make it fail silently if hardware occlusion isn't supported
	{
		if  ( m_SkipCounter ==  m_Skip ) { m_SkipCounter = 0; };		// Counter starts at 0 again at m_Skip 
		if ( m_SkipCounter == 0 && m_uintPixelCount != 0 ) // New or none visable objects must allways be tested but visable objects can be skiped
		{
			glBeginOcclusionQueryNV_ptr( m_uintQuery[0] );
		}
	}
}
	
void GLHardwareOcclusionQuery::endOcclusionQuery() 
{ 
	if( m_bHWOcclusionSupport )	// Make it fail silently if hardware occlusion isn't supported
	{
		if( m_SkipCounter == 0 && m_uintPixelCount != 0 ) // New or none visable objects must allways be tested but visable objects can be skiped
		{
			glEndOcclusionQueryNV_ptr();
		}
		m_SkipCounter++;
	}
}

bool GLHardwareOcclusionQuery::pullOcclusionQuery( unsigned int * NumOfFragments) 
{
	if( m_bHWOcclusionSupport )	// Make it fail silently if hardware occlusion isn't supported
	{
		glGetOcclusionQueryuivNV_ptr( m_uintQuery[0], GL_PIXEL_COUNT_NV, NumOfFragments );
	} 
	else
	{
		*NumOfFragments = 100000;		// Fails quitlly -> every object tested is visable.
	}

	m_uintPixelCount = *NumOfFragments; 

	return true;
}

//------------------------------------------------------------------
// OpenGL dosn't use the flag, but to fulfil the abstract interface we need to include this function.
// Using this function in OpenGL mode simple works the same way as calling the other function without the flag parameter,
// but in DX9 it works differentlly, see notes in the DX9 implementation.
//--
bool GLHardwareOcclusionQuery::pullOcclusionQuery( unsigned int* NumOfFragments, const HW_OCCLUSIONQUERY flag  ) 
{
	if( m_bHWOcclusionSupport )	// Make it fail silently if hardware occlusion isn't supported
	{
		glGetOcclusionQueryuivNV_ptr( m_uintQuery[0], GL_PIXEL_COUNT_NV, NumOfFragments );
	}
	else
	{
		*NumOfFragments = 100000;		// Fails quitlly -> every object tested is visable.
	}

	m_uintPixelCount = *NumOfFragments; 
	
	return true;
}


}


