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

#include "LexiStdAfx.h"
#include "LexiDialogObjectProperties.h"
#include "LexiExportObjectMesh.h"
#include "LexiOgreMeshCompiler.h"
#include "LexiOgreMaterialCompiler.h"
#include "LexiIntermediateBuilder.h"
#include "LexiIntermediateMesh.h"
#include "LexiDialogSelectNode.h"

#include <dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")

CObjectPropertiesDlg *CMeshExportObject::m_pEditDlg=NULL;
CDDObject* CMeshExportObject::m_pDDMetaDesc=NULL;
//

CMeshExportObject::CMeshExportObject(CDDObject *pConfig) : CExportObject(pConfig)
{
/*	m_iID = 0xffffffff;
	m_sName = "<unnamed>";
	m_sFilename = "<unknown>";
	m_pSceneNode = NULL;
	m_pDDMetaDesc = BuildMetaDesc();	*/	
	//m_pDDMetaDesc = BuildMetaDesc();	
}

CMeshExportObject::~CMeshExportObject()
{
//	if(m_pDDMetaDesc) m_pDDMetaDesc->Release();
	// Free scene node - no longer needed
//	if(m_pSceneNode) delete m_pSceneNode;
}

// Check if ExportObject supports a given ExportObject instance as parent
bool CMeshExportObject::SupportsParentType(const CExportObject *pParent) const
{	
	// Meshes can only have root as parent
	if(pParent==NULL || stricmp(pParent->GetType(), "root")!=0) return false;
	return true;
}

//
bool CMeshExportObject::SupportsMAXNode(INode *pMAXNode) const
{
	if(pMAXNode==NULL) return false;
	SClass_ID nClass = GetClassIDFromNode(pMAXNode);
	if(nClass == GEOMOBJECT_CLASS_ID) return true;
	return false;
}

// Get window for editing ExportObject properties
GDI::Window* CMeshExportObject::GetEditWindow(GDI::Window *pParent)
{
	if(m_pEditDlg==NULL)
	{
		m_pEditDlg=new CObjectPropertiesDlg(pParent);
		m_pEditDlg->Create();
		m_pDDMetaDesc=BuildMetaDesc();
		m_pEditDlg->Init(m_pDDMetaDesc, ".mesh");	
	}

	m_pEditDlg->SetInstance(m_pDDConfig, this);
	//LOGDEBUG "Window Created:\t%i (handle)", m_pEditDlg->m_hWnd);
	return m_pEditDlg;
}

// End edit
void CMeshExportObject::CloseEditWindow()
{
	if(m_pEditDlg!=NULL)
	{
		//LOGDEBUG "Window Ended:\t%i (handle)", m_pEditDlg->m_hWnd);
		m_pEditDlg->EndDialog(0);
		delete m_pEditDlg;		
		m_pEditDlg=NULL;
		if(m_pDDMetaDesc) 
		{
			m_pDDMetaDesc->Release();
			m_pDDMetaDesc=NULL;
		}
	}
}

// Called when object is first created [by user].
// This allows for wizard-style editing of required data
// If this function returns false, the object is not created
bool CMeshExportObject::OnCreate(CExporterPropertiesDlg *pPropDialog)
{
	if(GetMAXNodeID()==0xFFFFFFFF)
	{
		CSelectNodeDlg dlg((GDI::Window*)pPropDialog, this);
		if(dlg.DoModal() != IDOK) return false;	// user must select a valid node
	}
	
	// Initialize mesh name
	std::string sName = GetNameFromID(m_pDDConfig->GetInt("NodeID"));
	RemoveIllegalChars(sName);
	m_pDDConfig->SetString("Name", sName.c_str());

	// Initialize filename
	std::string sTemp = GetNameFromID(m_pDDConfig->GetInt("NodeID"));
	sTemp += ".mesh";	
	RemoveIllegalChars(sTemp);
	sTemp = "C:\\" + sTemp;		
	m_pDDConfig->SetString("FileName", sTemp.c_str());
	return true;
}
/*
//

void CMeshExportObject::Read(CDDObject* pConfig)
{
	m_iID = pConfig->GetInt("id", 0xffffffff);
	m_sName = pConfig->GetString("name", "<unnamed>");
	m_sFilename = pConfig->GetString("filename", "<unknown>");
	CExportObject::Read(pConfig);
}

void CMeshExportObject::Write(CDDObject* pConfig) const
{
	CExportObject::Write(pConfig);
}

//

CDDObject* CMeshExportObject::GetMetaDesc() const
{
	return m_pDDMetaDesc;
}

CDDObject* CMeshExportObject::GetEditMeta() const
{
	return m_pConfig;
}*/

