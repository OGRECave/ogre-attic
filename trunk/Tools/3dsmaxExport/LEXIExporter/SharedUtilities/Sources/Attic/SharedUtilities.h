/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
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

#ifndef __SHARED_UTILITIES_HEADER__
#define __SHARED_UTILITIES_HEADER__

#include <vector>
using namespace std;

//

#include <math.h>
#include <float.h>

//

#define _CRT_SECURE_NO_DEPRECATE

namespace SharedUtilities
{
// Fast template STL style utilities.
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

	void	append(const fastvector<_T>& _X)
	{
		reserve(m_iSize + _X.m_iSize);
		CopyMemory(m_pData+m_iSize,_X.m_pData, _X.m_iSize*sizeof(_T));
		m_iSize=m_iSize + _X.m_iSize;
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

#ifndef __FAST_MAP__
#define __FAST_MAP__

// NOTE! The fastmap<> is NOT threadsafe - you must implement own protection if needed.
// (only case should be lookup while remove/clear/map is used)
template<class _T> class fastmap
{
public:
	// Destructor
	virtual ~fastmap() 
	{
		clear(); 
		delete[] m_pHTable;
	};

	//! Base constructor
	fastmap(int iInitSize=50, int iGrowSize=20)
	{
		m_iGrowSize=iGrowSize;
		m_iSize=ClosestPrime(iInitSize);
		m_iCount=0;
		m_pHTable=new SItem*[m_iSize];
		for(unsigned int i=0; i < m_iSize; i++)
			m_pHTable[i] = NULL;
	}

	//! Copy constructor
	fastmap(const fastmap& other)
	{
		m_iGrowSize = 20;
		m_iSize = ClosestPrime(50);
		m_iCount = 0;
		m_pHTable = new SItem*[m_iSize];
		for(unsigned int i = 0; i < m_iSize; i++) m_pHTable[i] = NULL;

		assign(other);
	}

	// Assign
	void assign(const fastmap& other)
	{
		clear();

		for(unsigned int i = 0; i < other.m_iSize; i++)
		{
			const SItem* pCur = other.m_pHTable[i];
			while(pCur)
			{
				map(pCur->Key, pCur->Item);
				pCur = pCur->Next;
			}
		}
	}

	// Assign operator
	fastmap& operator = (const fastmap& other)
	{
		assign(other);
		return *this;
	}

	fastmap<_T>*	clone(void)
	{
		fastmap<_T> *pNew=new fastmap<_T>;
		pNew->m_iGrowSize=m_iGrowSize;
		pNew->m_iSize=m_iSize;
		pNew->m_iCount=m_iCount;
		pNew->m_pHTable=new SItem*[m_iSize];

		for(unsigned int i=0; i < m_iSize; i++)
		{
			pNew->m_pHTable[i]=NULL;
			SItem *pSrc=m_pHTable[i];
			SItem *pDst;
			SItem *pPrev=NULL;
			while(pSrc!=NULL)
			{
				pDst=new SItem;
				pDst->Key=_strdup(pSrc->Key);
				pDst->Item=pSrc->Item;
				pDst->Next=NULL;
				if(!pPrev) pNew->m_pHTable[i]=pDst;
				else pPrev->Next=pDst;
				pPrev=pDst;				
				pSrc=pSrc->Next;
			}
		}
		return pNew;
	}

	unsigned int size() const { return m_iCount; }
	unsigned int capacity() const { return m_iSize; }
	unsigned int growSize() const { return m_iGrowSize; }	

	void reserve(int iItems)
	{
		SItem** pOldTable = m_pHTable;
		unsigned int iOldSize = m_iSize;
		unsigned int iKey;
		if(iItems > m_iSize)
		{			
			m_iSize = ClosestPrime(iItems);
			m_pHTable=new SItem*[m_iSize];
			for(unsigned int t = 0; t < m_iSize; t++)
				m_pHTable[t] = NULL;
			SItem* p;
			SItem* tp;
			for(unsigned int i = 0; i < iOldSize; i++)
			{
				p = pOldTable[i];
				while (p != NULL)
				{	
					iKey = CalculateKey(p->Key);
					iKey = iKey % m_iSize;
					tp = p->Next;
					p->Next = m_pHTable[iKey];
					m_pHTable[iKey] = p;
					p = tp;
				}
			}			
			delete[] pOldTable;
		}
	}

	//! Clear map
	void clear(void)
	{
		SItem* p;
		SItem* tp;
		// Run through the table & free elements
		for(unsigned int i = 0; i < m_iSize; i++)
		{
			p = m_pHTable[i];
			while (p != NULL)
			{
				tp = p;
				p = p->Next;
				free(tp->Key);	// delete char * buffer
				delete tp;			// delete the SItem instance
			}
			m_pHTable[i] = NULL;
		}
		m_iCount=0;
	}

	//! Erase a key
	bool erase(const char *pszKey)
	{
		unsigned int iKey = CalculateKey(pszKey);
		iKey = iKey % m_iSize;
		SItem* m_pDelItem = m_pHTable[iKey];
		SItem** m_pLastItem = &m_pHTable[iKey];
		bool bDeleted = false; 

		//check for end of collision list
		while(m_pDelItem != NULL)
		{
			if(!strcmp(pszKey, m_pDelItem->Key))
			{
				*m_pLastItem = m_pDelItem->Next;
				bDeleted=true;
				free(m_pDelItem->Key);	// delete char * buffer
				delete m_pDelItem;			// delete SItem instance
				m_iCount--;
				break;
			}
			else
			{
				m_pLastItem = &m_pDelItem->Next;
				m_pDelItem = m_pDelItem->Next;
			}
		}	
		return bDeleted;
	}

	// Search for a key
	bool find(const char *pszKey, _T &item) const
	{
		unsigned int iKey = CalculateKey(pszKey);
		iKey = iKey % m_iSize;
		SItem* m_pSearchItem = m_pHTable[iKey];
		while(m_pSearchItem!=NULL)
		{
			if(!strcmp(pszKey, m_pSearchItem->Key))
			{	
				item=m_pSearchItem->Item;
				return true;
			}
			m_pSearchItem = m_pSearchItem->Next;
		}
		return false; 
	}	

	//! setup a value map
	void map(const char *pszKey, _T cVal, _T &olditem)
	{		
		unsigned int iKey = CalculateKey(pszKey);
		iKey = iKey % m_iSize;

		// Search for existing key
		SItem* m_pSearchItem = m_pHTable[iKey];
		while(m_pSearchItem!=NULL)
		{
			if(!strcmp(pszKey, m_pSearchItem->Key))
			{	// key found
				olditem=m_pSearchItem->Item;
				m_pSearchItem->Item=cVal;
				return;
			}
			m_pSearchItem = m_pSearchItem->Next;
		}

		// Key was not found, insert new
		SItem* pNewItem = new SItem;
		pNewItem->Key = _strdup(pszKey);
		pNewItem->Item = cVal;
		pNewItem->Next = m_pHTable[iKey];
		m_pHTable[iKey] = pNewItem;
		m_iCount++;
		ReHash();
	}

	//! setup a value map
	void map(const char *pszKey, _T cVal)
	{		
		unsigned int iKey = CalculateKey(pszKey);
		iKey = iKey % m_iSize;
		
		// Key was not found, insert new
		SItem* pNewItem = new SItem;
		pNewItem->Key = _strdup(pszKey);
		pNewItem->Item = cVal;
		pNewItem->Next = m_pHTable[iKey];
		m_pHTable[iKey] = pNewItem;
		m_iCount++;
		ReHash();
	}

	//! Get key names
	fastvector< const char * >	keys(void) const
	{
		fastvector<const char *> lList;
		lList.reserve(m_iSize);
		for(unsigned int i = 0; i < m_iSize; i++)
		{
			SItem *pCur=m_pHTable[i];
			while(pCur!=NULL)
			{				
				lList.push_back(pCur->Key);
				pCur=pCur->Next;
			}
		}
		return lList;
	}

	//! Get list of data elements
	fastvector< _T >	data(void) const
	{
		fastvector< _T > lList;
		lList.reserve(m_iSize);
		for(unsigned int i = 0; i < m_iSize; i++)
		{
			SItem *pCur=m_pHTable[i];
			while(pCur!=NULL)
			{				
				lList.push_back(pCur->Item);
				pCur=pCur->Next;
			}
		}
		return lList;
	}

	struct SDataPair
	{
		SDataPair(const char *pKey, _T data) { pszKey=pKey; Data=data; };
		const char *pszKey;
		_T Data;
	};

	// Get list of keys AND data
	fastvector< SDataPair >	iterate(void) const
	{
		fastvector< SDataPair > lList;
		lList.reserve(m_iSize);
		for(unsigned int i = 0; i < m_iSize; i++)
		{
			SItem *pCur=m_pHTable[i];
			while(pCur!=NULL)
			{				
				lList.push_back(SDataPair(pCur->Key,pCur->Item));
				pCur=pCur->Next;
			}
		}
		return lList;
	}

	// Get list of keys AND data
	vector< SDataPair >	iterateVector(void) const
	{
		vector< SDataPair > lList;
		lList.reserve(m_iSize);
		for(unsigned int i = 0; i < m_iSize; i++)
		{
			SItem *pCur=m_pHTable[i];
			while(pCur!=NULL)
			{				
				lList.push_back(SDataPair(pCur->Key,pCur->Item));
				pCur=pCur->Next;
			}
		}
		return lList;
	}

private:
	//! Check if need to expand the hash table
	void ReHash(void)
	{
		SItem** pOldTable = m_pHTable;
		unsigned int iOldSize = m_iSize;
		unsigned int iKey;
		if (m_iCount > m_iSize)
		{
			m_iSize = ClosestPrime(m_iSize+m_iGrowSize);
			m_pHTable=new SItem*[m_iSize];
			for(unsigned int t = 0; t < m_iSize; t++)
				m_pHTable[t] = NULL;
			SItem* p;
			SItem* tp;
			for(unsigned int i = 0; i < iOldSize; i++)
			{
				p = pOldTable[i];
				while (p != NULL)
				{	
					iKey = CalculateKey(p->Key);
					iKey = iKey % m_iSize;
					tp = p->Next;
					p->Next = m_pHTable[iKey];
					m_pHTable[iKey] = p;
					p = tp;
				}
			}			
			delete[] pOldTable;
		}
	}

	// Find the closest prime to the parameter
	unsigned int ClosestPrime(unsigned int iNumber) const
	{
		unsigned int iPNumber=iNumber;
		if(iNumber%2==0) iPNumber++;
		while (true)
		{
		unsigned int iHNumber=iPNumber/2;
		unsigned int i;
		for(i=3;i<=iHNumber;i++)
			if(iPNumber%i)
				iHNumber=iPNumber/i+1;
			else
				break;
		if (i>iHNumber)
			break;
		else
			iPNumber+=2;
		};
		return iPNumber;
	}

	// Calculate the hash key from a char buffer (Remember to mod with m_iSize)
	unsigned int CalculateKey(const char *pszStr) const
	{
		unsigned short iLow=0, iHigh=0;
		while(*pszStr)
		{
			iLow+=*pszStr++;
			iHigh+=iLow+3;
		}
		return (iLow)|(iHigh<<16);
	}

	// The internal hash structure
	typedef struct _SItem
	{
		char	*Key;
		_T		Item;
		_SItem	*Next;
	} SItem;

	// The hash table
	SItem **m_pHTable;

	// Number of keys inserted
	unsigned int m_iCount;

	// Number of entries in hash table
	unsigned int m_iSize;

	// Number of entries of which to grow
	unsigned int m_iGrowSize;	  
};

#endif	// End sentry

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

#ifndef __FAST_INT_MAP__
#define __FAST_INT_MAP__

template<class _T> class fastintmap
{
public:
	// Destructor
	virtual ~fastintmap() 
	{
		clear(); 
		delete[] m_pHTable;
	};

	//! Base constructor
	fastintmap(int iInitSize=50, int iGrowSize=20)
	{
		m_iGrowSize=iGrowSize;
		m_iSize=ClosestPrime(iInitSize);
		m_iCount=0;
		m_pHTable=new SItem*[m_iSize];
		for(unsigned int i=0; i < m_iSize; i++)
			m_pHTable[i] = NULL;
	}

	// Number of elements in map
	unsigned int size() { return m_iCount; }

	//! Clear map
	void clear(void)
	{
		SItem* p;
		SItem* tp;
		// Run through the table & free elements
		for(unsigned int i = 0; i < m_iSize; i++)
		{
			p = m_pHTable[i];
			while (p != NULL)
			{
				tp = p;
				p = p->Next;
				delete tp;			// delete the SItem instance
			}
			m_pHTable[i] = NULL;
		}
	}

	//! Erase a key
	bool erase(unsigned int iKey)
	{
		int iHashKey = iKey % m_iSize;
		SItem* m_pDelItem = m_pHTable[iHashKey];
		SItem** m_pLastItem = &m_pHTable[iHashKey];
		bool bDeleted = false; 

		//check for end of collision list
		while(m_pDelItem != NULL)
		{
			if(iKey==m_pDelItem->Key)
			{
				*m_pLastItem = m_pDelItem->Next;
				bDeleted=true;
				delete m_pDelItem;			// delete SItem instance
				m_iCount--;
				break;
			}
			else
			{
				m_pLastItem = &m_pDelItem->Next;
				m_pDelItem = m_pDelItem->Next;
			}
		}	
		return bDeleted;
	}

	//! Erase a key
	bool erase(unsigned int iKey, _T &item)
	{
		int iHashKey = iKey % m_iSize;
		SItem* m_pDelItem = m_pHTable[iHashKey];
		SItem** m_pLastItem = &m_pHTable[iHashKey];
		bool bDeleted = false; 

		//check for end of collision list
		while(m_pDelItem != NULL)
		{
			if(iKey==m_pDelItem->Key)
			{
				item=m_pDelItem->Item;
				*m_pLastItem = m_pDelItem->Next;
				bDeleted=true;
				delete m_pDelItem;			// delete SItem instance
				m_iCount--;
				break;
			}
			else
			{
				m_pLastItem = &m_pDelItem->Next;
				m_pDelItem = m_pDelItem->Next;
			}
		}	
		return bDeleted;
	}

	// Search for a key
	bool find(unsigned int iKey, _T &item) const
	{
		int iHashKey = iKey % m_iSize;
		SItem* m_pSearchItem = m_pHTable[iHashKey];
		while(m_pSearchItem!=NULL)
		{
			if(iKey==m_pSearchItem->Key)
			{	
				item=m_pSearchItem->Item;
				return true;
			}		
			m_pSearchItem = m_pSearchItem->Next;
		}
		return false; 
	}	

	//! setup a value map
	void map(unsigned int iKey, _T cVal, _T &olditem)
	{		
		int iHashKey = iKey % m_iSize;

		// Search for existing key
		SItem* m_pSearchItem = m_pHTable[iHashKey];
		while(m_pSearchItem!=NULL)
		{
			if(iKey==m_pSearchItem->Key)
			{	// key found
				olditem=m_pSearchItem->Item;
				m_pSearchItem->Item=cVal;
				return;
			}
			m_pSearchItem = m_pSearchItem->Next;
		}

		// Key was not found, insert new
		SItem* pNewItem = new SItem;
		pNewItem->Key = iKey;
		pNewItem->Item = cVal;
		pNewItem->Next = m_pHTable[iHashKey];
		m_pHTable[iHashKey] = pNewItem;
		m_iCount++;
		ReHash();
	}

	//! setup a value map
	void map(unsigned int iKey, _T cVal)
	{		
		int iHashKey = iKey % m_iSize;
		
		// Key was not found, insert new
		SItem* pNewItem = new SItem;
		pNewItem->Key = iKey;
		pNewItem->Item = cVal;
		pNewItem->Next = m_pHTable[iHashKey];
		m_pHTable[iHashKey] = pNewItem;
		m_iCount++;
		ReHash();
	}

	//! Get key values
	fastvector< unsigned int >	keys(void) const
	{
		fastvector<unsigned int> lList;
		for(unsigned int i = 0; i < m_iSize; i++)
		{
			SItem *pCur=m_pHTable[i];
			while(pCur!=NULL)
			{				
				lList.push_back(pCur->Key);
				pCur=pCur->Next;
			}
		}
		return lList;
	}

	//! Get list of data elements
	fastvector< _T >	data(void)
	{
		fastvector< _T > lList;
		for(unsigned int i = 0; i < m_iSize; i++)
		{
			SItem *pCur=m_pHTable[i];
			while(pCur!=NULL)
			{				
				lList.push_back(pCur->Item);
				pCur=pCur->Next;
			}
		}
		return lList;
	}

private:
	//! Check if need to expand the hash table
	void ReHash(void)
	{
		SItem** pOldTable = m_pHTable;
		unsigned int iOldSize = m_iSize;
		unsigned int iKey;
		if(m_iCount > m_iSize)
		{
			m_iSize = ClosestPrime(m_iSize+m_iGrowSize);
			m_pHTable=new SItem*[m_iSize];
			for(unsigned int t = 0; t < m_iSize; t++)
				m_pHTable[t] = NULL;
			SItem* p;
			SItem* tp;
			for(unsigned int i = 0; i < iOldSize; i++)
			{
				p = pOldTable[i];
				while (p != NULL)
				{	
					iKey = p->Key;
					int iHashKey = iKey % m_iSize;
					tp = p->Next;
					p->Next = m_pHTable[iHashKey];
					m_pHTable[iHashKey] = p;
					p = tp;
				}
			}			
			delete[] pOldTable;
		}
	}

	// Find the closest prime to the parameter
	unsigned int ClosestPrime(unsigned int iNumber) const
	{
		unsigned int iPNumber=iNumber;
		if(iNumber%2==0) iPNumber++;
		while (true)
		{
			unsigned int iHNumber=iPNumber/2;
			unsigned int i;
			for(i=3;i<=iHNumber;i++)
				if(iPNumber%i)
					iHNumber=iPNumber/i+1;
				else
					break;
			if (i>iHNumber)
				break;
			else
				iPNumber+=2;
		};
		return iPNumber;
	}

	// The internal hash structure
	typedef struct _SItem
	{
		unsigned int Key;
		_T		Item;
		_SItem	*Next;
	} SItem;

	_T	m_EmptyValue;

	// The hash table
	SItem **m_pHTable;

	// Number of keys inserted
	unsigned int m_iCount;

	// Number of entries in hash table
	unsigned int m_iSize;

	// Number of entries of which to grow
	unsigned int m_iGrowSize;	
};

#endif	// End sentry

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
											strcpy_s(m_pszString, m_iAllocSpace, other.m_pszString); }
	faststring(const char *pszOther) {	m_iAllocSpace=((int)strlen(pszOther)+32)&0xFFE0;m_pszString=new char[m_iAllocSpace];
										strcpy_s(m_pszString, m_iAllocSpace, pszOther); }
	faststring(const char cInit) {	m_iAllocSpace=32; m_pszString=new char[m_iAllocSpace]; m_pszString[0]=cInit; m_pszString[1]=0; }
	faststring(const char* pBuffer, unsigned int iLen) {	m_iAllocSpace=(iLen+32)&0xFFE0;m_pszString=new char[m_iAllocSpace];
															memcpy(m_pszString, pBuffer, iLen); m_pszString[iLen] = 0; }
	~faststring(void) { delete[] m_pszString; }

