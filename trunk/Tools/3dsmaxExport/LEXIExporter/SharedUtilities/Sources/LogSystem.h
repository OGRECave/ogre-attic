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

#ifndef __LOGSYSTEM__
#define __LOGSYSTEM__

// The basic loglevels
enum ELogLevel
{
	LOG_INFO=0,
	LOG_DEBUG=1,
	LOG_WARNING=2,
	LOG_ERROR=3,
	LOG_MSG_CUSTOM_BEGIN=0x10
};

// The LogReceiver interface - implement this interface and call CLogSystem::AddReceiver() to receive
// log messages.
class ILogReceiver : public CRefCount
{
public:
	virtual ~ILogReceiver(void) {};

	// Called when a log message is received.
	virtual void	ReceiveLogMessage(SYSTEMTIME &LogTime, const char *pszTimeStr, int iLevel, const char *pszMessage)=0;
};

// The logsystem is a singleton class - the instance will be created at the first Get() call.
// Please note: By default there is no LogReceivers associated with the logsystem.
class CLogSystem
{
public:
	// Constructor/Destructor
	CLogSystem(void);
	~CLogSystem(void);

	// Static get function - so we can reach the logsystem
	static CLogSystem *Get(void);

	// Set min. log level
	void	SetMinLogLevel(ELogLevel eLevel) { m_eMinLogLevel=eLevel; }

	// Add a message to log
	void	LogMessage(const void *pModule, int iLevel, const char *pszText, ...);

	// Register a system module
	void	RegisterModule(const void *pModulePointer, const char *pszModuleDesc);

	// Register a system module
	void	UnregisterModule(const void *pModulePointer);

	// Add a log receiver
	void	AddReceiver(ILogReceiver *pReceiver);

	// Remove a log receiver
	void	RemoveReceiver(ILogReceiver *pReceiver);

private:
	// Pointer to the instance of the log system
	static CLogSystem *m_pThis;

	// Minimum logging level
	ELogLevel		m_eMinLogLevel;

	// Map of registered modules
	fastintmap< char* >	m_lModules;

	// List of registered receivers
	fastvector< ILogReceiver* >	m_lReceivers;

	CRITICAL_SECTION	m_Crit;
};

/*
	LOG SYSTEM MACROS
*/
#ifndef _DISABLE_LOGGING
	
	// Module registration
	#define REGISTER_MODULE(pszDesc)	CLogSystem::Get()->RegisterModule(this, pszDesc);
	#define UNREGISTER_MODULE			CLogSystem::Get()->UnregisterModule(this);

	// Low priority log macros
	#ifndef _DISABLE_LOWPRIOLOG
		#define LOGINFO		CLogSystem::Get()->LogMessage(this, LOG_INFO,
		#define LOGDEBUG	CLogSystem::Get()->LogMessage(this, LOG_DEBUG,
	#else
		#define LOGINFO		(
		#define LOGDEBUG	(
	#endif

	// High priority log macros
	#define LOGWARNING	CLogSystem::Get()->LogMessage(this, LOG_WARNING,
	#define LOGERROR	CLogSystem::Get()->LogMessage(this, LOG_ERROR,
	#define LOGCUSTOM	CLogSystem::Get()->LogMessage(this, 

	// Macros for static / thread functions
	#define LOGSTATIC	CLogSystem::Get()->LogMessage(NULL,
	#define LOGTHREAD	CLogSystem::Get()->LogMessage(	
#else
	
	// Logging is disabled; all macros will create blank code
	#define REGISTER_MODULE(pThisPointer, pszDesc)
	#define UNREGISTER_MODULE(pThisPointer)
	#define LOGINFO		(
	#define LOGDEBUG	(
	#define LOGWARNING	(
	#define LOGERROR	(
	#define LOGSTATIC	(	
#endif

#endif
