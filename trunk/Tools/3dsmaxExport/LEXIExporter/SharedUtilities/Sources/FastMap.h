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
