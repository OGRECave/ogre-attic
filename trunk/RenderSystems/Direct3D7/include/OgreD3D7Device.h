/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright � 2000-2002 The OGRE Team
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
#define D3D_OVERLOADS

#ifndef __D3DDEVICE_H_
#define __D3DDEVICE_H_

// Precompiler options
#include "OgreD3D7Prerequisites.h"

namespace Ogre
{
    /** Encapsulates a Direct3D Device. Utility class for D3DRenderSystem. */
    class _OgreD3DExport D3DDevice
    {


    private:
        String mDeviceName;
        String mDeviceDescription;
        D3DDEVICEDESC7 mD3DDeviceDesc;
        bool mIsHardwareAccelerated;
        bool mNeedsZBuffer;
        std::vector<DDPIXELFORMAT> mDepthBufferFormats;
        ushort mStencilBufferDepth;



        LPDIRECT3D7 lpD3D;
        D3DVIEWPORT7 mViewport;
        D3DRECT rcViewportRect;

        DDDriver* mParentDriver;

        // Private functions
        void createViewport(void);
        void setViewportSize(void);




    public:
        // Constructors
        D3DDevice(); // Default
        D3DDevice(const D3DDevice &ob); // Copy
        D3DDevice(LPDIRECT3D7 lpDirect3D, LPSTR lpDeviceDesc, LPSTR lpDeviceName,
                    LPD3DDEVICEDESC7 lpD3DDeviceDesc);
        ~D3DDevice();
        void Cleanup(void);
        void logCaps(void);

        /** Creates a new per-rendering surface device */
        LPDIRECT3DDEVICE7 createDevice(LPDIRECTDRAWSURFACE7 renderTarget);
        LPDIRECT3D7 getID3D(void);

        // Overloaded operator =
        D3DDevice operator=(const D3DDevice &orig);

        // Information accessors
        String DeviceName(void);
        String DeviceDescription(void);
        bool HardwareAccelerated(void);
        bool NeedsZBuffer(void);

        void createDepthBuffer(LPDIRECTDRAWSURFACE7 renderTarget);


        void setViewMatrix(D3DMATRIX *mat);
        void setProjectionMatrix(D3DMATRIX *mat);
        void setWorldMatrix(D3DMATRIX *mat);

        void beginScene(void);
        void endScene(void);
        void setAmbientLight(float r, float g, float b);



        friend static HRESULT CALLBACK EnumZBuffersCallback(DDPIXELFORMAT* pddpf,
            VOID* pFormats);


        // Capability access
        bool CanMipMap(void);
        bool CanBilinearFilter(void);
        bool CanTrilinearFilter(void);
        int RenderBitDepth(void);
        int ZBufferBitDepth(void);
        bool CanHWTransformAndLight(void);
        int MaxSinglePassTextureLayers(void);
        ushort StencilBufferBitDepth(void);






    };
} // Namespace Ogre
#endif
