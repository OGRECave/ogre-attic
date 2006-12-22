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

// Constructor 
CDDObject::CDDObject(void)
{
	m_iRefCount=1;
	m_bChanged=false;
	m_pNotifier=0;
}

// Private destructor - use Release() to free object
CDDObject::~CDDObject(void)
{
	Clear();	
}

// Clear object of all mappings
void CDDObject::Clear(void)
{
	// Get list of elements
	fastvector< const CDDBase * > lList=m_mData.data();

	unsigned i=0;
	try 
	{
		// Release all elements
		for(i=0;i<lList.size();i++)
			lList[i]->Release();
	} catch(...)
	{
		LOGDEBUG "Caught exception while releasing object %d, %X", i, lList[i]);
	}

	// Clear map
	m_mData.clear();
	m_bChanged=true;	
}

// Remove a specific mapping
void CDDObject::RemoveData(const char *pszID)
{
	const CDDBase *pItem;
	if(m_mData.find(pszID,pItem))
	{
		pItem->Release();
		m_mData.erase(pszID);
		m_bChanged=true;
	}
}

// Check if object has been changed
bool CDDObject::HasChanged(bool bResetChange)
{
	bool bRet=m_bChanged;
	if(bResetChange) m_bChanged=false;
	return bRet;
}

// Deserialize from data stream
void CDDObject::FromDataStream(CDataStream *pStream, bool bAppend)
{
	// Free header and optionally clear existing keys	
	if(!bAppend) Clear();

	int iKeyCount=pStream->GetInt();
	if(!iKeyCount) return;

	for(int iKey=0;iKey<iKeyCount;iKey++)
	{
		EDDType eType=(EDDType)pStream->GetInt();
		const char *pszKey=pStream->GetString();
		const CDDBase *pOldItem;

		switch(eType)
		{
			case DD_OBJECT:
				{
				CDDObject *pNewDD=new CDDObject;
				pNewDD->FromDataStream(pStream, true);				
                m_mData.map(pszKey, pNewDD, pOldItem);				
				} break;
			case DD_OBJLIST:
				{
				int iCount=pStream->GetInt();
				CDDObjectList *pDDList=new CDDObjectList;
				for(int iSubDD=0;iSubDD<iCount;iSubDD++)
				{
					CDDObject *pNew=new CDDObject;
					pNew->FromDataStream(pStream, true);
					pDDList->m_lList.push_back(pNew);
				}
				m_mData.map(pszKey, pDDList, pOldItem);
				} break;
			case DD_INT:
				m_mData.map(pszKey, new CDDInt(pStream->GetInt()), pOldItem);				
				break;
			case DD_INTLIST:
				{
				int iCount=pStream->GetInt();
				int *pValues=new int[iCount];
				for(int i=0;i<iCount;i++)	pValues[i]=pStream->GetInt();				
				m_mData.map(pszKey, new CDDIntList(iCount, pValues), pOldItem);
				} break;
			case DD_FLOAT:
				m_mData.map(pszKey, new CDDFloat(pStream->GetFloat()), pOldItem);				
				break;
			case DD_FLOATLIST:
				{
				int iCount=pStream->GetInt();
				float *pfValues=new float[iCount];
				for(int i=0;i<iCount;i++)	pfValues[i]=pStream->GetFloat();
				m_mData.map(pszKey, new CDDFloatList(iCount, pfValues), pOldItem);
				} break;
			case DD_STRING:
				{
				m_mData.map(pszKey, new CDDString(pStream->GetString()), pOldItem);
				} break;
			case DD_STRINGLIST:
				{
				vector<faststring> lList;
				int iCount=pStream->GetInt();
				for(int iS=0;iS<iCount;iS++)
				{
					faststring s=pStream->GetString();
					lList.push_back(s);
				}
				m_mData.map(pszKey, new CDDStringList(lList));
				} break;
			case DD_BINARY:
				{
				unsigned iSize;
				const void *pData=pStream->GetBinary(iSize);				
				m_mData.map(pszKey, new CDDBinary(pData, iSize), pOldItem);
				} break;
			case DD_VEC3:
				{
				float fX = pStream->GetFloat();
				float fY = pStream->GetFloat();
				float fZ = pStream->GetFloat();
				m_mData.map(pszKey, new CDDVec3(fX, fY, fZ), pOldItem);				
				} break;
			case DD_VEC3LIST:
				{
				int iCount = pStream->GetInt();
				CVec3* pValues = new CVec3[iCount];
				for(int i = 0;i < iCount; i++)
				{
					pValues[i].x = pStream->GetFloat();
					pValues[i].y = pStream->GetFloat();
					pValues[i].z = pStream->GetFloat();
				}
				m_mData.map(pszKey, new CDDVec3List(iCount, pValues), pOldItem);
				} break;
			case DD_BOOL:
				m_mData.map(pszKey, new CDDBool(pStream->GetBool()), pOldItem);				
				break;
			case DD_VEC2:
				{
				float fX = pStream->GetFloat();
				float fY = pStream->GetFloat();
				m_mData.map(pszKey, new CDDVec2(fX, fY), pOldItem);				
				} break;
			case DD_VEC4:
				{
				float fX = pStream->GetFloat();
				float fY = pStream->GetFloat();
				float fZ = pStream->GetFloat();
				float fW = pStream->GetFloat();
				m_mData.map(pszKey, new CDDVec4(fX, fY, fZ, fW), pOldItem);				
				} break;
			case DD_VEC4LIST:
				{
				int iCount = pStream->GetInt();
				CVec4* pValues = new CVec4[iCount];
				for(int i = 0;i < iCount; i++)
				{
					pValues[i].x = pStream->GetFloat();
					pValues[i].y = pStream->GetFloat();
					pValues[i].z = pStream->GetFloat();
					pValues[i].w = pStream->GetFloat();
				}
				m_mData.map(pszKey, new CDDVec4List(iCount, pValues), pOldItem);
				} break;
			case DD_MATRIX:
				{
				float mat[16];
				for(unsigned int x = 0; x < 16; x++) mat[x] = pStream->GetFloat();
				m_mData.map(pszKey, new CDDMatrix(mat), pOldItem);
				} break;
			case DD_MATRIXLIST:
				{
				int iCount = pStream->GetInt();
				CMatrix* pValues = new CMatrix[iCount];
				for(int i = 0;i < iCount; i++)
				{
					float* pMat = (float*)pValues[i].mat;
					for(unsigned int x = 0; x < 16; x++) pMat[x] = pStream->GetFloat();
				}
				m_mData.map(pszKey, new CDDMatrixList(iCount, pValues), pOldItem);
				} break;
			default:
				LOGWARNING "Got invalid type in FromDataStream: %d", eType);
				continue;
		}
	}
	m_bChanged=true;
}

