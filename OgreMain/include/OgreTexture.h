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
#ifndef _Texture_H__
#define _Texture_H__

#include "OgrePrerequisites.h"

#include "OgreResource.h"

namespace Ogre {

    /** Abstract class representing a Texture resource.
        @return
            The actual concrete subclass which will exist for a texture
            is dependent on the rendering system in use (Direct3D, OpenGL etc).
            This class represents the commonalities, and is the one 'used'
            by programmers even though the real implementation could be
            different in reality. Texture objects are created through
            the 'create' method of the TextureManager concrete subclass.
     */
    class _OgreExport Texture : public Resource
    {
    public:

        /** Gets the number of mipmaps to be used for this texture.
        */
        unsigned short getNumMipMaps(void) {return mNumMipMaps;}

        /** Sets the number of mipmaps to be used for this texture.
            @note
                Must be set before calling any 'load' method.
        */
        void setNumMipMaps(unsigned short num) {mNumMipMaps = num;}

        /** Returns the gamma adjustment factor applied to this texture.
        */
        float getGamma(void) { return mGamma; }

        /** Sets the gamma adjustment factor applied to this texture.
            @note
                Must be called before any 'load' method.
        */
        void setGamma(float g) { mGamma = g; }

        unsigned int getHeight(void) { return mHeight; }
        unsigned int getWidth(void) { return mHeight; }

        void enable32Bit(bool setting = true) {setting ? mFinalBpp = 32 : mFinalBpp = 16;}

        /** Load a texture from a raw 24-bit RGB stream */
        virtual void loadRawRGB(void *buffer, int width, int height) = 0;
        /** Load a texture from a raw 32-bit RGBA stream */
        virtual void loadRawRGBA( void *buffer, int width, int height ) = 0;

    protected:
        // NB: No indexed colour support - deliberately
        unsigned long mHeight;
        unsigned long mWidth;

        unsigned short mNumMipMaps;
        float mGamma;

        unsigned short mSrcBpp;
        unsigned long mSrcWidth, mSrcHeight;
        unsigned short mFinalBpp;
        bool mHasAlpha;


        /** Internal method for gamma adjustment.
            Basic algo taken from Titan engine, copyright (c) 2000 Ignacio Castano Iguado
        */
        void applyGamma(unsigned char* p, int size, int bpp)
        {
            if (mGamma == 1.0f) return;

            float factor = mGamma;

            //NB only 24/32-bit supported
            if (bpp != 24 && bpp != 32) return;

            int stride = bpp >> 3;

            for(int i = 0; i < size/stride; i++, p+=stride) {
                float r,g,b;
                r = (float) p[0];
                g = (float) p[1];
                b = (float) p[2];
                r = r * factor / 255.0f;
                g = g * factor / 255.0f;
                b = b * factor / 255.0f;
                float scale=1.0f,tmp;
                if(r>1.0f && (tmp=(1.0f/r))<scale) scale=tmp;
                if(g>1.0f && (tmp=(1.0f/g))<scale) scale=tmp;
                if(b>1.0f && (tmp=(1.0f/b))<scale) scale=tmp;
                scale*=255.0f;
                r*=scale;
                g*=scale;
                b*=scale;
                p[0]=(unsigned char)r;
                p[1]=(unsigned char)g;
                p[2]=(unsigned char)b;
            }
        }


        // Inherits name, size values etc from Resource

        // No concrete representation
    };

}


#endif