	// Get actual string
	const char *c_str(void) const { return m_pszString; }

	// Reserve space for n chars
	void	reserve(int iNewCharCount) { if(iNewCharCount>m_iAllocSpace) {
											char *pNew=new char[iNewCharCount]; m_iAllocSpace=iNewCharCount;
											strcpy_s(pNew, iNewCharCount, m_pszString); delete[] m_pszString; m_pszString=pNew; } }

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
	void	add(const char *pszOther) { reserve((int)strlen(pszOther)+size()+32); strcat_s(m_pszString, m_iAllocSpace, pszOther); }
	void	add(const faststring &other) { reserve(other.size()+size()+32); strcat_s(m_pszString, m_iAllocSpace, other.m_pszString); }

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
											strcpy_s(m_pszString, m_iAllocSpace, pszStr); }	

	// Assign new string contents
	void	assign(const char *pszStr, int iLen) {	reserve((iLen+32)&0xFFE0);
													strncpy_s(m_pszString, m_iAllocSpace, pszStr, iLen);
													m_pszString[iLen]=0; }	

	// Index operator
	char&	operator[] (int iIndex) { return m_pszString[iIndex]; }

	// Assignment operator
	const faststring& operator =(const char *pszOther) {	reserve(((int)strlen(pszOther)+32)&0xFFE0);
															strcpy_s(m_pszString, m_iAllocSpace, pszOther);
															return *(this);		}
	// Assignment operator
	const faststring& operator =(const faststring &other) {	reserve(other.m_iAllocSpace);															
															strcpy_s(m_pszString, m_iAllocSpace, other.m_pszString);
															return *(this);		}

	int find(char byt) const { int len = (int)strlen(m_pszString); for(int x = 0; x < len; x++) { if(m_pszString[x] == byt) return x; } return -1; }