void CDDObject::ToDataStream(CDataStream *pStream) const
{
	// Get list of elements
	fastvector< fastmap<const CDDBase *>::SDataPair > lList=m_mData.iterate();

	// DDObject "header");	
	pStream->AddInt(lList.size());
	for(unsigned i=0;i<lList.size();i++)
	{		
		const CDDBase *pItem=lList[i].Data;
		EDDType eType=pItem->GetType();
		pStream->AddInt(eType);
		pStream->AddString(lList[i].pszKey);		
		switch(eType)
		{
			case DD_OBJECT:
				((CDDObject*)pItem)->ToDataStream(pStream);
				break;
			case DD_OBJLIST:
				{
				CDDObjectList *pList=(CDDObjectList*)pItem;
				pStream->AddInt(pList->m_lList.size());
				for(unsigned i=0;i<pList->m_lList.size();i++)
					pList->m_lList[i]->ToDataStream(pStream);
				} break;
			case DD_INT:
				pStream->AddInt(((CDDInt*)(pItem))->m_iValue);
				break;
			case DD_INTLIST:
				{
				CDDIntList *pList=(CDDIntList*)pItem;
				pStream->AddInt(pList->m_iCount);
				for(int i=0;i<pList->m_iCount;i++)	pStream->AddInt(pList->m_pValues[i]);
				} break;
			case DD_FLOAT:
				pStream->AddFloat(((CDDFloat*)(pItem))->m_fValue);
				break;
			case DD_FLOATLIST:
				{
				CDDFloatList *pList=(CDDFloatList*)pItem;
				pStream->AddInt(pList->m_iCount);
				for(int i=0;i<pList->m_iCount;i++)	pStream->AddFloat(pList->m_pValues[i]);
				} break;
			case DD_STRING:
				pStream->AddString(((CDDString*)(pItem))->m_sValue);
				break;
			case DD_STRINGLIST:
				{
				CDDStringList *pList=(CDDStringList*)pItem;
				pStream->AddInt(pList->m_Value.size());
				for(unsigned iS=0;iS<pList->m_Value.size();iS++)
					pStream->AddString(pList->m_Value[iS].c_str());
				} break;
			case DD_VEC3:
				{
				const CDDVec3* pVec3 = (const CDDVec3*)pItem;
				pStream->AddFloat(pVec3->m_Vec.x);
				pStream->AddFloat(pVec3->m_Vec.y);
				pStream->AddFloat(pVec3->m_Vec.z);
				} break;
			case DD_VEC3LIST:
				{
				CDDVec3List* pList = (CDDVec3List*)pItem;
				pStream->AddInt(pList->m_iCount);
				for(int i = 0; i < pList->m_iCount; i++)
				{
					pStream->AddFloat(pList->m_pValues[i].x);
					pStream->AddFloat(pList->m_pValues[i].y);
					pStream->AddFloat(pList->m_pValues[i].z);
				}
				} break;
			case DD_BOOL:
				pStream->AddBool(((CDDBool*)(pItem))->m_bValue);
				break;
			case DD_VEC2:
				{
				const CDDVec2* pVec2 = (const CDDVec2*)pItem;
				pStream->AddFloat(pVec2->m_Vec.x);
				pStream->AddFloat(pVec2->m_Vec.y);
				} break;
			case DD_VEC4:
				{
				const CDDVec4* pVec4 = (const CDDVec4*)pItem;
				pStream->AddFloat(pVec4->m_Vec.x);
				pStream->AddFloat(pVec4->m_Vec.y);
				pStream->AddFloat(pVec4->m_Vec.z);
				pStream->AddFloat(pVec4->m_Vec.w);
				} break;
			case DD_VEC4LIST:
				{
				CDDVec4List* pList = (CDDVec4List*)pItem;
				pStream->AddInt(pList->m_iCount);
				for(int i = 0; i < pList->m_iCount; i++)
				{
					pStream->AddFloat(pList->m_pValues[i].x);
					pStream->AddFloat(pList->m_pValues[i].y);
					pStream->AddFloat(pList->m_pValues[i].z);
					pStream->AddFloat(pList->m_pValues[i].w);
				}
				} break;
			case DD_MATRIX:
				{
				const CDDMatrix* pMatrix = (const CDDMatrix*)pItem;
				const float* pMat = (const float*)pMatrix->m_Matrix.mat;
				for(unsigned int x = 0; x < 16; x++) pStream->AddFloat(pMat[x]);
				} break;
			case DD_MATRIXLIST:
				{
				CDDMatrixList* pList = (CDDMatrixList*)pItem;
				pStream->AddInt(pList->m_iCount);
				for(int i = 0; i < pList->m_iCount; i++)
				{
					const CMatrix& Mat = pList->m_pValues[i];
					const float* pMat = (const float*)Mat.mat;
					for(unsigned int x = 0; x < 16; x++) pStream->AddFloat(pMat[x]);
				}
				} break;

			case DD_BINARY:
				{
				} break;
		}
	}
}

