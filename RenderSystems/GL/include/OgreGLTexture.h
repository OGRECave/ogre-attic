/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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

#ifndef __GLTEXTURE_H__
#define __GLTEXTURE_H__

#include "OgreGLPrerequisites.h"
#include "OgrePlatform.h"
#include "OgreRenderTexture.h"
#include "OgreTexture.h"
#include "OgreGLSupport.h"

namespace Ogre {

    class GLTexture : public Texture
    {
    public:
        // Constructor
        GLTexture(ResourceManager* creator, const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader, 
            GLSupport& support);

        virtual ~GLTexture();      

        /// @copydoc Texture::createInternalResources
        void createInternalResources(void);
        
        void loadImage( const Image &img );
        void loadImages( const std::vector<Image>& images );


        void createRenderTexture();

        void blitToTexture( const Image& src, 
            unsigned uStartX, unsigned uStartY );
			
		/// @copydoc Texture::getBuffer
		HardwarePixelBufferSharedPtr getBuffer(int face, int mipmap);

        // Takes the OGRE texture type (2d/3d/cube) and returns the appropriate GL one
        GLenum getGLTextureTarget(void) const;

        // Takes the OGRE pixel format and returns the appropriate GL one
        GLenum getGLTextureOriginFormat(void) const;

        // Takes the OGRE pixel format and returns the type that must be provided
        // to GL as data type for reading it into the GPU.
        GLenum getGLTextureOriginDataType(void) const;

        // Takes the OGRE pixel format and returns the type that must be provided
        // to GL as internal format.
        GLenum getGLTextureInternalFormat(void) const;

        GLuint getGLID() const
        { return mTextureID; }

    protected:
        /// @copydoc Resource::loadImpl
        void loadImpl(void);
        /// @copydoc Resource::unloadImpl
        void unloadImpl(void);

        void generateMipMaps( const uchar *data, bool useSoftware, bool isCompressed,
            size_t faceNumber );
        uchar* rescaleNPower2( const Image& src );
    private:
        GLuint mTextureID;
        GLSupport& mGLSupport;
    };

    /** Specialisation of SharedPtr to allow SharedPtr to be assigned to GLTexturePtr 
    @note Has to be a subclass since we need operator=.
    We could templatise this instead of repeating per Resource subclass, 
    except to do so requires a form VC6 does not support i.e.
    ResourceSubclassPtr<T> : public SharedPtr<T>
    */
    class GLTexturePtr : public SharedPtr<GLTexture> 
    {
    public:
        GLTexturePtr() : SharedPtr<GLTexture>() {}
        explicit GLTexturePtr(GLTexture* rep) : SharedPtr<GLTexture>(rep) {}
        GLTexturePtr(const GLTexturePtr& r) : SharedPtr<GLTexture>(r) {} 
        GLTexturePtr(const ResourcePtr& r) : SharedPtr<GLTexture>()
        {
			// lock & copy other mutex pointer
			OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
			OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
            pRep = static_cast<GLTexture*>(r.getPointer());
            pUseCount = r.useCountPointer();
            if (pUseCount)
            {
                ++(*pUseCount);
            }
        }

        /// Operator used to convert a ResourcePtr to a GLTexturePtr
        GLTexturePtr& operator=(const ResourcePtr& r)
        {
            if (pRep == static_cast<GLTexture*>(r.getPointer()))
                return *this;
            release();
			// lock & copy other mutex pointer
			OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
			OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
            pRep = static_cast<GLTexture*>(r.getPointer());
            pUseCount = r.useCountPointer();
            if (pUseCount)
            {
                ++(*pUseCount);
            }
            return *this;
        }
        /// Operator used to convert a TexturePtr to a GLTexturePtr
        GLTexturePtr& operator=(const TexturePtr& r)
        {
            if (pRep == static_cast<GLTexture*>(r.getPointer()))
                return *this;
            release();
			// lock & copy other mutex pointer
			OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
			OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
            pRep = static_cast<GLTexture*>(r.getPointer());
            pUseCount = r.useCountPointer();
            if (pUseCount)
            {
                ++(*pUseCount);
            }
            return *this;
        }
    };

    /// GL implementation of RenderTexture
    class GLRenderTexture : public RenderTexture
    {
    public:
        GLRenderTexture(const String& name, uint width, uint height, TextureType texType,  PixelFormat format) 
            : RenderTexture(name, width, height, texType, format) 
        {
            mGLTexture = mTexture;
        }

        void _copyToTexture(void);

        bool requiresTextureFlipping() const { return true; }
        virtual void writeContentsToFile( const String & filename );
    protected:
        GLTexturePtr mGLTexture;
    };
}

#endif // __GLTEXTURE_H__
