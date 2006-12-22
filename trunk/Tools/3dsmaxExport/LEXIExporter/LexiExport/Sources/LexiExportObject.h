/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Mark Folkenberg,
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

#ifndef __NDS_LexiExporter_ExportObject__
#define __NDS_LexiExporter_ExportObject__

//

#define DECLARE_EXPORT_OBJECT(classname, type, typname, icon) \
static CExportObject* _construct##classname() { return new classname; } \
static bool bDummyExp##classname = CExportObject::RegisterObject(type, typname, icon, _construct##classname); \
static classname* dummyGetPtrToClassName_##classname() { bool bDummy = bDummyExp##classname; return NULL; }

//

class CExportObject {

	friend class CExporter;
	friend class CExportProgressDlg;

	public:

		typedef struct {

			const char* m_pszType;
			const char* m_pszTypeName;
			HICON m_hIcon;

		} Desc;

	public:

		typedef CExportObject* (*Construct_FN)();

		static void Initialize();

		static bool RegisterObject(const char* pszType, const char* pszTypeName, unsigned int iIcon, Construct_FN pfnConstruct);
		static CExportObject* Construct(const char* pszType);
		static void EnumObjects(std::vector<Desc>& objlist);

	protected:

		std::string m_sType;

		static CExportProgressDlg* m_pExportProgressDlg;

		Ogre::SceneNode* m_pSceneNode;

		CDDObject* m_pDDEditMeta;
		CDDObject* m_pDDMetaDesc;

		//

		unsigned int m_iID;
		std::string m_sName;
		std::string m_sFilename;

	public:

		// Constructor/Destructor
		CExportObject(const char* pszType);
		virtual ~CExportObject();

		// Release object
		void Release();

		// Create scene node
		bool CreateSceneNode();

		// Read/Write
		virtual void Read(CDDObject* pConfig);
		virtual void Write(CDDObject* pConfig) const;

		// Type
		const char* GetType() const;
		const char* GetTypeName() const;

		// ID
		void SetID(unsigned int iID);
		unsigned int GetID() const;

		// Name
		void SetName(const char* pszName);
		const char* GetName() const;

		// Filename
		void SetFilename(const char* pszFilename);
		const char* GetFilename() const;

		// Edit object
		bool Edit(GDI::Window* pParent, const char* pszTitle, unsigned int iInitSelectedID);

		// Supports node class
		virtual bool SupportsClass(SClass_ID nClass) const = 0;

		// Get meta description
		virtual CDDObject* GetMetaDesc() const = 0;

		// Get meta edits
		virtual CDDObject* GetEditMeta() const = 0;

		// Export object
		virtual bool Export() const = 0;

		// Output export info, warnings, errors, etc.
		void OutputProgress(const char* pszText, unsigned int iLevel) const;

		// Default file extension
		virtual const char* GetDefaultFileExt() const;

};

typedef std::vector<CExportObject*> ExportObjectList;

//

const char* GetNameFromID(unsigned int iID);
INode* GetNodeFromID(unsigned int iID);
SClass_ID GetClassIDFromNodeID(unsigned int iID);
SClass_ID GetClassIDFromNode(INode* pNode);

//

#endif // __NDS_LexiExporter_ExportObject__