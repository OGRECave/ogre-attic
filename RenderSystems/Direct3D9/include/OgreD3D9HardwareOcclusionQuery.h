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

#ifndef _D3D9HARWAREOCCLUSIONQUERY_H__
#define _D3D9HARWAREOCCLUSIONQUERY_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreHardwareOcclusionQuery.h"


namespace Ogre {

	// If you use multiple rendering passes you can test only the first pass and all other passes don't have to be rendered 
	// if the first pass resultet has too few pixels visible.

	// Be sure to render all occlluder first and whats out so the RenderQue don't switch places on 
	// the occluding objects and the tested objects because it thinks it's more effective..

	/**
	* This is a class that is the DirectX9 implementation of 
	* hardware occlusion testing.
	*
	* @author Lee Sandberg, email lee@abcmedia.se
	*
	* Updated on 12/7/2004 by Chris McGuirk
	*/
	class D3D9HardwareOcclusionQuery : public HardwareOcclusionQuery
	{
		//----------------------------------------------------------------------
		// Public methods
		//--
	public:

		/**
		* Default object constructor
		* 
		*/
		D3D9HardwareOcclusionQuery( IDirect3DDevice9* pD3DDevice );

		/**
		* Object destructor
		*/
		~D3D9HardwareOcclusionQuery();

		//------------------------------------------------------------------
		// Occlusion query functions (see base class documentation for this)
		//--

		void beginOcclusionQuery();	
		void endOcclusionQuery();
		bool pullOcclusionQuery( unsigned int* NumOfFragments, const HW_OCCLUSIONQUERY flag = HWOCCLUSIONQUERY_FLUSH);
		unsigned int getLastQuerysPixelcount() { return mPixelCount; }

		// These functions are optional, it's a simple filter that simply skips some hardware occlusion tests on visible objects only
		// It's easy to use if you don't have to keep track of which objects are visible (can be skipped) and what objects arn't visible..
		// (None visible objects and object you introduce for the first time have always to be tested although the cheapest possible 
		// LOD (Level Of Detail) mesh and material-wise). 

		/**
		*   
		* Remarks This function allows you to set how often the hardware occlusion query is sent to the driver
		* if you set it to 0 every hw occlusion test is acctually made. If you set it to 1 only the half of your queries are sent 
		* 2 will result in 25% of all queries to acctualy be sent. 
		* This functionality is here because this class can keep track on visible and none visible objects for you.
		* Once you you set the SkipRate for any hardware occlusion instance it effects all others.
		*/

		void setSkipRate( int skip ) { mSkipInterval = skip; }			// Using 2 only 50 % of the tests are actully made and 3 results in only 33% of the tests. So on.
		int	 getSkipRate() { return mSkipInterval; } 

		//----------------------------------------------------------------------
		// Protected members
		//--
	protected:

		unsigned int		mPixelCount;		
		IDirect3DQuery9*	mpQuery;
		IDirect3DDevice9*   mpDevice;
		int					mSkipCounter;
		int					mSkipInterval;
		bool				mHasOcclusionSupport;
	};


}


#endif
