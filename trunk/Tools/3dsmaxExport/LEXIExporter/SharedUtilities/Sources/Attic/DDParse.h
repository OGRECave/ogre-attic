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

#ifndef __DD_PARSER__
#define __DD_PARSER__

class CDDParse
{
public:
	CDDParse(void);
	~CDDParse(void);

	// Parse the ASCII string to a DDObject
	// (can return NULL, if a fatal error occurs)
	CDDObject*	Parse(const char *pszASCII);

	// Try to load specified file and parse it as ASCII DDObject
	CDDObject*	ParseFromFile(const char *pszFilename);

private:	
	void	SkipSpaces(void);
	void	SkipToEOL(void);

	bool	IsTokenChar(const char cChar);
	bool	GetNextToken(faststring &sResult);
	void	DoReplace(faststring &sSearch, faststring &sReplace);

	CDDObject	*ParseObject(void);
	void	ParseInt(const char *pszID,CDDObject *pObj);
	void	ParseFloat(const char *pszID,CDDObject *pObj);
	void	ParseString(const char *pszID,CDDObject *pObj);
//	void	ParseBinary(const char *pszID,CDDObject *pObj);
	void	ParseDD(const char *pszID,CDDObject *pObj);
	void	ParseIntList(const char *pszID,CDDObject *pObj);
	void	ParseFloatList(const char *pszID,CDDObject *pObj);
	void	ParseStringList(const char *pszID,CDDObject *pObj);
	void	ParseVec3(const char *pszID,CDDObject *pObj);
	void	ParseVec3List(const char *pszID,CDDObject *pObj);
	void	ParseBool(const char *pszID,CDDObject *pObj);
	void	ParseVec2(const char *pszID,CDDObject *pObj);
	void	ParseVec4(const char *pszID,CDDObject *pObj);
	void	ParseVec4List(const char *pszID,CDDObject *pObj);
	void	ParseMatrix(const char *pszID,CDDObject *pObj);
	void	ParseMatrixList(const char *pszID,CDDObject *pObj);
	void	ParseDDList(const char *pszID,CDDObject *pObj);
	void	ParseUnknown(const char *pszType, const char *pszID,CDDObject *pObj);
	void	ParseDefine(void);
	void	ParseInclude(void);
	
//	map< CMapString, string >	m_lDefines;

	const char *m_pszDataPtr;	// Pointer to the data being parsed
	char	*m_pszData;			// Pointer to the data source

	int		m_iLine;
};

#endif	// End sentry
