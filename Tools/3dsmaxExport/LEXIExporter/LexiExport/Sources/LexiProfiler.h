/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Mark Folkenberg

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

#ifndef __NDS_LexiExporter_Profiler__
#define __NDS_LexiExporter_Profiler__

#define PROFILER_ENABLED 0

class CProfiler : public Ogre::Singleton<CProfiler>
{
	struct SProfileInfo
	{
		std::string		sName;
		unsigned int	iCallCount;
		double			fAverageTime;	//Milliseconds
		double			fTotalTime;		//Milliseconds
		LARGE_INTEGER	qStartTime;		//
		bool			bRunning;

		SProfileInfo( std::string profileName ) : iCallCount(0), fAverageTime(0.0), fTotalTime(0.0), bRunning(false)
		{
			sName = profileName;
			qStartTime.QuadPart = 0;
		}
	};

public:
	CProfiler();
	~CProfiler();

	static CProfiler& getSingleton( void );
	static CProfiler* getSingletonPtr( void );

	void StartSection(std::string sectionName );
	void EndSection(std::string sectionName );
	void WriteLog( std::string fileName );
	void CleanRecords( void );

private:

	bool m_bEnabled;

	double m_dFrequency;
	unsigned int m_iMaxNameLength;
	std::map< std::string, SProfileInfo* >	m_lProfiles;

};

#if PROFILER_ENABLED
#define START_PROFILE(name) CProfiler::getSingletonPtr()->StartSection(name);
#define END_PROFILE(name) CProfiler::getSingletonPtr()->EndSection(name);
#else
#define START_PROFILE(name) ;
#define END_PROFILE(name) ;
#endif

#endif