void CDDObject::Dump(ELogLevel eLogLevel) const
{
	// Get list of elements
	fastvector< fastmap<const CDDBase *>::SDataPair > lList=m_mData.iterate();

	// DDObject "header");
	LOGCUSTOM eLogLevel, "Dumping DDObject");
	for(unsigned i=0;i<lList.size();i++)
	{		
		const CDDBase *pItem=lList[i].Data;
		EDDType eType=pItem->GetType();
						
		switch(eType)
		{
			case DD_OBJECT:
				LOGCUSTOM eLogLevel, "%s=(ddobject) {", lList[i].pszKey);							
				((CDDObject*)pItem)->Dump(eLogLevel);
				LOGCUSTOM eLogLevel, "}; // end DDObject");							
				break;
			case DD_OBJLIST:
				{
				LOGCUSTOM eLogLevel, "%s=(ddlist):", lList[i].pszKey);							
				CDDObjectList *pList=((CDDObjectList*)pItem);
				for(unsigned j=0;j<pList->m_lList.size();j++)
				{
					LOGCUSTOM eLogLevel, " //List, object=%d", j);
					pList->m_lList[j]->Dump(eLogLevel);
				}
				} break;
			case DD_INT:
				LOGCUSTOM eLogLevel, "%s=(int)%d;", lList[i].pszKey, ((CDDInt*)(pItem))->m_iValue);				
				break;
			case DD_INTLIST:
				{
				LOGCUSTOM eLogLevel, "%s=(intlist);", lList[i].pszKey);							
				} break;
			case DD_FLOAT:
				LOGCUSTOM eLogLevel, "%s=(float)%f;", lList[i].pszKey, ((CDDFloat*)(pItem))->m_fValue);				
				break;
			case DD_FLOATLIST:
				{				
				LOGCUSTOM eLogLevel, "%s=(floatlist);", lList[i].pszKey);							
				} break;
			case DD_STRINGLIST:
				{
				LOGCUSTOM eLogLevel, "%s=(stringlist):", lList[i].pszKey);							
				CDDStringList *pList=((CDDStringList*)pItem);
				for(unsigned j=0;j<pList->m_Value.size();j++)
					LOGCUSTOM eLogLevel, "%s", pList->m_Value[j].c_str());
				}break;
			case DD_STRING:
				LOGCUSTOM eLogLevel, "%s=(string)\"%s\";", lList[i].pszKey, ((CDDString*)(pItem))->m_sValue);							
				break;
			case DD_VEC3:
				{
				const CDDVec3* pVec3 = (const CDDVec3*)pItem;
				LOGCUSTOM eLogLevel, "%s=(vec3)%f|%f|%f;", lList[i].pszKey, pVec3->m_Vec.x, pVec3->m_Vec.y, pVec3->m_Vec.z);
				} break;
			case DD_VEC3LIST:
				{				
				LOGCUSTOM eLogLevel, "%s=(vec3list);", lList[i].pszKey);							
				} break;
			case DD_BOOL:
				LOGCUSTOM eLogLevel, "%s=(bool)%s;", lList[i].pszKey, ((CDDBool*)(pItem))->m_bValue ? "true" : "false");
				break;
		}
	}
}

// Save object as ASCII
void CDDObject::SaveASCII(const char *pszFile) const
{
	HANDLE hFile=CreateFile(pszFile, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, 0);
	if(hFile==INVALID_HANDLE_VALUE) return;

	faststring sOut="";
	BuildString(sOut);
	
	DWORD dwBytesWritten;
	WriteFile(hFile, sOut.c_str(), sOut.size(), &dwBytesWritten, NULL);
	CloseHandle(hFile);
	return;
}

