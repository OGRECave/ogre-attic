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
