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
#include "OgreStringConverter.h"
#include "OgreVector3.h"
#include "OgreMatrix3.h"
#include "OgreMatrix4.h"
#include "OgreQuaternion.h"
#include "OgreColourValue.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    String StringConverter::toString(Real val)
    {
        String::StrStreamType stream;
        stream << val;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(int val)
    {
        String::StrStreamType stream;
        stream << val;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(unsigned int val)
    {
        String::StrStreamType stream;
        stream << val;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(long val)
    {
        String::StrStreamType stream;
        stream << val;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(unsigned long val)
    {
        String::StrStreamType stream;
        stream << val;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(const Vector3& val)
    {
        String::StrStreamType stream;
        stream << val.x << " " << val.y << " " << val.z;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(const Matrix3& val)
    {
        String::StrStreamType stream;
        stream << val[0][0] << " " 
            << val[0][1] << " "             
            << val[0][2] << " "             
            << val[1][0] << " "             
            << val[1][1] << " "             
            << val[1][2] << " "             
            << val[2][0] << " "             
            << val[2][1] << " "             
            << val[2][2];
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(bool val)
    {
        if (val)
            return "true";
        else
            return "false";

    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(const Matrix4& val)
    {
        String::StrStreamType stream;
        stream << val[0][0] << " " 
            << val[0][1] << " "             
            << val[0][2] << " "             
            << val[0][3] << " "             
            << val[1][0] << " "             
            << val[1][1] << " "             
            << val[1][2] << " "             
            << val[1][3] << " "             
            << val[2][0] << " "             
            << val[2][1] << " "             
            << val[2][2] << " "             
            << val[2][3] << " "             
            << val[3][0] << " "             
            << val[3][1] << " "             
            << val[3][2] << " "             
            << val[3][3];
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(const Quaternion& val)
    {
        String::StrStreamType stream;
        stream  << val.w << " " << val.x << " " << val.y << " " << val.z;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    String StringConverter::toString(const ColourValue& val)
    {
        String::StrStreamType stream;
        stream << val.r << " " << val.g << " " << val.b << " " << val.a;
        return stream.str();
    }
    //-----------------------------------------------------------------------
    Real StringConverter::parseReal(const String& val)
    {
        return atof(val.c_str());
    }
    //-----------------------------------------------------------------------
    int StringConverter::parseInt(const String& val)
    {
        return atoi(val.c_str());
    }
    //-----------------------------------------------------------------------
    unsigned int StringConverter::parseUnsignedInt(const String& val)
    {
        return atoi(val.c_str());
    }
    //-----------------------------------------------------------------------
    long StringConverter::parseLong(const String& val)
    {
        return atol(val.c_str());
    }
    //-----------------------------------------------------------------------
    unsigned long StringConverter::parseUnsignedLong(const String& val)
    {
        return atol(val.c_str());
    }
    //-----------------------------------------------------------------------
    bool StringConverter::parseBool(const String& val)
    {
        if (val == "true")
            return true;
        else
            return false;
    }
    //-----------------------------------------------------------------------
    Vector3 StringConverter::parseVector3(const String& val)
    {
        // Split on space
        std::vector<String> vec = val.split();

        if (vec.size() != 3)
        {
            return Vector3::ZERO;
        }
        else
        {
            return Vector3(parseReal(vec[0]),parseReal(vec[1]),parseReal(vec[2]));
        }

    }
    //-----------------------------------------------------------------------
    Matrix3 StringConverter::parseMatrix3(const String& val)
    {
        // Split on space
        std::vector<String> vec = val.split();

        if (vec.size() != 9)
        {
            return Matrix3::IDENTITY;
        }
        else
        {
            return Matrix3(parseReal(vec[0]),parseReal(vec[1]),parseReal(vec[2]),
                parseReal(vec[3]),parseReal(vec[4]),parseReal(vec[5]),
                parseReal(vec[6]),parseReal(vec[7]),parseReal(vec[8]));
        }
    }
    //-----------------------------------------------------------------------
    Matrix4 StringConverter::parseMatrix4(const String& val)
    {
        // Split on space
        std::vector<String> vec = val.split();

        if (vec.size() != 16)
        {
            return Matrix4::IDENTITY;
        }
        else
        {
            return Matrix4(parseReal(vec[0]),parseReal(vec[1]),parseReal(vec[2]), parseReal(vec[3]),
                parseReal(vec[4]),parseReal(vec[5]), parseReal(vec[6]), parseReal(vec[7]),
                parseReal(vec[8]),parseReal(vec[9]), parseReal(vec[10]), parseReal(vec[11]),
                parseReal(vec[12]),parseReal(vec[13]), parseReal(vec[14]), parseReal(vec[15]));
        }
    }
    //-----------------------------------------------------------------------
    Quaternion StringConverter::parseQuaternion(const String& val)
    {
        // Split on space
        std::vector<String> vec = val.split();

        if (vec.size() != 4)
        {
            return Quaternion::IDENTITY;
        }
        else
        {
            return Quaternion(parseReal(vec[0]),parseReal(vec[1]),parseReal(vec[2]), parseReal(vec[3]));
        }
    }
    //-----------------------------------------------------------------------
    ColourValue StringConverter::parseColourValue(const String& val)
    {
        // Split on space
        std::vector<String> vec = val.split();

        if (vec.size() == 4)
        {
            return ColourValue(parseReal(vec[0]),parseReal(vec[1]),parseReal(vec[2]), parseReal(vec[3]));
        }
        else if (vec.size() == 3)
        {
            return ColourValue(parseReal(vec[0]),parseReal(vec[1]),parseReal(vec[2]), 1.0f);
        }
        else
        {
            return ColourValue::Black;
        }
    }
}


