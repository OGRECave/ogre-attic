/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

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
#include "OgreStableHeaders.h"

#include "OgreFontManager.h"
#include "OgreFont.h"
#include "OgreSDDataChunk.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreStringVector.h"
#include "OgreException.h"

namespace Ogre
{
    //---------------------------------------------------------------------------------------------
    template<> FontManager * Singleton< FontManager >::ms_Singleton = 0;
    FontManager* FontManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    FontManager& FontManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
    //---------------------------------------------------------------------------------------------

    //---------------------------------------------------------------------
    Resource* FontManager::create(const String& name)
    {
	    // Check name not already used
	    if (getByName(name) != 0)
		    Except(Exception::ERR_DUPLICATE_ITEM, "Font " + name + " already exists.",
			    "FontManager::create");

	    Font* f = new Font(name);
        // Don't load yet, defer until used

        mResources.insert(ResourceMap::value_type(name, f));
        return f;
    }
    //---------------------------------------------------------------------
    void FontManager::parseScript( DataChunk& chunk )
    {
        String line;
        Font *pFont = 0;

        while( !chunk.isEOF() )
        {
            line = chunk.getLine();
            // Ignore blanks & comments
            if( !line.length() || line.substr( 0, 2 ) == "//" )
            {
                continue;
            }
            else
            {
			    if (pFont == 0)
			    {
				    // No current font
				    // So first valid data should be font name
				    pFont = (Font*)create(line);
				    // Skip to and over next {
                    chunk.skipUpTo("{");
			    }
			    else
			    {
				    // Already in font
				    if (line == "}")
				    {
					    // Finished 
					    pFont = 0;
                        // NB font isn't loaded until required
				    }
                    else
                    {
                        parseAttribute(line, pFont);
                    }
                }
            }
        }
    }
    //---------------------------------------------------------------------
    void FontManager::parseAllSources( const String& extension )
    {
        StringVector fontfiles;
        SDDataChunk chunk;

        std::vector< ArchiveEx * >::iterator i = mVFS.begin();

        for( ; i < mVFS.end(); i++ )
        {
            fontfiles = (*i)->getAllNamesLike( "./", extension );
            for( StringVector::iterator j = fontfiles.begin(); j != fontfiles.end(); j++ )
            {
                DataChunk *p = &chunk;
                (*i)->fileRead( *j, &p );
                parseScript( chunk );
            }
        }
        for( i = mCommonVFS.begin(); i < mCommonVFS.end(); i++ )
        {
            fontfiles = (*i)->getAllNamesLike( "./", extension );
            for( StringVector::iterator j = fontfiles.begin(); j != fontfiles.end(); j++ )
            {
                DataChunk *p = &chunk;
                (*i)->fileRead( *j, &p );
                parseScript( chunk );
            }
        }
    }
    //---------------------------------------------------------------------
    void FontManager::parseAttribute(const String& line, Font* pFont)
    {
        std::vector<String> params = line.split();
        String attrib = params[0].toLowerCase();
        if (attrib == "type")
        {
            // Check params
            if (params.size() != 2)
            {
                logBadAttrib(line, pFont);
                return;
            }
            // Set
            if (params[1].toLowerCase() == "truetype")
            {
                pFont->setType(FT_TRUETYPE);
            }
            else
            {
                pFont->setType(FT_IMAGE);
            }

        }
        else if (attrib == "source")
        {
            // Check params
            if (params.size() != 2)
            {
                logBadAttrib(line, pFont);
                return;
            }
            // Set
            pFont->setSource(params[1]);
        }
        else if (attrib == "glyph")
        {
            // Check params
            if (params.size() != 6)
            {
                logBadAttrib(line, pFont);
                return;
            }
            // Set
            pFont->setGlyphTexCoords(
                params[1].at(0), 
                StringConverter::parseReal(params[2]),
                StringConverter::parseReal(params[3]),
                StringConverter::parseReal(params[4]),
                StringConverter::parseReal(params[5]) );
        }
        else if (attrib == "size")
        {
            // Check params
            if (params.size() != 2)
            {
                logBadAttrib(line, pFont);
                return;
            }
            // Set
            pFont->setTrueTypeSize(
                StringConverter::parseReal(params[1]) );
        }
        else if (attrib == "resolution")
        {
            // Check params
            if (params.size() != 2)
            {
                logBadAttrib(line, pFont);
                return;
            }
            // Set
            pFont->setTrueTypeResolution(
                (uint)StringConverter::parseReal(params[1]) );
        }
        else if (attrib == "antialias_colour")
        {
        	// Check params
        	if (params.size() != 2)
        	{
                logBadAttrib(line, pFont);
                return;
        	}
        	// Set
            pFont->setAntialiasColour(StringConverter::parseBool(params[1]));
        }



    }
    //---------------------------------------------------------------------
    void FontManager::logBadAttrib(const String& line, Font* pFont)
    {
        LogManager::getSingleton().logMessage("Bad attribute line: " + line +
            " in font " + pFont->getName());

    }

}
