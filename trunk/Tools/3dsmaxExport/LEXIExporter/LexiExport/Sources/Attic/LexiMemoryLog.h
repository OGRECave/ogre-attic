/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Mark Folkenberg,
Bo Krohn

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

#ifndef __NDS_LexiExporter_MemoryLog__
#define __NDS_LexiExporter_MemoryLog__

// Records and keeps all log messages in memory for later viewing
class CMemoryLog : public ILogReceiver, public Ogre::LogListener
{
public:
	CMemoryLog();
	~CMemoryLog();

	struct SMessage
	{
		std::string sMessage;
		int		iLevel;
	};

	// List of log messages
	std::vector<SMessage*>	m_lMessages;

	// Clear all log messages
	void	Flush();

	// Query log to find out if any important messages (error/warning) has
	// been received since last check. The check flag is reset per default.
	bool	LogImportant(bool bResetFlag=true);

private:
	// Called when an exporter log message is received.
	void	ReceiveLogMessage(SYSTEMTIME &LogTime, const char *pszTimeStr, int iLevel, const char *pszMessage);		

	// Called when an Ogre log message is received
	void	write( const Ogre::String& name, const Ogre::String& message, 
					Ogre::LogMessageLevel lml = Ogre::LML_NORMAL, 
					bool maskDebug = false );

	// Called when an Ogre log message is received (Eihort)
	void	messageLogged( const Ogre::String& message, Ogre::LogMessageLevel lml, bool maskDebug, const Ogre::String &logName );

	// Set true whenever a warning or error is received
	bool	m_bImportantFlag;
};

#endif