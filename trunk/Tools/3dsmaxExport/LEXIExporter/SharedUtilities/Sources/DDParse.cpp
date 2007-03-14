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

CDDParse::CDDParse(void)
{
	REGISTER_MODULE("DDParse");
}
CDDParse::~CDDParse(void)
{
	UNREGISTER_MODULE;
}

// Parse the ASCII string to a DDObject
// (can return NULL, if a fatal error occurs)
CDDObject*	CDDParse::Parse(const char *pszASCII)
{
	m_pszData=new char[strlen(pszASCII)+2];
	strcpy(m_pszData,pszASCII);	
	m_pszDataPtr=m_pszData;
	m_iLine=1;

	CDDObject *pObj=ParseObject();
	delete[] m_pszData;
	return pObj;
}

// Try to load specified file and parse it as ASCII DDObject
CDDObject* CDDParse::ParseFromFile(const char *pszFilename)
{
	// Open file
	HANDLE hFile=CreateFile(pszFilename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	// Try with different sharing
	if(hFile==INVALID_HANDLE_VALUE) hFile=CreateFile(pszFilename, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);

	// Check error code
	if(hFile==INVALID_HANDLE_VALUE) 
	{
		LOGERROR "Unable to open config file: %s", pszFilename);
		return false;
	}
	// Get Size of file
	DWORD lLen=GetFileSize(hFile, NULL);

	// Allocate and read
	m_pszData=new char[lLen+2];
	DWORD dwRead=0;
	ReadFile(hFile, m_pszData, lLen, &dwRead, 0);
	if(dwRead!=lLen)
	{
		LOGERROR "Failed to read from config file: %s", pszFilename);
		CloseHandle(hFile);
		return false;
	}
	
	// Zero terminate
	m_pszData[lLen]=0;
	CloseHandle(hFile);
	
	m_pszDataPtr=m_pszData;
	m_iLine=1;

	// Replace
	char buffer[_MAX_PATH];
	LPTSTR pFilePart;
	GetFullPathName(pszFilename, _MAX_PATH, buffer, &pFilePart);
	*pFilePart=0;
	if(buffer[strlen(buffer)-1]=='\\') buffer[strlen(buffer)-1]=0;
//	if(buffer[strlen(buffer)-1]!='\\') strcat(buffer, "\\");
	faststring sSearch="$CurrentDir";
	faststring sReplace=buffer;
	DoReplace(sSearch, sReplace);

	// Parse 
	CDDObject *pObj=ParseObject();
	delete[] m_pszData;

	return pObj;	
}

// -----------
void CDDParse::ParseDefine(void)
{
	faststring sSearchStr, sReplaceStr;

	// Skip spaces to search string
	SkipSpaces();
	const char *pszStartSearch=m_pszDataPtr;

	// Skip to end of search directive
	while(!isspace(*m_pszDataPtr) && *m_pszDataPtr) ++m_pszDataPtr;
	if(!m_pszDataPtr) 
	{
		LOGWARNING "#define directive - abnormal string termination (missing space?) (line=%d)", m_iLine);
		return;
	}
	
	// Assign the string
	sSearchStr.assign(pszStartSearch,(m_pszDataPtr-pszStartSearch));	

	// Skip to replace token
	SkipSpaces();

	const char *pszEnd=strchr(m_pszDataPtr,'\r');
	if(!pszEnd) 
	{
		LOGWARNING "#define must run to EOL (line=%d)", m_iLine);
		return;
	}
	sReplaceStr.assign(m_pszDataPtr,(pszEnd-m_pszDataPtr));
	m_pszDataPtr=pszEnd+1;

	// Replace
	DoReplace(sSearchStr, sReplaceStr);
}

