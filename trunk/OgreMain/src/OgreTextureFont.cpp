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

#include "OgreTextureFont.h"

#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreString.h"
#include "OgreTextureManager.h"

#include <ft2build.h>
#include FT_FREETYPE_H

BEGIN_OGRE_NAMESPACE

std::map< String, std::map< String, TextureFont * > > TextureFont::ms_mapFonts;
FT_Library *TextureFont::ms_pLibrary = NULL;
unsigned TextureFont::ms_uNumFonts = 0;

TextureFont::TextureFont()
{
	if( ms_pLibrary == NULL )
	{
		ms_pLibrary = new FT_Library;
		if( FT_Init_FreeType( ms_pLibrary ) )
		{
			Except( 
				Exception::ERR_INTERNAL_ERROR, 
				"Could not initialize FreeType library!", 
				"TextureFont::loadFromFile" );
		}
		FT_Int iMaj, iMin, iRev;
		FT_Library_Version( *ms_pLibrary, &iMaj, &iMin, &iRev );
		LogManager::getSingleton().logMessage( LML_NORMAL, "Initialized FreeType version %d.%d.%d", iMaj, iMin, iRev );
		ms_uNumFonts++;
	}

	m_pFace = NULL;
}

TextureFont::TextureFont( const String& strFontFile )
{
	if( ms_pLibrary == NULL )
	{
		ms_pLibrary = new FT_Library;
		if( FT_Init_FreeType( ms_pLibrary ) )
		{
			Except( 
				Exception::ERR_INTERNAL_ERROR, 
				"Could not initialize FreeType library!", 
				"TextureFont::loadFromFile" );
		}
		FT_Int iMaj, iMin, iRev;
		FT_Library_Version( *ms_pLibrary, &iMaj, &iMin, &iRev );
		LogManager::getSingleton().logMessage( LML_NORMAL, "Initialized FreeType version %d.%d.%d", iMaj, iMin, iRev );
		ms_uNumFonts++;
	}

	m_pFace = NULL;

	loadFromFile( strFontFile );
}

void TextureFont::loadFromFile( const String& strFontFile )
{
	m_pFace = new FT_Face;
	if( FT_New_Face( *ms_pLibrary, strFontFile.c_str(), 0, m_pFace ) )
	{
		Except( 
			Exception::ERR_INTERNAL_ERROR, 
			"Could not create font face from file '" + strFontFile + "'!", 
			"TextureFont::loadFromFile" );
	}

	// delete old font mapping if exists
	if( ms_mapFonts[(*m_pFace)->family_name][(*m_pFace)->style_name] != NULL )
		delete ms_mapFonts[(*m_pFace)->family_name][(*m_pFace)->style_name];

	ms_mapFonts[(*m_pFace)->family_name][(*m_pFace)->style_name] = this;
}

TextureFont::~TextureFont()
{
	ms_mapFonts[(*m_pFace)->family_name][(*m_pFace)->style_name] = NULL;

	FT_Done_Face( *m_pFace );
	delete m_pFace; m_pFace = NULL;
	ms_uNumFonts--;

	if( ms_uNumFonts )
		FT_Done_FreeType( *ms_pLibrary );
}

void TextureFont::createTexture( const String& strTexName, const String& strString, unsigned uTexX, unsigned uTexY, unsigned uFontX /* = 600 */, unsigned uFontY /* = 0 */, unsigned uFontResX /* = 72 */, unsigned uFontResY /* = 0 */, unsigned uStartX /* = 1 */, unsigned uStartY /* = 1  */ )
{
	typedef unsigned char pixel[3];
	pixel *pTexture = new pixel[ uTexY * uTexX ];

	FT_Bool       use_kerning;
	FT_UInt       previous;
	FT_UInt       glyph_index;
	FT_UInt		  pen_x, pen_y, pen_dx, pen_dy;
	FT_GlyphSlot  slot;

	// make sure face is loaded
	assert( m_pFace );

	// clear texture
	memset( pTexture, 0, uTexX * uTexY * sizeof( pixel ) );

	use_kerning = FT_HAS_KERNING( (*m_pFace) );
	previous    = 0;
	
	if( FT_Set_Char_Size( *m_pFace, uFontX, uFontY, uFontResX, uFontResY ) )
	{
		Except(
			Exception::ERR_INTERNAL_ERROR,
			"Could not set font size!",
			"TextureFont::createTexture" );
	}

	pen_x = uStartX;
	pen_y = uStartY + ( (*m_pFace)->size->metrics.ascender >> 6 );

	for( size_t i = 0; i < strString.length(); i++ )
	{
		if( strString.at( i ) == '\n' )
		{
			previous = 0;
			pen_x = uStartX;
			pen_y += (*m_pFace)->size->metrics.height >> 6;
			continue;
		}

		glyph_index = FT_Get_Char_Index( *m_pFace, strString.at(i) );        
		FT_Load_Char( *m_pFace, (long)strString.at(i), FT_LOAD_RENDER );
		slot = (*m_pFace)->glyph;

		if( use_kerning && previous && glyph_index )
		{
			FT_Vector  delta;
			FT_Get_Kerning( *m_pFace, previous, glyph_index, 0, &delta );
			pen_x += delta.x >> 6;
		}

		unsigned char *pChar = slot->bitmap.buffer;

		FT_Int j, k;
		pen_dx = pen_x + slot->bitmap_left;
		pen_dy = pen_y - slot->bitmap_top;

		// if we're getting off the drawing area, go down a line
		if( slot->bitmap.width + pen_dx >= uTexX )
		{
			previous = 0;
			pen_x = 1;
			pen_y += (*m_pFace)->size->metrics.height >> 6;
			i--;
			continue;
		}

		for( j=0; j<slot->bitmap.rows; j++ )
		{
			for( k=0; k<slot->bitmap.width; k++ )
			{
				// pixel size is always 1 so we've got grays
				pTexture[ ( pen_dy + j ) * uTexY + pen_dx + k ][0] =
				pTexture[ ( pen_dy + j ) * uTexY + pen_dx + k ][1] =
				pTexture[ ( pen_dy + j ) * uTexY + pen_dx + k ][2] = *pChar;
				pChar++;
			}
		}

		pen_x += slot->advance.x >> 6;
	}

	TextureManager::getSingleton().loadRawRGB( strTexName, (unsigned char*)pTexture, uTexX, uTexY );

	delete [] pTexture;
}

