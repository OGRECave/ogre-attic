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
http://www.gnu.org/copyleft/lgpl.html.
-----------------------------------------------------------------------------
*/

#ifndef __LogManager_H__
#define __LogManager_H__

#include "OgrePrerequisites.h"

#include "OgreLog.h"
#include "OgreSingleton.h"

namespace Ogre {

    /** The log manager handles the creation and retrieval of logs for the
        application.
        @remarks
            This class will create new log files and will retrieve instances
            of existing ones. Other classes wishing to log output can either
            create a fresh log or retrieve an existing one to output to.
            One log is the default log, and is the one written to when the
            logging methods of this class are called.
    */
    class _OgreExport LogManager : public Singleton<LogManager>
    {
    protected:
        typedef std::map<String, Log*, std::less<String> > LogList;

        /// A list of all the logs the manager can access
        LogList mLogs;

        /// The default log to which output is done
        Log* mDefaultLog;

    public:
        LogManager();
        ~LogManager();

        /** Creates a new log with the given name.
            @param
                name The name to give the log e.g. 'Ogre.log'
            @param
                defaultLog If true, this is the default log output will be
                sent to if the generic logging methods on this class are
                used. The first log created is always the default log unless
                this parameter is set.
            @param
                debuggerOutput If true, output to this log will also be
                routed to the debugger's output window.
        */
        Log* createLog( const String& name, bool defaultLog = false, bool debuggerOutput = true );

        /** Retrieves a log managed by this class.
        */
        Log* getLog( const String& name);

        /** Returns a pointer to the default log.
        */
        Log* getDefaultLog();

        /** Log a message to the default log.
        */
        void logMessage( const String& message, LogMessageLevel lml = LML_NORMAL);

        /** Log a message, smarter form - allows a variable number of
            arguments.
            @note
                <br>The longest string that this function can output is 4096
                characters long.
        */
        void logMessage( LogMessageLevel lml, const char* szMessage, ... );

        /** Sets the level of detail of the default log.
        */
        void setLogDetail(LoggingLevel ll);

        /** Override standard Singleton retrieval.
            @remarks
                Why do we do this? Well, it's because the Singleton
                implementation is in a .h file, which means it gets compiled
                into anybody who includes it. This is needed for the
                Singleton template to work, but we actually only want it
                compiled into the implementation of the class based on the
                Singleton, not all of them. If we don't change this, we get
                link errors when trying to use the Singleton-based class from
                an outside dll.
            @par
                This method just delegates to the template version anyway,
                but the implementation stays in this single compilation unit,
                preventing link errors.
        */
        static LogManager& getSingleton(void);
    };

}

#endif