void CDDObject::BuildString(faststring &strout) const
{
	char buffer[8192];

	// Get list of elements
	fastvector< fastmap<const CDDBase *>::SDataPair > lList=m_mData.iterate();

	//
	for(unsigned i=0;i<lList.size();i++)
	{		
		const CDDBase *pItem=lList[i].Data;
		EDDType eType=pItem->GetType();
		strout.add(lList[i].pszKey);
						
		switch(eType)
		{
			case DD_OBJECT:
				strout.add("=(ddobject) {\r\n");				
				((CDDObject*)pItem)->BuildString(strout);
				strout.add("};\r\n");
				break;
			case DD_OBJLIST:
				{
				strout.add("=(ddlist) {");
				CDDObjectList *pList=(CDDObjectList*)pItem;				
				for(unsigned i=0;i<pList->m_lList.size();i++)
				{
					strout.add((i>0 ? ",{\r\n" : "{\r\n"));
					pList->m_lList[i]->BuildString(strout);
					strout.add("}");
				}
				strout.add("};\r\n");
				} break;
			case DD_INT:
				_snprintf(buffer, 8192, "=(int)%d;\r\n", ((CDDInt*)(pItem))->m_iValue);
				strout.add(buffer);				
				break;
			case DD_INTLIST:
				{
				strout.add("=(intlist) {");
				CDDIntList *pList=(CDDIntList*)pItem;				
				for(int i=0;i<pList->m_iCount;i++)	
				{
					_snprintf(buffer, 8192, "%s%d", (i>0 ? ", " : ""), pList->m_pValues[i]);
				}
				strout.add("};\r\n");
				} break;
			case DD_FLOAT:
				_snprintf(buffer, 8192, "=(float)%f;\r\n", ((CDDFloat*)(pItem))->m_fValue);
				strout.add(buffer);	
				break;
			case DD_FLOATLIST:
				{				
				strout.add("=(floatlist) {");
				CDDFloatList *pList=(CDDFloatList*)pItem;				
				for(int i=0;i<pList->m_iCount;i++)	
				{
					_snprintf(buffer, 8192, "%s%.2f", (i>0 ? ", " : ""), pList->m_pValues[i]);
					strout.add(buffer);
				}
				strout.add("};\r\n");
				} break;
			case DD_STRING:
				strout.add("=(string)\"");
				strout.add(((CDDString*)(pItem))->m_sValue);
				strout.add("\";\r\n");				
				break;
			case DD_STRINGLIST:
				{
				strout.add("=(stringlist) {\r\n");
				vector<faststring> &lList=((CDDStringList*)pItem)->m_Value;
				for(unsigned i=0;i<lList.size();i++)	
				{
					_snprintf(buffer, 8192, "%s\"%s\"\r\n", (i>0 ? ", " : ""), lList[i].c_str());
					strout.add(buffer);
				}
				strout.add("};\r\n");
				} break;
			case DD_VEC3:
				{
				const CDDVec3* pVec3 = (const CDDVec3*)pItem;
				_snprintf(buffer, 8192, "=(vec3)%f|%f|%f;\r\n", pVec3->m_Vec.x, pVec3->m_Vec.y, pVec3->m_Vec.z);
				strout.add(buffer);	
				} break;
			case DD_VEC3LIST:
				{				
				strout.add("=(vec3list) {");
				const CDDVec3List* pList = (const CDDVec3List*)pItem;
				for(int i = 0;i < pList->m_iCount; i++)
				{
					const CVec3& Vec3 = pList->m_pValues[i];
					_snprintf(buffer, 8192, "%s%f|%f|%f", (i>0 ? ", " : ""), Vec3.x, Vec3.y, Vec3.z);
					strout.add(buffer);
				}
				strout.add("};\r\n");
				} break;
			case DD_BOOL:
				_snprintf(buffer, 8192, "=(bool)%s;\r\n", ((CDDBool*)(pItem))->m_bValue ? "true" : "false");
				strout.add(buffer);				
				break;
			case DD_VEC2:
				{
				const CDDVec2* pVec2 = (const CDDVec2*)pItem;
				_snprintf(buffer, 8192, "=(vec2)%f|%f;\r\n", pVec2->m_Vec.x, pVec2->m_Vec.y);
				strout.add(buffer);	
				} break;
			case DD_VEC4:
				{
				const CDDVec4* pVec4 = (const CDDVec4*)pItem;
				_snprintf(buffer, 8192, "=(vec4)%f|%f|%f|%f;\r\n", pVec4->m_Vec.x, pVec4->m_Vec.y, pVec4->m_Vec.z, pVec4->m_Vec.w);
				strout.add(buffer);	
				} break;
			case DD_VEC4LIST:
				{				
				strout.add("=(vec4list) {");
				const CDDVec4List* pList = (const CDDVec4List*)pItem;
				for(int i = 0;i < pList->m_iCount; i++)
				{
					const CVec4& Vec4 = pList->m_pValues[i];
					_snprintf(buffer, 8192, "%s%f|%f|%f|%f", (i>0 ? ", " : ""), Vec4.x, Vec4.y, Vec4.z, Vec4.w);
					strout.add(buffer);
				}
				strout.add("};\r\n");
				} break;
			case DD_MATRIX:
				{
				const CDDMatrix* pMat = (const CDDMatrix*)pItem;
				_snprintf(buffer, 8192, "=(matrix)");
				const float* pMatFloats = (const float*)pMat->m_Matrix.mat;
				for(unsigned int x = 0; x < 16; x++) _snprintf(buffer + strlen(buffer), 8192 - strlen(buffer), "%s%f", x ? "|" : "", pMatFloats[x]);
				_snprintf(buffer + strlen(buffer), 8192 - strlen(buffer), ";\r\n");
				strout.add(buffer);	
				} break;
			case DD_MATRIXLIST:
				{				
				strout.add("=(matrixlist) {");
				const CDDMatrixList* pList = (const CDDMatrixList*)pItem;
				for(int i = 0;i < pList->m_iCount; i++)
				{
					const CMatrix& Mat = pList->m_pValues[i];
					_snprintf(buffer, 8192, "%s", (i>0 ? ", " : ""));
					const float* pMatFloats = (const float*)Mat.mat;
					for(unsigned int x = 0; x < 16; x++) _snprintf(buffer + strlen(buffer), 8192 - strlen(buffer), "%s%f", x ? "|" : "", pMatFloats[x]);
					strout.add(buffer);
				}
				strout.add("};\r\n");
				} break;
		}
	}
}

