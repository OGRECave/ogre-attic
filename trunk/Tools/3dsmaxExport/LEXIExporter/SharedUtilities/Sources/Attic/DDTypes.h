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
