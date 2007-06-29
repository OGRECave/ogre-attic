#include "LexiStdAfx.h"

CProfiler* CProfiler::ms_Singleton = new CProfiler();

CProfiler& CProfiler::getSingleton( void )
{	
	assert(ms_Singleton);
	return ( *ms_Singleton ); 
}

CProfiler* CProfiler::getSingletonPtr( void )
{ 
	return ms_Singleton; 
}

CProfiler::CProfiler() : m_iMaxNameLength(0)
{

#if PROFILER_ENABLED
	m_bEnabled = true;
#else
	m_bEnabled = false;
#endif

	LARGE_INTEGER qFreq;
	QueryPerformanceFrequency(&qFreq);
	m_dFrequency = qFreq.QuadPart;
	m_dFrequency/=1000.0;
}

CProfiler::~CProfiler()
{
}

void CProfiler::StartSection(const char* pszSectionName)
{
	SProfileInfo* profile = NULL;
	TProfileMap::const_iterator iter = m_lProfiles.find(pszSectionName);
	if(iter == m_lProfiles.end())
	{
		profile = new SProfileInfo;
		m_lProfiles.insert(TProfileMap::value_type(pszSectionName, profile));
	}
	else
		profile = iter->second;

	if(profile->bRunning)
		LOGERROR "Profile (%s) is already running!", pszSectionName);

	unsigned int iNameLen = strlen(pszSectionName);
	if(m_iMaxNameLength < iNameLen) m_iMaxNameLength = iNameLen;

	profile->iCallCount++;
	profile->bRunning = true;
	QueryPerformanceCounter( &(profile->qStartTime) );	
}

void CProfiler::EndSection(const char* pszSectionName)
{
	SProfileInfo* profile = NULL;
	TProfileMap::const_iterator iter = m_lProfiles.find(pszSectionName);
	if(iter == m_lProfiles.end())
	{
		LOGERROR "Profile (%s) not created!", pszSectionName);
		return;
	}
	else
		profile = iter->second;

	if(!profile->bRunning)
	{
		LOGERROR "Profile (%s) is not running!", pszSectionName);
		return;
	}

	profile->bRunning = false;
	LARGE_INTEGER qEnd;
	QueryPerformanceCounter( &qEnd );

	qEnd.QuadPart-=profile->qStartTime.QuadPart;
	double time = qEnd.QuadPart;
	time/=m_dFrequency;

	profile->fTotalTime += time;
	profile->fAverageTime = profile->fTotalTime/double(profile->iCallCount);
}

void CProfiler::WriteLog(const char* pszFilename)
{
	if(!m_bEnabled)
		return;

	std::stringstream outString;

	// Header
	outString	<< setfill(' ') << std::setw(m_iMaxNameLength+1) << left << "Profile: "
				<< std::setw(10+1) << right << "Count: "
				<< std::setw(10+1) << right << "Total: "
				<< std::setw(10+1) << right << "Average: "
				<< std::endl
				<< setfill('-') << std::setw(m_iMaxNameLength+1+11+11+10) << '-' << std::endl;

	TProfileMap::const_iterator iter = m_lProfiles.begin();
	while(iter != m_lProfiles.end())
	{
		const SProfileInfo* profile = iter->second;
		outString	<< setfill('.') << std::setw(m_iMaxNameLength) << left << iter->first.c_str() << " "	//Profile
					<< setfill(' ') << std::setw(10) << right << profile->iCallCount << " "					//Count
					<< std::setw(10) << right << setprecision(1) << fixed << profile->fTotalTime << " "		//Total
					<< std::setw(10) << right << setprecision(1) << fixed << profile->fAverageTime			//Average
					<< std::endl;
		iter++;
	}


	HANDLE hFileHandle=CreateFile(pszFilename, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, NULL);	
	if(hFileHandle!=INVALID_HANDLE_VALUE) 
	{
		DWORD dwWritten;
		std::string outstr = outString.str();
		const char *pszMessage = outstr.c_str();
		WriteFile(hFileHandle, pszMessage, (DWORD)strlen(pszMessage), &dwWritten, NULL);
		CloseHandle(hFileHandle);
	}
	else
		LOGERROR "Could not write profile report file: %s", pszFilename);

	CleanRecords();
}

void CProfiler::CleanRecords( void )
{
	TProfileMap::const_iterator iter = m_lProfiles.begin();
	while(iter!= m_lProfiles.end())
	{
		delete iter->second;
		iter++;
	}
	m_lProfiles.clear();
}