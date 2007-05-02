/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Mark Folkenberg,
Bo Krohn
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

#ifndef __NDS_LexiExporter_ExportObject__
#define __NDS_LexiExporter_ExportObject__

//
#define DECLARE_EXPORT_OBJECT(classname, type, typname, icon) \
static CExportObject* _construct##classname(CDDObject *pConfig) { return new classname(pConfig); } \
static bool bDummyExp##classname = CExportObject::RegisterObject(type, typname, icon, _construct##classname); \
static classname* dummyGetPtrToClassName_##classname() { bool bDummy = bDummyExp##classname; return NULL; }

//
class CExporterPropertiesDlg;

//
// Base class for all export objects. Derive from this class to implement a new export
// object type.
class CExportObject 
{
public:
	// Constructor will setup m_pDDConfig and create children from DDList 'Children'
	CExportObject(CDDObject *pConfig);
	virtual ~CExportObject();

	// Release object
	void	Release();

	// Save configuration. 
	// Base implementation will store contents of m_pDDConfig and traverse children
	// generating DDList called "Children".
	virtual void	SaveConfig(CDDObject *pOutput) const;

	// Type
	const char* GetType() const;
	const char* GetTypeName() const;	
	const char* GetName() const;

	// Get additional description string
	virtual const char* GetDesc() const;

	// Enable/disable object during export (childs may still be exported)
	void	SetEnabled(bool bEnabled);
	bool	GetEnabled() const;

	// Called when object is first created [by user].
	// This allows for wizard-style editing of required data.
	// If this function returns false, the object is not created
	virtual bool OnCreate(CExporterPropertiesDlg *pPropDialog)=0;

public:
	// Get window for editing ExportObject properties
	virtual GDI::Window* GetEditWindow(GDI::Window *pParent) =0;

	// Close currently open edit window.
	// NOTE: If two ExportObject instances return the same window handle
	// in GetEditWindow(), it will not be closed.
	virtual void CloseEditWindow() =0;	

public:
	// Supports node class. Default implementation just returns false.
	virtual bool SupportsMAXNode(INode *pMAXNode) const;

	// Get/Set selected MAX node. Default implementation read/writes it from
	// the m_pDDConfig object on a key called "NodeID"
	virtual void SetMAXNodeID(unsigned int iMAXNodeID);
	virtual unsigned int GetMAXNodeID() const;

public:
	// Check if ExportObject supports a given ExportObject instance/type as parent
	virtual bool SupportsParentType(const CExportObject *pParent) const =0;

	// Set new parent object. This will automatically add current instance
	// as child on the parent and remove it from the old parent (if available)
	void	SetParent(CExportObject *pParent);
	CExportObject* GetParent() const;

public:
	// Add child to this instance
	void	AddChild(CExportObject *pChild);

	// Remove child from instance.
	void	RemoveChild(CExportObject *pChild);

	// Check if ExportObject has any children
	bool	HasChildren() const;

	// Get number of children - optionally recurse to count all subchildren
	unsigned int GetChildCount(bool bRecursive);

	// Get list of children attached to current instance
	std::vector<CExportObject*> GetChildren() const;

	// Export object and child object(s) if any. If bForceAll is true, ExportObjects
	// are exported even though they are not enabled.
	// NOTE: Base implementation calls Export() on all children and increments GlobalStep on
	// supplied Progress Dialog.
	virtual bool Export(CExportProgressDlg *pProgressDlg, bool bForceAll);

public:
	//
	// Static functions for global register/enum/construct of ExportObjects
	//
	typedef struct {

		const char* m_pszType;
		const char* m_pszTypeName;
		HICON m_hIcon;

	} Desc;

	typedef CExportObject* (*Construct_FN)(CDDObject *pConfig);

	// Initialize ExportObject base
	static void Initialize();

	// Register ExportObject
	static bool RegisterObject(const char* pszType, const char* pszTypeName, unsigned int iIcon, Construct_FN pfnConstruct);

	// ExportObject factory function
	static CExportObject* Construct(CDDObject *pConfig);

	// Enumerate all registered ExportObjects
	static void EnumObjects(std::vector<Desc>& objlist);

	// Get config
	CDDObject* GetConfig() const;

protected:
	std::string m_sType;
	std::string m_sDesc;
//	std::string	m_sName;

//	Ogre::SceneNode* m_pSceneNode;

	CDDObject* m_pDDConfig;	
	bool	m_bEnabled;

private:
	CExportObject	*m_pParent;
	std::vector<CExportObject*> m_lChildren;
};

//
const char* GetNameFromID(unsigned int iID);
INode* GetNodeFromID(unsigned int iID);
SClass_ID GetClassIDFromNodeID(unsigned int iID);
SClass_ID GetClassIDFromNode(INode* pNode);

//

#endif // __NDS_LexiExporter_ExportObject__