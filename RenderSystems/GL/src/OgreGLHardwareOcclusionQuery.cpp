/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright (c) 2000-2005 The OGRE Team
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

/**
  * This is a class that is the base class of the query class for 
  * hardware occlusion testing.
  *
  * @author Lee Sandberg email: lee@abcmedia.se
  *
  * Updated on 12/7/2004 by Chris McGuirk
  * - Implemented ARB_occlusion_query
  */

/**
  * Default object constructor
  * 
  */
GLHardwareOcclusionQuery::GLHardwareOcclusionQuery() 
{ 
	mPixelCount = 0; 
	mSkipCounter = 0;
	mSkipInterval = 0;

	// Check for hardware occlusion support
	// This is a hack to see if hw occlusion is supported. pointer is 0 if it's not supported.
    if(glGenQueriesARB_ptr != 0)
    {
		mHasOcclusionSupport = true;
	}
	else
	{
		mHasOcclusionSupport = false;
	}

	if(mHasOcclusionSupport)
	{
		glGenQueriesARB_ptr(1, &mQueryID );	
	}
}

/**
  * Object destructor
  */
GLHardwareOcclusionQuery::~GLHardwareOcclusionQuery() 
{ 
	if( mHasOcclusionSupport )
	{
		glDeleteQueriesARB_ptr(1, &mQueryID);  
	}	
}

//------------------------------------------------------------------
// Occlusion query functions (see base class documentation for this)
//--
void GLHardwareOcclusionQuery::beginOcclusionQuery() 
{ 
	// Make it fail silently if hardware occlusion isn't supported
	if(mHasOcclusionSupport)
	{
		// Counter starts at 0 again at mSkipInterval 
		if(mSkipCounter == mSkipInterval) 
		{ 
			mSkipCounter = 0; 
		}

		if ( mSkipCounter == 0)
		{
			glBeginQueryARB_ptr(GL_SAMPLES_PASSED_ARB, mQueryID);
		}
	}
}
	
void GLHardwareOcclusionQuery::endOcclusionQuery() 
{ 
	// Make it fail silently if hardware occlusion isn't supported
	if(mHasOcclusionSupport)
	{
		if( mSkipCounter == 0)
		{
			glEndQueryARB_ptr(GL_SAMPLES_PASSED_ARB);
		}

		mSkipCounter++;
	}
}

//------------------------------------------------------------------
// OpenGL dosn't use the flag paramter.
//------------------------------------------------------------------
bool GLHardwareOcclusionQuery::pullOcclusionQuery( unsigned int* NumOfFragments, const HW_OCCLUSIONQUERY flag  ) 
{
	if( mHasOcclusionSupport )	// Make it fail silently if hardware occlusion isn't supported
	{
		glGetQueryObjectuivARB_ptr(mQueryID, GL_QUERY_RESULT_ARB, NumOfFragments);
	}
	else
	{
		*NumOfFragments = 100000;		// Fails quitlly -> every object tested is visable.
	}

	mPixelCount = *NumOfFragments; 
	
	return true;
}


}


