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
#include <vector>

CDDObject *SharedUtilities::GetCommandLineParms(void)
{
	// Get command line parameters for current process
	LPSTR lpStr=GetCommandLine();

	// Create new object
	CDDObject *pParms=new CDDObject;
	
	// Break command line into seperate strings
	std::vector< std::string > lCmdParms;
	bool	bActiveParm=false;
	char	cParmDelimiter=' ';
	LPSTR	pszParmStart=lpStr;
	while(*lpStr!=0)
	{
		// Check if we hit end-of-parameter delimiter (' ' or '"')
		if(*lpStr==cParmDelimiter)
		{
			std::string sNew;

			// Length of parameter
			int iLen=((int)(lpStr-pszParmStart));
			if(iLen>0)
			{
				// Yep, we have a valid parameter string - add it to the list of RAW command strings
				sNew.assign(pszParmStart, iLen);
				lCmdParms.push_back(sNew);
			}
			// Skip '"' and all white spaces
			if(*lpStr=='"') *lpStr++;
			while(isspace(*lpStr) && *lpStr!=0) lpStr++;

			// We assume next command is delimited by a ' ' (this will change, if we encounter a '"')
			cParmDelimiter=' ';
			pszParmStart=lpStr;
			bActiveParm=false;
		} else if(*lpStr=='"')
		{
			// We hit a '"' - then the parameter will be delimited by another '"'
			cParmDelimiter='"';
			++lpStr;
			if(!bActiveParm) 
			{
				// It was during the beginning of a parameter, skip the '"'
				pszParmStart=lpStr;
				bActiveParm=true;
			}
		} else 
		{
			// Skip to next character
			++lpStr;
			bActiveParm=true;
		}
	}
	// Check if there was one last parameter
	std::string sNew;
	int iLen=((int)(lpStr-pszParmStart));
	if(iLen>0)
	{
		sNew.assign(pszParmStart, iLen);
		lCmdParms.push_back(sNew);
	}

	// OK - check if we got any parameters at all?
	if(!lCmdParms.size()) return pParms;

	// Setup base information
	pParms->SetString("AppPath", lCmdParms[0].c_str());
//	pParms->SetStringList("RAWParms", lCmdParms);

	// Parse parameters (note that first argument is skipped - assumed to be application path)	
	for(unsigned p=1;p<lCmdParms.size();p++)
	{
		// Setup pointers
		const char *pArg=lCmdParms[p].c_str();

		// Try to find '=' or ':' - they delimit a value pair
		const char *pSep=strchr(pArg, '=');
		if(!pSep) pSep=strchr(pArg, ':');

		// If the current parameter string was delimited, we parse it!
		if(pSep)
		{
			// Skip tokens, spaces and '"'
			while((*pArg=='/' || *pArg=='+' || *pArg=='-' || isspace(*pArg) || *pArg=='"') && *pArg!=0) pArg++;
			if(!*pArg) continue;	// broken argument

			// Extract parameter name
			std::string sParmName;
			sParmName.assign(pArg, (int)(pSep-pArg));
//			for(unsigned ic=0;ic<sParmName.size();ic++)
//				tolower(sParmName[ic]);

			// Skip tokens, spaces and '"'
			while((*pSep=='=' || *pSep==':' || isspace(*pSep) || *pSep=='"') && *pSep!=0) pSep++;
			if(!pSep) continue;	// broken argument
            
			// Try to convert the value to a long
			char *pStop;
			long iValue=strtol(pSep, &pStop, 0);
			if(!iValue && pStop==pSep)
			{
				// Conversion failed - we assume it is a string
				pParms->SetString(sParmName.c_str(), pSep);
			}
			else 
			{
				// A comma seperated list of integers?
				if(*pStop==',')
				{
					// Yep, create list
					fastvector<int> lIntList;

					// Add the current value to the list
					lIntList.push_back(iValue);

					// Enter loop..
					do {
						// Skip past the ',' and all whitespaces
						pSep=pStop+1;
						while(isspace(*pSep) && *pSep!=0) pSep++;

						// Check if we hit last character
						if(*pSep==0) break;

						// Try to convert current string
						long iValue=strtol(pSep, &pStop, 0);

						// If it went well, we add the value to the list
						if(iValue || pStop!=pSep) lIntList.push_back(iValue);

						// Continue as long as the argument string is valid
					} while(pStop!=pSep && *pStop!=0);

					// Put the assembled integer list into the parameter object
					pParms->SetIntList(sParmName.c_str(), lIntList);
				} else
				{
					// Put the integer value into the parameter object
					pParms->SetInt(sParmName.c_str(), iValue);
				}
			}
		} else
		{
			// Skip any token charaters and add the parameter as an integer to the parameter object
			if(*pArg=='/' || *pArg=='+' || *pArg=='-')
				pParms->SetInt(pArg+1, *pArg);
			else
				pParms->SetInt(pArg, *pArg);
		}
	}

	// Return object
	return pParms;
}