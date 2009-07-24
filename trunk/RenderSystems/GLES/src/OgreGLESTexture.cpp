/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#include "OgreGLESTexture.h"
#include "OgreGLESSupport.h"
#include "OgreGLESPixelFormat.h"
#include "OgreGLESRenderSystem.h"
#include "OgreGLESHardwarePixelBuffer.h"

#include "OgreTextureManager.h"
#include "OgreImage.h"
#include "OgreLogManager.h"
#include "OgreCamera.h"
#include "OgreException.h"
#include "OgreRoot.h"
#include "OgreCodec.h"
#include "OgreImageCodec.h"
#include "OgreStringConverter.h"
#include "OgreBitwise.h"

namespace Ogre {
    static inline void doImageIO(const String &name, const String &group,
                                 const String &ext,
                                 std::vector<Image> &images,
                                 Resource *r)
    {
        size_t imgIdx = images.size();
        images.push_back(Image());

        DataStreamPtr dstream =
            ResourceGroupManager::getSingleton().openResource(
                name, group, true, r);

        images[imgIdx].load(dstream, ext);
    }

    GLESTexture::GLESTexture(ResourceManager* creator, const String& name,
                             ResourceHandle handle, const String& group, bool isManual,
                             ManualResourceLoader* loader, GLESSupport& support)
        : Texture(creator, name, handle, group, isManual, loader),
          mTextureID(0), mGLSupport(support)
    {
    }

    GLESTexture::~GLESTexture()
    {
        // have to call this here rather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        if (isLoaded())
        {
            unload();
        }
        else
        {
            freeInternalResources();
        }
    }

    GLenum GLESTexture::getGLESTextureTarget(void) const
    {
        return GL_TEXTURE_2D;
    }

    // Creation / loading methods
    void GLESTexture::createInternalResourcesImpl(void)
    {
        GL_CHECK_ERROR;

        mWidth = GLESPixelUtil::optionalPO2(mWidth);
        mHeight = GLESPixelUtil::optionalPO2(mHeight);
        mDepth = GLESPixelUtil::optionalPO2(mDepth);

        mFormat = TextureManager::getSingleton().getNativeFormat(TEX_TYPE_2D,
                                                                 mFormat, mUsage);
        size_t maxMips = GLESPixelUtil::getMaxMipmaps(mWidth, mHeight, mDepth, mFormat);
        mNumMipmaps = mNumRequestedMipmaps;
        if (mNumMipmaps > maxMips)
            mNumMipmaps = maxMips;

        glGenTextures(1, &mTextureID);
        GL_CHECK_ERROR;
        glBindTexture(GL_TEXTURE_2D, mTextureID);
        GL_CHECK_ERROR;
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        GL_CHECK_ERROR;
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        GL_CHECK_ERROR;
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        GL_CHECK_ERROR;
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        GL_CHECK_ERROR;

        mMipmapsHardwareGenerated =
            Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_AUTOMIPMAP);

