/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2002 The OGRE Team
Also see acknowledgements in Readme.html

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License (LGPL) as 
published by the Free Software Foundation; either version 2.1 of the 
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public 
License for more details.

You should have received a copy of the GNU Lesser General Public License 
along with this library; if not, write to the Free Software Foundation, 
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA or go to
http://www.gnu.org/copyleft/lesser.txt
-------------------------------------------------------------------------*/

#ifndef _Font_H__
#define _Font_H__

#include "OgrePrerequisites.h"
#include "OgreTextureManager.h"
#include "OgreMaterial.h"
#include "OgreMaterialManager.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H


namespace Ogre
{
    // Define the number of glyphs allowed
    // We ignore 0-31 since these are control characters
#if OGRE_WCHAR_T_STRINGS
    // Allow wide chars
    #define OGRE_NUM_GLYPHS (1024 - 32)
#else
    // Allow 8-bit ASCII 
    // (we don't want to offend people with charcodes 127-256 in their name eh cearny? ;)
    // Only chars 33+ are any use though
    #define OGRE_NUM_GLYPHS (256 - 32)
#endif

    // How to look up chars
    #define OGRE_GLYPH_INDEX(c) c - 33

    /** Enumerates the types of Font usable in the engine. */
    enum FontType
    {
        /// Generated from a truetype (.ttf) font
        FT_TRUETYPE = 1,
        /// Loaded from an image created by an artist
        FT_IMAGE = 2
    };


    /** Class representing a font in the system.
    @remarks
    This class is simply a way of getting a font texture into the OGRE system and
    to easily retrieve the texture coordinates required to accurately render them.
    Fonts can either be loaded from precreated textures, or the texture can be generated
    using a truetype font. You can either create the texture manually in code, or you
    can use a .fontdef script to define it (probably more practical since you can reuse
    the definition more easily)
    */
    class _OgreExport Font : public Resource
    {
    protected:
        /// The type of font
        FontType mType;

        /// Source of the font (either an image name or a truetype font)
        String mSource;

        /// Size of the truetype font, in points
        Real mTtfSize;
        /// Resolution (dpi) of truetype font
        uint mTtfResolution;


        /// Start u coords
        Real mTexCoords_u1[OGRE_NUM_GLYPHS];
        /// End u coords
        Real mTexCoords_u2[OGRE_NUM_GLYPHS];
        /// Start v coords
        Real mTexCoords_v1[OGRE_NUM_GLYPHS];
        /// End v coords
        Real mTexCoords_v2[OGRE_NUM_GLYPHS];

        /// Aspect ratio between x and y (width / height)
        Real mAspectRatio[OGRE_NUM_GLYPHS];

        /// The material which is generated for this font
        Material *mpMaterial;

        /// for TRUE_TYPE font only
        bool mAntialiasColour;

        /// Freetype link 
        static FT_Library mLibrary;
        static bool mFreetypeInitDone;

        /// Internal method for loading from ttf
        void createTextureFromFont(void);

    public:

        /** Constructor.
        @param name Mandatory name, must be unique.
        */
        Font( const String& name);
        virtual ~Font();

        /** Sets the type of font. Must be set before loading. */
        void setType(FontType ftype);

        /** Gets the type of font. */
        FontType getType(void);

        /** Sets the source of the font.
        @remarks
            If you have created a font of type FT_IMAGE, this method tells the
            Font which image to use as the source for the characters. So the parameter 
            should be the name of an appropriate image file. Note that when using an image
            as a font source, you will also need to tell the font where each character is
            located using setGlyphTexCoords (for each character).
        @par
            If you have created a font of type FT_TRUETYPE, this method tells the
            Font which .ttf file to use to generate the text. You will also need to call 
            setTrueTypeSize and setTrueTypeResolution.
        @param source An image file or a truetype font, depending on the type of this font
        */
        void setSource(const String& source);

        /** Gets the source this font (either an image or a truetype font).
        */
        const String& getSource(void);

