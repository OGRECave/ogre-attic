/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2001 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/
#include "OgreColourValue.h"

#include "OgreBitwise.h"

namespace Ogre {

    ColourValue ColourValue::Black = ColourValue(0.0,0.0,0.0);
    ColourValue ColourValue::White = ColourValue(1.0,1.0,1.0);
    ColourValue ColourValue::Red = ColourValue(1.0,0.0,0.0);
    ColourValue ColourValue::Green = ColourValue(0.0,1.0,0.0);
    ColourValue ColourValue::Blue = ColourValue(0.0,0.0,1.0);

    //---------------------------------------------------------------------
    inline ColourValue::ColourValue(float red, float green, float blue, float alpha)
    {
        r = red;
        g = green;
        b = blue;
        a = alpha;
    }
    //---------------------------------------------------------------------
    unsigned long ColourValue::getAsLongRGBA(void) const
    {
        unsigned char val8;
        unsigned long val32 = 0;

        // Convert to 32bit pattern
        // (RGBA = 8888)

        // Red
        val8 = (unsigned char)(r * 255);
        val32 = val8 << 24;

        // Green
        val8 = (unsigned char)(g * 255);
        val32 += val8 << 16;

        // Blue
        val8 = (unsigned char)(b * 255);
        val32 += val8 << 8;

        // Alpha
        val8 = (unsigned char)(a * 255);
        val32 += val8;

        return val32;
    }
    //---------------------------------------------------------------------
    unsigned long ColourValue::getAsLongARGB(void) const
    {
        unsigned char val8;
        unsigned long val32 = 0;

        // Convert to 32bit pattern
        // (ARGB = 8888)

        // Alpha
        val8 = (unsigned char)(a * 255);
        val32 = val8 << 24;

        // Red
        val8 = (unsigned char)(r * 255);
        val32 += val8 << 16;

        // Green
        val8 = (unsigned char)(g * 255);
        val32 += val8 << 8;

        // Blue
        val8 = (unsigned char)(b * 255);
        val32 += val8;


        return val32;
    }
    //---------------------------------------------------------------------
    ABGR ColourValue::getAsLongABGR(void) const
    {
        unsigned char val8;
        unsigned long val32 = 0;

        // Convert to 32bit pattern
        // (ABRG = 8888)

        // Alpha
        val8 = (unsigned char)(a * 255);
        val32 = val8 << 24;

        // Blue
        val8 = (unsigned char)(b * 255);
        val32 += val8 << 16;

        // Green
        val8 = (unsigned char)(g * 255);
        val32 += val8 << 8;

        // Red
        val8 = (unsigned char)(r * 255);
        val32 += val8;


        return val32;
    }
    //---------------------------------------------------------------------
    bool ColourValue::operator==(const ColourValue& rhs) const
    {
        return (r == rhs.r &&
            g == rhs.g &&
            b == rhs.b &&
            a == rhs.a);
    }
    //---------------------------------------------------------------------
    bool ColourValue::operator!=(const ColourValue& rhs) const
    {
        return !(*this == rhs);
    }

}

