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
  * Updated on 13/9/2005 by Tuan Kuranes email: tuan.kuranes@free.fr
  */
//------------------------------------------------------------------
/**
  * Default object constructor
  * 
  */
GLHardwareOcclusionQuery::GLHardwareOcclusionQuery() 
{ 
	// Check for hardware occlusion support
	// This is a hack to see if hw occlusion is supported. pointer is 0 if it's not supported.
    if (glGenQueriesARB == 0)
    {
        OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
                    "Cannot allocate a Hardware query. This video card doesn't supports it, sorry.", 
                    "GLHardwareOcclusionQuery::GLHardwareOcclusionQuery" );
    }
    glGenQueriesARB(1, &mQueryID );	
	
}
//------------------------------------------------------------------
/**
  * Object destructor
  */
GLHardwareOcclusionQuery::~GLHardwareOcclusionQuery() 
{ 
	glDeleteQueriesARB(1, &mQueryID);  
}
//------------------------------------------------------------------
void GLHardwareOcclusionQuery::beginOcclusionQuery() 
{ 
	glBeginQueryARB(GL_SAMPLES_PASSED_ARB, mQueryID);
}
//------------------------------------------------------------------
void GLHardwareOcclusionQuery::endOcclusionQuery() 
{ 
    glEndQueryARB(GL_SAMPLES_PASSED_ARB);
}
//------------------------------------------------------------------
bool GLHardwareOcclusionQuery::pullOcclusionQuery( unsigned int* NumOfFragments ) 
{
	glGetQueryObjectuivARB(mQueryID, GL_QUERY_RESULT_ARB, (GLuint*)NumOfFragments);
	mPixelCount = *NumOfFragments;
	return true;
}
//------------------------------------------------------------------
bool GLHardwareOcclusionQuery::isStillOutstanding(void)
{    
      GLuint available;

      glGetQueryObjectuivARB(mQueryID, GL_QUERY_RESULT_AVAILABLE_ARB, &available);
      return !(available == GL_TRUE);  
} 

}


