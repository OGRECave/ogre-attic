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
#include "OgreWin32Timer.h"

namespace Ogre {
    //-------------------------------------------------------------------------
    void Win32Timer::reset()
    {
        Timer::reset();
        QueryPerformanceFrequency(&mFrequency);
        QueryPerformanceCounter(&mStartTime);
    }
    //-------------------------------------------------------------------------
    unsigned long Win32Timer::getMilliseconds()
    {
        LARGE_INTEGER curTime;
        LONGLONG newTicks;

        QueryPerformanceCounter(&curTime);

        newTicks = (curTime.QuadPart - mStartTime.QuadPart);
        // Scale by 1000 in order to get millisecond precision
        newTicks *= 1000;
        newTicks /= mFrequency.QuadPart;

        return (unsigned long)newTicks;
	}
    //-------------------------------------------------------------------------
	unsigned long Win32Timer::getMicroseconds()
	{
		LARGE_INTEGER curTime;
		LONGLONG newTicks;

		QueryPerformanceCounter(&curTime);

		newTicks = (curTime.QuadPart - mStartTime.QuadPart);
		// Scale by 1000000 in order to get microsecond precision
		newTicks *= (Real)1000000.0/(Real)mFrequency.QuadPart;

		return (unsigned long)newTicks;
	}

} 