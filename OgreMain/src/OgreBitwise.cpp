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
#include "OgreBitwise.h"

namespace Ogre {
    FORCEINLINE int Bitwise::getBitShift(DWORD mask)
    {
        if (mask == 0x0 )
            return 0;

        if( mask & 0x1 )
            return 0;
        if( mask & 0x2 )
            return 1;
        if( mask & 0x4 )
            return 2;
        if( mask & 0x8 )
            return 3;
        if( mask & 0x10 )
            return 4;
        if( mask & 0x20 )
            return 5;
        if( mask & 0x40 )
            return 6;
        if( mask & 0x80 )
            return 7;
        if( mask & 0x100 )
            return 8;
        if( mask & 0x200 )
            return 9;
        if( mask & 0x400 )
            return 10;
        if( mask & 0x800 )
            return 11;
        if( mask & 0x1000 )
            return 12;
        if( mask & 0x2000 )
            return 13;
        if( mask & 0x4000 )
            return 14;
        if( mask & 0x8000 )
            return 15;
        if( mask & 0x10000 )
            return 16;
        if( mask & 0x20000 )
            return 17;
        if( mask & 0x40000 )
            return 18;
        if( mask & 0x80000 )
            return 19;
        if( mask & 0x100000 )
            return 20;
        if( mask & 0x200000 )
            return 21;
        if( mask & 0x400000 )
            return 22;
        if( mask & 0x800000 )
            return 23;
        if( mask & 0x1000000 )
            return 24;
        if( mask & 0x2000000 )
            return 25;
        if( mask & 0x4000000 )
            return 26;
        if( mask & 0x8000000 )
            return 27;
        if( mask & 0x10000000 )
            return 28;
        if( mask & 0x20000000 )
            return 29;
        if( mask & 0x40000000 )
            return 30;
        if( mask & 0x80000000 )
            return 31;

        return -1;
    }

    // ConvertBitPattern
    void Bitwise::convertBitPattern(
        void* srcValue,  void* srcBitMask,  int srcBitCount,
        void* destValue, void* destBitMask, int destBitCount )
    {

        DWORD src, dest, srcMask, destMask;
        int srcBitShift, destBitShift;
        long srcMax, destMax;
        //static DWORD *pdwSrcVal, *pdwSrcBMask;
        //static WORD *pwSrcVal, *pwSrcBMask;

        // For simplicity, all values are placed
        // In 32-bit patterns for manipulation

        // init
        src = dest = srcMask = destMask = 0;

        //
        // Retrieve info
        //
        // Get source value & mask
        if( srcBitCount == 24 || srcBitCount == 32 )
        {
            src = *((DWORD*)srcValue);
            srcMask = *((DWORD*)srcBitMask);
        }
        else if( srcBitCount == 16 )
        {
            src = *((WORD*)srcValue);
            srcMask = *((WORD*)srcBitMask);
        }
        else if( srcBitCount == 8 )
        {
            src = *((BYTE*)srcValue);
            srcMask = *((BYTE*)srcBitMask);
        }

        // Get dest mask
        if (destBitCount == 24 || destBitCount == 32)
            destMask = *((DWORD*)destBitMask);
        else if (destBitCount == 16)
            destMask = *((WORD*)destBitMask);
        else if (destBitCount == 8)
            destMask = *((BYTE*)destBitMask);

        // Mask off irrelevant source value bits (if any)
        src = src & srcMask;

        // Shift source down to bottom of DWORD
        srcBitShift = getBitShift(srcMask);
        src = src >> srcBitShift;

        // Get max value possible in source from srcMask
        srcMax = srcMask >> srcBitShift;

        // Get max avaiable in dest
        destBitShift = getBitShift(destMask);
        destMax = destMask >> destBitShift;

        // Scale source value into destination, and shift back
        dest = (src * destMax) / srcMax;
        dest = dest << destBitShift;

        // Finally, assign to value based on dest pointer type
        if (destBitCount == 24 || destBitCount == 32)
            *((DWORD*)destValue) = dest;
        else if (destBitCount == 16)
            *((WORD*)destValue) = (WORD)dest;
        else if (destBitCount == 8)
            *((BYTE*)destValue) = (BYTE)dest;
    }
};