// -----------
void CDDParse::DoReplace(faststring &sSearchStr, faststring &sReplaceStr)
{
	int iOverlap=sSearchStr.size()-sReplaceStr.size();
	// Enough or more space to simply replace?
	if(iOverlap>=0)
	{
		char *pszReplace=(char*)m_pszDataPtr;
		while(pszReplace!=0)
		{
			pszReplace=strstr(pszReplace,sSearchStr.c_str());
			if(pszReplace)
			{
				strncpy(pszReplace, sReplaceStr.c_str(), sReplaceStr.size());
				pszReplace+=sReplaceStr.size();

				if(iOverlap)
					memcpy(pszReplace,pszReplace+iOverlap,strlen(pszReplace)-iOverlap);
			}
		}
	} else
	{
		int iReplaceCount=0;
		char *pszReplace=(char*)m_pszDataPtr;
		while(pszReplace!=0)
		{
			pszReplace=strstr(pszReplace,sSearchStr.c_str());
			if(pszReplace)
			{
				iReplaceCount++;			
				pszReplace+=sSearchStr.size();
			}
		}
		if(iReplaceCount)
		{	
			iOverlap=-iOverlap;	// negate the overlap count

			// BOUNDS check this section! 
			char *pszNewData=new char[(iReplaceCount*iOverlap)+strlen(m_pszData)+2];
			memset(pszNewData, 0, (iReplaceCount*iOverlap)+strlen(m_pszData)+2);

			// Copy the original data
			strcpy(pszNewData,m_pszData);

			// Get the index
			int iIndex=m_pszDataPtr-m_pszData;

			// Setup the replacment pointer
			pszReplace=pszNewData+iIndex;
			while(pszReplace!=0)
			{
				// Try to find the search string
				pszReplace=strstr(pszReplace,sSearchStr.c_str());
				if(pszReplace)
				{
					// Found, move the rest to make room for the replacement
					memmove(	pszReplace+sReplaceStr.size(),
								pszReplace+(sReplaceStr.size()-iOverlap),
								strlen(pszReplace)-(sReplaceStr.size()-iOverlap));
					strncpy(pszReplace,sReplaceStr.c_str(),sReplaceStr.size());
					pszReplace+=sReplaceStr.size();
				}
			}
			// Free the original data and setup the pointers for the new.
			delete[] m_pszData;
			m_pszDataPtr=pszNewData+iIndex;
			m_pszData=pszNewData;
		}
	}
}

// -----------
void CDDParse::ParseInclude(void)
{
	faststring sIncPath;
	GetNextToken(sIncPath);

	// THIS IS A FIX: The first time the #include occurs it has not been replaced.
	char buffer1[_MAX_PATH];
	strcpy(buffer1, sIncPath.c_str());
	char* pDest = strstr(buffer1,"$CurrentDir");
	if(pDest != NULL)
	{
		//replace $CurrentDir token
		pDest = strstr(buffer1,"\\");
		pDest++;
		// Replace
		char buffer2[_MAX_PATH];
		LPTSTR pFilePart2;
		GetFullPathName(pDest, _MAX_PATH, buffer2, &pFilePart2);

		sIncPath.clear();
		sIncPath.add( buffer2 );
	}

	// Open file
	HANDLE hFile=CreateFile(sIncPath.c_str(),GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);

	// Try with different sharing
	if(hFile==INVALID_HANDLE_VALUE) hFile=CreateFile(sIncPath.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);

	// 
	if(hFile==INVALID_HANDLE_VALUE)
	{
		LOGERROR "Unable to open include file");
		return ;
	}
	DWORD dwSize=GetFileSize(hFile,NULL);
	if(!dwSize) 
	{
		CloseHandle(hFile);
		return;
	}

	char *pszNewData=new char[dwSize+strlen(m_pszData)+2];
/*	int iIndex=m_pszDataPtr-m_pszData;

	m_pszDataPtr=pszNewData+iIndex;
	// Copy the first of the original data
	memcpy(pszNewData,m_pszData,iIndex);*/

	// Copy the included data
	DWORD dwBytesRead;
//	ReadFile(hFile,pszNewData+iIndex,dwSize,&dwBytesRead,NULL);
	ReadFile(hFile,pszNewData,dwSize,&dwBytesRead,NULL);
	CloseHandle(hFile);
	
//	memcpy(pszNewData+iIndex+dwBytesRead,m_pszData+iIndex,strlen(m_pszData)-iIndex);
	memcpy(pszNewData+dwBytesRead,m_pszDataPtr,strlen(m_pszDataPtr));
	pszNewData[dwBytesRead+strlen(m_pszDataPtr)]=0;
	delete[] m_pszData;
	m_pszData=pszNewData;
	m_pszDataPtr=m_pszData;

	// Replace
	char buffer[_MAX_PATH];
	LPTSTR pFilePart;
	GetFullPathName(sIncPath.c_str(), _MAX_PATH, buffer, &pFilePart);
	*pFilePart=0;
//	if(buffer[strlen(buffer)-1]!='\\') strcat(buffer, "\\");
	if(buffer[strlen(buffer)-1]=='\\') buffer[strlen(buffer)-1]=0;
	faststring sSearch="$CurrentDir";
	faststring sReplace=buffer;
	DoReplace(sSearch, sReplace);
}

