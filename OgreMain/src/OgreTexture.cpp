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

#include "OgreTexture.h"

BEGIN_OGRE_NAMESPACE

void Texture::applyGamma( uchar *p, int size, int bpp )
{
    if( mGamma == 1.0f )
        return;

    float factor = mGamma;

    //NB only 24/32-bit supported
    if( bpp != 24 && bpp != 32 ) return;

    int stride = bpp >> 3;

    for(int i = 0; i < size/stride; i++, p+=stride) 
    {
        float r, g, b;

        r = (float)p[0];
        g = (float)p[1];
        b = (float)p[2];

        r = r * factor / 255.0f;
        g = g * factor / 255.0f;
        b = b * factor / 255.0f;

        float scale = 1.0f, tmp;

        if( r > 1.0f && (tmp=(1.0f/r)) < scale )
            scale = tmp;
        if( g > 1.0f && (tmp=(1.0f/g)) < scale )
            scale = tmp;
        if( b > 1.0f && (tmp=(1.0f/b)) < scale )
            scale = tmp;

        scale *= 255.0f;

        r *= scale; g *= scale; b *= scale;

        p[0] = (uchar)r;
        p[1] = (uchar)g;
        p[2] = (uchar)b;
    }
}

END_OGRE_NAMESPACE
