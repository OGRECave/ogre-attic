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
#include "OgreStableHeaders.h"

#include "OgreLogManager.h"

#include "OgreException.h"

// Required for va_start, va_end and sprintf
#include <stdio.h>
#include <stdarg.h>

namespace Ogre {

    //-----------------------------------------------------------------------
    template<> LogManager* Singleton<LogManager>::ms_Singleton = 0;
    //-----------------------------------------------------------------------
    LogManager::LogManager()
    {
        mDefaultLog = NULL;
    }
    //-----------------------------------------------------------------------
    LogManager::~LogManager()
    {
        // Destroy all logs
        LogList::iterator i;
        for (i = mLogs.begin(); i != mLogs.end(); ++i)
        {
            delete i->second;
        }
    }
    //-----------------------------------------------------------------------
    Log* LogManager::createLog( const String& name, bool defaultLog, bool debuggerOutput)
    {
        Log* newLog = new Log(name, debuggerOutput);

        if( !mDefaultLog || defaultLog )
        {
            mDefaultLog = newLog;
        }

        mLogs.insert( LogList::value_type( name, newLog ) );

        return newLog;
    }
    //-----------------------------------------------------------------------
    Log* LogManager::getDefaultLog()
    {
        if (!mDefaultLog)
            Except(Exception::ERR_INVALIDPARAMS, "No logs created yet. ", "LogManager::getDefaultLog");

        return mDefaultLog;
    }
    //-----------------------------------------------------------------------
    Log* LogManager::getLog( const String& name)
    {
        LogList::iterator i = mLogs.find(name);
        if (i != mLogs.end())
            return i->second;
        else
            Except(Exception::ERR_INVALIDPARAMS, "Log not found. ", "LogManager::getLog");


    }
    //-----------------------------------------------------------------------
    void LogManager::logMessage( const String& message, LogMessageLevel lml)
    {
        getDefaultLog()->logMessage(message, lml);
    }
    //-----------------------------------------------------------------------
    void LogManager::logMessage( LogMessageLevel lml, const char* szMessage, ... )
    {
        static char szBuffer[4097];
        va_list list;
        va_start( list, szMessage );

        vsnprintf( szBuffer, 4096, szMessage, list );
        getDefaultLog()->logMessage( szBuffer, lml );

        va_end( list );
    }
    //-----------------------------------------------------------------------
    void LogManager::setLogDetail(LoggingLevel ll)
    {
        getDefaultLog()->setLogDetail(ll);
    }
    //-----------------------------------------------------------------------
    LogManager& LogManager::getSingleton(void)
    {
        return Singleton<LogManager>::getSingleton();
    }
}
