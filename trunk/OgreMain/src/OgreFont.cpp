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

#include "OgreFont.h"
#include "OgreMaterialManager.h"
#include "OgreTextureManager.h"
#include "OgreFontManager.h"
#include "OgreSDDataChunk.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H



namespace Ogre
{
    //---------------------------------------------------------------------
    
    //---------------------------------------------------------------------
    Font::Font( const String& name)
    {
        mName = name;
        mType = FT_TRUETYPE;
        mpMaterial = NULL;
        mSource = "";
        mTtfSize = 0;
        mTtfResolution = 0;
        mAntialiasColour = false;



    }
    //---------------------------------------------------------------------
    Font::~Font()
    {

    }
    //---------------------------------------------------------------------
    void Font::setType(FontType ftype)
    {
        mType = ftype;
    }
    //---------------------------------------------------------------------
    FontType Font::getType(void)
    {
        return mType;
    }
    //---------------------------------------------------------------------
    void Font::setSource(const String& source)
    {
        mSource = source;
    }
    //---------------------------------------------------------------------
    void Font::setTrueTypeSize(Real ttfSize)
    {
        mTtfSize = ttfSize;
    }
    //---------------------------------------------------------------------
    void Font::setTrueTypeResolution(uint ttfResolution)
    {
        mTtfResolution = ttfResolution;
    }
    //---------------------------------------------------------------------
    const String& Font::getSource(void)
    {
        return mSource;
    }
    //---------------------------------------------------------------------
    Real Font::getTrueTypeSize(void)
    {
        return mTtfSize;
    }
    //---------------------------------------------------------------------
    uint Font::getTrueTypeResolution(void)
    {
        return mTtfResolution;
    }
    //---------------------------------------------------------------------
    std::pair< uint, uint > Font::StrBBox( const String & text, Real char_height, RenderWindow & window )
    {
        std::pair< uint, uint > ret( 0, 0 );
        Real vsX, vsY, veX, veY;
        int w, h; 
        
        // These are not used, but are required byt the function calls.
        int cdepth, left, top;

        window.getMetrics( w, h, cdepth, left, top );

        for( uint i = 0; i < text.length(); i++ )
        {
            getGlyphTexCoords( text[ i ], vsX, vsY, veX, veY );

            // Calculate view-space width and height of char
            vsY = char_height;
            vsX = getGlyphAspectRatio( text[ i ] ) * char_height;

            ret.second += vsX * w;
            if( vsY * h > ret.first || ( i && text[ i - 1 ] == '\n' ) )
                ret.first += vsY * h;
        }

        return ret;
    }
    //---------------------------------------------------------------------
    void Font::load()
    {
        if (!mIsLoaded)
        {
            // Create a new material
            if( !( mpMaterial = reinterpret_cast< Material * >( 
                MaterialManager::getSingleton().create( "Fonts/" + mName ) ) ) )
            {
                Except(Exception::ERR_INTERNAL_ERROR, 
                    "Error creating new material!", "Font::load" );
            }

            Material::TextureLayer *texLayer;
            bool blendByAlpha = true;
            if (mType == FT_TRUETYPE)
            {
                createTextureFromFont();
                texLayer = mpMaterial->getTextureLayer(0);
                // Always blend by alpha
                blendByAlpha = true;
            }
            else
            {
                texLayer = mpMaterial->addTextureLayer(mSource);
                Texture* tex = (Texture*)TextureManager::getSingleton().getByName(mSource);
                blendByAlpha = tex->hasAlpha();
            }
            // Clamp to avoid fuzzy edges
            texLayer->setTextureAddressingMode( Material::TextureLayer::TAM_CLAMP );

            // Set up blending
            if (blendByAlpha)
            {
                mpMaterial->setSceneBlending( SBT_TRANSPARENT_ALPHA );
            }
            else
            {
                // Use add if no alpha (assume black background)
                mpMaterial->setSceneBlending(SBT_ADD);
            }
        }
        mIsLoaded = true;
    }
    //---------------------------------------------------------------------
    void Font::unload()
    {
        mIsLoaded = false;
    }
    //---------------------------------------------------------------------
    void Font::createTextureFromFont(void)
    {
		FT_Library ftLibrary;
		// Init freetype
        if( FT_Init_FreeType( &ftLibrary ) )
            Except( Exception::ERR_INTERNAL_ERROR, "Could not init FreeType library!",
            "Font::Font");

		uint i, l, m, n;
        int j, k;

        FT_Face face;
        uint tex_side = 1024;
        uint data_width = tex_side * 4;
        // Add a gap between letters vert and horz
        // prevents nasty artefacts when letters are too close together
        uint char_spacer = 5;

        uchar* imageData = new uchar[tex_side * tex_side * 4];


        // Locate ttf file
        SDDataChunk ttfchunk;
        FontManager::getSingleton()._findResourceData(mSource, ttfchunk);
        // Load font
        if( FT_New_Memory_Face( ftLibrary, ttfchunk.getPtr(), (FT_Long)ttfchunk.getSize() , 0, &face ) )
            Except( Exception::ERR_INTERNAL_ERROR, 
            "Could not open font face!", "Font::createTextureFromFont" );


        // Convert our point size to freetype 26.6 fixed point format
        FT_F26Dot6 ftSize = (FT_F26Dot6)(mTtfSize * (1 << 6));
        if( FT_Set_Char_Size( face, ftSize, 0, mTtfResolution, 0 ) )
            Except( Exception::ERR_INTERNAL_ERROR, 
            "Could not set char size!", "Font::createTextureFromFont" );

        FILE *fo_def = stdout;

        int max_height = 0, max_width = 0, max_bear = 0;

        uint startGlyph = 33;
        uint endGlyph = 167;

        // Calculate maximum width, height and bearing
        for( i = startGlyph, l = 0, m = 0, n = 0; i < endGlyph; i++ )
        {
            FT_Int glyph_index = FT_Get_Char_Index( face, i );
            FT_Load_Glyph( face, glyph_index, FT_LOAD_RENDER );
            FT_Render_Glyph( face->glyph, ft_render_mode_normal );

            if( ( 2 * ( face->glyph->bitmap.rows << 6 ) - face->glyph->metrics.horiBearingY ) > max_height )
                max_height = ( 2 * ( face->glyph->bitmap.rows << 6 ) - face->glyph->metrics.horiBearingY );
            if( face->glyph->metrics.horiBearingY > max_bear )
                max_bear = face->glyph->metrics.horiBearingY;

            if( (face->glyph->advance.x >> 6 ) + ( face->glyph->metrics.horiBearingX >> 6 ) > max_width)
                max_width = (face->glyph->advance.x >> 6 ) + ( face->glyph->metrics.horiBearingX >> 6 );
        }

        for( i = startGlyph, l = 0, m = 0, n = 0; i < endGlyph; i++ )
        {
            FT_Error ftResult;

            FT_Int glyph_index = FT_Get_Char_Index( face, i );
            // Load glyph
            ftResult = FT_Load_Glyph( face, glyph_index, FT_LOAD_RENDER );
            if (ftResult)
            {
                // problem loading this glyph, continue
                LogManager::getSingleton().logMessage("Info: cannot load glyph " +
                    StringConverter::toString(glyph_index) + " in font " + mName);
                continue;
            }
            ftResult = FT_Render_Glyph( face->glyph, ft_render_mode_normal );
            if (ftResult)
            {
                // problem rendering this glyph, continue
                LogManager::getSingleton().logMessage("Info: cannot render glyph " +
                    StringConverter::toString(glyph_index) + " in font " + mName);
                continue;
            }

            FT_Int advance = (face->glyph->advance.x >> 6 ) + ( face->glyph->metrics.horiBearingX >> 6 );

            unsigned char* buffer = face->glyph->bitmap.buffer;

            if (!buffer)
            {
                // Yuck, FT didn't detect this but generated a null pointer!
                LogManager::getSingleton().logMessage("Info: Freetype returned null for glyph " +
                    StringConverter::toString(glyph_index) + " in font " + mName);
                continue;
            }

            int y_bearnig = ( max_bear >> 6 ) - ( face->glyph->metrics.horiBearingY >> 6 );

            for( j = 0; j < face->glyph->bitmap.rows; j++ )
            {
                int row = j + m + y_bearnig;
                int col = l;
                uchar* pDest = &imageData[(row * data_width) + l * 4];   
                for( k = 0; k < face->glyph->bitmap.width; k++ )
                {
                    if (mAntialiasColour)
                    {
                        // Use the same greyscale pixel for all components RGBA
                        *pDest++= *buffer;
                        *pDest++= *buffer;
                        *pDest++= *buffer;
                    }
                    else
                    {
                        // Clamp colour to full white or off
                        if (*buffer > 0)
                        {
                            *pDest++= 0xFF;
                            *pDest++= 0xFF;
                            *pDest++= 0xFF;
                        }
                        else
                        {
                            *pDest++= 0;
                            *pDest++= 0;
                            *pDest++= 0;
                        }
                    }
                    // Always use the greyscale value for alpha
                    *pDest++= *buffer++;
                }
            }

            this->setGlyphTexCoords( i, 
                (Real)l / (Real)tex_side,  // u1
                1.0 - (Real)m / (Real)tex_side,  // v1
                (Real)( l + ( face->glyph->advance.x >> 6 ) ) / (Real)tex_side, // u2
                1.0 - ( ( m + ( max_height >> 6 ) ) / (Real)tex_side ) // v2
                );

            // Advance a column
            l += (advance + char_spacer);

            // If at end of row
            if( tex_side - 1 < l + ( advance ) )
            {
                m += ( max_height >> 6 ) + char_spacer;
                l = n = 0;
            }
        }

        SDDataChunk imgchunk(imageData, tex_side * tex_side * 4);

        Image img; 
		img.loadRawData( imgchunk, tex_side, tex_side, PF_A8R8G8B8 );


        String texName = mName + "Texture";
        TextureManager::getSingleton().loadImage( texName , img );
        mpMaterial->addTextureLayer( texName );
        
        // SDDatachunk will delete imageData

		FT_Done_FreeType(ftLibrary);
    }


}