        if ((mUsage & TU_AUTOMIPMAP) &&
            mNumRequestedMipmaps && mMipmapsHardwareGenerated)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
            GL_CHECK_ERROR;
        }

        // Allocate internal buffer so that glTexSubImageXD can be used
        // Internal format
        GLenum format = GLESPixelUtil::getClosestGLInternalFormat(mFormat, mHwGamma);

        size_t width = mWidth;
        size_t height = mHeight;
        size_t depth = mDepth;

        if (PixelUtil::isCompressed(mFormat))
        {
            // Compressed formats
            size_t size = PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat);

            // Provide temporary buffer filled with zeroes as glCompressedTexImageXD does not
            // accept a 0 pointer like normal glTexImageXD
            // Run through this process for every mipmap to pregenerate mipmap piramid

            uint8* tmpdata = new uint8[size];
            memset(tmpdata, 0, size);

            for (size_t mip = 0; mip <= mNumMipmaps; mip++)
            {
                size = PixelUtil::getMemorySize(width, height, depth, mFormat);

                // TODO: DJR - Add support for PVRTC here
                glCompressedTexImage2D(GL_TEXTURE_2D,
                                       mip,
                                       format,
                                       width, height,
                                       0,
                                       size,
                                       tmpdata);
                GL_CHECK_ERROR;

                if(width > 1)
                {
                    width = width / 2;
                }
                if(height > 1)
                {
                    height = height / 2;
                }
                if(depth > 1)
                {
                    depth = depth / 2;
                }
            }
            delete [] tmpdata;
        }
        else
        {
            // Run through this process to pregenerate mipmap pyramid
            for(size_t mip=0; mip<=mNumMipmaps; mip++)
            {
                glTexImage2D(GL_TEXTURE_2D,
                             mip,
                             format,
                             width, height,
                             0,
                             format,
                             GL_UNSIGNED_BYTE, 0);
//                LogManager::getSingleton().logMessage("GLESTexture::create - Mip: " + StringConverter::toString(mip) +
//                                                      " Width: " + StringConverter::toString(width) +
//                                                      " Height: " + StringConverter::toString(height) +
//                                                      " Internal Format: " + StringConverter::toString(format)
//                                                      );
                GL_CHECK_ERROR;

                if (width > 1)
                {
                    width = width / 2;
                }
                if (height > 1)
                {
                    height = height / 2;
                }
            }
        }

        _createSurfaceList();

        // Get final internal format
        mFormat = getBuffer(0,0)->getFormat();
    }

    void GLESTexture::createRenderTexture(void)
    {
        // Create the GL texture
        // This already does everything neccessary
        createInternalResources();
    }

    void GLESTexture::prepareImpl()
    {
        if (mUsage & TU_RENDERTARGET) return;

        String baseName, ext;
        size_t pos = mName.find_last_of(".");
        baseName = mName.substr(0, pos);

        if (pos != String::npos)
        {
            ext = mName.substr(pos+1);
        }

        LoadedImages loadedImages = LoadedImages(new std::vector<Image>());

        if (mTextureType == TEX_TYPE_2D)
        {
            doImageIO(mName, mGroup, ext, *loadedImages, this);

            // If this is a cube map, set the texture type flag accordingly.
//            if ((*loadedImages)[0].hasFlag(IF_CUBEMAP) &&
//                Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_CUBEMAPPING))
//            {
//                if(getSourceFileType() == "dds")
//                {
//                    // XX HACK there should be a better way to specify whether 
//                    // all faces are in the same file or not
//                    doImageIO(mName, mGroup, ext, *loadedImages, this);
//                }
//                else
//                {
//                    vector<Image>::type images(6);
//                    ConstImagePtrList imagePtrs;
//                    static const String suffixes[6] = {"_rt", "_lf", "_up", "_dn", "_fr", "_bk"};
//                    
//                    for(size_t i = 0; i < 6; i++)
//                    {
//                        String fullName = baseName + suffixes[i];
//                        if (!ext.empty())
//                            fullName = fullName + "." + ext;
//                        // find & load resource data intro stream to allow resource
//                        // group changes if required
//                        doImageIO(fullName,mGroup,ext,*loadedImages,this);
//                    }
//                }
//            }
            // If this is a volumetric texture set the texture type flag accordingly.
            if ((*loadedImages)[0].getDepth() > 1)
            {
                OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                            "**** Unsupported 3D texture type ****",
                            "GLESTexture::prepare" );
            }
        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                        "**** Unknown texture type ****",
                        "GLESTexture::prepare");
        }

        mLoadedImages = loadedImages;
    }

    void GLESTexture::unprepareImpl()
    {
        mLoadedImages.setNull();
    }

    void GLESTexture::loadImpl()
    {
        if (mUsage & TU_RENDERTARGET)
        {
            createRenderTexture();
            return;
        }

        // Now the only copy is on the stack and will be cleaned in case of
        // exceptions being thrown from _loadImages
        LoadedImages loadedImages = mLoadedImages;
        mLoadedImages.setNull();

        // Call internal _loadImages, not loadImage since that's external and 
        // will determine load status etc again
        ConstImagePtrList imagePtrs;

        for (size_t i=0 ; i<loadedImages->size() ; ++i)
        {
            imagePtrs.push_back(&(*loadedImages)[i]);
        }

        _loadImages(imagePtrs);
    }

    void GLESTexture::freeInternalResourcesImpl()
    {
        mSurfaceList.clear();
        glDeleteTextures(1, &mTextureID);
        GL_CHECK_ERROR;
    }

    void GLESTexture::_createSurfaceList()
    {
        mSurfaceList.clear();

        // For all faces and mipmaps, store surfaces as HardwarePixelBufferSharedPtr
        bool wantGeneratedMips = (mUsage & TU_AUTOMIPMAP)!=0;

        // Do mipmapping in software? (uses GLU) For some cards, this is still needed. Of course,
        // only when mipmap generation is desired.
        bool doSoftware = wantGeneratedMips && !mMipmapsHardwareGenerated && getNumMipmaps();

        for (size_t face = 0; face < getNumFaces(); face++)
        {
            for (size_t mip = 0; mip <= getNumMipmaps(); mip++)
            {
                GLESHardwarePixelBuffer *buf = new GLESTextureBuffer(mName,
                                                                     getGLESTextureTarget(),
                                                                     mTextureID,
                                                                     mWidth, mHeight,
                                                                     GLESPixelUtil::getClosestGLInternalFormat(mFormat, mHwGamma),
                                                                     face,
                                                                     mip,
                                                                     static_cast<HardwareBuffer::Usage>(mUsage),
                                                                     doSoftware && mip==0, mHwGamma, mFSAA);

                mSurfaceList.push_back(HardwarePixelBufferSharedPtr(buf));

                /// Check for error
                if (buf->getWidth() == 0 ||
                    buf->getHeight() == 0 ||
                    buf->getDepth() == 0)
                {
                    OGRE_EXCEPT(
                        Exception::ERR_RENDERINGAPI_ERROR,
                        "Zero sized texture surface on texture "+getName()+
                            " face "+StringConverter::toString(face)+
                            " mipmap "+StringConverter::toString(mip)+
                            ". The GL probably driver refused to create the texture.",
                            "GLESTexture::_createSurfaceList");
                }
            }
        }
    }

    HardwarePixelBufferSharedPtr GLESTexture::getBuffer(size_t face, size_t mipmap)
    {
        if (face >= getNumFaces())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Face index out of range",
                        "GLESTexture::getBuffer");
        }

        if (mipmap > mNumMipmaps)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Mipmap index out of range",
                        "GLESTexture::getBuffer");
        }

        unsigned int idx = face * (mNumMipmaps + 1) + mipmap;
        assert(idx < mSurfaceList.size());
        return mSurfaceList[idx];
    }
}