// Merge this DDObject and another one
void CDDObject::MergeWith(const CDDObject *pOther, bool bOverwrite)
{
	fastvector< fastmap< const CDDBase* >::SDataPair > lDataList=pOther->m_mData.iterate();

	// Copy data
	for(unsigned i=0;i<lDataList.size();i++)
	{
		if(lDataList[i].Data)
		{
			CDDBase *pClone=lDataList[i].Data->Clone();
			if(pClone) 
			{
				// Check if we want to overwrite
				if(bOverwrite)
				{
					const CDDBase *pOldItem=NULL;

					// This map function will overwrite, if data exists and pOldItem will be valid
					m_mData.map(lDataList[i].pszKey, pClone, pOldItem);
					if(pOldItem) pOldItem->Release();
					continue;
				}

				// If the item exists already, we skip the clone
				const CDDBase *pTemp;			
				if(m_mData.find(lDataList[i].pszKey, pTemp)) 
				{
					pClone->Release();
					continue;
				}

				// Insert item
				m_mData.map(lDataList[i].pszKey, pClone);
			}
		}
	}	
	m_bChanged=true;
}

// Make a new object, containing all changed or added keys,
// compared to pOther.
CDDObject *CDDObject::BuildChanges(const CDDObject *pOther) const
{
	CDDObject *pChanges=NULL; //new CDDObject;
	fastvector<const char *>	lKeys=m_mData.keys();
	for(unsigned iKey=0;iKey<lKeys.size();iKey++)
	{
		const CDDBase *pSrc, *pCompare=NULL;		
		m_mData.find(lKeys[iKey], pSrc);
		if(!pOther->m_mData.find(lKeys[iKey], pCompare) || pSrc->GetType()!=pCompare->GetType())
		{
			if(!pChanges) pChanges=new CDDObject;
			pChanges->m_mData.map(lKeys[iKey], pSrc->Clone());
			continue;
		}
		bool bChanged=false;
		switch(pSrc->GetType())
		{
			case DD_OBJECT:
				{
				// Compare object and insert compared object
				CDDObject *pComp=((CDDObject*)pSrc)->BuildChanges((CDDObject*)pCompare);
				if(pComp) 
				{
					if(!pChanges) pChanges=new CDDObject;
					pChanges->m_mData.map(lKeys[iKey], pComp);		
				}
				} break;
			case DD_OBJLIST:
				{
				fastvector<const CDDObject *> &lSrcList=((CDDObjectList*)pSrc)->m_lList;
				fastvector<const CDDObject *> &lDstList=((CDDObjectList*)pCompare)->m_lList;
				if(lSrcList.size()!=lDstList.size())
				{
					if(!pChanges) pChanges=new CDDObject;
					pChanges->m_mData.map(lKeys[iKey], pCompare);	
					pCompare->AddRef();
				} else
				{
					for(unsigned i=0;i<lSrcList.size();i++)
					{
						CDDObject *pComp=lSrcList[i]->BuildChanges(lDstList[i]);
						if(pComp)
						{
							pComp->Release();
							if(!pChanges) pChanges=new CDDObject;
							pChanges->m_mData.map(lKeys[iKey], pCompare);		
							pCompare->AddRef();
							break;
						}
					}
				}
				} break;
			case DD_INT:
				if(((CDDInt*)(pSrc))->m_iValue!=((CDDInt*)(pCompare))->m_iValue) bChanged=true;
				break;
			case DD_INTLIST:
				break;
			case DD_FLOAT:
				if(((CDDFloat*)(pSrc))->m_fValue!=((CDDFloat*)(pCompare))->m_fValue) bChanged=true;
				break;
			case DD_FLOATLIST:
				break;
			case DD_STRING:
				if(strcmp(((CDDString*)(pSrc))->m_sValue,((CDDString*)(pCompare))->m_sValue)!=0) bChanged=true;
				break;
			case DD_STRINGLIST:
				{
				CDDStringList *pSrcList=(CDDStringList*)pSrc;
				CDDStringList *pCmpList=(CDDStringList*)pCompare;
				if(pSrcList->m_Value.size()!=pCmpList->m_Value.size()) bChanged=true;
				else
				{
					for(unsigned iS=0;iS<pSrcList->m_Value.size();iS++)
						if(pSrcList->m_Value[iS].compare(pCmpList->m_Value[iS])!=0) 
						{
							bChanged=true;
							break;
						}
				}
				} break;
			case DD_BINARY:
				break;
			case DD_VEC3:
				{
				const CDDVec3* pVec3S = (const CDDVec3*)pSrc;
				const CDDVec3* pVec3C = (const CDDVec3*)pCompare;
				if(memcmp(&pVec3S->m_Vec, &pVec3C->m_Vec, sizeof(CVec3))) bChanged = true;
				} break;
			case DD_VEC3LIST:
				break;
			case DD_BOOL:
				if(((CDDBool*)(pSrc))->m_bValue != ((CDDBool*)(pCompare))->m_bValue) bChanged = true;
				break;
			case DD_VEC2:
				{
				const CDDVec2* pVec2S = (const CDDVec2*)pSrc;
				const CDDVec2* pVec2C = (const CDDVec2*)pCompare;
				if(memcmp(&pVec2S->m_Vec, &pVec2C->m_Vec, sizeof(CVec2))) bChanged = true;
				} break;
			case DD_VEC4:
				{
				const CDDVec4* pVec4S = (const CDDVec4*)pSrc;
				const CDDVec4* pVec4C = (const CDDVec4*)pCompare;
				if(memcmp(&pVec4S->m_Vec, &pVec4C->m_Vec, sizeof(CVec4))) bChanged = true;
				} break;
			case DD_MATRIX:
				{
				const CDDMatrix* pMatS = (const CDDMatrix*)pSrc;
				const CDDMatrix* pMatC = (const CDDMatrix*)pCompare;
				if(memcmp(&pMatS->m_Matrix, &pMatC->m_Matrix, sizeof(CMatrix))) bChanged = true;
				} break;
			case DD_MATRIXLIST:
				break;
		}
		if(bChanged)
		{
			if(!pChanges) pChanges=new CDDObject;
			pChanges->m_mData.map(lKeys[iKey], pSrc->Clone());		
		}
	}
	return pChanges;
}

