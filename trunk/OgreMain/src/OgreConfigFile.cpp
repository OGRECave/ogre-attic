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
#include "OgreStableHeaders.h"
#include "OgreConfigFile.h"

#include "OgreException.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    ConfigFile::ConfigFile()
    {
    }
    //-----------------------------------------------------------------------
    void ConfigFile::load(const String& filename, const String& separators, bool trimWhitespace)
    {
        FILE *fp;
        char rec[100], *ret;
        String optName, optVal;

        mSettings.clear();

        // Open and parse entire file
        fp = fopen(filename.c_str(), "r");
        if( !fp )
            Except(
                Exception::ERR_FILE_NOT_FOUND, "'" + filename + "' file not found!", "ConfigFile::load" );

        ret = fgets(rec, 100, fp);
        while (ret != NULL)
        {
            String tst = rec;
            StringUtil::trim(tst);
            // Ignore comments & blanks
            if (tst.length() > 0 && tst.at(0) != '#' && tst.at(0) != '@' && tst.at(0) != '\n')
            {
                // Tokenise on tab
                char* pName = strtok(rec, separators.c_str());
                char* pVal = strtok(NULL, "\n");
                if (pName && pVal)
                {
                    String optName = pName;
                    String optVal = pVal;
                    if (trimWhitespace)
                    {
                        StringUtil::trim(optVal);
                        StringUtil::trim(optName);
                    }
                    mSettings.insert(std::multimap<String, String>::value_type(optName, optVal));
                }
            }
            ret = fgets(rec, 100, fp);
        }

        fclose(fp);


    }
    //-----------------------------------------------------------------------
    String ConfigFile::getSetting(const String& key) const
    {
        std::multimap<String, String>::const_iterator i;

        i = mSettings.find(key);
        if (i == mSettings.end())
        {
            return "";
        }
        else
        {
            return i->second;
        }
    }
    //-----------------------------------------------------------------------
    StringVector ConfigFile::getMultiSetting(const String& key) const
    {
        StringVector ret;

        std::multimap<String, String>::const_iterator i;

        i = mSettings.find(key);
        // Iterate over matches
        while (i != mSettings.end() && i->first == key)
        {
            ret.push_back(i->second);
            ++i;
        }

        return ret;


    }
    //-----------------------------------------------------------------------
    ConfigFile::SettingsIterator ConfigFile::getSettingsIterator(void)
    {
        return SettingsIterator(mSettings.begin(), mSettings.end());
    }

}