CDDObject* CMeshExportObject::BuildMetaDesc( void )
{
	CDDObject* pDDMetaDesc = new CDDObject();

	fastvector< const CDDObject* > lSettings;
	CDDObject* pDDMetaElement;

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID","normalsID");
	pDDMetaElement->SetString("Type","bool");
	pDDMetaElement->SetString("Group","Export Settings");
	pDDMetaElement->SetString("Caption","Vertex Normals");
	pDDMetaElement->SetString("Help","Export vertex normals");
	//pDDMetaElement->SetBool("normalsID",false);
	pDDMetaElement->SetBool("Default", true);
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID","vertexColorsID");
	pDDMetaElement->SetString("Type","bool");
	pDDMetaElement->SetString("Group","Export Settings");
	pDDMetaElement->SetString("Caption","Vertex Colors");
	pDDMetaElement->SetString("Help","Export baked vertex colors");
	//pDDMetaElement->SetBool("vertexColorsID",false);
	pDDMetaElement->SetBool("Default", true);
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID","uvID");
	pDDMetaElement->SetString("Type","bool");
	pDDMetaElement->SetString("Group","Export Settings");
	pDDMetaElement->SetString("Caption","Texture Coordinates");
	pDDMetaElement->SetString("Help","Export texture vertex coordinates");
	//pDDMetaElement->SetBool("uvID",false);
	pDDMetaElement->SetBool("Default", true);
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID","reindexID");
	pDDMetaElement->SetString("Type","bool");
	pDDMetaElement->SetString("Group","Export Settings");
	pDDMetaElement->SetString("Caption","Reindex Vertices");
	pDDMetaElement->SetString("Help","Optimize Index and Vertex buffer");
	//pDDMetaElement->SetBool("reindexID",false);
	pDDMetaElement->SetBool("Default", true);
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID","collapseHierarchy");
	pDDMetaElement->SetString("Type","bool");
	pDDMetaElement->SetString("Group","Export Settings");
	pDDMetaElement->SetString("Caption","Collapse Hierarchy");
	pDDMetaElement->SetString("Help","Collapse entire hierarchy into one mesh");
	pDDMetaElement->SetBool("Default", false);
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID","copyTextureMaps");
	pDDMetaElement->SetString("Type","bool");
	pDDMetaElement->SetString("Group","Resources");
	pDDMetaElement->SetString("Caption","Copy Textures");
	pDDMetaElement->SetString("Help","Copy Textures To Output Folder");
	pDDMetaElement->SetBool("Default", true);
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID","copyShaders");
	pDDMetaElement->SetString("Type","bool");
	pDDMetaElement->SetString("Group","Resources");
	pDDMetaElement->SetString("Caption","Copy Shaders");
	pDDMetaElement->SetString("Help","Copy Shaders To Output Folder");
	pDDMetaElement->SetBool("Default", true);
	lSettings.push_back(pDDMetaElement);

	pDDMetaDesc->SetDDList("MetaList", lSettings, false);

	return pDDMetaDesc;
}

//
typedef std::map< Ogre::String, CIntermediateMaterial*> MAT_LIST;

