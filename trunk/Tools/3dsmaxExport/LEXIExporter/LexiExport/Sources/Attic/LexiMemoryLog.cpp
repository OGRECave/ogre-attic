#include "LexiStdAfx.h"

CMemoryLog::CMemoryLog()
{
	CLogSystem::Get()->AddReceiver(this);	
}

CMemoryLog::~CMemoryLog()
{	
	CLogSystem::Get()->RemoveReceiver(this);
	Flush();
}

// Clear all log messages
void CMemoryLog::Flush()
{
	// Delete all log messages
	for(unsigned i=0;i<m_lMessages.size();i++)
		delete m_lMessages[i];
	
	// Clear log
	m_lMessages.clear();
	m_bImportantFlag=false;
}

// Query log to find out if any important messages (error/warning) has
// been received since last check. The check flag is reset per default.
bool CMemoryLog::LogImportant(bool bResetFlag)
{
	bool bRet=m_bImportantFlag;
	if(bResetFlag) m_bImportantFlag=false;
	return bRet;
}

void CMemoryLog::write( const Ogre::String& name, const Ogre::String& message, Ogre::LogMessageLevel lml, bool maskDebug)
{
	SMessage *pNewMessage=new SMessage;	
	
	pNewMessage->iLevel=4;
	pNewMessage->sMessage=message.c_str();
	m_lMessages.push_back(pNewMessage);
}

//
void CMemoryLog::ReceiveLogMessage(SYSTEMTIME &LogTime, const char *pszTimeStr, int iLevel, const char *pszMessage)
{
	SMessage *pNewMessage=new SMessage;
	pNewMessage->iLevel=iLevel;
	pNewMessage->sMessage=pszMessage;
	pNewMessage->sMessage[strlen(pszMessage)-2]=0;	// Remove \r\n
	m_lMessages.push_back(pNewMessage);
	m_bImportantFlag|=(iLevel==LOG_WARNING || iLevel==LOG_ERROR);	
}