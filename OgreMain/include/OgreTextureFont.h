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
#ifndef __TextureFont_H__
#define __TextureFont_H__

#include "OgrePrerequisites.h"

// Hack to avoid needing to include the freetype
// headers in every client build
struct FT_LibraryRec_;
struct FT_FaceRec_;

namespace Ogre {

    class _OgreExport TextureFont
    {
    public:
        typedef std::map< String, std::map< String, TextureFont * > > TextureFontMap;

    protected:
	    static FT_LibraryRec_ **ms_pLibrary;    
	    static TextureFontMap ms_mapFonts;

	    FT_FaceRec_ **m_pFace;
	    static unsigned ms_uNumFonts;

    public:
	    TextureFont();
	    TextureFont( const String& strFontFile );
    	
	    void loadFromFile( const String& strFontFile );
    	
	    virtual ~TextureFont();	

	    void createTexture( 
		    const String& strTexName, const String& strString, 
		    unsigned uTexX, unsigned uTexY, 
		    unsigned uFontX = 600, unsigned uFontY = 0,
		    unsigned uFontResX = 72, unsigned uFontResY = 0, 
		    unsigned uStartX = 1, unsigned uStartY = 1 );	

        void createAlphaMask(
            const String& strTexName, const String& strString,
            unsigned uTexX, unsigned uTexY,
            uchar cRed, uchar cGreen, uchar cBlue,
            unsigned uFontX = 600, unsigned uFontY = 0,
            unsigned uFontResX = 72, unsigned uFontResY = 0,
            unsigned uStartX = 1, unsigned uStartY = 1 );

        static const TextureFontMap& getColl()
        {
            return ms_mapFonts;
        }
    };

} //namespace

#endif

