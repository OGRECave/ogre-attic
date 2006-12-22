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

#ifndef __FAST_STRING__
#define __FAST_STRING__

// A fast stringclass implementation w. same naming convention as the STL string
class faststring
{
public:
	//! Constructors
	faststring(void) { m_iAllocSpace=32; m_pszString=new char[m_iAllocSpace]; m_pszString[0] = 0; }
	faststring(const faststring &other) { m_iAllocSpace=other.m_iAllocSpace;
											m_pszString=new char[m_iAllocSpace];
											strcpy(m_pszString, other.m_pszString); }
	faststring(const char *pszOther) {	m_iAllocSpace=((int)strlen(pszOther)+32)&0xFFE0;m_pszString=new char[m_iAllocSpace];
										strcpy(m_pszString, pszOther); }
	faststring(const char cInit) {	m_iAllocSpace=32; m_pszString=new char[m_iAllocSpace]; m_pszString[0]=cInit; m_pszString[1]=0; }
	faststring(const char* pBuffer, unsigned int iLen) {	m_iAllocSpace=(iLen+32)&0xFFE0;m_pszString=new char[m_iAllocSpace];
															memcpy(m_pszString, pBuffer, iLen); m_pszString[iLen] = 0; }
	~faststring(void) { delete[] m_pszString; }

	// Get actual string
	const char *c_str(void) const { return m_pszString; }

	// Reserve space for n chars
	void	reserve(int iNewCharCount) { if(iNewCharCount>m_iAllocSpace) {
											char *pNew=new char[iNewCharCount]; m_iAllocSpace=iNewCharCount;
											strcpy(pNew, m_pszString); delete[] m_pszString; m_pszString=pNew; } }

	// Retrieve length of string
	unsigned size(void) const { return (unsigned)strlen(m_pszString); }

	//
	void	cut(int iNewLen) { m_pszString[iNewLen]=0; }
	void	clear() { cut(0); }

	// Check for string equality. Same return value as strcmp.
	int		compare(const char *pszOther) const { return strcmp(m_pszString, pszOther); }
	int		compare(const faststring &other) const { return strcmp(m_pszString, other.m_pszString); }

	// Check for string equality regardless of case. Same return value as strcmpi.
	int		comparei(const char *pszOther) const { return _stricmp(m_pszString, pszOther); }
	int		comparei(const faststring &other) const { return _stricmp(m_pszString, other.m_pszString); }

	// Add string
	void	add(const char *pszOther) { reserve((int)strlen(pszOther)+size()+32); strcat(m_pszString, pszOther); }
	void	add(const faststring &other) { reserve(other.size()+size()+32); strcat(m_pszString, other.m_pszString); }

	// Add character
	void	add(const char ch) 
	{ 
		unsigned iLen=size();
		reserve(iLen+2); 
		m_pszString[iLen++]=ch; 
		m_pszString[iLen++]=0;
	}

	// Assign new string contents
	void	assign(const char *pszStr) {	reserve(((int)strlen(pszStr)+32)&0xFFE0);
											strcpy(m_pszString, pszStr); }	

	// Assign new string contents
	void	assign(const char *pszStr, int iLen) {	reserve((iLen+32)&0xFFE0);
													strncpy(m_pszString, pszStr, iLen);
													m_pszString[iLen]=0; }	

	// Index operator
	char&	operator[] (int iIndex) { return m_pszString[iIndex]; }

	// Assignment operator
	const faststring& operator =(const char *pszOther) {	reserve(((int)strlen(pszOther)+32)&0xFFE0);
															strcpy(m_pszString, pszOther);
															return *(this);		}
	// Assignment operator
	const faststring& operator =(const faststring &other) {	reserve(other.m_iAllocSpace);															
															strcpy(m_pszString, other.m_pszString);
															return *(this);		}

	int find(char byt) const { int len = (int)strlen(m_pszString); for(int x = 0; x < len; x++) { if(m_pszString[x] == byt) return x; } return -1; }

private:
	// string data
	char	*m_pszString;
	// current allocation size
	int		m_iAllocSpace;
};

#endif