// -----------
CDDObject *CDDParse::ParseObject(void)
{
	CDDObject	*pNewObject=new CDDObject;

	do 
	{
		faststring sID;
		if(!GetNextToken(sID)) break;
		if(sID[0]=='}') break;	// End of section
		// Include / Define statments
		if(sID[0]=='#') 
		{
			faststring sTmp;
			GetNextToken(sTmp);
			// Handle defines
			if(_stricmp(sTmp.c_str(),"define")==0)
			{
				ParseDefine();
				continue;
			}

			// Handle includes
			if(_stricmp(sTmp.c_str(),"include")==0)
			{
				ParseInclude();
				continue;
			}
		}

		if(IsTokenChar(sID[0]))
		{
			LOGWARNING  "Expected to find value identifier, but found token char (line=%d, ID=%s) ", m_iLine, sID.c_str());
			break;
		}
		faststring sTmp;
		if(!GetNextToken(sTmp)) 
		{
			LOGWARNING  "Expected token after value identifier (line=%d, ID=%s) ", m_iLine, sID.c_str());
			break;
		}
		if(sTmp[0]!='=') LOGWARNING  "Expected '=' sign after value identifier (line=%d, ID=%s, Value=%s)", m_iLine, sID.c_str(), sTmp.c_str());
		if(!GetNextToken(sTmp)) 
		{
			LOGWARNING  "Expected token after '=' (line=%d, ID=%s, Value=%s)", m_iLine, sID.c_str(), sTmp.c_str());
			break;
		}

		faststring sType="";
		// Is the next token type specifier?
		if(sTmp[0]=='(') 
		{	// yep, let's parse the type			
			if(!GetNextToken(sType)) 
			{
				LOGWARNING  "Expected type specifier after '(' (line=%d)", m_iLine);
				break;
			}
			if(!GetNextToken(sTmp) || sTmp[0]!=')')
			{
				LOGWARNING  "Expected ')' after type specifier (line=%d)", m_iLine);
				break;
			}

			if(sType.compare("int")==0)
				ParseInt(sID.c_str(), pNewObject);
			else if(sType.compare("float")==0)
				ParseFloat(sID.c_str(), pNewObject);
			else if(sType.compare("string")==0)
				ParseString(sID.c_str(), pNewObject);
//			else if(sType.compare("binary")==0)
//				ParseBinary(sID.c_str(), pNewObject);
			else if(sType.compare("ddobject")==0)
				ParseDD(sID.c_str(), pNewObject);
			else if(sType.compare("intlist")==0)
				ParseIntList(sID.c_str(), pNewObject);
			else if(sType.compare("floatlist")==0)
				ParseFloatList(sID.c_str(), pNewObject);
			else if(sType.compare("stringlist")==0)
				ParseStringList(sID.c_str(),pNewObject);
			else if(sType.compare("ddlist")==0)
				ParseDDList(sID.c_str(), pNewObject);
			else if(sType.compare("vec3")==0)
				ParseVec3(sID.c_str(), pNewObject);
			else if(sType.compare("vec3list")==0)
				ParseVec3List(sID.c_str(), pNewObject);
			else if(sType.compare("bool")==0)
				ParseBool(sID.c_str(), pNewObject);
			else if(sType.compare("vec2")==0)
				ParseVec2(sID.c_str(), pNewObject);
			else if(sType.compare("vec4")==0)
				ParseVec4(sID.c_str(), pNewObject);
			else if(sType.compare("vec4list")==0)
				ParseVec4List(sID.c_str(), pNewObject);
			else if(sType.compare("matrix")==0)
				ParseMatrix(sID.c_str(), pNewObject);
			else if(sType.compare("matrixlist")==0)
				ParseMatrixList(sID.c_str(), pNewObject);
			else
				LOGWARNING  "Unknown type identifier found (%s) at line %d", sType.c_str(), m_iLine);
		} else
		{
			ParseUnknown(sTmp.c_str(), sID.c_str(), pNewObject);
		}		

		GetNextToken(sTmp);
		if(sTmp[0]!=';')
			LOGWARNING  "Expected ';' after value (line=%d, sID=%s, Value=%s)", m_iLine-1, sID.c_str(), sTmp.c_str());
	} while(true);
	return pNewObject;
}

// -----------
bool CDDParse::IsTokenChar(const char cChar)
{
	return (cChar=='=' || cChar==',' || cChar==';' ||
			cChar=='{' || cChar=='}' || cChar=='(' || cChar==')' || cChar=='#' );
}