bool CMeshExportObject::Export(CExportProgressDlg *pProgressDlg, bool bForceAll) const
{
	bool returnVal = false;
	Ogre::String sFilename;
	CIntermediateMesh* pIMesh = NULL;
	COgreMeshCompiler* pOgreMeshCompiler = NULL;

	if(m_bEnabled || bForceAll)
	{
	try 
	{
		// We have two step to begin with
		pProgressDlg->InitLocal(2);

		LOGDEBUG "Starting...");
		CIntermediateBuilder::Get()->Clear();
		CIntermediateBuilder::Get()->SetConfig(m_pDDConfig);

		// BREAK UP THIS METHOD INTO SMALLER FUNCTION CALLS

		// SPLIT THIS CLASS INTO A MESH AND SKINNED MESH.

		pProgressDlg->LocalStep("StaticMesh: Reading max data");
		LOGDEBUG "Creating Hierarchy...");

		Ogre::SceneNode* pNode = CIntermediateBuilder::Get()->CreateHierarchy(GetMAXNodeID(), true, false);
		if(pNode == NULL)
		{
			LOGERROR "No node with such ID: %d", GetMAXNodeID());
			return false;
		}

		LOGDEBUG "Reading config ...");
		bool bCollaps = m_pDDConfig->GetBool("collapseHierarchy", false);
		bool bCopyTextures = m_pDDConfig->GetBool("copyTextureMaps", false);
		bool bCopyShaders = m_pDDConfig->GetBool("copyShaders", false);

		sFilename = m_pDDConfig->GetString("FileName");

		///// BELONGS IN A STATIC HIERARCHY TYPE
		if(bCollaps)
		{
			pProgressDlg->LocalStep("StaticMesh: Collapsing Mesh");
			std::list<std::string> clist;
			clist.push_back("position");
			clist.push_back("normal");
			clist.push_back("uv1");
			Ogre::SceneNode* pColNode = CIntermediateBuilder::Get()->CollapseHierarchy(pNode, clist, "Collapsed");
			delete pNode;
			pNode = pColNode;
		}
		else
		{
			pProgressDlg->LocalStep();
		}


		pIMesh = (CIntermediateMesh*)pNode->getAttachedObject(0);

		LOGINFO "Exporting mesh: %s", pIMesh->getName().c_str());

		returnVal = CExportObject::Export(pProgressDlg, bForceAll);	


		// We have some remaining steps

		bool bInOneFile = true;
		MAT_LIST lMaterials;
		bool bMatRetrieved = CIntermediateBuilder::Get()->GetMaterials(lMaterials);

		pProgressDlg->InitLocal(3+(lMaterials.size()));
		pProgressDlg->LocalStep("StaticMesh: Creating Ogre Mesh..");
		pOgreMeshCompiler = new COgreMeshCompiler(pIMesh, m_pDDConfig, sFilename);

		Ogre::String sBaseName;
		Ogre::String sPath;
		Ogre::StringUtil::splitFilename(sFilename, sBaseName, sPath);
		
		int n = sPath.find("/");
		while(n != Ogre::String::npos)
		{
			sPath.replace(n,1,"\\");
			sPath.insert(n,"\\");
			n = sPath.find("/");
		}

		if(!::MakeSureDirectoryPathExists(sPath.c_str()))
		{
			LOGERROR "Error while attempting to create path: %s", sPath.c_str());
			delete pOgreMeshCompiler;
			delete pIMesh;
			return false;
		}

		pProgressDlg->LocalStep("StaticMesh: Writing Ogre Mesh..");
		LOGINFO "Writing Ogre Mesh (%s) ...", sFilename.c_str());
		pOgreMeshCompiler->WriteOgreMesh(sFilename);

		pProgressDlg->LocalStep("StaticMesh: Exporting Ogre Materials..");
		LOGINFO "Exporting material(s) ...");
		if ( bMatRetrieved )
		{
			Ogre::MaterialSerializer matWriter;
			MAT_LIST::iterator it = lMaterials.begin();

			while (it != lMaterials.end())
			{
				Ogre::String matDesc("StaticMesh: Exporting Ogre Material: ");
				matDesc += it->second->GetName();
				pProgressDlg->LocalStep(matDesc.c_str());
				COgreMaterialCompiler matComp( it->second );
				if(bInOneFile)
				{
					LOGDEBUG "Queueing material ...");
					matWriter.queueForExport( matComp.GetOgreMaterial() );
				}
				else
				{
					LOGDEBUG "Exporting individual material ...");
					try
					{
						matWriter.exportMaterial( matComp.GetOgreMaterial(), Ogre::String(sFilename+".material"));
					} catch (Ogre::Exception e) 
					{
						LOGERROR "OgreExeception caught: %s", e.getDescription().c_str()); 
					}
				}

				if(bCopyTextures) matComp.CopyTextureMaps( sPath );
				if(bCopyShaders) matComp.CopyShaderSources( sPath );
				it++;
			}
			try	
			{
				LOGDEBUG "Exporting Global material file...");
				matWriter.exportQueued( Ogre::String(sFilename+".material") );
			} catch (Ogre::Exception e) 
			{
				LOGERROR "OgreExeception caught: %s",  e.getDescription().c_str()); 
			}
		}



	} catch(Ogre::Exception e)
	{
		LOGERROR "OgreException caught: %s", e.getDescription().c_str());
	} catch(...)
	{
		LOGERROR "Caught unhandled exception in CMeshExportObject::Export()");
	}
	}


	LOGDEBUG "Cleaning up...");
	delete pOgreMeshCompiler;
	delete pIMesh;

	LOGINFO "..Done!");

	return returnVal;

}

