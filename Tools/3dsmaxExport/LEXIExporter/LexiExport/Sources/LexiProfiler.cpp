#include "LexiStdAfx.h"

#include <dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")

template<> CProfiler* Ogre::Singleton<CProfiler>::ms_Singleton = new CProfiler();

CProfiler& CProfiler::getSingleton( void )
{	
	assert(ms_Singleton);
	//if(ms_Singleton==NULL)
	//	ms_Singleton = new CProfiler();
	return ( *ms_Singleton ); 
}

CProfiler* CProfiler::getSingletonPtr( void )
{ 
	//if(ms_Singleton==NULL)
	//	ms_Singleton = new CProfiler();

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

void CProfiler::StartSection(std::string sectionName )
{
	SProfileInfo* profile = NULL;
	std::map< std::string, SProfileInfo* >::iterator iter = m_lProfiles.find(sectionName);
	if(iter == m_lProfiles.end())
	{
		profile = new SProfileInfo(sectionName);
		m_lProfiles.insert(std::pair< std::string, SProfileInfo* >(sectionName, profile));
	}
	else
		profile = iter->second;

	if(profile->bRunning)
		LOGERROR "Profile (%s) is already running!", profile->sName.c_str());

	if(m_iMaxNameLength < profile->sName.length())
		m_iMaxNameLength = profile->sName.length();

	profile->iCallCount++;
	profile->bRunning = true;
	QueryPerformanceCounter( &(profile->qStartTime) );	
}

void CProfiler::EndSection(std::string sectionName )
{
	SProfileInfo* profile = NULL;
	std::map< std::string, SProfileInfo* >::iterator iter = m_lProfiles.find(sectionName);
	if(iter == m_lProfiles.end())
	{
		LOGERROR "Profile (%s) not created!", sectionName.c_str());
		return;
	}
	else
		profile = iter->second;

	if(!profile->bRunning)
	{
		LOGERROR "Profile (%s) is not running!", profile->sName.c_str());
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

void CProfiler::WriteLog( std::string fileName )
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

	std::map< std::string, SProfileInfo* >::iterator iter = m_lProfiles.begin();
	while(iter!= m_lProfiles.end())
	{
		SProfileInfo* profile = iter->second;
		outString	<< setfill('.') << std::setw(m_iMaxNameLength) << left << profile->sName << " "			//Profile
					<< setfill(' ') << std::setw(10) << right << profile->iCallCount << " "					//Count
					<< std::setw(10) << right << setprecision(1) << fixed << profile->fTotalTime << " "		//Total
					<< std::setw(10) << right << setprecision(1) << fixed << profile->fAverageTime			//Average
					<< std::endl;
		//LOGINFO "Profile: %s \tCount %i \tTotal: %.2f \tAverage %.2f", profile->sName.c_str(), profile->iCallCount, profile->fTotalTime, profile->fAverageTime);
		iter++;
	}

	// We always write to the <MAXDIR>\LEXIExporter\Logs Directory
	char szAppPath[MAX_PATH] = "";
	::GetModuleFileName(NULL,szAppPath,sizeof(szAppPath) - 1);
	Ogre::String cwd(szAppPath);
	Ogre::String fileNameTMP, filePath;
	Ogre::StringUtil::splitFilename(cwd, fileNameTMP, filePath);

	int n = filePath.find("/");
	while(n != Ogre::String::npos)
	{
		filePath.replace(n,1,"\\");
		n = filePath.find("/");
	}

	filePath+="LEXIExporter\\Logs\\";

	fileName = filePath + fileName;

	if(::MakeSureDirectoryPathExists(filePath.c_str()))
	{
		//filePath+=fileName;
		HANDLE m_hFileHandle=CreateFile(fileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, NULL);	
		if(m_hFileHandle==INVALID_HANDLE_VALUE) 
			LOGERROR "Could not write profile report file: %s", fileName.c_str());

		DWORD dwWritten;
		std::string outstr = outString.str();
		const char *pszMessage = outstr.c_str();
		WriteFile(m_hFileHandle, pszMessage, (DWORD)strlen(pszMessage), &dwWritten, NULL);

		CloseHandle(m_hFileHandle);
	}
	else
		LOGERROR "Could not write profile report file: %s", fileName.c_str());

	CleanRecords();
}

void CProfiler::CleanRecords( void )
{
	std::map< std::string, SProfileInfo* >::iterator iter = m_lProfiles.begin();
	while(iter!= m_lProfiles.end())
	{
		delete iter->second;
		iter++;
	}
	m_lProfiles.clear();
}