// -----------
bool CDDParse::GetNextToken(faststring& sResult)
{	
	// Skip white spacing
	do 
	{
		SkipSpaces();
		if(!*m_pszDataPtr) return false;		
		if(*m_pszDataPtr=='/' && *(m_pszDataPtr+1)=='*')
		{
			m_pszDataPtr+=2;
			m_pszDataPtr=strstr(m_pszDataPtr, "*/");
			if(m_pszDataPtr==NULL) return false;
			m_pszDataPtr+=2;
			continue;
		}
		if(*m_pszDataPtr!='/' || *(m_pszDataPtr+1)!='/') break;
		SkipToEOL();
	} while(true);

	// Check for end of line (or data)
	if(!*m_pszDataPtr || *m_pszDataPtr=='\r' || *m_pszDataPtr=='\n') return false;
	
	// Check if we hit a token char
	if(IsTokenChar(*m_pszDataPtr))
	{
		sResult=*m_pszDataPtr;
		m_pszDataPtr++;
		return true;
	}

	// Check for quoted token
	if( *m_pszDataPtr=='"' )
	{
		m_pszDataPtr++;	// skip quote

		// TODO! This section should check for escaped characters

		// Find endquote
		const char *pszEnd=strchr(m_pszDataPtr,'"');
		if(pszEnd==NULL) 
		{
			LOGWARNING  "Missing endquote after '%.20s' at line %d", m_pszDataPtr, m_iLine);
			return false;
		}

		// Assign entire quoted expression
		sResult.assign(m_pszDataPtr,(pszEnd-m_pszDataPtr));

		// Go to end of quoted expression and skip endquote
		m_pszDataPtr=pszEnd+1;
		return true;
	}

	const char *pszBegin=m_pszDataPtr;
	while( *m_pszDataPtr && !isspace(*m_pszDataPtr) && !IsTokenChar(*m_pszDataPtr))
	{		
		m_pszDataPtr++;
	}

	sResult.assign(pszBegin, (m_pszDataPtr)-pszBegin);
	return true;
}

// -----------
void CDDParse::ParseUnknown(const char *pszUnknown, const char *pszID,CDDObject *pObj)
{
	char *pEndPtr;
	int iValue=strtol(pszUnknown,&pEndPtr,0);
	if(!iValue && pEndPtr==pszUnknown)
		pObj->SetString(pszID, pszUnknown);
	else		
		pObj->SetInt(pszID, iValue);
}

// -----------
void CDDParse::ParseInt(const char *pszID,CDDObject *pObj)
{
	faststring sNumber;
	if(GetNextToken(sNumber))
	{
		char *pEndPtr;
		int iValue=strtol(sNumber.c_str(),&pEndPtr,0);
		pObj->SetInt(pszID,iValue);
	} else
	{
		LOGWARNING "Could not get string value (parsing int %s) (line=%d)", pszID, m_iLine);
	}
}

// -----------
void CDDParse::ParseFloat(const char *pszID,CDDObject *pObj)
{
	faststring sNumber;
	if(GetNextToken(sNumber))
	{
		char *pEndPtr;
		double fValue=strtod(sNumber.c_str(),&pEndPtr);
		pObj->SetFloat(pszID,(float)fValue);
	} else
	{
		LOGWARNING  "Could not get string value (parsing float %s) (line=%d)", pszID, m_iLine);
	}
}

// -----------
void CDDParse::ParseString(const char *pszID,CDDObject *pObj)
{
	faststring sString;
	if(GetNextToken(sString))
	{
		pObj->SetString(pszID,sString.c_str());
	} else
	{
		LOGWARNING "Could not get string value at line %d", m_iLine);
	}
}
/*
// -----------
void CDDParse::ParseBinary(const char *pszID,CDDObject *pObj)
{
	string sBinary;
	if(GetNextToken(sBinary) && sBinary.size()>1)
	{
		if(sBinary.size()&1)
			LOGWARNING "Binary value has uneven number of hex values");

		int iBinLen=sBinary.size()/2;
		BYTE *pBuffer=new BYTE[iBinLen];
		char digitbuf[6];
		strcpy(digitbuf,"0x");		// enforce heximal decode
		digitbuf[4]=0;
		for(int i=0;i<iBinLen;i++)
		{			
			// copy the digits to the buffer
			// (digitbuffer will then have the format 0xXX, where XX is the two copied
			//  ascii digits)
			strncpy(&digitbuf[2],sBinary.c_str()+(i*2),2);
			// convert the string to actual value
			char *pTmpPtr;
			pBuffer[i]=(BYTE)strtoul(digitbuf,&pTmpPtr,0);
		}
		pObj->SetBinary(sID.c_str(),pBuffer,iBinLen);
		delete pBuffer;
	} else
	{
		LOGERROR "Could not get string value (parsing binary)");
	}
}*/

