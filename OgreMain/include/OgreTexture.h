/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#ifndef _Texture_H__
#define _Texture_H__

#include "OgrePrerequisites.h"

#include "OgreResource.h"
#include "OgreImage.h"

namespace Ogre {

    /** Enum identifying the texture usage
    */
    enum TextureUsage
    {
        TU_DEFAULT = 0x0, /// default usage
        TU_RENDERTARGET = 0x1 /// this texture will be a render target, ie. used as a target for render to texture
    };

    /** Enum identifying the texture type
    */
    enum TextureType
    {
        /// 1D texture, used in combination with 1D texture coordinates
        TEX_TYPE_1D = 1,
        /// 2D texture, used in combination with 2D texture coordinates (default)
        TEX_TYPE_2D = 2,
        /// 3D volume texture, used in combination with 3D texture coordinates
        TEX_TYPE_3D = 3,
        /// 3D cube map, used in combination with 3D texture coordinates
        TEX_TYPE_CUBE_MAP = 4
    };

    /** Abstract class representing a Texture resource.
        @remarks
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
        Texture() {}
        Texture(const String& name, TextureType texType, uint width, uint height, uint depth, uint num_mips, PixelFormat format, TextureUsage usage);

        /** Gets the type of texture 
        */
        TextureType getTextureType(void) const { return mTextureType; }

        /** Gets the number of mipmaps to be used for this texture.
        */
        unsigned short getNumMipMaps(void) const {return mNumMipMaps;}

        /** Sets the number of mipmaps to be used for this texture.
            @note
                Must be set before calling any 'load' method.
        */
        void setNumMipMaps(unsigned short num) {mNumMipMaps = num;}

        /** Returns the gamma adjustment factor applied to this texture.
        */
        float getGamma(void) const { return mGamma; }

        /** Sets the gamma adjustment factor applied to this texture.
            @note
                Must be called before any 'load' method.
        */
        void setGamma(float g) { mGamma = g; }

        /** Returns the height of the texture.
        */
        unsigned int getHeight(void) const { return mHeight; }

        /** Returns the width of the texture.
        */
        unsigned int getWidth(void) const { return mWidth; }

        /** Returns the depth of the texture (only applicable for 3D textures).
        */
        unsigned int getDepth(void) const { return mDepth; }

        /** Returns both the width and height of the texture.
        */
        std::pair< uint, uint > getDimensions() const { return std::pair< uint, uint >( mWidth, mHeight ); }

        /** Returns the TextureUsage indentifier for this Texture
        */
        TextureUsage getUsage() const
        {
            return mUsage;
        }

        /** Blits the contents of src on the texture.
            @deprecated
                This feature is superseded by the blitImage function.
            @param
                src the image with the source data
        */
        virtual void blitToTexture( 
            const Image &src, unsigned uStartX, unsigned uStartY ) = 0;

        /** Blits a rect from an image to the texture.
            @param
                src The image with the source data.
            @param
                imgRect The data rect to be copied from the image.
            @param
                texRect The rect in which to copy the data in the texture.
        */
        virtual void blitImage(
            const Image& src, const Image::Rect imgRect, const Image::Rect texRect )
        {
        }

		/** Copies (and maybe scales to fit) the contents of this texture to
			another texture. */
		virtual void copyToTexture( Texture * target ) {}

        /** Loads the data from an image.
        */
        virtual void loadImage( const Image &img ) = 0;
			
		/** Loads the data from the raw memory area.
		*/
		virtual void loadRawData( const DataChunk &pData, 
			ushort uWidth, ushort uHeight, PixelFormat eFormat) ;

        void enable32Bit( bool setting = true ) 
        {
            setting ? mFinalBpp = 32 : mFinalBpp = 16;
        }

		/** Returns the pixel format for the texture surface. */
		virtual PixelFormat getFormat() const
		{
			return mFormat;
		}

        /** Returns true if the texture has an alpha layer. */
        virtual bool hasAlpha(void) const
        {
            return mHasAlpha;
        }

    protected:
        unsigned long mHeight;
        unsigned long mWidth;
        unsigned long mDepth;

        unsigned short mNumMipMaps;
        float mGamma;

        TextureType mTextureType;
		PixelFormat mFormat;
        TextureUsage mUsage;

        unsigned short mSrcBpp;
        unsigned long mSrcWidth, mSrcHeight;
        unsigned short mFinalBpp;
        bool mHasAlpha;
    };

	/** Specialisation of SharedPtr to allow SharedPtr to be assigned to TexturePtr 
	@note Has to be a subclass since we need operator=.
		We could templatise this instead of repeating per Resource subclass, 
		except to do so requires a form VC6 does not support i.e.
		ResourceSubclassPtr<T> : public SharedPtr<T>
	*/
	class _OgreExport TexturePtr : public SharedPtr<Texture> 
	{
	public:
		TexturePtr() : SharedPtr<Texture>() {}
		TexturePtr(Texture* rep) : SharedPtr<Texture>(rep) {}
		TexturePtr(const TexturePtr& r) : SharedPtr<Texture>(r) {} 
		TexturePtr(const ResourcePtr& r) : SharedPtr<Texture>()
		{
			pRep = static_cast<Texture*>(r.getPointer());
			pUseCount = r.useCountPointer();
			if (pUseCount)
			{
				++(*pUseCount);
			}
		}

		/// Operator used to convert a ResourcePtr to a TexturePtr
		TexturePtr& operator=(const ResourcePtr& r)
		{
			if (pRep == static_cast<Texture*>(r.getPointer()))
				return *this;
			release();
			pRep = static_cast<Texture*>(r.getPointer());
			pUseCount = r.useCountPointer();
			if (pUseCount)
			{
				++(*pUseCount);
			}
			return *this;
		}
	};

}

#endif