// Create a clone of the object instance
CDDBase* CDDObject::Clone(void) const
{
	CDDObject *pClone=new CDDObject;
	fastvector< fastmap< const CDDBase* >::SDataPair > lDataList=m_mData.iterate();

	// Clone data
	for(unsigned i=0;i<lDataList.size();i++)
	{
		if(lDataList[i].Data)
		{
			pClone->m_mData.map(lDataList[i].pszKey, lDataList[i].Data->Clone());
		}
	}
	return pClone;
}

// Set notifier instance. Use this to get notified when object data changes
void CDDObject::SetNotifier(IDDNotify *pNotifier)
{
	m_pNotifier=pNotifier;
}

// Query Functions
// ---------------

// Retrieve a list of keys
fastvector<const char *> CDDObject::GetKeyNames(void) const
{
	return m_mData.keys();
}

// Find out which type of data a key holds (DDUnknown if not found)
EDDType CDDObject::GetKeyType(const char *pszID) const
{
	const CDDBase *pItem;
	if(m_mData.find(pszID,pItem))
	{
		return pItem->GetType();
	}
	return DD_UNKNOWN;
}

// Get integer data
int	CDDObject::GetInt(const char *pszID, int iDefValue) const
{
	const CDDBase *pItem;
	if(m_mData.find(pszID,pItem))
	{
#ifdef GET_CHECK_TYPE
		if(pItem->GetType()==DD_INT)
#endif
			return ((const CDDInt*)(pItem))->m_iValue;		
	}
	return iDefValue;
}
// Get a list of integers
int *CDDObject::GetIntList(const char *pszID, int &iElemCount) const
{
	const CDDBase *pItem;
	if(m_mData.find(pszID,pItem))
	{
#ifdef GET_CHECK_TYPE
		if(pItem->GetType()!=DD_INTLIST) return NULL;
#endif
		iElemCount=((const CDDIntList*)(pItem))->m_iCount;
		return ((const CDDIntList*)(pItem))->m_pValues;		
	}
	return NULL;
}

// Get floating point data
float CDDObject::GetFloat(const char *pszID, float fDefValue) const
{
	const CDDBase *pItem;
	if(m_mData.find(pszID,pItem))
	{
#ifdef GET_CHECK_TYPE
		if(pItem->GetType()==DD_FLOAT)
#endif
		return ((const CDDFloat*)(pItem))->m_fValue;		
	}
	return fDefValue;
}
// Get a list of floats
float *CDDObject::GetFloatList(const char *pszID, int &iElemCount) const
{
	const CDDBase *pItem;
	if(m_mData.find(pszID,pItem))
	{
#ifdef GET_CHECK_TYPE
		if(pItem->GetType()!=DD_FLOATLIST) return NULL;
#endif
		iElemCount=((const CDDFloatList*)(pItem))->m_iCount;
		return ((const CDDFloatList*)(pItem))->m_pValues;		
	}
	return NULL;
}

// Get string data
const char *CDDObject::GetString(const char *pszID, const char *pszDefault) const
{
	const CDDBase *pItem;
	if(m_mData.find(pszID,pItem))
	{
#ifdef GET_CHECK_TYPE
		if(pItem->GetType()==DD_STRING)
#endif
		return ((const CDDString*)(pItem))->m_sValue;		
	}
	return pszDefault;
}

// Get string data
vector<faststring> CDDObject::GetStringList(const char *pszID) const
{
	const CDDBase *pItem;
	if(m_mData.find(pszID,pItem))
	{
#ifdef GET_CHECK_TYPE
		if(pItem->GetType()==DD_STRINGLIST)
#endif
		return ((const CDDStringList*)(pItem))->m_Value;		
	}
	vector<faststring> lEmpty;
	return lEmpty;
}

// Get a DDObject
CDDObject* CDDObject::GetDDObject(const char *pszID) const
{
	const CDDBase *pItem;
	if(m_mData.find(pszID,pItem))
	{
#ifdef GET_CHECK_TYPE
		if(pItem->GetType()==DD_OBJECT)
#endif
		return (CDDObject*)(pItem);		
	}
	return NULL;
}

// Get a DDObject List
fastvector<const CDDObject*> CDDObject::GetDDList(const char *pszID) const
{
	fastvector<const CDDObject*> lEmpty;
	const CDDBase *pItem;
	if(m_mData.find(pszID,pItem))
	{
#ifdef GET_CHECK_TYPE
		if(pItem->GetType()!=DD_OBJLIST) return lEmpty;
#endif
		return ((const CDDObjectList*)(pItem))->m_lList;		
	}
	return lEmpty;
}

