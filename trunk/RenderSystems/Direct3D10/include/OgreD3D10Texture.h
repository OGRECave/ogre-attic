/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef __D3D8TEXTURE_H__
#define __D3D8TEXTURE_H__

#include "OgreD3D10Prerequisites.h"
#include "OgreTexture.h"
#include "OgreRenderTexture.h"
#include "OgreImage.h"
#include "OgreException.h"
#include "OgreD3D10HardwarePixelBuffer.h"

#include "OgreNoMemoryMacros.h"
#include <d3d10.h>
#include <d3dx10.h>
#include <dxerr.h>
#include "OgreMemoryMacros.h"

namespace Ogre {
	class D3D10Texture : public Texture
	{
	protected:
		/// D3DDevice pointer
		ID3D10Device		*mpDev;		
		/// 1D/2D normal texture pointer
		ID3D10Texture1D		*mp1DTex;	
		
		ID3D10Texture2D		*mpNormTex;	
		/// cubic texture pointer
		ID3D10Texture2D	*mpCubeTex;	
        /// Volume texture
        ID3D10Texture3D *mpVolumeTex;
        /// actual texture pointer
		ID3D10Resource	*mpTex;		

		/// cube texture individual face names
		String							mCubeFaceNames[6];
		/// device creation parameters
//		D3DDEVICE_CREATION_PARAMETERS	mDevCreParams;
		/// back buffer pixel format
		DXGI_FORMAT						mBBPixelFormat;
		/// The memory pool being used
//		D3DPOOL							mD3DPool;
		/// device capabilities pointer
//		D3DCAPS9						mDevCaps;
		// Dynamic textures?
		bool                            mDynamicTextures;
		/// Vector of pointers to subsurfaces
		typedef std::vector<HardwarePixelBufferSharedPtr> SurfaceList;
		SurfaceList						mSurfaceList;
	
        /// Initialise the device and get formats
        void _initDevice(void);
		/// internal method, load a cube texture
		void _loadCubeTex();
		/// internal method, load a normal texture
		void _loadNormTex();
		/// internal method, load a 1D texture
		void _load1DTex();
		/// internal method, load a volume texture
		void _loadVolumeTex();

		/// internal method, create a blank normal 1D texture
		void _create1DTex();
		/// internal method, create a blank normal 2D texture
		void _createNormTex();
		/// internal method, create a blank cube texture
		void _createCubeTex();
		/// internal method, create a blank cube texture
		void _createVolumeTex();

		/// internal method, return a D3D pixel format for texture creation
		DXGI_FORMAT _chooseD3DFormat();

		/// @copydoc Texture::createInternalResourcesImpl
		void createInternalResourcesImpl(void);
		/// free internal resources
		void freeInternalResourcesImpl(void);
		/// internal method, set Texture class source image protected attributes
		void _setSrcAttributes(unsigned long width, unsigned long height, unsigned long depth, PixelFormat format);
		/// internal method, set Texture class final texture protected attributes
		void _setFinalAttributes(unsigned long width, unsigned long height, unsigned long depth, PixelFormat format);
		/// internal method, return the best by hardware supported filter method
//		D3DTEXTUREFILTERTYPE _getBestFilterMethod();
		/// internal method, return true if the device/texture combination can use dynamic textures
		bool _canUseDynamicTextures(DWORD srcUsage, D3D10_RESOURCE_DIMENSION srcType, DXGI_FORMAT srcFormat);
		/// internal method, return true if the device/texture combination can auto gen. mip maps
		bool _canAutoGenMipmaps(DWORD srcUsage, D3D10_RESOURCE_DIMENSION srcType, DXGI_FORMAT srcFormat);
		
		/// internal method, the cube map face name for the spec. face index
		String _getCubeFaceName(unsigned char face) const
		{ assert(face < 6); return mCubeFaceNames[face]; }
		
		/// internal method, create D3D10HardwarePixelBuffers for every face and
		/// mipmap level. This method must be called after the D3D texture object was created
		void _createSurfaceList(void);

        /// overriden from Resource
        void loadImpl();
	public:
		/// constructor 
        D3D10Texture(ResourceManager* creator, const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader, 
            ID3D10Device *pD3DDevice);
		/// destructor
		~D3D10Texture();

		/// overriden from Texture
		void copyToTexture( TexturePtr& target );


		/// @copydoc Texture::getBuffer
		HardwarePixelBufferSharedPtr getBuffer(size_t face, size_t mipmap);
		