private:
	// string data
	char	*m_pszString;
	// current allocation size
	int		m_iAllocSpace;
};

#endif

// Reference counting
/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
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

#ifndef __REFCOUNT__
#define __REFCOUNT__

// Reference count base - derive from this class to 
class CRefCount
{
public:
	CRefCount(void) { m_iRefCount=1; }
	virtual ~CRefCount(void) {}

	// Called when refcount reaches zero
	virtual void OnFinalRelease(void) { delete this; }

	// Increment reference count
//	void AddRef(void) const { ++(((CRefCount*)this)->m_iRefCount); }
	void AddRef(void) const 
	{ 
		InterlockedIncrement(&((CRefCount*)this)->m_iRefCount); 
	}

	// Relase object (decrease reference count)
//	void Release(void) const { if(--(((CRefCount*)this)->m_iRefCount)<=0) 
//											((CRefCount*)this)->OnFinalRelease(); }
	virtual void Release(void) const 
	{
		if(InterlockedDecrement(&((CRefCount*)this)->m_iRefCount)<=0)
			((CRefCount*)this)->OnFinalRelease();
	}
								

	// Retrive current reference count
	int	GetRefCount(void) const { return m_iRefCount; }

protected:
//	signed int	m_iRefCount;
	volatile long m_iRefCount;
};

#endif


// Logging
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

#ifndef __LOGSYSTEM__
#define __LOGSYSTEM__

// The basic loglevels
enum ELogLevel
{
	LOG_INFO=0,
	LOG_DEBUG=1,
	LOG_WARNING=2,
	LOG_ERROR=3,
	LOG_MSG_CUSTOM_BEGIN=0x10
};

// The LogReceiver interface - implement this interface and call CLogSystem::AddReceiver() to receive
// log messages.
class ILogReceiver : public CRefCount
{
public:
	virtual ~ILogReceiver(void) {};

	// Called when a log message is received.
	virtual void	ReceiveLogMessage(SYSTEMTIME &LogTime, const char *pszTimeStr, int iLevel, const char *pszMessage)=0;
};

// The logsystem is a singleton class - the instance will be created at the first Get() call.
// Please note: By default there is no LogReceivers associated with the logsystem.
class CLogSystem
{
public:
	// Constructor/Destructor
	CLogSystem(void);
	~CLogSystem(void);

	// Static get function - so we can reach the logsystem
	static CLogSystem *Get(void);

	// Set min. log level
	void	SetMinLogLevel(ELogLevel eLevel) { m_eMinLogLevel=eLevel; }

	// Add a message to log
	void	LogMessage(const void *pModule, int iLevel, const char *pszText, ...);

	// Register a system module
	void	RegisterModule(const void *pModulePointer, const char *pszModuleDesc);

	// Register a system module
	void	UnregisterModule(const void *pModulePointer);

	// Add a log receiver
	void	AddReceiver(ILogReceiver *pReceiver);

	// Remove a log receiver
	void	RemoveReceiver(ILogReceiver *pReceiver);

private:
	// Pointer to the instance of the log system
	static CLogSystem *m_pThis;

	// Minimum logging level
	ELogLevel		m_eMinLogLevel;

	// Map of registered modules
	fastintmap< char* >	m_lModules;

	// List of registered receivers
	fastvector< ILogReceiver* >	m_lReceivers;

	CRITICAL_SECTION	m_Crit;
};

/*
	LOG SYSTEM MACROS
*/
#ifndef _DISABLE_LOGGING
	
	// Module registration
	#define REGISTER_MODULE(pszDesc)	CLogSystem::Get()->RegisterModule(this, pszDesc);
	#define UNREGISTER_MODULE			CLogSystem::Get()->UnregisterModule(this);

	// Low priority log macros
	#ifndef _DISABLE_LOWPRIOLOG
		#define LOGINFO		CLogSystem::Get()->LogMessage(this, LOG_INFO,
		#define LOGDEBUG	CLogSystem::Get()->LogMessage(this, LOG_DEBUG,
	#else
		#define LOGINFO		(
		#define LOGDEBUG	(
	#endif

	// High priority log macros
	#define LOGWARNING	CLogSystem::Get()->LogMessage(this, LOG_WARNING,
	#define LOGERROR	CLogSystem::Get()->LogMessage(this, LOG_ERROR,
	#define LOGCUSTOM	CLogSystem::Get()->LogMessage(this, 

	// Macros for static / thread functions
	#define LOGSTATIC	CLogSystem::Get()->LogMessage(NULL,
	#define LOGTHREAD	CLogSystem::Get()->LogMessage(	
#else
	
	// Logging is disabled; all macros will create blank code
	#define REGISTER_MODULE(pThisPointer, pszDesc)
	#define UNREGISTER_MODULE(pThisPointer)
	#define LOGINFO		(
	#define LOGDEBUG	(
	#define LOGWARNING	(
	#define LOGERROR	(
	#define LOGSTATIC	(	
#endif

#endif

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

#ifndef __CONSOLE_LOGGER__
#define __CONSOLE_LOGGER__

class CConsoleLogger : public ILogReceiver
{
public:
	CConsoleLogger(bool bLogTimes=false, ELogLevel eMinLevel=LOG_INFO);
	~CConsoleLogger(void);

	// Set level color. Mostly used for custom logging levels.
	void SetLevelColor(int iMessageLevel, WORD wColor);	

	// Called from the logsystem
	void ReceiveLogMessage(SYSTEMTIME &LogTime, const char *pszTimeStr, int iMessageLevel, const char *pszMessage);

private:
	bool			m_bLogTimes;
	WORD			m_wColors[255];
	HANDLE			m_hConsoleHandle;
	ELogLevel		m_eMinLevel;
};

#endif

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

#ifndef __FILE_LOGGER__
#define __FILE_LOGGER__

class CFileLogger : public ILogReceiver
{
public:
	CFileLogger(const char *pszFile, bool bLogTimes=true, ELogLevel eMinLevel=LOG_INFO);
	~CFileLogger(void);

	// Called from the logsystem
	void ReceiveLogMessage(SYSTEMTIME &LogTime, const char *pszTimeStr, int iMessageLevel, const char *pszMessage);

private:	
	bool			m_bLogTimes;
	HANDLE			m_hFileHandle;
	ELogLevel		m_eMinLevel;
};

#endif


// Datastream
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

	//
	void	Reserve(unsigned int iNewAlloc);

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


// Math
/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
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

/////////////////////////////////////////////////////
//
//  Math Defines
//
/////////////////////////////////////////////////////

#ifndef __Math_Defines__
#define __Math_Defines__

//

#define TV_PI					(3.1415926535897932384626433832795f)
#define TV_HalfPI				(TV_PI * 0.5f)
#define TV_PI2					(TV_PI * 2.0f)

#define UtilDegToRad(angle)		(0.017453293f * angle)
#define UtilRadToDeg(angle)		(57.295779513f * angle)

#define F_Min					FLT_MIN
#define F_Max					FLT_MAX

#define F_MinValue				(0.0000005f)

#define F_Square(v)				((v) * (v))

//

inline void sincos(const float& angle, float& s, float& c)
{
	s = sinf(angle);
	c = cosf(angle);
}

//

#endif // __Math_Defines__

/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
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

//@£$sdk "Vector2 Class"

#ifndef __Accuro_Vector2__
#define __Accuro_Vector2__

#pragma once

//

class CVec2 {

	friend class CMatrix;

	public:

		static const CVec2 _zero;
		static const CVec2 _x;
		static const CVec2 _y;

		// Constructors/Destructor
		inline CVec2()
		{
		}
		inline CVec2(const CVec2& v)
		{
			x=v.x;
			y=v.y;
		}
		inline CVec2(float fX, float fY)
		{
			x=fX;
			y=fY;
		}
		inline CVec2(const float* v)
		{
			x=v[0];
			y=v[1];
		}

		// Direct access to vector
		float x, y;

		// Assignment
		inline CVec2& operator = (const CVec2& v)
		{
			x=v.x;
			y=v.y;

			return *this;
		}

		// Comparing
		inline bool operator == (const CVec2& v) const
		{
			return (x==v.x && y==v.y);
		}

		inline bool operator != (const CVec2& v) const
		{
			return (x!=v.x || y!=v.y);
		}

		// Operators
		inline CVec2 CVec2::operator - () const
		{
			return CVec2(-x, -y);
		}

		inline CVec2 CVec2::operator + (const CVec2& v) const
		{
			return CVec2(x+v.x, y+v.y);
		}

