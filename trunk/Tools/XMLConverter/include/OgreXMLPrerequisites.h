/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2002 Steven J. Streeting
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

#ifndef __XMLPrerequisites_H__
#define __XMLPrerequisites_H__

#include "OgrePrerequisites.h"
#include "OgreString.h"
#include "OgreStringConverter.h"

// Include Xerces headers
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>

namespace Ogre {
    // ---------------------------------------------------------------------------
    //  This is a simple class that lets us do easy (though not terribly efficient)
    //  trancoding of char* data to XMLCh data.
    // ---------------------------------------------------------------------------
    class XStr
    {
    public :
        // -----------------------------------------------------------------------
        //  Constructors and Destructor
        // -----------------------------------------------------------------------
        XStr(const char* const toTranscode)
        {
            // Call the private transcoding method
            XMLString::transcode(toTranscode, bufUnicode, 1024);
        }
        XStr(String toTranscode)
        {
            // Call the private transcoding method
            XMLString::transcode(toTranscode.c_str(), bufUnicode, 1024);
        }
        XStr(Real toTranscode)
        {
            String temp = StringConverter::toString(toTranscode);
            // Call the private transcoding method
            XMLString::transcode(temp.c_str(), bufUnicode, 1024);
        }
        XStr(unsigned short toTranscode)
        {
            String temp = StringConverter::toString(toTranscode);
            // Call the private transcoding method
            XMLString::transcode(temp.c_str(), bufUnicode, 1024);
        }
        XStr(short toTranscode)
        {
            String temp = StringConverter::toString(toTranscode);
            // Call the private transcoding method
            XMLString::transcode(temp.c_str(), bufUnicode, 1024);
        }
        XStr(unsigned long toTranscode)
        {
            String temp = StringConverter::toString(toTranscode);
            // Call the private transcoding method
            XMLString::transcode(temp.c_str(), bufUnicode, 1024);
        }
        XStr(long toTranscode)
        {
            String temp = StringConverter::toString(toTranscode);
            // Call the private transcoding method
            XMLString::transcode(temp.c_str(), bufUnicode, 1024);
        }


        ~XStr()
        {
        }


        // -----------------------------------------------------------------------
        //  Getter methods
        // -----------------------------------------------------------------------
        const XMLCh* unicodeForm() const
        {
            return bufUnicode;
        }

    private :
        XMLCh bufUnicode[1024];
    };

    #define X(str) XStr(str).unicodeForm()

}

#endif
