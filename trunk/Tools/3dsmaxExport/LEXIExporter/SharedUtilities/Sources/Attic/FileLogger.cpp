/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
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

#include "stdafx.h"

CFileLogger::CFileLogger(const char *pszFile, bool bLogTimes, ELogLevel eMinLevel)
{
	m_bLogTimes=bLogTimes;
	m_eMinLevel=eMinLevel;
	m_hFileHandle=CreateFile(pszFile, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, NULL);	
}
CFileLogger::~CFileLogger(void)
{
	CloseHandle(m_hFileHandle);
//	FreeConsole();
}

// Called from the logsystem
void CFileLogger::ReceiveLogMessage(SYSTEMTIME &LogTime, const char *pszTimeStr, int iMessageLevel, const char *pszMessage)
{
	// Do we want to write the message?
	if(iMessageLevel<m_eMinLevel || m_hFileHandle==INVALID_HANDLE_VALUE) return;

	// Write to console
	DWORD dwWritten;
	if(m_bLogTimes)
		WriteFile(m_hFileHandle, pszTimeStr, (DWORD)strlen(pszTimeStr), &dwWritten, NULL);
	WriteFile(m_hFileHandle, pszMessage, (DWORD)strlen(pszMessage), &dwWritten, NULL);
}