		inline CVec2 CVec2::operator - (const CVec2& v) const
		{
			return CVec2(x-v.x, y-v.y);
		}

		inline CVec2 CVec2::operator * (const CVec2& v) const
		{
			return CVec2(x*v.x, y*v.y);
		}

		inline CVec2 CVec2::operator / (const CVec2& v) const
		{
			return CVec2(x/v.x, y/v.y);
		}

		inline void add(const CVec2& v)
		{
			x += v.x;
			y += v.y;
		}

		inline void add(const CVec2& v1, const CVec2& v2)
		{
			x = v1.x + v2.x;
			y = v1.y + v2.y;
		}

		inline CVec2& CVec2::operator += (const CVec2& v)
		{
			x += v.x;
			y += v.y;
			return *this;
		}

		inline void subtract(const CVec2& v)
		{
			x += v.x;
			y += v.y;
		}

		inline void subtract(const CVec2& v1, const CVec2& v2)
		{
			x = v1.x - v2.x;
			y = v1.y - v2.y;
		}

		inline CVec2& CVec2::operator -= (const CVec2& v)
		{
			x -= v.x;
			y -= v.y;
			return *this;
		}

		inline CVec2& CVec2::operator *= (const CVec2& v)
		{
			x *= v.x;
			y *= v.y;
			return *this;
		}

		inline CVec2& CVec2::operator /= (const CVec2& v)
		{
			x /= v.x;
			y /= v.y;
			return *this;
		}

		inline CVec2 CVec2::operator * (float r) const
		{
			return CVec2(r*x, r*y);
		}

		inline void multiply(const CVec2& v1, const CVec2& v2)
		{
			x = v1.x * v2.x;
	        y = v1.y * v2.y;
		}

		inline void multiply(const CVec2& v)
		{
			x *= v.x;
	        y *= v.y;
		}

		inline void negate(const CVec2& v)
		{
			x = -v.x;
			y = -v.y;
		}

		inline void negate()
		{
			x = -x;
			y = -y;
		}

		inline float dot(const CVec2& v) const
		{
			return x * v.x + y * v.y;
		}

		inline void scale(float scalar, const CVec2& v)
		{
			x = scalar * v.x;
	        y = scalar * v.y;
		}

		inline void scale(float scalar)
		{
			x *= scalar;
	        y *= scalar;
		}

		inline void addScaled(const CVec2& v1, float scalar, const CVec2& v2)
		{
			x = v1.x + scalar * v2.x;
	        y = v1.y + scalar * v2.y;
		}

		inline void addScaled(float scalar, const CVec2& v)
		{
			x += scalar * v.x;
	        y += scalar * v.y;
		}

		inline void combine(float scalar1, const CVec2& v1, float scalar2, const CVec2& v2)
		{
			x = scalar1 * v1.x + scalar2 * v2.x;
	        y = scalar1 * v1.y + scalar2 * v2.y;
		}

		inline void normalize()
		{
			float t = length();
	        scale(1.0f / t);
		}

		inline void perpendicular(const CVec2& v)
		{
			float t = v.x;
			x = -v.y;
			y = t;
		}

		inline void perpendicular()
		{
			float t = x;
			x = -y;
			y = t;
		}

		inline float length2() const
		{
			return F_Square(x) + F_Square(y);
		}

		inline float length() const
		{
			return sqrtf(length2());
		}

		inline float distance2(const CVec2& v) const
		{
			return (F_Square(x - v.x) + F_Square(y - v.y));
		}

		inline float distance(const CVec2& v) const
		{
			return sqrtf(distance2(v));
		}

		inline void lerp(const float& scalar, const CVec2& v1, const CVec2& v2)
		{ 
			x = v1.x + scalar * (v2.x - v1.x);
			y = v1.y + scalar * (v2.y - v1.y);
		}

		inline float angle(const CVec2& v) const
		{
			return acosf(dot(v) / (length() * v.length()));
		}

		inline CVec2 CVec2::operator / (float r) const
		{
			float ri = 1.0f/r;
			return CVec2(ri*x, ri*y);
		}

		inline CVec2& CVec2::operator *= (float r)
		{
			x *= r;
			y *= r;
			return *this;
		}

		inline CVec2& CVec2::operator /= (float r)
		{
			float ri = 1.0f/r;
			x *= ri;
			y *= ri;
			return *this;
		}

		inline void clamp()
		{
			if(x < 0.0f) x = 0.0f;
			else if(x > 1.0f) x = 1.0f;
			if(y < 0.0f) y = 0.0f;
			else if(y > 1.0f) y = 1.0f;
		}

		inline void clamp(float min, float max)
		{
			if(x < min) x = min;
			else if(x > max) x = max;
			if(y < min) y = min;
			else if(y > max) y = max;
		}

		inline void zero()
		{
			x = y = 0.0f;
		}

};

//

#endif // __Accuro_Vector2__

/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
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

/////////////////////////////////////////////////////
//
//  Vector3 class
//
/////////////////////////////////////////////////////

#ifndef __CVector3_Class__
#define __CVector3_Class__

//

class CMatrix;

class CVec3 {

	friend class CMatrix;

	public:

		static const CVec3 _zero;
		static const CVec3 _x;
		static const CVec3 _y;
		static const CVec3 _z;

		// Constructors/Destructor
		inline CVec3()
		{
		}
		inline CVec3(const CVec3& v)
		{
			x=v.x;
			y=v.y;
			z=v.z;
		}
		inline CVec3(float fX, float fY, float fZ)
		{
			x=fX;
			y=fY;
			z=fZ;
		}
		inline CVec3(const float* v)
		{
			x=v[0];
			y=v[1];
			z=v[2];
		}

		// Direct access to vector
		float x, y, z;

		// Ccess operators
		inline float& operator [] (unsigned int iIndex)
		{
			return ((float*)&x)[iIndex];
		}
		inline const float& operator [] (unsigned int iIndex) const
		{
			return ((const float*)&x)[iIndex];
		}

		// Assignment
		inline CVec3& operator = (const CVec3& v)
		{
			x=v.x;
			y=v.y;
			z=v.z;

			return *this;
		}

		// Comparing
		inline bool operator == (const CVec3& v) const
		{
			return (x==v.x && y==v.y && z==v.z);
		}

		inline bool operator != (const CVec3& v) const
		{
			return (x!=v.x || y!=v.y || z!=v.z);
		}

		// Operators
		inline CVec3 operator - () const
		{
			return CVec3(-x, -y, -z);
		}

		inline CVec3 operator + (const CVec3& v) const
		{
			return CVec3(x+v.x, y+v.y, z+v.z);
		}

		inline CVec3 operator - (const CVec3& v) const
		{
			return CVec3(x-v.x, y-v.y, z-v.z);
		}

		inline CVec3 operator * (const CVec3& v) const
		{
			return CVec3(x*v.x, y*v.y, z*v.z);
		}

		inline CVec3 operator / (const CVec3& v) const
		{
			return CVec3(x/v.x, y/v.y, z/v.z);
		}

		inline void add(const CVec3& v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
		}

		inline void add(const CVec3& v1, const CVec3& v2)
		{
			x = v1.x + v2.x;
			y = v1.y + v2.y;
			z = v1.z + v2.z;
		}

		inline CVec3& operator += (const CVec3& v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
			return *this;
		}

		inline void subtract(const CVec3& v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
		}

		inline void subtract(const CVec3& v1, const CVec3& v2)
		{
			x = v1.x - v2.x;
			y = v1.y - v2.y;
			z = v1.z - v2.z;
		}

		inline CVec3& operator -= (const CVec3& v)
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			return *this;
		}

		inline CVec3& operator *= (const CVec3& v)
		{
			x *= v.x;
			y *= v.y;
			z *= v.z;
			return *this;
		}

		inline CVec3& operator /= (const CVec3& v)
		{
			x /= v.x;
			y /= v.y;
			z /= v.z;
			return *this;
		}

		inline CVec3 operator * (float r) const
		{
			return CVec3(r*x, r*y, r*z);
		}

		inline CVec3 operator / (float r) const
		{
			float ri = 1.0f / r;
			return CVec3(ri * x, ri * y, ri * z);
		}

		inline CVec3& operator *= (float r)
		{
			x *= r;
			y *= r;
			z *= r;
			return *this;
		}

		inline CVec3& operator /= (float r)
		{
			float ri = 1.0f / r;
			x *= ri;
			y *= ri;
			z *= ri;
			return *this;
		}

		// Functions
		inline float dot(const CVec3& v) const
		{
			return (x*v.x + y*v.y + z*v.z);
		}

		inline void cross(const CVec3& v)
		{
			const float a = (y * v.z) - (z * v.y);
			const float b = (z * v.x) - (x * v.z);
			const float c = (x * v.y) - (y * v.x);
			x = a;
			y = b;
			z = c;
		}

		inline void cross(const CVec3& v1, const CVec3& v2)
		{
			x = (v1.y * v2.z) - (v1.z * v2.y);
			y = (v1.z * v2.x) - (v1.x * v2.z);
			z = (v1.x * v2.y) - (v1.y * v2.x);
		}

		inline void negate()
		{
			x = -x;
			y = -y;
			z = -z;
		}

		inline void negate(const CVec3& vector)
		{
			x = -vector.x;
			y = -vector.y;
			z = -vector.z;
		}

		inline float length() const
		{
			return sqrtf(x*x + y*y + z*z);
		}

		inline float length2() const
		{
			return x*x + y*y + z*z;
		}

		inline void normalize()
		{
			float scalar = 1.0f / sqrtf(x*x + y*y + z*z);
			x *= scalar;
			y *= scalar;
			z *= scalar;
		}

		inline float distance(const CVec3& v) const
		{
			float fX = v.x-x;
			float fY = v.y-y;
			float fZ = v.z-z;

			return sqrtf(fX*fX + fY*fY + fZ*fZ);
		}

		inline float distance2(const CVec3& v) const
		{
			float fX = v.x - x;
			float fY = v.y - y;
			float fZ = v.z - z;

			return fX*fX + fY*fY + fZ*fZ;
		}

		inline void scale(float scalar, const CVec3& vector)
		{
			x = scalar * vector.x;
	        y = scalar * vector.y;
	        z = scalar * vector.z;
		}

		inline void scale(float scalar)
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
		}

	    inline void addScaled(const CVec3& v1, float scalar, const CVec3& v2)
		{
			x = v1.x + scalar * v2.x;
			y = v1.y + scalar * v2.y;
			z = v1.z + scalar * v2.z;
		}

	    inline void addScaled(float scalar, const CVec3& vector)
		{
			x += scalar * vector.x;
			y += scalar * vector.y;
			z += scalar * vector.z;
		}

		inline void lerp(const float& scalar, const CVec3& v1, const CVec3& v2)
		{ 
			x = v1.x + scalar * (v2.x - v1.x);
			y = v1.y + scalar * (v2.y - v1.y);
			z = v1.z + scalar * (v2.z - v1.z);
		}

		inline void reflect(const CVec3& v)
		{
			float a = ((x * v.x) + (y * v.y) + (z * v.z)) * 2.0f;

			x *= a;
			x -= v.x;

			y *= a;
			y -= v.y;

			z *= a;
			z -= v.z;
		}

		inline void clamp()
		{
			if(x < 0.0f) x = 0.0f;
			else if(x > 1.0f) x = 1.0f;
			if(y < 0.0f) y = 0.0f;
			else if(y > 1.0f) y = 1.0f;
			if(z < 0.0f) z = 0.0f;
			else if(z > 1.0f) z = 1.0f;
		}

		inline void clamp(float min, float max)
		{
			if(x < min) x = min;
			else if(x > max) x = max;
			if(y < min) y = min;
			else if(y > max) y = max;
			if(z < min) z = min;
			else if(z > max) z = max;
		}

		inline unsigned int largestAxis() const
		{
			const float* pVals = &x;
			unsigned int l = 0;
			if(pVals[1] > pVals[l]) l = 1;
			if(pVals[2] > pVals[l]) l = 2;
			return l;
		}

		inline void zero()
		{
			x = y = z = 0.0f;
		}

};

