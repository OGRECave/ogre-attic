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
#include FT_GLYPH_H

BEGIN_OGRE_NAMESPACE

void renderTexString( 
    std::vector< FT_Glyph >& glyphs, 
    std::vector< FT_Vector >& vectors,    
    byte *pTex, 
    unsigned uTexX, unsigned uTexY, 
    unsigned uBitR, unsigned uBitW, 
    unsigned uStartX, unsigned uStartY )
{
    // UNDONE implement renderTexString
}

void renderTexChar( byte *pBuffer, byte *pTex, unsigned uTexX, unsigned uTexY, unsigned uBitR, unsigned uBitW, unsigned uStartX, unsigned uStartY )
{
    typedef byte pixel[3];
    pixel *pLocalTex = (pixel *)pTex;
    unsigned char *pLocalBuff = pBuffer;

    for( unsigned i = 0; i < uBitR; i++ )
    {
        for( unsigned j = 0; j < uBitW; j++ )
        {
            if( *pLocalBuff )
            {
                (*( pLocalTex + ( ( uStartY + i ) * uTexY + uStartX + j )))[0] = 
                (*( pLocalTex + ( ( uStartY + i ) * uTexY + uStartX + j )))[1] = 
                (*( pLocalTex + ( ( uStartY + i ) * uTexY + uStartX + j )))[2] = *pLocalBuff;                
            }
            pLocalBuff++;
        }
    }
}

void renderAlphaChar( byte *pBuffer, byte *pTex, unsigned uTexX, unsigned uTexY, unsigned uBitR, unsigned uBitW, unsigned uStartX, unsigned uStartY, unsigned cRed, unsigned cGreen, unsigned cBlue )
{
    typedef byte pixel[4];
    pixel *pLocalTex = (pixel *)pTex;
    unsigned char *pLocalBuff = pBuffer;

    for( unsigned i = 0; i < uBitR; i++ )
    {
        for( unsigned j = 0; j < uBitW; j++ )
        {
            if( *pLocalBuff )
            {
                (*( pLocalTex + ( ( uStartY + i ) * uTexY + uStartX + j )))[0] = cRed;
                (*( pLocalTex + ( ( uStartY + i ) * uTexY + uStartX + j )))[1] = cGreen;
                (*( pLocalTex + ( ( uStartY + i ) * uTexY + uStartX + j )))[2] = cBlue;
                (*( pLocalTex + ( ( uStartY + i ) * uTexY + uStartX + j )))[3] = *pLocalBuff;                
            }
            pLocalBuff++;
        }
    }
}

void renderAlphaString( 
    std::list< FT_Glyph >& glyphs, 
    std::list< FT_Vector >& vectors,
    byte *pTex, 
    unsigned uTexX, unsigned uTexY, 
    unsigned uStartX, unsigned uStartY, 
    unsigned cRed, unsigned cGreen, unsigned cBlue  )
{
    std::list< FT_Glyph >::iterator it1 = glyphs.begin();
    std::list< FT_Vector >::iterator it2 = vectors.begin();

    for( ; it1 != glyphs.end(); ++it1, ++it2 )
    {
        FT_Glyph  image;
        FT_Vector pen;

        image = (*it1);

        pen.x = uStartX + (*it2).x;
        pen.y = uStartY + (*it2).y;

        if( !FT_Glyph_To_Bitmap( &image, ft_render_mode_normal, &pen, 0 ) )
        {
            FT_BitmapGlyph  bit = (FT_BitmapGlyph)image;            
            renderAlphaChar( 
                bit->bitmap.buffer, 
                pTex, 
                uTexX, uTexY, 
                bit->bitmap.rows, bit->bitmap.width, 
                pen.x, pen.y, cRed, cGreen, cBlue );
            FT_Done_Glyph( image );
        }
    }
}