		/// retrieves a pointer to the actual texture
		ID3D10Resource *getTexture() 
		{ assert(mpTex); return mpTex; }
		/// retrieves a pointer to the normal 1D/2D texture
		ID3D10Texture2D *getNormTexture()
		{ assert(mpNormTex); return mpNormTex; }
		/// retrieves a pointer to the cube texture
		ID3D10Texture2D *getCubeTexture()
		{ assert(mpCubeTex); return mpCubeTex; }
		
		

		/// For dealing with lost devices - release the resource if in the default pool (and return true)
		bool releaseIfDefaultPool(void);
		/// For dealing with lost devices - recreate the resource if in the default pool (and return true)
		bool recreateIfDefaultPool(ID3D10Device * pDev);

    };

    /** Specialisation of SharedPtr to allow SharedPtr to be assigned to D3D10TexturePtr 
    @note Has to be a subclass since we need operator=.
    We could templatise this instead of repeating per Resource subclass, 
    except to do so requires a form VC6 does not support i.e.
    ResourceSubclassPtr<T> : public SharedPtr<T>
    */
    class D3D10TexturePtr : public SharedPtr<D3D10Texture> 
    {
    public:
        D3D10TexturePtr() : SharedPtr<D3D10Texture>() {}
        explicit D3D10TexturePtr(D3D10Texture* rep) : SharedPtr<D3D10Texture>(rep) {}
        D3D10TexturePtr(const D3D10TexturePtr& r) : SharedPtr<D3D10Texture>(r) {} 
        D3D10TexturePtr(const ResourcePtr& r) : SharedPtr<D3D10Texture>()
        {
			// lock & copy other mutex pointer
            OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
            {
			    OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
			    OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
                pRep = static_cast<D3D10Texture*>(r.getPointer());
                pUseCount = r.useCountPointer();
                if (pUseCount)
                {
                    ++(*pUseCount);
                }
            }
        }
		D3D10TexturePtr(const TexturePtr& r) : SharedPtr<D3D10Texture>()
		{
			*this = r;
		}

        /// Operator used to convert a ResourcePtr to a D3D10TexturePtr
        D3D10TexturePtr& operator=(const ResourcePtr& r)
        {
            if (pRep == static_cast<D3D10Texture*>(r.getPointer()))
                return *this;
            release();
			// lock & copy other mutex pointer
            OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
            {
			    OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
			    OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
                pRep = static_cast<D3D10Texture*>(r.getPointer());
                pUseCount = r.useCountPointer();
                if (pUseCount)
                {
                    ++(*pUseCount);
                }
            }
			else
			{
				// RHS must be a null pointer
				assert(r.isNull() && "RHS must be null if it has no mutex!");
				setNull();
			}
            return *this;
        }
        /// Operator used to convert a TexturePtr to a D3D10TexturePtr
        D3D10TexturePtr& operator=(const TexturePtr& r)
        {
            if (pRep == static_cast<D3D10Texture*>(r.getPointer()))
                return *this;
            release();
			// lock & copy other mutex pointer
            OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
            {
			    OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
			    OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
                pRep = static_cast<D3D10Texture*>(r.getPointer());
                pUseCount = r.useCountPointer();
                if (pUseCount)
                {
                    ++(*pUseCount);
                }
            }
			else
			{
				// RHS must be a null pointer
				assert(r.isNull() && "RHS must be null if it has no mutex!");
				setNull();
			}
            return *this;
        }
    };

    /// RenderTexture implementation for D3D10
    class D3D10RenderTexture : public RenderTexture
    {
    public:
		D3D10RenderTexture(const String &name, D3D10HardwarePixelBuffer *buffer):
			RenderTexture(buffer, 0)
		{ 
			mName = name;
		}
        ~D3D10RenderTexture() {}

        void rebind(D3D10HardwarePixelBuffer *buffer)
        {
            mBuffer = buffer;
            mWidth = mBuffer->getWidth();
            mHeight = mBuffer->getHeight();
            mColourDepth = Ogre::PixelUtil::getNumElemBits(mBuffer->getFormat());
        }

        virtual void update(void);

		virtual void getCustomAttribute( const String& name, void *pData )
        {
		/*	if(name == "DDBACKBUFFER")
            {
                IDXGISurface ** pSurf = (IDXGISurface **)pData;
				*pSurf = static_cast<D3D10HardwarePixelBuffer*>(mBuffer)->getSurface();
				return;
            }
            else */
			if(name == "HWND")
            {
                HWND *pHwnd = (HWND*)pData;
                *pHwnd = NULL;
                return;
            }
			else if(name == "BUFFER")
			{
				*static_cast<HardwarePixelBuffer**>(pData) = mBuffer;
				return;
			}
		}

		bool requiresTextureFlipping() const { return false; }
	};

}

#endif
