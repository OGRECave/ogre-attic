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
#ifndef _D3DTexture_H__
#define _D3DTexture_H__

#include "OgreD3D7Prerequisites.h"
#include "OgreTexture.h"
#include <ddraw.h>
#include <d3d.h>

namespace Ogre {

    /** Direct3D-specific Texture resource implementation. */
    class D3DTexture : public Texture
    {
    public:
        // Constructor, called from D3DTextureManager
        D3DTexture(String name, LPDIRECT3DDEVICE7 lpDirect3dDevice);
        virtual ~D3DTexture();
        void load(void);
        void loadImage( const Image &img );
        void blitToTexture( const Image &src, unsigned uStartX, unsigned uStartY );        
        void unload(void);

        // D3D-specific
        LPDIRECTDRAWSURFACE7 getDDSurface(void);
    protected:
        LPDIRECT3DDEVICE7 mD3DDevice;
        LPDIRECTDRAWSURFACE7 mSurface;    // Surface of the (first) device-specific texture

        void loadFromBMP(void);
        void createSurfaces(void);
        void releaseSurfaces(void);
        void copyBitmapToSurface(HBITMAP hBitmap);
        void copyMemoryToSurface(unsigned char* pBuffer);

        void generateMipMaps(void);

    };
}

#endif