//

#endif // __CVector3_Class__

/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
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

/////////////////////////////////////////////////////
//
//  Vector4 class
//
/////////////////////////////////////////////////////

#ifndef __CVector4_Class__
#define __CVector4_Class__

//

class CVec4 {

	friend class CMatrix;

	public:

		static const CVec4 _zero;
		static const CVec4 _x;
		static const CVec4 _y;
		static const CVec4 _z;
		static const CVec4 _w;

		// Constructors/Destructor
		inline CVec4()
		{
		}
		inline CVec4(const CVec4& v)
		{
			x=v.x;
			y=v.y;
			z=v.z;
			w=v.w;
		}
		inline CVec4(float fX, float fY, float fZ, float fW)
		{
			x=fX;
			y=fY;
			z=fZ;
			w=fW;
		}
		inline CVec4(const float* v)
		{
			x=v[0];
			y=v[1];
			z=v[2];
			w=v[3];
		}

		// Direct access to vector
		float x, y, z, w;

		// Ccess operators
		inline float& operator [] (unsigned int iIndex)
		{
			return ((float*)&x)[iIndex];
		}
		inline const float& operator [] (unsigned int iIndex) const
		{
			return ((const float*)&x)[iIndex];
		}

		// Assignment
		inline CVec4& operator = (const CVec4& v)
		{
			x=v.x;
			y=v.y;
			z=v.z;
			w=v.w;

			return *this;
		}

		// Comparing
		inline bool operator == (const CVec4& v) const
		{
			return (x==v.x && y==v.y && z==v.z && w==v.w);
		}

		inline bool operator != (const CVec4& v) const
		{
			return (x!=v.x || y!=v.y || z!=v.z || w!=v.w);
		}

		// Operators
		inline CVec4 operator - () const
		{
			return CVec4(-x, -y, -z, -w);
		}

		inline CVec4 operator + (const CVec4& v) const
		{
			return CVec4(x+v.x, y+v.y, z+v.z, w+v.w);
		}

		inline CVec4 operator - (const CVec4& v) const
		{
			return CVec4(x-v.x, y-v.y, z-v.z, w-v.w);
		}

		inline CVec4 operator * (const CVec4& v) const
		{
			return CVec4(x*v.x, y*v.y, z*v.z, w*v.w);
		}

		inline CVec4 operator / (const CVec4& v) const
		{
			return CVec4(x/v.x, y/v.y, z/v.z, w/v.w);
		}

		inline void add(const CVec4& v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
			w += v.w;
		}

		inline void add(const CVec4& v1, const CVec4& v2)
		{
			x = v1.x + v2.x;
			y = v1.y + v2.y;
			z = v1.z + v2.z;
			w = v1.w + v2.w;
		}

		inline CVec4& operator += (const CVec4& v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
			w += v.w;
			return *this;
		}

		inline void subtract(const CVec4& v)
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			w -= v.w;
		}

		inline void subtract(const CVec4& v1, const CVec4& v2)
		{
			x = v1.x - v2.x;
			y = v1.y - v2.y;
			z = v1.z - v2.z;
			w = v1.w - v2.w;
		}

		inline CVec4& operator -= (const CVec4& v)
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			w -= v.w;
			return *this;
		}

		inline CVec4& operator *= (const CVec4& v)
		{
			x *= v.x;
			y *= v.y;
			z *= v.z;
			w *= v.w;
			return *this;
		}

		inline CVec4& operator /= (const CVec4& v)
		{
			x /= v.x;
			y /= v.y;
			z /= v.z;
			w /= v.w;
			return *this;
		}

		inline CVec4 operator * (float r) const
		{
			return CVec4(r*x, r*y, r*z, r*w);
		}

		inline CVec4 operator / (float r) const
		{
			float ri = 1.0f/r;
			return CVec4(ri*x, ri*y, ri*z, ri*w);
		}

		inline CVec4& operator *= (float r)
		{
			x *= r;
			y *= r;
			z *= r;
			w *= r;
			return *this;
		}

		inline CVec4& operator /= (float r)
		{
			float ri = 1.0f/r;
			x *= ri;
			y *= ri;
			z *= ri;
			w *= ri;
			return *this;
		}

		inline void scale(float scalar, const CVec4& v)
		{
			x = scalar * v.x;
			y = scalar * v.y;
			z = scalar * v.z;
			w = scalar * v.w;
		}

		inline void scale(float scalar)
		{
			x *= scalar;
	        y *= scalar;
	        z *= scalar;
	        w *= scalar;
		}

		inline void lerp(const float& scalar, const CVec4& v1, const CVec4& v2)
		{ 
			x = v1.x + scalar * (v2.x - v1.x);
			y = v1.y + scalar * (v2.y - v1.y);
			z = v1.z + scalar * (v2.z - v1.z);
			w = v1.w + scalar * (v2.w - v1.w);
		}

		inline void lerp3(const float& scalar, const CVec4& v1, const CVec4& v2)
		{ 
			x = v1.x + scalar * (v2.x - v1.x);
			y = v1.y + scalar * (v2.y - v1.y);
			z = v1.z + scalar * (v2.z - v1.z);
			w = v1.w;
		}

		inline void normalize3()
		{
			float st = 1.0f / sqrtf(x*x + y*y + z*z);
			x *= st;
	        y *= st;
	        z *= st;
		}

		inline void normalize()
		{
			float st = 1.0f / sqrtf(x*x + y*y + z*z);
			x *= st;
	        y *= st;
	        z *= st;
	        w *= st;
		}

		inline void clamp()
		{
			if(x < 0.0f) x = 0.0f;
			else if(x > 1.0f) x = 1.0f;
			if(y < 0.0f) y = 0.0f;
			else if(y > 1.0f) y = 1.0f;
			if(z < 0.0f) z = 0.0f;
			else if(z > 1.0f) z = 1.0f;
			if(w < 0.0f) w = 0.0f;
			else if(w > 1.0f) w = 1.0f;
		}

		inline void clamp(float min, float max)
		{
			if(x < min) x = min;
			else if(x > max) x = max;
			if(y < min) y = min;
			else if(y > max) y = max;
			if(z < min) z = min;
			else if(z > max) z = max;
			if(w < min) w = min;
			else if(w > max) w = max;
		}

		inline void zero()
		{
			x = y = z = w = 0.0f;
		}

		void fromMatrix(const CMatrix& m);
};

//

#endif // __CVector4_Class__

/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
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

/////////////////////////////////////////////////////
//
//  Matrix 4x4 class
//
/////////////////////////////////////////////////////

#ifndef __Matrix_4x4_Class__
#define __Matrix_4x4_Class__

//

class CMatrix {

	public:

		static const CMatrix _identity;
		static const CMatrix _zero;

		// Constructors/Destructor
		inline CMatrix()
		{
		}
		inline CMatrix(const CMatrix& m)
		{
			memcpy(mat, m.mat, 4 * 4 * sizeof(float));
		}
		inline CMatrix(const float* m)
		{
			memcpy(mat, m, 4 * 4 * sizeof(float));
		}

		// Direct access to matrix
		float mat[4][4];

		// Assignment
		inline CMatrix& operator = (const CMatrix& m)
		{
			memcpy(mat, m.mat, 4*4*sizeof(float));

			return *this;
		}

		// Comparing
		inline bool operator == (const CMatrix& m) const
		{
			return memcmp(mat, m.mat, 4*4*sizeof(float))==0 ? true : false;
		}

		inline bool operator != (const CMatrix& m) const
		{
			return memcmp(mat, m.mat, 4*4*sizeof(float))!=0 ? true : false;
		}

		inline bool IsIdentity() const
		{
			return memcmp(mat, _identity.mat, 4 * 4 * sizeof(float)) ? false : true;
		}

