/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#include "OgreStableHeaders.h"
#include "OgreString.h"

#include "OgreStringVector.h"

namespace Ogre {

    String String::BLANK = String("");
    //-----------------------------------------------------------------------
    void String::trim(bool left, bool right)
    {
        size_t lspaces, rspaces, len = length(), i;

        lspaces = rspaces = 0;

        if( left )
        {
            // Find spaces / tabs on the left
            for( i = 0;
                i < len && ( at(i) == ' ' || at(i) == '\t' || at(i) == '\r');
                ++lspaces, ++i );
        }
        
        if( right && lspaces < len )
        {
            // Find spaces / tabs on the right
            for( i = len - 1;
                i >= 0 && ( at(i) == ' ' || at(i) == '\t' || at(i) == '\r');
                rspaces++, i-- );
        }

        *this = substr(lspaces, len-lspaces-rspaces);
    }

    //-----------------------------------------------------------------------
    std::vector<String> String::split( const String& delims, unsigned int maxSplits) const
    {
        // static unsigned dl;
        std::vector<String> ret;
        unsigned int numSplits = 0;

        // Use STL methods 
        size_t start, pos;
        start = 0;
        do 
        {
            pos = find_first_of(delims, start);
            if (pos == start)
            {
                // Do nothing
                start = pos + 1;
            }
            else if (pos == npos || (maxSplits && numSplits == maxSplits))
            {
                // Copy the rest of the string
                ret.push_back( substr(start) );
                break;
            }
            else
            {
                // Copy up to delimiter
                ret.push_back( substr(start, pos - start) );
                start = pos + 1;
            }
            // parse up to next real data
            start = find_first_not_of(delims, start);
            ++numSplits;

        } while (pos != npos);



        return ret;
    }

    //-----------------------------------------------------------------------
    String String::toLowerCase(void) 
    {
        std::transform(
            begin(),
            end(),
            begin(),
            static_cast<int(*)(int)>(::tolower) );

        return *this;
    }

    //-----------------------------------------------------------------------
    String String::toUpperCase(void) 
    {
        std::transform(
            begin(),
            end(),
            begin(),
            static_cast<int(*)(int)>(::toupper) );

        return *this;
    }
    //-----------------------------------------------------------------------
    Real String::toReal(void) const
    {
        return (Real)atof(this->c_str());
    }
    //-----------------------------------------------------------------------
    bool String::startsWith(const String& pattern, bool lowerCase) const
    {
        size_t thisLen = this->length();
        size_t patternLen = pattern.length();
        if (thisLen < patternLen || patternLen == 0)
            return false;

        String startOfThis = substr(0, patternLen);
        if (lowerCase)
            startOfThis.toLowerCase();

        return (startOfThis == pattern);
    }
    //-----------------------------------------------------------------------
    bool String::endsWith(const String& pattern, bool lowerCase) const
    {
        size_t thisLen = this->length();
        size_t patternLen = pattern.length();
        if (thisLen < patternLen || patternLen == 0)
            return false;

        String endOfThis = substr(thisLen - patternLen, patternLen);
        if (lowerCase)
            endOfThis.toLowerCase();

        return (endOfThis == pattern);
    }
    //-----------------------------------------------------------------------

}

