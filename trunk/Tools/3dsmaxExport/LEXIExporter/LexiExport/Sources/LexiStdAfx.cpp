/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Mark Folkenberg,
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

#include "LexiStdAfx.h"
#include "LexiMaxExport.h"

#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

//

void RemoveIllegalChars(std::string& sString)
{
	std::string sRtn;

	for(unsigned int x = 0; x < sString.size(); x++)
	{
		char c = sString[x];
		if(c == ':' || c == '<' || c == '>' || c == '\\' || c == '/' || c == '?') continue;
		sRtn += c;
	}

	sString = sRtn;
}

//

bool ValidateFilenames()
{
	std::string sFilename;
	std::string sError;

	sFilename = FixupFilename("test.test", "mesh");
	if(sFilename.size() < 4 || sFilename[1] != ':' || sFilename[2] != '\\') sError += "- Meshes\r\n";

	sFilename = FixupFilename("test.test", "skeleton");
	if(sFilename.size() < 4 || sFilename[1] != ':' || sFilename[2] != '\\') sError += "- Skeletons\r\n";

	sFilename = FixupFilename("test.test", "material");
	if(sFilename.size() < 4 || sFilename[1] != ':' || sFilename[2] != '\\') sError += "- Materials\r\n";

	sFilename = FixupFilename("test.test", "texture");
	if(sFilename.size() < 4 || sFilename[1] != ':' || sFilename[2] != '\\') sError += "- Textures\r\n";

	sFilename = FixupFilename("test.test", "shader");
	if(sFilename.size() < 4 || sFilename[1] != ':' || sFilename[2] != '\\') sError += "- Shaders\r\n";

	sFilename = FixupFilename("test.test", "scene");
	if(sFilename.size() < 4 || sFilename[1] != ':' || sFilename[2] != '\\') sError += "- Scenes\r\n";

	sFilename = FixupFilename("test.test", "animation");
	if(sFilename.size() < 4 || sFilename[1] != ':' || sFilename[2] != '\\') sError += "- Animations\r\n";

	if(sError.size())
	{
		char strCurDir[512];
		GetCurrentDirectory(510, strCurDir);

		char strTemp[4096];
		sprintf(strTemp, "The following object(s) will be exported relatively to current directory (%s):\r\n\r\n%s\r\nContinue exporting anyway?", strCurDir, sError.c_str());

		unsigned int iRS = MessageBox(CExporter::Get()->GetMax()->GetMAXHWnd(), strTemp, "Warning", MB_ICONWARNING | MB_YESNO);
		if(iRS == IDNO) return false;
	}

	return true;
}

std::string FixupFilename(const char* pszFilename, const char* pszType)
{
	unsigned int iLen = strlen(pszFilename);
	if(iLen >= 3 && pszFilename[1] == ':' && pszFilename[2] == '\\') return pszFilename;

	CDDObject* pConfig = CExporter::Get()->GetRootConfig();
	if(!pConfig) return pszFilename;

	std::string sPath;
	if(!stricmp(pszType, "mesh")) sPath = pConfig->GetString("DefaultMeshFolder", "");
	else if(!stricmp(pszType, "skeleton")) sPath = pConfig->GetString("DefaultSkeletonFolder", "");
	else if(!stricmp(pszType, "material")) sPath = pConfig->GetString("DefaultMaterialFolder", "");
	else if(!stricmp(pszType, "texture")) sPath = pConfig->GetString("DefaultTextureFolder", "");
	else if(!stricmp(pszType, "shader")) sPath = pConfig->GetString("DefaultShaderFolder", "");
	else if(!stricmp(pszType, "scene")) sPath = pConfig->GetString("DefaultSceneFolder", "");
	else if(!stricmp(pszType, "animation")) sPath = pConfig->GetString("DefaultAnimationFolder", "");
	else if(!stricmp(pszType, "root")) sPath = pConfig->GetString(ResolveMacros("$(username)@$(hostname).RootFolder").c_str(), "");
	else return pszFilename;

	if(sPath.size() && sPath[sPath.size() - 1] != '\\') sPath += "\\";
	sPath += pszFilename;

	return stricmp(pszType, "root") ? FixupFilename(sPath.c_str(), "root") : sPath;
}

//

std::string ResolveMacros(const char* pszString)
{
	std::string sRtn;

	unsigned int iLen = strlen(pszString);
	for(unsigned int x = 0; x < iLen; )
	{
		if(x <= (iLen - 11) && !memcmp(pszString + x, "$(hostname)", 11))
		{
			char strHostname[256];
			if(gethostname(strHostname, 256)) strcpy(strHostname, "localhost");
			else _strlwr(strHostname);

			sRtn += strHostname;
			x += 11;
		}
		else if(x <= (iLen - 11) && !memcmp(pszString + x, "$(username)", 11))
		{
			DWORD iUNLen = 254;
			char strUsername[256];
			if(!GetUserName(strUsername, &iUNLen)) strcpy(strUsername, "localuser");
			else _strlwr(strUsername);

			sRtn += strUsername;
			x += 11;
		}
		else
		{
			sRtn += pszString[x++];
		}
	}

	return sRtn;
}

//

std::string GetLEXIRoot(void)
{
	char szAppPath[MAX_PATH] = "";
	::GetModuleFileName(NULL,szAppPath,sizeof(szAppPath) - 1);

	char szDrive[MAX_PATH];
	char szPath[MAX_PATH];
	_splitpath(szAppPath,szDrive,szPath,NULL,NULL);
	strcat(szDrive,szPath);
	strcat(szDrive,"/LEXIExporter/");

	return szDrive;
}

