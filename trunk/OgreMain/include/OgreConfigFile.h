/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

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
#ifndef __ConfigFile_H__
#define __ConfigFile_H__

#include "OgrePrerequisites.h"

#include "OgreString.h"
#include "OgreStringVector.h"
#include "OgreIteratorWrappers.h"

namespace Ogre {

    /** Class for quickly loading settings from a text file.
        @remarks
            This class is designed to quickly parse a simple file containing
            key/value pairs, mainly for use in configuration settings.
        @par
            This is a very simplified approach, no multiple values per key
            are allowed, no grouping or context is being kept etc.
        @par
            By default the key/values pairs are tokenised based on a
            separator of Tab, the colon (:) or equals (=) character. Each
            key - value pair must end in a carriage return.
    */
    class _OgreExport ConfigFile
    {
    public:

        ConfigFile();
        void load(const String& filename, const String& separators = "\t:=");

        /** Gets the first setting from the file with the named key. */
        String getSetting(const String& key);
        /** Gets all settings from the file with the named key. */
        StringVector getMultiSetting(const String& key);

        typedef MapIterator< std::multimap<String, String> > SettingsIterator;
        /** Gets an iterator for stepping through all the keys / values in the file. */
        SettingsIterator getSettingsIterator(void);

    protected:
        std::multimap<String, String> mSettings;
    };

}


#endif
