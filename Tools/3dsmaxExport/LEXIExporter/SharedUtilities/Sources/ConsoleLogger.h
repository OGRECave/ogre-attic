/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Lasse Tassing

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

#ifndef __CONSOLE_LOGGER__
#define __CONSOLE_LOGGER__

class CConsoleLogger : public ILogReceiver
{
public:
	CConsoleLogger(bool bLogTimes=false, ELogLevel eMinLevel=LOG_INFO);
	~CConsoleLogger(void);

	// Set level color. Mostly used for custom logging levels.
	void SetLevelColor(int iMessageLevel, WORD wColor);	

	// Called from the logsystem
	void ReceiveLogMessage(SYSTEMTIME &LogTime, const char *pszTimeStr, int iMessageLevel, const char *pszMessage);

private:
	bool			m_bLogTimes;
	WORD			m_wColors[255];
	HANDLE			m_hConsoleHandle;
	ELogLevel		m_eMinLevel;
};

#endif