// Get binary data
const void *CDDObject::GetBinary(const char *pszID, int &iByteLength) const
{
	const CDDBase *pItem;
	if(m_mData.find(pszID,pItem))
	{
#ifdef GET_CHECK_TYPE
		if(pItem->GetType()!=DD_BINARY) return NULL;
#endif
		iByteLength=((const CDDBinary*)(pItem))->m_iSize;
		return ((const CDDBinary*)(pItem))->m_pData;		
	}
	return NULL;
}

// Get vec3 data
CVec3 CDDObject::GetVec3(const char *pszID, const CVec3& vDefValue) const
{
	const CDDBase *pItem;
	if(m_mData.find(pszID,pItem))
	{
#ifdef GET_CHECK_TYPE
		if(pItem->GetType()==DD_VEC3)
#endif
		return ((const CDDVec3*)(pItem))->m_Vec;
	}
	return vDefValue;
}
// Get a list of vec3's
CVec3* CDDObject::GetVec3List(const char *pszID, int &iElemCount) const
{
	const CDDBase *pItem;
	if(m_mData.find(pszID,pItem))
	{
#ifdef GET_CHECK_TYPE
		if(pItem->GetType()!=DD_VEC3LIST) return NULL;
#endif
		iElemCount=((const CDDVec3List*)(pItem))->m_iCount;
		return ((const CDDVec3List*)(pItem))->m_pValues;		
	}
	return NULL;
}

// Get boolean data
bool CDDObject::GetBool(const char *pszID, bool bDefValue) const
{
	const CDDBase *pItem;
	if(m_mData.find(pszID,pItem))
	{
#ifdef GET_CHECK_TYPE
		if(pItem->GetType()==DD_BOOL)
#endif
			return ((const CDDBool*)(pItem))->m_bValue;		
	}
	return bDefValue;
}

// Get vec2 data
CVec2 CDDObject::GetVec2(const char *pszID, const CVec2& vDefValue) const
{
	const CDDBase *pItem;
	if(m_mData.find(pszID,pItem))
	{
#ifdef GET_CHECK_TYPE
		if(pItem->GetType()==DD_VEC2)
#endif
		return ((const CDDVec2*)(pItem))->m_Vec;
	}
	return vDefValue;
}

// Get vec4 data
CVec4 CDDObject::GetVec4(const char *pszID, const CVec4& vDefValue) const
{
	const CDDBase *pItem;
	if(m_mData.find(pszID,pItem))
	{
#ifdef GET_CHECK_TYPE
		if(pItem->GetType()==DD_VEC4)
#endif
		return ((const CDDVec4*)(pItem))->m_Vec;
	}
	return vDefValue;
}
// Get a list of vec4's
CVec4* CDDObject::GetVec4List(const char *pszID, int &iElemCount) const
{
	const CDDBase *pItem;
	if(m_mData.find(pszID,pItem))
	{
#ifdef GET_CHECK_TYPE
		if(pItem->GetType()!=DD_VEC4LIST) return NULL;
#endif
		iElemCount=((const CDDVec4List*)(pItem))->m_iCount;
		return ((const CDDVec4List*)(pItem))->m_pValues;		
	}
	return NULL;
}

// Get matrix data
CMatrix CDDObject::GetMatrix(const char *pszID, const CMatrix& mDefValue) const
{
	const CDDBase *pItem;
	if(m_mData.find(pszID,pItem))
	{
#ifdef GET_CHECK_TYPE
		if(pItem->GetType()==DD_MATRIX)
#endif
		return ((const CDDMatrix*)(pItem))->m_Matrix;
	}
	return mDefValue;
}
// Get a list of matrices
CMatrix* CDDObject::GetMatrixList(const char *pszID, int &iElemCount) const
{
	const CDDBase *pItem;
	if(m_mData.find(pszID,pItem))
	{
#ifdef GET_CHECK_TYPE
		if(pItem->GetType()!=DD_MATRIXLIST) return NULL;
#endif
		iElemCount=((const CDDMatrixList*)(pItem))->m_iCount;
		return ((const CDDMatrixList*)(pItem))->m_pValues;		
	}
	return NULL;
}

// Data Store functions
// ---------------------------

// Store integer 	
void CDDObject::SetInt(const char *pszID, int iData)
{
	const CDDBase *pOld=NULL;
	m_mData.map(pszID, new CDDInt(iData), pOld);
	if(pOld) pOld->Release();
	m_bChanged=true;
	if(m_pNotifier) m_pNotifier->OnChanged(this, pszID);
}
// Set a list of integers
void CDDObject::SetIntList(const char *pszID, const int *pValues, int iCount)
{
	const CDDBase *pOld=NULL;
	m_mData.map(pszID, new CDDIntList(pValues, iCount), pOld);
	if(pOld) pOld->Release();
	m_bChanged=true;
	if(m_pNotifier) m_pNotifier->OnChanged(this, pszID);
}
void CDDObject::SetIntList(const char *pszID, fastvector<int> &lList)
{
	SetIntList(pszID, &lList[0], lList.size());
}

// Store float	
void CDDObject::SetFloat(const char *pszID, float fData)
{
	const CDDBase *pOld=NULL;
	m_mData.map(pszID, new CDDFloat(fData), pOld);
	if(pOld) pOld->Release();
	m_bChanged=true;
	if(m_pNotifier) m_pNotifier->OnChanged(this, pszID);
}
// Set a list of floats
void CDDObject::SetFloatList(const char *pszID, const float *pValues, int iCount)
{
	const CDDBase *pOld=NULL;
	m_mData.map(pszID, new CDDFloatList(pValues, iCount), pOld);
	if(pOld) pOld->Release();
	m_bChanged=true;
	if(m_pNotifier) m_pNotifier->OnChanged(this, pszID);
}
void CDDObject::SetFloatList(const char *pszID, fastvector<float> &lList)
{
	SetFloatList(pszID, &lList[0], lList.size());
}

