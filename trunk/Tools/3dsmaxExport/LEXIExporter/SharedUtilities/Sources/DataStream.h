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

#ifndef __DATA_STREAM__
#define __DATA_STREAM__

// Helper class. Binary packages can be build and serialized using this class
class CDataStream : public CRefCount
{
public:
	CDataStream(unsigned iInitSize=512, unsigned iGrow=512);
	CDataStream(const CDataStream &other);
	CDataStream(const void *pData, unsigned iSize, CRefCount *pDataOwner=NULL);
	CDataStream(void *pData, unsigned iSize);	// NOTE! This takes ownership of data
	~CDataStream(void);

	// Insertion
	void	InsertInt(int iData, int iPosition=0);
	void	InsertString(const char *pszData, int iPosition=0);

	// Add
	void	AddByte(unsigned char bDate);
	void	AddInt(int iData);
	void	AddFloat(float fData);
	void	AddString(const char *pszData);
	void	AddBinary(const void *pData, unsigned iLen);
	void	AddRAW(const void *pData, unsigned iLen);		// Adds raw data to the stream
	void	AddBool(bool bData);

	// Retrieve
	unsigned char GetByte() const;
	int		GetInt(void) const;
	float	GetFloat(void) const;
	const char *GetString(void) const;
	const void *GetBinary(unsigned &iLen) const;
	const void *GetRAW(unsigned iLen) const;				// Get raw data from the stream (from current position)
	bool	GetBool() const;

	// Reset position
	void	Reset(void)  { SetPosition(0); }
	void	SetPosition(unsigned iPos) const;
	int		GetPosition(void) const;

	// Retrieve base data (mostly used when serializing the stream)
	const void*	GetBaseData(void) const { return m_pData; }

	unsigned	GetAllocSize(void) const { return m_iAllocSize; }
	
private:
	void	Realloc(int iSpaceNeeded);

	// Released, if valid
	const CRefCount	*m_pDataOwner;
	
	// Data
	bool	m_bOwnData;
	char	*m_pData;
	mutable unsigned	m_iPosition;
	unsigned	m_iAllocSize;
	unsigned	m_iGrowSize;

#ifdef _DEBUG
    unsigned	m_iReallocCount;
#endif
};

#endif
