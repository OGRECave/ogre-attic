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
#include "OgreStableHeaders.h"

#include "OgreController.h"

#include "OgreLogManager.h"

namespace Ogre
{
    //-----------------------------------------------------------------------
    Controller::Controller(SharedPtr<ControllerValue> src, SharedPtr<ControllerValue> dest, SharedPtr<ControllerFunction> func)
		: mSource(src), mDest(dest), mFunc(func)
    {
		mEnabled = true;
    }
    //-----------------------------------------------------------------------
    Controller::~Controller()
    {
    }
    //-----------------------------------------------------------------------
    void Controller::setSource(SharedPtr<ControllerValue> src)
    {
        mSource = src;
    }
    //-----------------------------------------------------------------------
    SharedPtr<ControllerValue> Controller::getSource(void)
    {
        return mSource;
    }
    //-----------------------------------------------------------------------
    void Controller::setDestination(SharedPtr<ControllerValue> dest)
    {
        mDest = dest;
    }
    //-----------------------------------------------------------------------
    SharedPtr<ControllerValue> Controller::getDestination(void)
    {
        return mDest;
    }
    //-----------------------------------------------------------------------
    void Controller::setFunction(SharedPtr<ControllerFunction> func)
    {
        mFunc = func;
    }
    //-----------------------------------------------------------------------
    SharedPtr<ControllerFunction> Controller::getFunction(void)
    {
        return mFunc;
    }
    //-----------------------------------------------------------------------
    void Controller::setEnabled(bool enabled)
	{
		mEnabled = enabled;
	}
    //-----------------------------------------------------------------------
	bool Controller::getEnabled(void) const
	{
		return mEnabled;
	}

    //-----------------------------------------------------------------------
    void Controller::update(void)
    {
		if(mEnabled)
			mDest->setValue(mFunc->calculate(mSource->getValue()));
    }
}
