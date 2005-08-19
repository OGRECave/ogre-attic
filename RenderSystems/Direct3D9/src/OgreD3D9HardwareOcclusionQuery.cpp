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
#include "OgreD3D9HardwareOcclusionQuery.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreException.h"

namespace Ogre {

	/**
	* This is a class that is the DirectX9 implementation of 
	* hardware occlusion testing.
	*
	* @author Lee Sandberg
	*
	* Updated on 12/7/2004 by Chris McGuirk
	* Updated on 4/8/2005 by Tuan Kuranes email: tuan.kuranes@free.fr
	*/

	/**
	* Default object constructor
	*/
	D3D9HardwareOcclusionQuery::D3D9HardwareOcclusionQuery( IDirect3DDevice9* pD3DDevice ) 
	{ 
		mpDevice = pD3DDevice;
		mPixelCount = 0; 
		mSkipCounter = 0;
		mSkipInterval = 0;
		mHasOcclusionSupport = false;

		// create the occlusion query
		HRESULT hr = mpDevice->CreateQuery(D3DQUERYTYPE_OCCLUSION, &mpQuery);

		if ( hr != D3D_OK ) 
		{
			mHasOcclusionSupport = false;
		}
		else 
		{
			mHasOcclusionSupport = true;
		}
	}

	/**
	* Object destructor
	*/
	D3D9HardwareOcclusionQuery::~D3D9HardwareOcclusionQuery() 
	{ 
		SAFE_RELEASE(mpQuery); 
	}

	//------------------------------------------------------------------
	// Occlusion query functions (see base class documentation for this)
	//--
	void D3D9HardwareOcclusionQuery::beginOcclusionQuery() 
	{
		// Make it fail silently if hardware occlusion isn't supported
		if(mHasOcclusionSupport)
		{
			// Counter starts at 0 again at mSkipInterval 
			if(mSkipCounter == mSkipInterval) 
			{ 
				mSkipCounter = 0; 
			}

			if (mSkipCounter == 0)
			{
				mpQuery->Issue(D3DISSUE_BEGIN); 
			}
		}
	}

	void D3D9HardwareOcclusionQuery::endOcclusionQuery() 
	{ 
		// Make it fail silently if hardware occlusion isn't supported
		if(mHasOcclusionSupport)
		{
			if(mSkipCounter == 0)
			{
				mpQuery->Issue(D3DISSUE_END); 
                mIsQueryResultStillOutstanding = true;
			}

			// The skip counter is increased
			mSkipCounter++; 
		}
	}

	//------------------------------------------------------------------
	// This version of pullOcclusionQuery causes the DX9 API/Driver to not flush all commands to the 3D card
	// to allow a fast result from the query, but the batching of API calls to the card will be normal. 
	// But the query wont be processed until the card receives the query in the next batch.
	// Note: OpenGL dosn't use this flag at all so the application running OpenGL won't display any different behaviour.
	//--
	bool D3D9HardwareOcclusionQuery::pullOcclusionQuery( unsigned int* NumOfFragments, const HW_OCCLUSIONQUERY flag  ) 
	{

		// Make it fail silently if hardware occlusion isn't supported
		if(mHasOcclusionSupport)
		{
            // in case you didn't check if query arrived and want the result now.
            if (mIsQueryResultStillOutstanding)
            {
			    DWORD d3dFlags = (flag == HWOCCLUSIONQUERY_FLUSH) ? D3DGETDATA_FLUSH : 0;

		        HRESULT hr;

			    // run until success (http://www.gamedev.net/reference/programming/features/occlusionculling/page2.asp)
			    while (hr = mpQuery->GetData( NumOfFragments, sizeof( *NumOfFragments ), d3dFlags) == S_FALSE)
                    ;
			    mPixelCount = *NumOfFragments;
            }
            else
            {
                // we already stored result from last frames.
                 *NumOfFragments = mPixelCount;
            }
		}
		else 
		{
			// fail silently if not supported, assume visible i suppose
			mPixelCount = 100000;
		}

		return true;
	}
    //------------------------------------------------------------------
    bool D3D9HardwareOcclusionQuery::isStillOutstanding(void)
    {
    if(mHasOcclusionSupport)
    {
        // in case you already asked for this query
        if (!mIsQueryResultStillOutstanding)
            return false;

        DWORD d3dFlags = 0;// not asking for flushed queries here.


        if (mpQuery->GetData( &mPixelCount, sizeof( mPixelCount ), d3dFlags) == S_FALSE)
            return true;

        mIsQueryResultStillOutstanding = false;
        return false;
             
    }
    else
    {
        return false;
    }
       
    } 

}