void computeBBox( FT_BBox& bbox, std::list< FT_Glyph >& glyphList, std::list< FT_Vector >& vectorList )
{
    bbox.xMin = bbox.yMin = 32000;
    bbox.xMax = bbox.yMax = -32000;

    // for each glyph image, compute its bounding box, translate it,
    // and grow the string bbox
    std::list< FT_Glyph >::iterator it1 = glyphList.begin();
    std::list< FT_Vector >::iterator it2 = vectorList.begin();    
    for( ; it1 != glyphList.end(); ++it1, ++it2 )
    {
        FT_BBox   glyph_bbox;

        FT_Glyph_Get_CBox( (*it1), ft_glyph_bbox_pixels, &glyph_bbox );

        glyph_bbox.xMin += (*it2).x;
        glyph_bbox.xMax += (*it2).x;
        glyph_bbox.yMin += (*it2).y;
        glyph_bbox.yMax += (*it2).y;

        if (glyph_bbox.xMin < bbox.xMin)
            bbox.xMin = glyph_bbox.xMin;

        if (glyph_bbox.yMin < bbox.yMin)
            bbox.yMin = glyph_bbox.yMin;

        if (glyph_bbox.xMax > bbox.xMax)
            bbox.xMax = glyph_bbox.xMax;

        if (glyph_bbox.yMax > bbox.yMax)
            bbox.yMax = glyph_bbox.yMax;
    }

    // check that we really grew the string bbox
    if ( bbox.xMin > bbox.xMax )
    {
        bbox.xMin = 0;
        bbox.yMin = 0;
        bbox.xMax = 0;
        bbox.yMax = 0;
    }
}

//-----------------------------------------------------------------------------
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

        renderTexChar( 
            slot->bitmap.buffer, 
            (byte *)&pTexture[0][0], 
            uTexX, uTexY, 
            slot->bitmap.rows, slot->bitmap.width, 
            pen_dx, pen_dy );        

		pen_x += slot->advance.x >> 6;
	}

    Image img;
    img.loadRawData( 
        DataChunk( pTexture, uTexX * uTexY * 3 ), 
        uTexX, uTexY, Image::FMT_RGB );
    TextureManager::getSingleton().loadImage( strTexName, img );	

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

    std::list< FT_Glyph > glyphs;
    std::list< FT_Vector > vectors;

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
        FT_Vector diff;
        FT_Glyph lett;
        
        glyph_index = FT_Get_Char_Index( *m_pFace, strString.at(i) );                
        slot = (*m_pFace)->glyph;

        if( use_kerning && previous && glyph_index )
        {
            FT_Vector  delta;
            FT_Get_Kerning( *m_pFace, previous, glyph_index, 0, &delta );
            pen_x += delta.x >> 6;
        }        
        diff.x = pen_x;
        diff.y = pen_y;        

        FT_Load_Glyph( (*m_pFace), glyph_index, FT_LOAD_DEFAULT );
        FT_Get_Glyph( (*m_pFace)->glyph, &(lett) );

        glyphs.push_back( lett );
        vectors.push_back( diff );        

        pen_x += slot->advance.x >> 6;
    }

    FT_BBox bbox;
    computeBBox( bbox, glyphs, vectors );

    // compute string dimensions in integer pixels
    unsigned string_width  = (bbox.xMax - bbox.xMin);
    unsigned string_height = (bbox.yMax - bbox.yMin);

    // compute start pen position in 26.6 cartesian pixels
    unsigned start_x = (( uTexX - string_width )/2);
    unsigned start_y = (( uTexY - string_height)/2);

    renderAlphaString(
        glyphs,
        vectors,
        (byte*)pTexture,
        uTexX, uTexY,
        start_x, start_y,
        cRed, cGreen, cBlue );

    Image img;
    img.loadRawData( 
        DataChunk( pTexture, uTexX * uTexY * 4 ), 
        uTexX, uTexY, Image::FMT_RGB_ALPHA );
    TextureManager::getSingleton().loadImage( strTexName, img );

    delete [] pTexture;
}

END_OGRE_NAMESPACE