// Store string	
void CDDObject::SetString(const char *pszID, const char *pszData)
{
	const CDDBase *pOld=NULL;
	m_mData.map(pszID, new CDDString(pszData), pOld);
	if(pOld) pOld->Release();
	m_bChanged=true;
	if(m_pNotifier) m_pNotifier->OnChanged(this, pszID);
}

// Store stringlist
void CDDObject::SetStringList(const char *pszID, vector<faststring> &lList)
{
	const CDDBase *pOld=NULL;
	m_mData.map(pszID, new CDDStringList(lList), pOld);
	if(pOld) pOld->Release();
	m_bChanged=true;
	if(m_pNotifier) m_pNotifier->OnChanged(this, pszID);
}

// Store DDObject
void CDDObject::SetDDObject(const char *pszID, const CDDObject *pObj)
{
	const CDDBase *pOld=NULL;
	pObj->AddRef();
	m_mData.map(pszID, pObj, pOld);
	if(pOld) pOld->Release();
	m_bChanged=true;
	if(m_pNotifier) m_pNotifier->OnChanged(this, pszID);
}

// Store DDObject List
void CDDObject::SetDDList(const char *pszID, fastvector<const CDDObject *> &lList, bool bAddRef)
{
	const CDDBase *pOld=NULL;
	m_mData.map(pszID, new CDDObjectList(lList, bAddRef), pOld);	
	if(pOld) pOld->Release();
	m_bChanged=true;
	if(m_pNotifier) m_pNotifier->OnChanged(this, pszID);
}

// Set a list of floats
void CDDObject::SetBinary(const char *pszID, const void *pData, unsigned iSize)
{
	const CDDBase *pOld=NULL;
	m_mData.map(pszID, new CDDBinary(pData, (int)iSize), pOld);
	if(pOld) pOld->Release();
	m_bChanged=true;
	if(m_pNotifier) m_pNotifier->OnChanged(this, pszID);
}

// Store vec3
void CDDObject::SetVec3(const char *pszID, const CVec3& vVec)
{
	const CDDBase *pOld=NULL;
	m_mData.map(pszID, new CDDVec3(vVec.x, vVec.y, vVec.z), pOld);
	if(pOld) pOld->Release();
	m_bChanged=true;
	if(m_pNotifier) m_pNotifier->OnChanged(this, pszID);
}
// Set a list of vec3's
void CDDObject::SetVec3List(const char *pszID, const CVec3* pValues, int iCount)
{
	const CDDBase *pOld=NULL;
	m_mData.map(pszID, new CDDVec3List(pValues, iCount), pOld);
	if(pOld) pOld->Release();
	m_bChanged=true;
	if(m_pNotifier) m_pNotifier->OnChanged(this, pszID);
}

// Store boolean
void CDDObject::SetBool(const char *pszID, bool bData)
{
	const CDDBase *pOld=NULL;
	m_mData.map(pszID, new CDDBool(bData), pOld);
	if(pOld) pOld->Release();
	m_bChanged=true;
	if(m_pNotifier) m_pNotifier->OnChanged(this, pszID);
}

// Store vec2
void CDDObject::SetVec2(const char *pszID, const CVec2& vVec)
{
	const CDDBase *pOld=NULL;
	m_mData.map(pszID, new CDDVec2(vVec.x, vVec.y), pOld);
	if(pOld) pOld->Release();
	m_bChanged=true;
	if(m_pNotifier) m_pNotifier->OnChanged(this, pszID);
}

// Store vec4
void CDDObject::SetVec4(const char *pszID, const CVec4& vVec)
{
	const CDDBase *pOld=NULL;
	m_mData.map(pszID, new CDDVec4(vVec.x, vVec.y, vVec.z, vVec.w), pOld);
	if(pOld) pOld->Release();
	m_bChanged=true;
	if(m_pNotifier) m_pNotifier->OnChanged(this, pszID);
}
// Set a list of vec4's
void CDDObject::SetVec4List(const char *pszID, const CVec4* pValues, int iCount)
{
	const CDDBase *pOld=NULL;
	m_mData.map(pszID, new CDDVec4List(pValues, iCount), pOld);
	if(pOld) pOld->Release();
	m_bChanged=true;
	if(m_pNotifier) m_pNotifier->OnChanged(this, pszID);
}

// Store matrix
void CDDObject::SetMatrix(const char *pszID, const CMatrix& mMat)
{
	const CDDBase *pOld=NULL;
	m_mData.map(pszID, new CDDMatrix((const float*)mMat.mat), pOld);
	if(pOld) pOld->Release();
	m_bChanged=true;
	if(m_pNotifier) m_pNotifier->OnChanged(this, pszID);
}
// Set a list of matrices
void CDDObject::SetMatrixList(const char *pszID, const CMatrix* pValues, int iCount)
{
	const CDDBase *pOld=NULL;
	m_mData.map(pszID, new CDDMatrixList(pValues, iCount), pOld);
	if(pOld) pOld->Release();
	m_bChanged=true;
	if(m_pNotifier) m_pNotifier->OnChanged(this, pszID);
}

