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

#include "StdAfx.h"

// Static instance pointer
CLogSystem* CLogSystem::m_pThis=NULL;

CLogSystem::CLogSystem(void)
{
	m_eMinLogLevel=LOG_INFO;
	InitializeCriticalSection(&m_Crit);
}
CLogSystem::~CLogSystem(void)
{
	if(m_pThis==this) m_pThis=NULL;

	EnterCriticalSection(&m_Crit);

	// Release all receivers
	for(unsigned i=0;i<m_lReceivers.size();i++)
		m_lReceivers[i]->Release();
	m_lReceivers.clear();

	DeleteCriticalSection(&m_Crit);
}

// Static get function - so we can reach the logsystem
CLogSystem *CLogSystem::Get(void)
{
	if(m_pThis==NULL) m_pThis=new CLogSystem;
	return m_pThis;
}

// Add a custom level message
void CLogSystem::LogMessage(const void *pModule, int iLevel, const char *pszText, ...)
{
	// Check if we want to log this message?
	if(iLevel<m_eMinLogLevel) return;

	EnterCriticalSection(&m_Crit);
	try 
	{
		static char buff1[4096];	
		static char buff2[4096];	

		// Get the module descriptor
		char *pszModule="Legacy";
		if(pModule)
		{
			pszModule="Unknown";
			m_lModules.find((unsigned int)pModule, pszModule);
		}		

		// Parse the string
		va_list args;
		va_start(args, pszText);
		_vsnprintf(buff1, sizeof(buff1) , pszText, args);

		// Create string containing both module and logmessage
		_snprintf(buff2, sizeof(buff2), "%s: %s\r\n", pszModule, buff1);

		// Get timestamp
		SYSTEMTIME	lSysTime;
		GetLocalTime(&lSysTime);
		_snprintf(buff1, sizeof(buff1), "%02u:%02u:%02u/%02u", lSysTime.wHour, lSysTime.wMinute, lSysTime.wSecond, lSysTime.wMilliseconds);

		// Distribute message to receivers
		for(unsigned i=0;i<m_lReceivers.size();i++)
			if(m_lReceivers[i]!=pModule) m_lReceivers[i]->ReceiveLogMessage(lSysTime, buff1, iLevel, buff2);		

	} catch(...)
	{
		// Nothing much to do, is there?
	}

	LeaveCriticalSection(&m_Crit);
}

// Register a system module
void CLogSystem::RegisterModule(const void *pModulePointer, const char *pszModuleDesc)
{
	EnterCriticalSection(&m_Crit);
	m_lModules.map((int)pModulePointer, _strdup(pszModuleDesc));
	LeaveCriticalSection(&m_Crit);
}

// Register a system module
void CLogSystem::UnregisterModule(const void *pModulePointer)
{
	EnterCriticalSection(&m_Crit);
	char *pOldDesc=NULL;
	m_lModules.erase((int)pModulePointer, pOldDesc);
	if(pOldDesc) free(pOldDesc);
	LeaveCriticalSection(&m_Crit);
}

// Add a log receiver
void CLogSystem::AddReceiver(ILogReceiver *pReceiver)
{
	pReceiver->AddRef();
	m_lReceivers.push_back(pReceiver);	
}

// Remove a log receiver
void CLogSystem::RemoveReceiver(ILogReceiver *pReceiver)
{
	for(unsigned int i=0;i<m_lReceivers.size();i++)
	{
		if(m_lReceivers[i]==pReceiver)
		{
			pReceiver->Release();
			m_lReceivers.erase(i);
			return;
		}
	}
}