		// Operators
		inline CMatrix operator + (const CMatrix& m) const
		{
			CMatrix ma;

			for(unsigned int r=0; r<4; r++)
			{
				for(unsigned int c=0; c<4; c++)
				{
					ma.mat[r][c] = mat[r][c] + m.mat[r][c];
				}
			}

			return ma;
		}

		inline CMatrix& operator += (const CMatrix& m)
		{
			for(unsigned int r=0; r<4; r++)
			{
				for(unsigned int c=0; c<4; c++)
				{
					mat[r][c] += m.mat[r][c];
				}
			}

			return *this;
		}

		inline CMatrix operator - (const CMatrix& m) const
		{
			CMatrix ma;

			for(unsigned int r=0; r<4; r++)
			{
				for(unsigned int c=0; c<4; c++)
				{
					ma.mat[r][c] = mat[r][c] - m.mat[r][c];
				}
			}

			return ma;
		}

		inline CMatrix& operator -= (const CMatrix& m)
		{
			for(unsigned int r=0; r<4; r++)
			{
				for(unsigned int c=0; c<4; c++)
				{
					mat[r][c] -= m.mat[r][c];
				}
			}

			return *this;
		}

		inline CMatrix operator * (float f) const
		{
			CMatrix ma;

			for(unsigned int r=0; r<4; r++)
			{
				for(unsigned int c=0; c<4; c++)
				{
					ma.mat[r][c] = mat[r][c] * f;
				}
			}

			return ma;
		}

		inline CMatrix& operator *= (float f)
		{
			for(unsigned int r=0; r<4; r++)
			{
				for(unsigned int c=0; c<4; c++)
				{
					mat[r][c] *= f;
				}
			}

			return *this;
		}

		inline CMatrix operator / (float f) const
		{
			CMatrix ma;

			for(unsigned int r=0; r<4; r++)
			{
				for(unsigned int c=0; c<4; c++)
				{
					ma.mat[r][c] = mat[r][c] / f;
				}
			}

			return ma;
		}

		inline CMatrix& operator /= (float f)
		{
			for(unsigned int r=0; r<4; r++)
			{
				for(unsigned int c=0; c<4; c++)
				{
					mat[r][c] /= f;
				}
			}

			return *this;
		}

		// Functions
		inline void makeIdentity()
		{
			mat[0][0] = 1.0f; mat[0][1] = 0.0f; mat[0][2] = 0.0f; mat[0][3] = 0.0f;
			mat[1][0] = 0.0f; mat[1][1] = 1.0f; mat[1][2] = 0.0f; mat[1][3] = 0.0f;
			mat[2][0] = 0.0f; mat[2][1] = 0.0f; mat[2][2] = 1.0f; mat[2][3] = 0.0f;
			mat[3][0] = 0.0f; mat[3][1] = 0.0f; mat[3][2] = 0.0f; mat[3][3] = 1.0f; 
		}

		void makeLookAt(const CVec3& eye, const CVec3& point, const CVec3& up);

		void makeLookAtDirection(const CVec3& eye, const CVec3& dir, const CVec3& up);

		void makePerspective(float left, float right, float bottom, float top, float znear, float zfar);

		void makePerspectiveFOV(float hfov, float vfov, float aspect, float znear, float zfar);

		void makeOrthogonalPerspective(float left, float right, float bottom, float top, float znear, float zfar);

		inline void makeRotation(float angle, const CVec3& axis)
		{
			makeRotationRadians(UtilDegToRad(angle), axis);
		}

		inline void makeRotationRadians(float angle, const CVec3& axis)
		{
			setRotationRadians(angle, axis);
			setRow(3, CVec4::_w);
			setColumn(3, CVec3::_zero);
		}

		inline void makeRotation(float anglex, float angley, float anglez)
		{
			makeRotationRadians(UtilDegToRad(anglex), UtilDegToRad(angley), UtilDegToRad(anglez));
		}

		inline void makeRotationRadians(float anglex, float angley, float anglez)
		{
			setRotationRadians(anglex, angley, anglez);
			setRow(3, CVec4::_w);
			setColumn(3, CVec3::_zero);
		}

		inline void setRotation(float angle, const CVec3& axis)
		{
			setRotationRadians(UtilDegToRad(angle), axis);
		}

		void setRotationRadians(float angle, const CVec3& axis);

		inline void setRotation(float anglex, float angley, float anglez)
		{
			setRotationRadians(UtilDegToRad(anglex), UtilDegToRad(angley), UtilDegToRad(anglez));
		}

		void setRotationRadians(float anglex, float angley, float anglez);

		inline void getRotation(float& anglex, float& angley, float& anglez)
		{
			getRotationRadians(anglex, angley, anglez);

	        anglex = UtilRadToDeg(anglex);
	        angley = UtilRadToDeg(angley);
	        anglez = UtilRadToDeg(anglez);
		}

		void getRotationRadians(float& anglex, float& angley, float& anglez);

		inline void makeScale(const CVec3& scale)
		{
			makeIdentity();
			mat[0][0] = scale.x;
			mat[1][1] = scale.y;
			mat[2][2] = scale.z;
		}

		inline void makeScale(float scalar)
		{
			makeIdentity();
			mat[0][0] = scalar;
			mat[1][1] = scalar;
			mat[2][2] = scalar;
		}

		inline void setScale(const CVec3& scale)
		{
			mat[0][0] = scale.x;
			mat[1][1] = scale.y;
			mat[2][2] = scale.z;
		}

		inline void preScale(const CVec3& vector)
		{
			mat[0][0] *= vector.x;
			mat[0][1] *= vector.x;
			mat[0][2] *= vector.x;
			mat[0][3] *= vector.x;

			mat[1][0] *= vector.y;
			mat[1][1] *= vector.y;
			mat[1][2] *= vector.y;
			mat[1][3] *= vector.y;

			mat[2][0] *= vector.z;
			mat[2][1] *= vector.z;
			mat[2][2] *= vector.z;
			mat[2][3] *= vector.z;
		}

		inline void postScale(const CVec3& vector)
		{
			mat[0][0] *= vector.x;
			mat[0][1] *= vector.y;
			mat[0][2] *= vector.z;

			mat[1][0] *= vector.x;
			mat[1][1] *= vector.y;
			mat[1][2] *= vector.z;

			mat[2][0] *= vector.x;
			mat[2][1] *= vector.y;
			mat[2][2] *= vector.z;

			mat[3][0] *= vector.x;
			mat[3][1] *= vector.y;
			mat[3][2] *= vector.z;
		}

		inline void makeTranslation(const CVec3& translation)
		{
			mat[0][0] = 1.0f; mat[0][1] = 0.0f; mat[0][2] = 0.0f; mat[0][3] = 0.0f;
			mat[1][0] = 0.0f; mat[1][1] = 1.0f; mat[1][2] = 0.0f; mat[1][3] = 0.0f;
			mat[2][0] = 0.0f; mat[2][1] = 0.0f; mat[2][2] = 1.0f; mat[2][3] = 0.0f;
			mat[3][0] = translation.x;
			mat[3][1] = translation.y;
			mat[3][2] = translation.z;
			mat[3][3] = 1.0f;
		}

		void multiply(const CMatrix& m);

		void multiply(const CMatrix& m1, const CMatrix& m2);

		inline void preTranslation(const CVec3& translation)
		{
			mat[3][0] +=  translation.x * mat[0][0]
						+ translation.y * mat[1][0]
						+ translation.z * mat[2][0];

			mat[3][1] +=  translation.x * mat[0][1]
						+ translation.y * mat[1][1]
						+ translation.z * mat[2][1];
			
			mat[3][2] +=  translation.x * mat[0][2]
						+ translation.y * mat[1][2]
						+ translation.z * mat[2][2];
			
			mat[3][3] +=  translation.x * mat[0][3]
						+ translation.y * mat[1][3]
						+ translation.z * mat[2][3];
		}

		inline void postTranslation(const CVec3& translation)
		{
			mat[0][0] += mat[0][3] * translation.x;
			mat[0][1] += mat[0][3] * translation.y;
			mat[0][2] += mat[0][3] * translation.z;

			mat[1][0] += mat[1][3] * translation.x;
			mat[1][1] += mat[1][3] * translation.y;
			mat[1][2] += mat[1][3] * translation.z;

			mat[2][0] += mat[2][3] * translation.x;
			mat[2][1] += mat[2][3] * translation.y;
			mat[2][2] += mat[2][3] * translation.z;

			mat[3][0] += mat[3][3] * translation.x;
			mat[3][1] += mat[3][3] * translation.y;
			mat[3][2] += mat[3][3] * translation.z;
		}

		inline void preMultiply(const CMatrix& m)
		{
			multiply(m);
		}

		inline void postMultiply(const CMatrix& m)
		{
			multiply(*this, m);
		}

		inline void preRotation(float angle, const CVec3& axis)
		{
			preRotationRadians(UtilDegToRad(angle), axis);
		}

		inline void preRotationRadians(float angle, const CVec3& axis)
		{
			CMatrix m;
			m.makeRotationRadians(angle, axis);
			preMultiply(m);
		}

		inline void postRotation(float angle, const CVec3& axis)
		{
			postRotationRadians(UtilDegToRad(angle), axis);
		}

		inline void postRotationRadians(float angle, const CVec3& axis)
		{
			CMatrix m;
			m.makeRotationRadians(angle, axis);
			postMultiply(m);
		}

		inline void getTranslation(CVec3& translation) const
		{
			translation.x=mat[3][0];
			translation.y=mat[3][1];
			translation.z=mat[3][2];
		}

		inline void setTranslation(const CVec3& translation)
		{
			mat[3][0] = translation.x;
			mat[3][1] = translation.y;
			mat[3][2] = translation.z;
		}

		inline void getTranslation(CMatrix& translation) const
		{
			translation.makeTranslation(CVec3(mat[3][0], mat[3][1], mat[3][2]));
		}

		inline void transformPoint(CVec3* point) const
		{
			transformPoints(point, point, 1);
		}

