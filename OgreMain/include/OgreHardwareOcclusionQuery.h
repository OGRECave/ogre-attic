/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2002 The OGRE Team
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
#ifndef _HardwareOcclusionQuery__
#define _HardwareOcclusionQuery__

// Precompiler options
#include "OgrePrerequisites.h"

namespace Ogre {


/**
  * Hardware occlusion query flags
  */
typedef enum _OCCLUSIONQUERY 
{
	HWOCCLUSIONQUERY_FLUSH,   /** Direct3D uses this, but not OpenGL */
	HWOCCLUSIONQUERY_NOFLUSH, /** To decide if the driver should flush all batched API calls to serve an occlusion query faster. */
} HW_OCCLUSIONQUERY;

/**
  * This is a abstract class that that provides the interface for the query class for 
  * hardware occlusion.
  *
  * @author Lee Sandberg
  */
class _OgreExport HardwareOcclusionQuery
{
//----------------------------------------------------------------------
// Public methods
//--
public:
	/**
	  * Object public member functions
	  */

	/**
	  * Default object constructor
	  * 
	  */
	HardwareOcclusionQuery();

	/**
	  * Object destructor
	  */
	virtual ~HardwareOcclusionQuery();

	/**
	  * Starts the hardware occlusion query
	  * @Remarks	Simple usage: Create one or more OcclusionQuery object one per outstanding query or one per tested object 
	  *				OcclusionQuery* m_pOcclusionQuery;
	  *				createOcclusionQuery( &m_pOcclusionQuery );
	  *				In the rendering loop:
	  *				Draw all occluders
	  *				m_pOcclusionQuery->startOcclusionQuery();
	  *				Draw the polygons to be tested
	  *				m_pOcclusionQuery->endOcclusionQuery();
	  *
	  *				Results must be pulled using:
	  *				UINT	m_uintNumberOfPixelsVisable;
	  *				pullOcclusionQuery( &m_dwNumberOfPixelsVisable );
	  *				You may not get the result directlly after the first pass or frame.
	  *				Objects not visable must be tested every frame, visable objects may be tested less freqvent.
	  *			
	  */
	virtual void beginOcclusionQuery() = 0;

	/**
	  * Ends the hardware occlusion test
	  */
	virtual void endOcclusionQuery() = 0;

	/**
      * Pulls the hardware occlusion query too see if there is a result.
      * @retval NumOfFragments will get the resulting number of fragments.
      * @return True if success or false if not.
	  * @Remarks Hardware occlusion is an assyncronius process the result may take a frame or so.
	  *			one idea is to test pass1 and if not visable skip pass2. Also note that objects
	  *			not visable must be tested evrey frame. Visable objects don't need testing every frame.
	  *			Testing non visable objects can be don unlit, no texture with low LOD object.
	  *        0 will generate all the levels till 1x1. [default: 0]
      */
	virtual bool pullOcclusionQuery( unsigned int* NumOfFragments ) = 0;

	/**
      * Pulls the hardware occlusion query too see if there is a result.
      * @retval NumOfFragments will get the resulting number of fragments.
      * @return True if success or false if not.
	  * @Remarks In DX9 mode specifying OCCLUSIONQUERY_FLUSH as the flag, will case the driver to flush whatever API calls are batched.
	  * In OpenGL mode it makes no difference if you specify OCCLUSIONQUERY_FLUSH or OCCLUSIONQUERY_NOFLUSH.
      */
	virtual bool pullOcclusionQuery( unsigned int* NumOfFragments, const HW_OCCLUSIONQUERY flag  ) = 0;

	/**
	  * Let's you get the last pixel count with out doing the hardware occlusion test
	  * @return The last fragment count from the last test.
	  * Remarks This function won't give you new values, just the old value.
	  */
	virtual unsigned int getLastQuerysPixelcount() = 0;

	/**
	  *   
	  * @Remarks This function allows you to set how often the hardware occlusion really are sent to the driver
	  * if you set it to 0 every hw occlusion test is acctually made. If you set it to 1 only the half of your queries are sent 
	  * for all visable objects. 2 will result in 25% of all queries to acctualy be sent. 
	  * New and none visable objects will be tested all the time.
	  * This functionality is here because this class can keep track on visable and none visable objects for you.
	  * Once you you set the SkipRate for any hardware occlusion instance it effects all others.
	  */

	virtual void	setSkipRate( int skip ) = 0;
	virtual int		getSkipRate() = 0;

//----------------------------------------------------------------------
// Private members
//--
private:

};

}
#endif

