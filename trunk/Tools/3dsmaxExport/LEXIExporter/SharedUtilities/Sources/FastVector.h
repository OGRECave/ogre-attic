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

#ifndef __FAST_VECTOR__
#define __FAST_VECTOR__

// NOTE! The fastvector<> template can NOT be used with classes (pointers are OK), since it does
// not use new/delete for storage allocation.
template<class _T> class fastvector
{
public:
	virtual ~fastvector() { HeapFree(GetProcessHeap(), 0, m_pData); };

	// Base constructor
	fastvector(void)
	{
		m_iGrowSize=10;
		m_iAllocSize=m_iGrowSize;
		m_iSize=0;
		m_pData=(_T*)HeapAlloc(GetProcessHeap(), 0, m_iAllocSize*sizeof(_T));
	}

	// Copy Constructor
	fastvector(const fastvector<_T>& other) 
	{
		m_iAllocSize=other.m_iAllocSize;
		m_iGrowSize=other.m_iGrowSize;
		m_iSize=other.m_iSize;
		m_pData=(_T*)HeapAlloc(GetProcessHeap(), 0, m_iAllocSize*sizeof(_T));
		CopyMemory(m_pData,other.m_pData,m_iSize*sizeof(_T));
	}

	// Data assignment operator
	fastvector(const _T *pData, unsigned iElemCount) 
	{
		m_iAllocSize=iElemCount;
		m_iGrowSize=10;
		m_iSize=iElemCount;
		m_pData=(_T*)HeapAlloc(GetProcessHeap(), 0, m_iAllocSize*sizeof(_T));
		CopyMemory(m_pData,pData,iElemCount*sizeof(_T));
	}

	// Assignment operator
	fastvector<_T>& operator=(const fastvector<_T>& _X)
	{
		reserve(_X.m_iSize);
		m_iSize=_X.m_iSize;
		CopyMemory(m_pData,_X.m_pData,m_iSize*sizeof(_T));
		return (*this);
	}

	void	assign(const _T *pData, unsigned iElemCount)
	{
		reserve(iElemCount);
		m_iSize=iElemCount;
		CopyMemory(m_pData,pData,iElemCount*sizeof(_T));
	}
	void	assign(const fastvector<_T>& _X)
	{
		reserve(_X.m_iSize);
		m_iSize=_X.m_iSize;
		CopyMemory(m_pData,_X.m_pData,m_iSize*sizeof(_T));
	}

	void	reserve(unsigned iElemCount)
	{
		if(iElemCount>m_iAllocSize)
		{
			m_iAllocSize=iElemCount;
			m_pData=(_T*)HeapReAlloc(GetProcessHeap(), 0, (void*)m_pData, m_iAllocSize*sizeof(_T));
		}
	}

	// Retrieve number of elements in the list
	unsigned	size(void) const { return m_iSize; };

	// Check if list is empty
	bool	empty(void) const { return m_iSize==0; };

	// Indexing operators
	const _T& operator[](unsigned iIndex) const
	{
		return (*(m_pData + iIndex)); 
	}
	_T& operator[](unsigned iIndex)
	{
		return (*(m_pData + iIndex)); 
	}

	// Element insertion
	void push_back(const _T& _X) 
	{
		if(m_iSize+1>m_iAllocSize)
			reserve(m_iAllocSize+m_iGrowSize);
		m_pData[m_iSize]=_X;
		m_iSize++;
	}

	const _T& pop_back(void)
	{
		return m_pData[--m_iSize];
	}

	void erase(int iIndex)
	{
		int iMoveSize=m_iSize-iIndex;
		if(iMoveSize>1) MoveMemory( m_pData+iIndex, m_pData+iIndex+1, (iMoveSize-1)*sizeof(_T));
		m_iSize--;
	}

	// Clear contents of the list
	void clear(void) 
	{
		m_iSize=0;
	}	

	void sort(int (__cdecl *compare )(const void *, const void *))
	{
		qsort((void*)m_pData, m_iSize, sizeof(_T), compare);
	}

private:
	unsigned int	m_iGrowSize;
	unsigned int	m_iSize;
	unsigned int	m_iAllocSize;
	_T	*m_pData;
};

#endif	// End sentry