		inline void transformPoints(CVec3* points, unsigned int iCount) const
		{
			transformPoints(points, points, iCount);
		}

		inline void transformPoint(CVec4* point) const
		{
			transformPoints(point, point, 1);
		}

		inline void transformPoints(CVec4* points, unsigned int iCount) const
		{
			transformPoints(points, points, iCount);
		}

		void transformPoints(const CVec3* from, CVec3* to, unsigned int iCount) const;
		void transformPoints(const CVec4* from, CVec4* to, unsigned int iCount) const;

		inline void transformVector(CVec3* vec) const
		{
			transformVectors(vec, vec, 1);
		}

		inline void transformVectors(CVec3* vecs, unsigned int iCount) const
		{
			transformVectors(vecs, vecs, iCount);
		}

		inline void transformVector(CVec4* vec) const
		{
			transformVectors(vec, vec, 1);
		}

		inline void transformVectors(CVec4* vecs, unsigned int iCount) const
		{
			transformVectors(vecs, vecs, iCount);
		}

		void transformVectors(const CVec3* from, CVec3* to, unsigned int iCount) const;
		void transformVectors(const CVec4* from, CVec4* to, unsigned int iCount) const;

		void invert();

		void transpose();

		float determinant() const;

		// Columns
		inline void getColumn(unsigned int iCol, CVec3& v) const
		{
			v.x=mat[0][iCol];
			v.y=mat[1][iCol];
			v.z=mat[2][iCol];
		}

		inline void getColumn(unsigned int iCol, CVec4& v) const
		{
			v.x=mat[0][iCol];
			v.y=mat[1][iCol];
			v.z=mat[2][iCol];
			v.w=mat[3][iCol];
		}

		inline void setColumn(unsigned int iCol, const CVec3& v)
		{
			mat[0][iCol]=v.x;
			mat[1][iCol]=v.y;
			mat[2][iCol]=v.z;
		}

		inline void setColumn(unsigned int iCol, const CVec4& v)
		{
			mat[0][iCol]=v.x;
			mat[1][iCol]=v.y;
			mat[2][iCol]=v.z;
			mat[3][iCol]=v.w;
		}

		// Rows
		inline void getRow(unsigned int iRow, CVec3& v) const
		{
			v.x=mat[iRow][0];
			v.y=mat[iRow][1];
			v.z=mat[iRow][2];
		}

		inline void getRow(unsigned int iRow, CVec4& v) const
		{
			v.x=mat[iRow][0];
			v.y=mat[iRow][1];
			v.z=mat[iRow][2];
			v.w=mat[iRow][3];
		}

		inline void setRow(unsigned int iRow, const CVec3& v)
		{
			mat[iRow][0]=v.x;
			mat[iRow][1]=v.y;
			mat[iRow][2]=v.z;
		}

		inline void setRow(unsigned int iRow, const CVec4& v)
		{
			mat[iRow][0]=v.x;
			mat[iRow][1]=v.y;
			mat[iRow][2]=v.z;
			mat[iRow][3]=v.w;
		}

		void adjoint(const CMatrix& in, CMatrix& out);

};

//

#endif // __Matrix_4x4_Class__


// DDObject
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

#ifndef __DD_NOTIFY__
#define __DD_NOTIFY__

// Forward declare DDObject class
class CDDObject;

// Notification interface for DDObjects
class IDDNotify
{
public:
	virtual ~IDDNotify() {};

	virtual void OnChanged(const CDDObject *pInstance, const char *pszKey)=0;
};

#endif

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

#ifndef __DD_BASE__
#define __DD_BASE__

// Dynamic Data types
enum EDDType
{
	DD_UNKNOWN=0,
	DD_OBJECT,
	DD_OBJLIST,
	DD_INT,
	DD_INTLIST,
	DD_FLOAT,
	DD_FLOATLIST,
	DD_STRING,
	DD_STRINGLIST,
	DD_BINARY,
	DD_VEC3,
	DD_VEC3LIST,
	DD_BOOL,
	DD_VEC2,
	DD_VEC4,
	DD_VEC4LIST,
	DD_MATRIX,
	DD_MATRIXLIST
};

// Dynamic Data Baseclass
class CDDBase : public CRefCount
{
public:
	virtual ~CDDBase(void) {};

	// Retrieve type of object
	virtual EDDType	GetType(void) const =0;

	// Create a clone of the object instance
	virtual CDDBase* Clone(void) const =0;
};

#endif

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

#ifndef __DD_OBJECT__
#define __DD_OBJECT__

#define GET_CHECK_TYPE

//#pragma warning(disable: 4275)
#pragma warning(disable: 4251)

class CDDObject : public CDDBase
{
public:
	// Constructor 
	CDDObject(void);

	// Clear object of all mappings
	void 	Clear(void);
	// Remove a specific mapping
	void	RemoveData(const char *pszID);	

	// Check if object has been changed. If bRecurse is true, all
	// contained DDObjects (DDList included) will also be checked.
	bool	HasChanged(bool bResetChange, bool bRecurse);

	// Deserialize from data stream
	void	FromDataStream(CDataStream *pStream, bool bAppend=false);
	void	ToDataStream(CDataStream *pStream) const;

	// Merge another DDObject into this one
	void	MergeWith(const CDDObject *pOther, bool bOverwrite=false);

	// Dump DDObject
	void	Dump(ELogLevel eLogLevel=LOG_INFO) const;
	
	// Save object as ASCII
	void	SaveASCII(const char *pszFile) const;

	// Make a new object, containing all changed or added keys,
	// compared to pOther.
	CDDObject	*BuildChanges(const CDDObject *pOther) const;

	// Create a clone of the object instance
	CDDBase* Clone(void) const;

	// Set notifier instance. Use this to get notified when object data changes
	void	AddNotifier(IDDNotify *pNotifier);
	void	RemoveNotifier(IDDNotify *pNotifier);

	// Query Functions
	// ---------------

	// Retrieve a list of keys
	fastvector<const char *>	GetKeyNames(void) const;

	// Find out which type of data a key holds (DDUnknown if not found)
	EDDType GetKeyType(const char *pszID) const;

	// Retrieve this type
	EDDType	GetType(void) const { return DD_OBJECT; }


	// Data Retrival functions
	// -----------------------

	// Get integer data
	int		GetInt(const char *pszID, int iDefValue=0) const;	
	// Get a list of integers
	int		*GetIntList(const char *pszID, int &iElemCount) const;

	// Get floating point data
	float	GetFloat(const char *pszID, float fDefValue=0.0f) const;
	// Get a list of floats
	float	*GetFloatList(const char *pszID, int &iElemCount) const;

	// Get string data
	const char *GetString(const char *pszID, const char *pszDefault="") const;
	// Get list of strings
	vector<faststring>	GetStringList(const char *pszID) const;

	// Get a DDObject
	CDDObject* GetDDObject(const char *pszID) const;
	// Get a DDObject List	
	fastvector<const CDDObject*> GetDDList(const char *pszID) const;

	// Get binary data
	const void *GetBinary(const char *pszID, int &iByteLength) const;	

	// Get vec3 data
	CVec3	GetVec3(const char *pszID, const CVec3& vDefValue = CVec3::_zero) const;
	// Get a list of vec3's
	CVec3* GetVec3List(const char *pszID, int &iElemCount) const;

	// Get bool data
	bool	GetBool(const char *pszID, bool bDefValue = false) const;

	// Get vec2 data
	CVec2	GetVec2(const char *pszID, const CVec2& vDefValue = CVec2::_zero) const;

	// Get vec4 data
	CVec4	GetVec4(const char *pszID, const CVec4& vDefValue = CVec4::_zero) const;
	// Get a list of vec4's
	CVec4* GetVec4List(const char *pszID, int &iElemCount) const;

	// Get matrix data
	CMatrix	GetMatrix(const char *pszID, const CMatrix& vDefValue = CMatrix::_zero) const;
	// Get a list of matrices
	CMatrix* GetMatrixList(const char *pszID, int &iElemCount) const;


	// Data Store functions
	// ---------------------------

	// Store integer 	
	void	SetInt(const char *pszID, int iData);
	// Set a list of integers
	void	SetIntList(const char *pszID, const int *pValues, int iCount);
	void	SetIntList(const char *pszID, fastvector<int> &lList);

	// Store float	
	void	SetFloat(const char *pszID, float fData);		
	// Set a list of floats
	void	SetFloatList(const char *pszID, const float *pValues, int iCount);
	void	SetFloatList(const char *pszID, fastvector<float> &lList);

	// Store string
	void	SetString(const char *pszID, const char *pszStr);	
	void	SetStringList(const char *pszID, vector<faststring> &lList);

	// Store DDObject
	void	SetDDObject(const char *pszID, const CDDObject *pObj);
	// Store DDObject List
	void	SetDDList(const char *pszID, fastvector<const CDDObject *> &lList, bool bAddRef=true);

	// Store binary
	void	SetBinary(const char *pszID, const void *pData, unsigned iByteLength);

	// Store vec3
	void	SetVec3(const char *pszID, const CVec3& vVec);		
	void	SetVec3List(const char *pszID, const CVec3* pValues, int iCount);

	// Store bool
	void	SetBool(const char *pszID, bool bBool);		

	// Store vec2
	void	SetVec2(const char *pszID, const CVec2& vVec);		

	// Store vec4
	void	SetVec4(const char *pszID, const CVec4& vVec);		
	void	SetVec4List(const char *pszID, const CVec4* pValues, int iCount);

	// Store matrix
	void	SetMatrix(const char *pszID, const CMatrix& mMat);		
	void	SetMatrixList(const char *pszID, const CMatrix* pValues, int iCount);


private:
	// Private destructor - use Release() to free object
	~CDDObject(void);

	void	BuildString(faststring &strout) const;

	// Marks any changes to the object
	bool	m_bChanged;

	// List of listeners on object
	fastvector<IDDNotify *> m_lListeners;