        /** Sets the size of a truetype font (only required for FT_TRUETYPE). 
        @param ttfSize The size of the font in points. Note that the
            size of the font does not affect how big it is on the screen, just how large it is in
            the texture and thus how detailed it is.
        */
        void setTrueTypeSize(Real ttfSize);
        /** Gets the resolution (dpi) of the font used to generate the texture
        (only required for FT_TRUETYPE).
        @param ttfResolution The resolution in dpi
        */
        void setTrueTypeResolution(uint ttfResolution);

        /** Gets the point size of the font used to generate the texture.
        @remarks
            Only applicable for FT_TRUETYPE Font objects.
            Note that the size of the font does not affect how big it is on the screen, 
            just how large it is in the texture and thus how detailed it is.            
        */
        Real getTrueTypeSize(void);
        /** Gets the resolution (dpi) of the font used to generate the texture.
        @remarks
            Only applicable for FT_TRUETYPE Font objects.
        */
        uint getTrueTypeResolution(void);

        /** See Resource. */
        virtual void load();
        /** See Resource. */
        virtual void unload();

        /** Returns the teture coordinates of the associated glyph. 
            @remarks Parameter is a short to allow both ASCII and wide chars.
            @param id The character code
            @param u1, u2, v1, v2 location to place the results
        */
        inline void getGlyphTexCoords(OgreChar id, Real& u1, Real& v1, Real& u2, Real& v2 ) const
        {
            OgreChar idx = OGRE_GLYPH_INDEX(id);
            u1 = mTexCoords_u1[ idx ];
            v1 = mTexCoords_v1[ idx ];
            u2 = mTexCoords_u2[ idx ];
            v2 = mTexCoords_v2[ idx ];
        }

        /** Sets the texture coordinates of a glyph.
        @remarks
            You only need to call this if you're setting up a font loaded from a texture manually.
        @note
            Also sets the aspect ratio (width / height) of this character. 
        */
        inline void setGlyphTexCoords( OgreChar id, Real u1, Real v1, Real u2, Real v2 )
        {
            OgreChar idx = OGRE_GLYPH_INDEX(id);
            mTexCoords_u1[ idx ] = u1;
            mTexCoords_v1[ idx ] = v1;
            mTexCoords_u2[ idx ] = u2;
            mTexCoords_v2[ idx ] = v2;
            mAspectRatio[ idx ] = ( u2 - u1 ) / ( v1 - v2 );
        }
        /** Gets the aspect ratio (width / height) of this character. */
        inline Real getGlyphAspectRatio( OgreChar id ) const
        {
            OgreChar idx = OGRE_GLYPH_INDEX(id);
            return mAspectRatio[ idx ];
        }
        /** Sets the aspect ratio (width / height) of this character.
        @remarks
            You only need to call this if you're setting up a font loaded from a texture manually,
            and your aspect ratio is really freaky.
        */
        inline void setGlyphAspectRatio( OgreChar id, Real ratio )
        {
            OgreChar idx = OGRE_GLYPH_INDEX(id);
            mAspectRatio[ idx ] = ratio;
        }
        /** Gets the material generated for this font. 
        @remarks
            This will only be valid after the Font has been loaded. 
        */
        inline const Material * getMaterial() const
        {
            return mpMaterial;
        }
        /** Gets the material generated for this font. 
        @remarks
            This will only be valid after the Font has been loaded. 
        */
        inline Material * getMaterial()
        {
            return mpMaterial;
        }
        /** Sets whether or not the colour of this font is antialiased as it is generated
            from a true type font.
        @remarks
        	This is valid only for a FT_TRUETYPE font. If you are planning on using 
            alpha blending to draw your font, then it is a good idea to set this to
            false (which is the default), otherwise the darkening of the font will combine
            with the fading out of the alpha around the edges and make your font look thinner
            than it should. However, if you intend to blend your font using a colour blending
            mode (add or modulate for example) then it's a good idea to set this to true, in
            order to soften your font edges.
        */
        inline void setAntialiasColour(bool enabled)
        {
        	mAntialiasColour = enabled;
        }

        inline bool getAntialiasColour(void)
        {
            return mAntialiasColour;
        }
    };
}

#endif
