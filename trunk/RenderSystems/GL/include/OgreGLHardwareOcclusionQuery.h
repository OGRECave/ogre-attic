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

/*
The nVidia occlusion query extension is defined in glext.h so you don't 
need anything else. You do need to look up the function, we provide a 
GLSupport class to do this, which has platform implementations for 
getProcAddress. Check the way that extensions like glActiveTextureARB are 
initialised and used in glRenderSystem and copy what is done there.



  To do: fix so dx7 and DX9 checks and flags if HW Occlusion is supported
  See the openGl dito for ideas what to do.

*/



//GL_ActiveTextureARB_Func* glActiveTextureARB_ptr = (GL_ActiveTextureARB_Func)mGLSupport->getProcAddress("glActiveTextureARB");

#ifndef __GLHARDWAREOCCLUSIONQUERY_H__
#define __GLHARDWAREOCCLUSIONQUERY_H__

#include "OgreGLPrerequisites.h"
#include "OgreHardwareOcclusionQuery.h"


namespace Ogre { 


// If you use multiple rendering passes you can test only the first pass and all other passes don't have to be rendered 
// if the first pass result has too few pixels visible.

// Be sure to render all occluder first and whats out so the RenderQue don't switch places on 
// the occluding objects and the tested objects because it thinks it's more effective..


/**
  * This is a class that is the base class of the query class for 
  * hardware occlusion.
  *
  * @author Lee Sandberg email: lee@abcmedia.se
  * Updated on 4/8/2005 by Tuan Kuranes email: tuan.kuranes@free.fr
  */

class GLHardwareOcclusionQuery : public HardwareOcclusionQuery
{
//----------------------------------------------------------------------
// Public methods
//--
public:
	/**
	  * Default object constructor
	  * 
	  */
	GLHardwareOcclusionQuery();
	/**
	  * Object destructor
	  */
	~GLHardwareOcclusionQuery();

	//------------------------------------------------------------------
	// Occlusion query functions (see base class documentation for this)
	//--

	void beginOcclusionQuery();
	void endOcclusionQuery();
	bool pullOcclusionQuery( unsigned int* NumOfFragments, const HW_OCCLUSIONQUERY flag  ); 
	unsigned int getLastQuerysPixelcount() { return mPixelCount; }
	bool isStillOutstanding(void);

	// This functions are optional, it's a simple filter that simply skips some hardware occlusion tests on visible objects only
	// It's easy to use if you don't have to keep track on which objects are visible (can be skipped) and what objects arn't visible..
	// (None visible objects and object you introduce for the first time have always to be tested although the cheapest possible 
	// LOD (Level Of Detail) mesh and material wise). 

	/**
	  *   
	  * Remarks This function allows you to set how often the hardware occlusion really sent to the driver
	  * if you set it to 0 every hardware occlusion test is actually made. If you set it to 2 only 50% of your queries are sent. 
	  * for all visible objects. 3 will result in 33% of all queries to actually be sent and so on. 
	  * New and none visible objects will be tested all the time.
	  * This functionality is here because this class can keep track on visible and none visible objects for you.
	  * Once you you set the SkipRate for any hardware occlusion instance it effects all others.
	  */

	void setSkipRate( int skip ) { mSkipInterval = skip; }		// Using 2 only 50 % of the tests are actually made and 3 results in only 33% of the tests. So on.
	int	 getSkipRate() { return mSkipInterval; } 


//----------------------------------------------------------------------
// Protected members
//--
protected:

	unsigned int	mPixelCount;
	GLuint			mQueryID;
	bool			m_bOcclusionQuery;
	int				mSkipCounter;
	int				mSkipInterval;
	bool			mHasOcclusionSupport;
};

}

#endif 

