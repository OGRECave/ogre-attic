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

	// Check if object has been changed
	bool	HasChanged(bool bResetChange);

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
	void	SetNotifier(IDDNotify *pNotifier);

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

	// 
	IDDNotify *m_pNotifier;

	// Map of data
	fastmap< const CDDBase * >	m_mData;
};

//typedef CRefCountPtr<CDDObject> CDDObjectRef;

#endif
