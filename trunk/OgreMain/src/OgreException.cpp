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
#include "OgreException.h"

#include "OgreRoot.h"
#include "OgreLogManager.h"

namespace Ogre {
    Exception* Exception::last = 0;
    std::list<String> Exception::msFunctionStack;

    Exception::Exception(int num, const String& desc, const String& src) :
        line( 0 ),
        number( num ),
        description( desc ),
        source( src ),
        file( "" )
    {
        // Log this error
        LogManager::getSingleton().logMessage(this->getFullDescription());

        // Set last
        last = this;
    }

    Exception::Exception(int num, const String& desc, const String& src, char* fil, long lin) :
        line( lin ),
        number( num ),
        description( desc ),
        source( src ),
        file( fil )
    {
        // Log this error
        LogManager::getSingleton().logMessage(this->getFullDescription());

        // Set last
        last = this;
    }

    Exception::Exception(const Exception& rhs)
        : line( rhs.line ), number( rhs.number ), description( rhs.description ), source( rhs.source ), file( rhs.file )
    {
    }

    void Exception::operator = ( const Exception& rhs )
    {
        description = rhs.description;
        number = rhs.number;
        source = rhs.source;
        file = rhs.file;
        line = rhs.line;
    }

    String Exception::getFullDescription(void) const
    {
        char strNum[12];
        String desc;

        /** \todo Optimize! Add all string assignments on one line, thus compromising readability */
        sprintf( strNum, "%d", number );
        desc =  "An exception has been thrown!\n"
                "\n"
                "-----------------------------------\nDetails:\n-----------------------------------\n"
                "Error #: ";
        desc += strNum;
        desc += "\nFunction: ";
        desc += source;
        desc += "\nDescription: ";
        desc += description;
        desc += ". ";

        if( line > 0 )
        {
            desc += "\nFile: ";
            desc += file;

            char szLine[20];

            desc += "\nLine: ";
            snprintf(szLine, 20, "%ld", line);

            desc += szLine;
        }

#ifdef OGRE_STACK_UNWINDING
        String funcstack;
        String currfunc;

        funcstack += "\nStack unwinding: ";

        for( std::list<String>::const_iterator it = msFunctionStack.begin(); it != msFunctionStack.end(); ++it )
        {
            funcstack += *it;
            funcstack += "()<-";
        }

        desc += funcstack;
        desc += "<<beginning of stack>>";
#endif

        return desc;
    }

    int Exception::getNumber(void) throw()
    {
        return number;
    }

    Exception* Exception::getLastException(void) throw()
    {
        return last;
    }

    //-----------------------------------------------------------------------
    void Exception::_pushFunction( const String& strFuncName ) throw()
    {
        msFunctionStack.push_front( strFuncName );
    }

    //-----------------------------------------------------------------------
    void Exception::_popFunction() throw()
    {
        msFunctionStack.pop_front();
    }
}