// -----------
void CDDParse::ParseDD(const char *pszID,CDDObject *pObj)
{
	faststring sTmp;
	GetNextToken(sTmp);
	if(sTmp[0]!='{') 
		LOGWARNING  "DDObject definitions should start with '{' (line=%d)", m_iLine);
	CDDObject *pDDObj=ParseObject();
	if(pDDObj) 
	{
		pObj->SetDDObject(pszID,pDDObj);
		pDDObj->Release();
	}
}

// -----------
void CDDParse::ParseDDList(const char *pszID,CDDObject *pObj)
{
	fastvector<const CDDObject*> lDDObjects;

	faststring sTmp;
	GetNextToken(sTmp);
	if(sTmp[0]!='{') 
		LOGWARNING  "DDlist definitions should start with '{' (line=%d)", m_iLine);

	do 
	{
		GetNextToken(sTmp);
		if(sTmp[0]!='{') 
			LOGWARNING  "DDObject definitions should start with '{' (line=%d)", m_iLine);
		CDDObject *pDDObj=ParseObject();
		lDDObjects.push_back(pDDObj);
		// pDDObj->Release() should not be necessary

		GetNextToken(sTmp);
	} while(sTmp[0]==',');
	if(sTmp[0]!='}')
		LOGWARNING  "DDList definition should end with '}' (line=%d)", m_iLine);
	pObj->SetDDList(pszID, lDDObjects,false);
}

// -----------
void CDDParse::ParseIntList(const char *pszID,CDDObject *pObj)
{
	fastvector<int> lValues;
	int iLevel=0;
	faststring sTmp="";
	do 
	{		
		GetNextToken(sTmp);
		if(sTmp[0]=='{') iLevel++;
		else if(sTmp[0]=='}') iLevel--;
		else if(!IsTokenChar(sTmp[0]))
		{
			char *pEndPtr;
			lValues.push_back(strtol(sTmp.c_str(),&pEndPtr,0));
		}
	} while(iLevel || sTmp[0]!='}');
	pObj->SetIntList(pszID, lValues);
}

// -----------
void CDDParse::ParseFloatList(const char *pszID,CDDObject *pObj)
{
	fastvector<float> lValues;
	int iLevel=0;
	faststring sTmp="";
	do 
	{		
		GetNextToken(sTmp);
		if(sTmp[0]=='{') iLevel++;
		else if(sTmp[0]=='}') iLevel--;
		else if(!IsTokenChar(sTmp[0]))
		{
			char *pEndPtr;
			lValues.push_back((float)strtod(sTmp.c_str(),&pEndPtr));
		}
	} while(iLevel || sTmp[0]!='}');
	pObj->SetFloatList(pszID, lValues);
}

// -----------
void CDDParse::ParseStringList(const char *pszID,CDDObject *pObj)
{
	vector<faststring> lValues;
	int iLevel=0;
	faststring sTmp="";
	do 
	{		
		GetNextToken(sTmp);
		if(sTmp[0]=='{') iLevel++;
		else if(sTmp[0]=='}') iLevel--;
		else if(!IsTokenChar(sTmp[0]))
			lValues.push_back(sTmp);
	} while(iLevel || sTmp[0]!='}');
	pObj->SetStringList(pszID, lValues);
}
/*
// -----------
void CDDParse::ParseStringList(const char *pszID,CDDObject *pObj)
{
	vector<string> lValues;
	int iLevel=0;
	string sTmp="";
	do 
	{		
		GetNextToken(sTmp);
		if(sTmp[0]=='{') iLevel++;
		else if(sTmp[0]=='}') iLevel--;
		else if(!IsTokenChar(sTmp[0]))
			lValues.push_back(sTmp);
	} while(iLevel || sTmp[0]!='}');
	pObj->SetStringList(sID.c_str(),&lValues);
}*/

void CDDParse::SkipToEOL(void)
{
	while(*m_pszDataPtr!='\n' && *m_pszDataPtr) m_pszDataPtr++;
//	++m_iLine;
}