void TextureFont::createAlphaMask( const String& strTexName, const String& strString, unsigned uTexX, unsigned uTexY, byte cRed, byte cGreen, byte cBlue, unsigned uFontX /* = 600 */, unsigned uFontY /* = 0 */, unsigned uFontResX /* = 72 */, unsigned uFontResY /* = 0 */, unsigned uStartX /* = 1 */, unsigned uStartY /* = 1  */ )
{
    typedef unsigned char pixel[4];
    pixel *pTexture = new pixel[ uTexY * uTexX ];

    FT_Bool       use_kerning;
    FT_UInt       previous;
    FT_UInt       glyph_index;
    FT_UInt		  pen_x, pen_y, pen_dx, pen_dy;
    FT_GlyphSlot  slot;

    // make sure face is loaded
    assert( m_pFace );

    // clear texture
    memset( pTexture, 0, uTexX * uTexY * sizeof( pixel ) );

    use_kerning = FT_HAS_KERNING( (*m_pFace) );
    previous    = 0;

    if( FT_Set_Char_Size( *m_pFace, uFontX, uFontY, uFontResX, uFontResY ) )
    {
        Except(
            Exception::ERR_INTERNAL_ERROR,
            "Could not set font size!",
            "TextureFont::createTexture" );
    }

    pen_x = uStartX;
    pen_y = uStartY + ( (*m_pFace)->size->metrics.ascender >> 6 );

    for( size_t i = 0; i < strString.length(); i++ )
    {
        if( strString.at( i ) == '\n' )
        {
            previous = 0;
            pen_x = uStartX;
            pen_y += (*m_pFace)->size->metrics.height >> 6;
            continue;
        }

        glyph_index = FT_Get_Char_Index( *m_pFace, strString.at(i) );        
        FT_Load_Char( *m_pFace, (long)strString.at(i), FT_LOAD_RENDER );
        slot = (*m_pFace)->glyph;

        if( use_kerning && previous && glyph_index )
        {
            FT_Vector  delta;
            FT_Get_Kerning( *m_pFace, previous, glyph_index, 0, &delta );
            pen_x += delta.x >> 6;
        }

        unsigned char *pChar = slot->bitmap.buffer;

        FT_Int j, k;
        pen_dx = pen_x + slot->bitmap_left;
        pen_dy = pen_y - slot->bitmap_top;

        // if we're getting off the drawing area, go down a line
        if( slot->bitmap.width + pen_dx >= uTexX )
        {
            previous = 0;
            pen_x = 1;
            pen_y += (*m_pFace)->size->metrics.height >> 6;
            i--;
            continue;
        }

        for( j=0; j<slot->bitmap.rows; j++ )
        {
            for( k=0; k<slot->bitmap.width; k++ )
            {
                // pixel size is always 1 so we've got grays                
                pTexture[ ( pen_dy + j ) * uTexY + pen_dx + k ][0] = cRed;
                pTexture[ ( pen_dy + j ) * uTexY + pen_dx + k ][1] = cGreen;
                pTexture[ ( pen_dy + j ) * uTexY + pen_dx + k ][2] = cBlue;
                pTexture[ ( pen_dy + j ) * uTexY + pen_dx + k ][3] = *pChar;
                pChar++;
            }
        }

        pen_x += slot->advance.x >> 6;
    }

    TextureManager::getSingleton().loadRawRGBA( strTexName, (unsigned char*)pTexture, uTexX, uTexY );

    delete [] pTexture;
}

END_OGRE_NAMESPACE