	// Map of data
	fastmap< const CDDBase * >	m_mData;
};

//typedef CRefCountPtr<CDDObject> CDDObjectRef;

#endif
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

#ifndef __DD_TYPES__
#define __DD_TYPES__

// Integer
class CDDInt : public CDDBase
{
public:
	CDDInt(int iValue) { m_iValue=iValue; }
	~CDDInt(void) {};

	// Return type
	EDDType	GetType(void) const { return DD_INT; }

	// Create a clone of the object instance
	CDDBase* Clone(void) const { return new CDDInt(m_iValue); }

	// Data
	int		m_iValue;
};

// Integer list
class CDDIntList : public CDDBase
{
public:
	CDDIntList(int iCount, int *pValues) { m_pValues=pValues;m_iCount=iCount; }
	CDDIntList(const int *pValues, int iCount) { m_pValues=new int[iCount]; m_iCount=iCount;CopyMemory(m_pValues, pValues, iCount*sizeof(int)); }
	~CDDIntList(void) { delete []m_pValues; };

	// Return type
	EDDType	GetType(void) const { return DD_INTLIST; }

	// Create a clone of the object instance
	CDDBase* Clone(void) const { return new CDDIntList(m_pValues, m_iCount); }

	// Data
	int		m_iCount;
	int		*m_pValues;
};

// Float
class CDDFloat : public CDDBase
{
public:
	CDDFloat(float fValue) { m_fValue=fValue; }
	~CDDFloat(void) {};

	// Return type
	EDDType	GetType(void) const { return DD_FLOAT; }

	// Create a clone of the object instance
	CDDBase* Clone(void) const { return new CDDFloat(m_fValue); }

	// Data
	float	m_fValue;
};

// Float list
class CDDFloatList : public CDDBase
{
public:
	CDDFloatList(int iCount, float *pValues) { m_pValues=pValues;m_iCount=iCount; }
	CDDFloatList(const float *pValues, int iCount) { m_pValues=new float[iCount];m_iCount=iCount;CopyMemory(m_pValues, pValues, iCount*sizeof(float)); }
	~CDDFloatList(void) { delete []m_pValues; }

	// Return type
	EDDType	GetType(void) const { return DD_FLOATLIST; }

	// Create a clone of the object instance
	CDDBase* Clone(void) const { return new CDDFloatList(m_pValues, m_iCount); }

	// Data
	int		m_iCount;
	float	*m_pValues;	
};

// String
class CDDString : public CDDBase
{
public:
	CDDString(const char *pszStr) { m_sValue=_strdup(pszStr); }
	~CDDString(void) { free(m_sValue); };

	// Return type
	EDDType	GetType(void) const { return DD_STRING; }

	// Create a clone of the object instance
	CDDBase* Clone(void) const { return new CDDString(m_sValue); }

	// Data
	char	*m_sValue;
};

// String list
class CDDStringList : public CDDBase
{
public:
	CDDStringList(vector<faststring> &value) { m_Value=value; }
	~CDDStringList(void) { };

	// Return type
	EDDType	GetType(void) const { return DD_STRINGLIST; }

	// Create a clone of the object instance
	CDDBase* Clone(void) const {	vector<faststring> newvalue=m_Value;
									return new CDDStringList(newvalue); }

	// Data
	vector<faststring>	m_Value;
};

// DDObject list
class CDDObjectList : public CDDBase
{
public:
	CDDObjectList(void) {};	
	CDDObjectList(fastvector<const CDDObject*> const &lList, bool bAddRef) { 
							for(unsigned i=0;i<lList.size();i++) { 
									m_lList.push_back(lList[i]);
									if(bAddRef) lList[i]->AddRef();}	}
	CDDObjectList(fastvector<const CDDObject*> const &lList) { 
							for(unsigned i=0;i<lList.size();i++) { 
									m_lList.push_back((CDDObject*)lList[i]->Clone());
									}	}
	~CDDObjectList(void) { for(unsigned i=0;i<m_lList.size();i++) m_lList[i]->Release(); }

	// Return type
	EDDType	GetType(void) const { return DD_OBJLIST; }

	// Create a clone of the object instance
	CDDBase* Clone(void) const { return new CDDObjectList(m_lList); }

	// The list
	fastvector<const CDDObject*>	m_lList;
};

// Float list
class CDDBinary : public CDDBase
{
public:
	CDDBinary(int iSize, void *pData) { m_pData=pData;m_iSize=iSize; }
	CDDBinary(const void *pData, int iSize) { m_pData=new char[iSize];m_iSize=iSize;CopyMemory(m_pData, pData, iSize); }
	~CDDBinary(void) { delete []m_pData; }

	// Return type
	EDDType	GetType(void) const { return DD_BINARY; }

	// Create a clone of the object instance
	CDDBase* Clone(void) const { return new CDDBinary(m_pData, m_iSize); }

	// Data
	unsigned	m_iSize;
	void		*m_pData;	
};

// Vec3
class CDDVec3 : public CDDBase
{
public:
	CDDVec3(float fX, float fY, float fZ) { m_Vec.x = fX; m_Vec.y = fY; m_Vec.z = fZ; }
	~CDDVec3(void) {}

	// Return type
	EDDType	GetType(void) const { return DD_VEC3; }

	// Create a clone of the object instance
	CDDBase* Clone(void) const { return new CDDVec3(m_Vec.x, m_Vec.y, m_Vec.z); }

	// Data
	CVec3 m_Vec;
};

// Vec3 list
class CDDVec3List : public CDDBase
{
public:
	CDDVec3List(int iCount, CVec3* pValues) { m_pValues = pValues; m_iCount = iCount; }
	CDDVec3List(const CVec3* pValues, int iCount) { m_pValues = new CVec3[iCount]; m_iCount = iCount; CopyMemory(m_pValues, pValues, iCount * sizeof(CVec3)); }
	~CDDVec3List(void) { delete []m_pValues; }

	// Return type
	EDDType	GetType(void) const { return DD_VEC3LIST; }

	// Create a clone of the object instance
	CDDBase* Clone(void) const { return new CDDVec3List(m_pValues, m_iCount); }

	// Data
	int		m_iCount;
	CVec3*	m_pValues;	
};

// Boolean
class CDDBool : public CDDBase
{
public:
	CDDBool(bool bValue) { m_bValue = bValue; }
	~CDDBool(void) {}

	// Return type
	EDDType	GetType(void) const { return DD_BOOL; }

	// Create a clone of the object instance
	CDDBase* Clone(void) const { return new CDDBool(m_bValue); }

	// Data
	bool		m_bValue;
};

// Vec2
class CDDVec2 : public CDDBase
{
public:
	CDDVec2(float fX, float fY) { m_Vec.x = fX; m_Vec.y = fY; }
	~CDDVec2(void) {}

	// Return type
	EDDType	GetType(void) const { return DD_VEC2; }

	// Create a clone of the object instance
	CDDBase* Clone(void) const { return new CDDVec2(m_Vec.x, m_Vec.y); }

	// Data
	CVec2 m_Vec;
};

// Vec4
class CDDVec4 : public CDDBase
{
public:
	CDDVec4(float fX, float fY, float fZ, float fW) { m_Vec.x = fX; m_Vec.y = fY; m_Vec.z = fZ; m_Vec.w = fW; }
	~CDDVec4(void) {}

	// Return type
	EDDType	GetType(void) const { return DD_VEC4; }

	// Create a clone of the object instance
	CDDBase* Clone(void) const { return new CDDVec4(m_Vec.x, m_Vec.y, m_Vec.z, m_Vec.w); }

	// Data
	CVec4 m_Vec;
};

// Vec4 list
class CDDVec4List : public CDDBase
{
public:
	CDDVec4List(int iCount, CVec4* pValues) { m_pValues = pValues; m_iCount = iCount; }
	CDDVec4List(const CVec4* pValues, int iCount) { m_pValues = new CVec4[iCount]; m_iCount = iCount; CopyMemory(m_pValues, pValues, iCount * sizeof(CVec4)); }
	~CDDVec4List(void) { delete []m_pValues; }

	// Return type
	EDDType	GetType(void) const { return DD_VEC4LIST; }

	// Create a clone of the object instance
	CDDBase* Clone(void) const { return new CDDVec4List(m_pValues, m_iCount); }

	// Data
	int		m_iCount;
	CVec4*	m_pValues;
};

// Matrix
class CDDMatrix : public CDDBase
{
public:
	CDDMatrix(const float* pValues) { CopyMemory(m_Matrix.mat, pValues, 4 * 4 * 4); }
	~CDDMatrix(void) {}

	// Return type
	EDDType	GetType(void) const { return DD_MATRIX; }

	// Create a clone of the object instance
	CDDBase* Clone(void) const { return new CDDMatrix((const float*)m_Matrix.mat); }

	// Data
	CMatrix m_Matrix;
};

// Matrix list
class CDDMatrixList : public CDDBase
{
public:
	CDDMatrixList(int iCount, CMatrix* pValues) { m_pValues = pValues; m_iCount = iCount; }
	CDDMatrixList(const CMatrix* pValues, int iCount) { m_pValues = new CMatrix[iCount]; m_iCount = iCount; CopyMemory(m_pValues, pValues, iCount * sizeof(CMatrix)); }
	~CDDMatrixList(void) { delete []m_pValues; }

	// Return type
	EDDType	GetType(void) const { return DD_MATRIXLIST; }

	// Create a clone of the object instance
	CDDBase* Clone(void) const { return new CDDMatrixList(m_pValues, m_iCount); }

	// Data
	int			m_iCount;
	CMatrix*	m_pValues;
};

#endif

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


// Commandline Parser
extern CDDObject *GetCommandLineParms(void);
}

using namespace SharedUtilities;
/*
#ifdef _DEBUG
	#pragma comment(linker, "/DEFAULTLIB:SharedUtilitiesd.lib")
#else
	#pragma comment(linker, "/DEFAULTLIB:SharedUtilities.lib")
#endif
*/
#endif