void CDDParse::SkipSpaces(void)
{
	while(*m_pszDataPtr && (*m_pszDataPtr=='\r' || *m_pszDataPtr=='\n' || isspace(*m_pszDataPtr)))
	{
		if(*(m_pszDataPtr++)=='\r') ++m_iLine;
	}
}

// -----------
void CDDParse::ParseVec3(const char *pszID,CDDObject *pObj)
{
	faststring sNumber;
	if(GetNextToken(sNumber))
	{
		char *pEndPtr;
		double fValue=strtod(sNumber.c_str(),&pEndPtr);
		pObj->SetFloat(pszID,(float)fValue);
	} else
	{
		LOGWARNING  "Could not get string value (parsing vec3 %s) (line=%d)", pszID, m_iLine);
	}
}

// -----------
void CDDParse::ParseVec3List(const char *pszID,CDDObject *pObj)
{
	fastvector<float> lValues;
	int iLevel=0;
	faststring sTmp="";
	do 
	{		
		GetNextToken(sTmp);
		if(sTmp[0]=='{') iLevel++;
		else if(sTmp[0]=='}') iLevel--;
		else if(!IsTokenChar(sTmp[0]))
		{
			char *pEndPtr;
			lValues.push_back((float)strtod(sTmp.c_str(),&pEndPtr));
		}
	} while(iLevel || sTmp[0]!='}');
	pObj->SetFloatList(pszID, lValues);
}

// -----------
void CDDParse::ParseBool(const char *pszID,CDDObject *pObj)
{
	faststring sNumber;
	if(GetNextToken(sNumber))
	{
		bool bValue = _stricmp(sNumber.c_str(), "true") == 0 ? true : false;
		pObj->SetBool(pszID,bValue);
	} else
	{
		LOGWARNING "Could not get string value (parsing int %s) (line=%d)", pszID, m_iLine);
	}
}

// -----------
void CDDParse::ParseVec2(const char *pszID,CDDObject *pObj)
{
	faststring sNumber;
	if(GetNextToken(sNumber))
	{
		char *pEndPtr;
		double fValue=strtod(sNumber.c_str(),&pEndPtr);
		pObj->SetFloat(pszID,(float)fValue);
	} else
	{
		LOGWARNING  "Could not get string value (parsing vec2 %s) (line=%d)", pszID, m_iLine);
	}
}

// -----------
void CDDParse::ParseVec4(const char *pszID,CDDObject *pObj)
{
	faststring sNumber;
	if(GetNextToken(sNumber))
	{
		char *pEndPtr;
		double fValue=strtod(sNumber.c_str(),&pEndPtr);
		pObj->SetFloat(pszID,(float)fValue);
	} else
	{
		LOGWARNING  "Could not get string value (parsing vec4 %s) (line=%d)", pszID, m_iLine);
	}
}

// -----------
void CDDParse::ParseVec4List(const char *pszID,CDDObject *pObj)
{
	fastvector<float> lValues;
	int iLevel=0;
	faststring sTmp="";
	do 
	{		
		GetNextToken(sTmp);
		if(sTmp[0]=='{') iLevel++;
		else if(sTmp[0]=='}') iLevel--;
		else if(!IsTokenChar(sTmp[0]))
		{
			char *pEndPtr;
			lValues.push_back((float)strtod(sTmp.c_str(),&pEndPtr));
		}
	} while(iLevel || sTmp[0]!='}');
	pObj->SetFloatList(pszID, lValues);
}

// -----------
void CDDParse::ParseMatrix(const char *pszID,CDDObject *pObj)
{
	faststring sNumber;
	if(GetNextToken(sNumber))
	{
		char *pEndPtr;
		double fValue=strtod(sNumber.c_str(),&pEndPtr);
		pObj->SetFloat(pszID,(float)fValue);
	} else
	{
		LOGWARNING  "Could not get string value (parsing matrix %s) (line=%d)", pszID, m_iLine);
	}
}

// -----------
void CDDParse::ParseMatrixList(const char *pszID,CDDObject *pObj)
{
	fastvector<float> lValues;
	int iLevel=0;
	faststring sTmp="";
	do 
	{		
		GetNextToken(sTmp);
		if(sTmp[0]=='{') iLevel++;
		else if(sTmp[0]=='}') iLevel--;
		else if(!IsTokenChar(sTmp[0]))
		{
			char *pEndPtr;
			lValues.push_back((float)strtod(sTmp.c_str(),&pEndPtr));
		}
	} while(iLevel || sTmp[0]!='}');
	pObj->SetFloatList(pszID, lValues);
}

