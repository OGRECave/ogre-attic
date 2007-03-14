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

CDataStream::CDataStream(unsigned iInitSize, unsigned iGrow)
{
	// Initialize data
	m_iAllocSize=iInitSize;
	m_iGrowSize=iGrow;
	m_iPosition=0;
	m_pData=new char[m_iAllocSize];
	m_bOwnData=true;
	m_pDataOwner=NULL;
#ifdef _DEBUG
	m_iReallocCount=0; 
#endif
}
CDataStream::CDataStream(const CDataStream &other)
{
	// Setup data from other instance (reference only, data will be copied if needed)
	m_iAllocSize=other.m_iAllocSize;
	m_iGrowSize=other.m_iGrowSize;
	m_iPosition=other.m_iPosition;
	m_pData=other.m_pData;	
	m_bOwnData=false;
	m_pDataOwner=&other;
	other.AddRef();
#ifdef _DEBUG
	m_iReallocCount=0; 
#endif
}
CDataStream::CDataStream(const void *pOther, unsigned iSize, CRefCount *pDataOwner)
{
	// Setup data, optionally including data owner
	m_iAllocSize=iSize;
	m_iGrowSize=512;
	m_iPosition=0;
	m_pData=(char*)pOther;
	m_bOwnData=false;
	m_pDataOwner=pDataOwner;
	if(m_pDataOwner) m_pDataOwner->AddRef();
#ifdef _DEBUG
	m_iReallocCount=0; 
#endif
}
CDataStream::CDataStream(void *pOther, unsigned iSize)
{
	// Setup data, optionally including data owner
	m_iAllocSize=iSize;
	m_iGrowSize=512;
	m_iPosition=0;
	m_pData=(char*)pOther;
	m_bOwnData=true;	
	m_pDataOwner=0;
#ifdef _DEBUG
	m_iReallocCount=0; 
#endif
}

CDataStream::~CDataStream(void)
{
	if(m_bOwnData)	delete[] m_pData;
	else if(m_pDataOwner) m_pDataOwner->Release();
}

// Insert integer at a position
void CDataStream::InsertInt(int iData, int iPosition)
{
	if(m_iPosition+4>m_iAllocSize) Realloc(4);
	memmove(m_pData+iPosition+4, m_pData+iPosition, m_iPosition-iPosition);
	*((int*)(m_pData+iPosition))=iData;
	m_iPosition+=4;
}
void CDataStream::InsertString(const char *pszData, int iPosition)
{
	int iLen=strlen(pszData)+1;

	if(m_iPosition+iLen>m_iAllocSize) Realloc(iLen);
	memmove(m_pData+iPosition+iLen, m_pData+iPosition, m_iPosition-iPosition);
	memcpy(m_pData+iPosition, pszData, iLen);	
	m_iPosition+=iLen;
}

// Add
void CDataStream::AddByte(unsigned char bData)
{
	if(m_iPosition+1>m_iAllocSize || !m_bOwnData) Realloc(1);
	*((unsigned char*)(m_pData+m_iPosition))=bData;
	m_iPosition+=1;
}
void CDataStream::AddInt(int iData)
{
	if(m_iPosition+4>m_iAllocSize || !m_bOwnData) Realloc(4);
	*((int*)(m_pData+m_iPosition))=iData;
	m_iPosition+=4;
}
void CDataStream::AddFloat(float fData)
{
	if(m_iPosition+4>m_iAllocSize || !m_bOwnData) Realloc(4);
	*((float*)(m_pData+m_iPosition))=fData;
	m_iPosition+=4;
}
/*void CDataStream::AddString(const faststring &sData)
{
	int iLen=sData.size()+1;
	if(m_iPosition+iLen>m_iAllocSize || !m_bOwnData) Realloc(iLen);
	memcpy(m_pData+m_iPosition, sData.c_str(), iLen);
	m_iPosition+=iLen;
}*/
void CDataStream::AddString(const char *pszData)
{
	int iLen=strlen(pszData)+1;
	if(m_iPosition+iLen>m_iAllocSize || !m_bOwnData) Realloc(iLen);
	memcpy(m_pData+m_iPosition, pszData, iLen);
	m_iPosition+=iLen;
}
void CDataStream::AddBinary(const void *pData, unsigned iLen)
{
	AddInt(iLen);
	if(m_iPosition+iLen>m_iAllocSize) Realloc(iLen);
	memcpy(m_pData+m_iPosition, pData, iLen);
	m_iPosition+=iLen;
}
void CDataStream::AddRAW(const void *pData, unsigned iLen)
{
	if(m_iPosition+iLen>m_iAllocSize) Realloc(iLen);
	memcpy(m_pData+m_iPosition, pData, iLen);
	m_iPosition+=iLen;
}
void CDataStream::AddBool(bool bData)
{
	if(m_iPosition+1>m_iAllocSize || !m_bOwnData) Realloc(1);
	*((bool*)(m_pData+m_iPosition))=bData;
	m_iPosition+=1;
}

// Retrieve
unsigned char CDataStream::GetByte(void) const
{
	m_iPosition+=1;
	return *(unsigned char*)((char*)(m_pData+(m_iPosition-1)));
}
int	CDataStream::GetInt(void) const
{
	m_iPosition+=4;
	return *(int*)((char*)(m_pData+(m_iPosition-4)));
}
float CDataStream::GetFloat(void) const
{
	m_iPosition+=4;
	return *(float*)((char*)(m_pData+(m_iPosition-4)));
}
const char *CDataStream::GetString(void) const
{
	const char *pStr=m_pData+m_iPosition;
	m_iPosition+=strlen(pStr)+1;
	return pStr;
}
const void *CDataStream::GetBinary(unsigned &iLen) const
{
	iLen=GetInt();
	const void *pRet=m_pData+m_iPosition;
	m_iPosition+=iLen;
	return pRet;
}
const void* CDataStream::GetRAW(unsigned int iLen) const
{
	const void *pRet=m_pData+m_iPosition;
	m_iPosition+=iLen;
	return pRet;
}
bool CDataStream::GetBool() const
{
	m_iPosition+=1;
	return *(bool*)((char*)(m_pData+(m_iPosition-1)));
}

// Reset position
void CDataStream::SetPosition(unsigned iPos) const
{
	if(iPos<=m_iAllocSize && iPos>=0) m_iPosition=iPos;
}
int	CDataStream::GetPosition(void) const
{
	return m_iPosition;
}

//
void CDataStream::Reserve(unsigned int iNewAlloc)
{
	if(m_iAllocSize<iNewAlloc) Realloc(iNewAlloc-m_iAllocSize);
}

void CDataStream::Realloc(int iSpaceNeeded)
{
	// Allocate new chunk
	char *pNewData=new char[m_iAllocSize+iSpaceNeeded+m_iGrowSize];
	memcpy(pNewData, m_pData, m_iAllocSize);
	if(m_bOwnData) delete[] m_pData;
	m_pData=pNewData;
	m_iAllocSize+=iSpaceNeeded+m_iGrowSize;

	// Did we previously own data?
	if(!m_bOwnData)
	{
		// Nope, did we have a data owner reference?
		if(m_pDataOwner) 
		{
			// Yep, release data owner
			m_pDataOwner->Release();
			m_pDataOwner=NULL;
		}
		// We own the data now
		m_bOwnData=true;
	}

#ifdef _DEBUG
	if(++m_iReallocCount>5) LOGWARNING "Performance warning, datastream realloccount>5 %X, size=%d", (int)this, m_iAllocSize);
#endif
}