
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
http://www.gnu.org/copyleft/lgpl.html.
-----------------------------------------------------------------------------
*/

#include "OgreBillboard.h"

#include "OgreBillboardSet.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    Billboard::Billboard():
        mOwnDimensions(false),
		mPosition(Vector3::ZERO),
        mDirection(Vector3::ZERO),        
        mParentSet(0),
        mColour(ColourValue::White)
    {
    }
    //-----------------------------------------------------------------------
    Billboard::~Billboard()
    {
    }
    //-----------------------------------------------------------------------
    Billboard::Billboard(const Vector3& position, BillboardSet* owner, const ColourValue& colour)
    {
        mPosition = position;
        mOwnDimensions = false;
        mParentSet = owner;
        mColour = colour;
    }
    //-----------------------------------------------------------------------
    void Billboard::setPosition(const Vector3& position)
    {
        mPosition = position;
    }
    //-----------------------------------------------------------------------
    void Billboard::setPosition(Real x, Real y, Real z)
    {
        mPosition.x = x;
        mPosition.y = y;
        mPosition.z = z;
    }
    //-----------------------------------------------------------------------
    const Vector3& Billboard::getPosition(void) const
    {
        return mPosition;
    }
    //-----------------------------------------------------------------------
    void Billboard::setDimensions(Real width, Real height)
    {
        mOwnDimensions = true;
        mWidth = width;
        mHeight = height;
        mParentSet->_notifyBillboardResized();
    }
    //-----------------------------------------------------------------------
    bool Billboard::hasOwnDimensions(void) const
    {
        return mOwnDimensions;
    }
    //-----------------------------------------------------------------------
    void Billboard::_notifyOwner(BillboardSet* owner)
    {
        mParentSet = owner;
    }
    //-----------------------------------------------------------------------
    void Billboard::setColour(const ColourValue& colour)
    {
        mColour = colour;
    }
    //-----------------------------------------------------------------------
    const ColourValue& Billboard::getColour(void) const
    {
        return mColour;
    }




}

