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

#include "stdafx.h"

CConsoleLogger::CConsoleLogger(bool bLogTimes, ELogLevel eMinLevel)
{
	m_bLogTimes=bLogTimes;

	// Setup colors
	memset(m_wColors,0xFF,sizeof(WORD)*255);
	m_wColors[LOG_INFO]		=FOREGROUND_GREEN|FOREGROUND_INTENSITY;
	m_wColors[LOG_DEBUG]	=FOREGROUND_BLUE|FOREGROUND_INTENSITY;
	m_wColors[LOG_WARNING]	=FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_INTENSITY;
	m_wColors[LOG_ERROR]	=FOREGROUND_RED|FOREGROUND_INTENSITY;
	
	m_eMinLevel=eMinLevel;
	AllocConsole();
	m_hConsoleHandle=GetStdHandle(STD_OUTPUT_HANDLE);
}
CConsoleLogger::~CConsoleLogger(void)
{
	FreeConsole();
}

// Set level color. Mostly used for custom logging levels.
void CConsoleLogger::SetLevelColor(int iMessageLevel, WORD wColor)
{
	if(iMessageLevel<255 && iMessageLevel>=0) m_wColors[iMessageLevel]=wColor;
}

// Called from the logsystem
void CConsoleLogger::ReceiveLogMessage(SYSTEMTIME &LogTime, const char *pszTimeStr, int iMessageLevel, const char *pszMessage)
{
	// Do we want to write the message?
	if(iMessageLevel<m_eMinLevel) return;

	// Setup color
	if(iMessageLevel<255 && m_wColors[iMessageLevel]!=0xFFFF)
		SetConsoleTextAttribute(m_hConsoleHandle, m_wColors[iMessageLevel]);

	// Write to console
	DWORD dwWritten;
	if(m_bLogTimes)
		WriteConsole(m_hConsoleHandle, pszTimeStr, (DWORD)strlen(pszTimeStr), &dwWritten, NULL);
	WriteConsole(m_hConsoleHandle, pszMessage, (DWORD)strlen(pszMessage), &dwWritten, NULL);
}