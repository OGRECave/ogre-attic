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

#include "OgreLog.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    Log::Log( const String& name, bool debuggerOuput )
    {
        mfpLog.open(name);
        mDebugOut = debuggerOuput;
        mLogLevel = LL_NORMAL;
    }
    //-----------------------------------------------------------------------
    Log::~Log()
    {
        mfpLog.close();
    }
    //-----------------------------------------------------------------------
    void Log::logMessage( const String& message, LogMessageLevel lml )
    {
        if ((mLogLevel + lml) >= LOG_THRESHOLD)
        {
            if (mDebugOut)
                fprintf( stderr, "%s\n", message.c_str());

            // Write time into log
           struct tm *pTime;
           time_t ctTime; time(&ctTime);
           pTime = gmtime( &ctTime );
            mfpLog << pTime->tm_hour << ":" << pTime->tm_min << ":" << pTime->tm_sec << ": " << message << std::endl;

            // Flush stcmdream to ensure it is written (incase of a crash, we need log to be up to date)
            mfpLog.flush();
        }

    }
    //-----------------------------------------------------------------------
    void Log::setLogDetail(LoggingLevel ll)
    {
        mLogLevel = ll;
    }
}
