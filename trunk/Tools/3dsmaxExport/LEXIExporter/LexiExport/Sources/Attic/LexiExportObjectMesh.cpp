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
	REGISTER_MODULE("Mesh Export Object")
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
	UNREGISTER_MODULE
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
//	sTemp = "C:\\" + sTemp;		
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
	pDDMetaElement->SetString("ID","useSharedGeometryID");
	pDDMetaElement->SetString("Type","bool");
	pDDMetaElement->SetString("Group","Export Settings");
	pDDMetaElement->SetString("Caption","Shared Geometry");
	pDDMetaElement->SetString("Help","Export vertex data into one shared buffer. Otherwise keep a dedicated vertex buffer for each submesh. (If you plan to do hardware optimized pose blending ensure this option if set to off)");
	pDDMetaElement->SetBool("Default", false);
	lSettings.push_back(pDDMetaElement);

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
	pDDMetaElement->SetString("ID","exportEdgesID");
	pDDMetaElement->SetString("Type","bool");
	pDDMetaElement->SetString("Group","Export Settings");
	pDDMetaElement->SetString("Caption","Edges");
	pDDMetaElement->SetString("Help","Generate and export edges. This is usually used with shadow techniques.");
	pDDMetaElement->SetBool("Default", false);
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID","exportTangentsID");
	pDDMetaElement->SetString("Type","bool");
	pDDMetaElement->SetString("Group","Export Settings");
	pDDMetaElement->SetString("Caption","Tangents");
	pDDMetaElement->SetString("Help","Generate and export tangents. Check the Log to see which uv set it is written to.");
	pDDMetaElement->SetString("Condition", "$uvID==true && $normalsID==true");
	pDDMetaElement->SetBool("Default", false);
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
	pDDMetaElement->SetString("ID","XmlID");
	pDDMetaElement->SetString("Type","bool");
	pDDMetaElement->SetString("Group","Export Settings");
	pDDMetaElement->SetString("Caption","Export as XML");
	pDDMetaElement->SetString("Help","Export into a human readable XML file. (Filename will be have .xml extension)");
	pDDMetaElement->SetBool("Default", false);
	lSettings.push_back(pDDMetaElement);

	//pDDMetaElement = new CDDObject();
	//pDDMetaElement->SetString("ID","collapseHierarchy");
	//pDDMetaElement->SetString("Type","bool");
	//pDDMetaElement->SetString("Group","Export Settings");
	//pDDMetaElement->SetString("Caption","Collapse Hierarchy");
	//pDDMetaElement->SetString("Help","Collapse entire hierarchy into one mesh");
	//pDDMetaElement->SetBool("Default", false);
	//lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID","exportMaterialsID");
	pDDMetaElement->SetString("Type","bool");
	pDDMetaElement->SetString("Group","Export Settings");
	pDDMetaElement->SetString("Caption","Export Materials");
	pDDMetaElement->SetString("Help","Write Ogre3D materials files.");
	pDDMetaElement->SetBool("Default", true);
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID","exportSingleMaterialFileID");
	pDDMetaElement->SetString("Type","bool");
	pDDMetaElement->SetString("Group","Resources");
	pDDMetaElement->SetString("Caption","Export Single Material File");
	pDDMetaElement->SetString("Help","Write Ogre3D materials in a single file. Warning! This might lead to duplicate material definitions when exporting several meshes.");
	pDDMetaElement->SetString("Condition", "$exportMaterialsID==true");
	pDDMetaElement->SetBool("Default", false);
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID","copyTextureMaps");
	pDDMetaElement->SetString("Type","bool");
	pDDMetaElement->SetString("Group","Resources");
	pDDMetaElement->SetString("Caption","Copy Textures");
	pDDMetaElement->SetString("Help","Copy Textures To Output Folder");
	pDDMetaElement->SetString("Condition", "$exportMaterialsID==true");
	pDDMetaElement->SetBool("Default", true);
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID","copyShaders");
	pDDMetaElement->SetString("Type","bool");
	pDDMetaElement->SetString("Group","Resources");
	pDDMetaElement->SetString("Caption","Copy Shaders");
	pDDMetaElement->SetString("Help","Copy Shaders To Output Folder");
	pDDMetaElement->SetString("Condition", "$exportMaterialsID==true");
	pDDMetaElement->SetBool("Default", true);
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID","overrideExtensionID");
	pDDMetaElement->SetString("Type","bool");
	pDDMetaElement->SetString("Group","Resources");
	pDDMetaElement->SetString("Caption","Convert Textures while copying.");
	pDDMetaElement->SetString("Help","Convert Referenced Texture Maps to a specific format, e.g. .dds.");
	pDDMetaElement->SetString("Condition", "$copyTextureMaps==true && $exportMaterialsID==true");
	pDDMetaElement->SetBool("Default", false);
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID","extensionID");
	pDDMetaElement->SetString("Type","selection");
	std::vector<faststring> lExtensions;
	lExtensions.push_back(".dds");
	lExtensions.push_back(".png");
	lExtensions.push_back(".tga");
	lExtensions.push_back(".bmp");
	lExtensions.push_back(".jpg");
	lExtensions.push_back(".gif");
	pDDMetaElement->SetStringList("Strings",lExtensions);
	pDDMetaElement->SetString("Group","Resources");
	pDDMetaElement->SetString("Caption","Out Format");
	pDDMetaElement->SetString("Help","Format to which the textures will be converted.");
	pDDMetaElement->SetString("Condition", "$overrideExtensionID==true && $copyTextureMaps==true && $exportMaterialsID==true");
	pDDMetaElement->SetString("Default",".dds");
	lSettings.push_back(pDDMetaElement);
			

	pDDMetaDesc->SetDDList("MetaList", lSettings, false);

	return pDDMetaDesc;
}

//
typedef std::map< Ogre::String, CIntermediateMaterial*> MAT_LIST;

bool CMeshExportObject::Export(CExportProgressDlg *pProgressDlg, bool bForceAll)
{
	START_PROFILE("CMeshExportObject::Export()");
	bool returnVal = false;
	COgreMeshCompiler* pOgreMeshCompiler = NULL;
	Ogre::SceneNode* pNode = NULL;

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

			/////////////////////////////////////
			//MessageBox(NULL, "PreIM","BREAK!",0);
			/////////////////////////////////////


			pNode = CIntermediateBuilder::Get()->CreateHierarchy(GetMAXNodeID(), false, false);
			if(pNode == NULL)
			{
				LOGERROR "ERROR during export. Export Aborted!");
				return false;
			}

			LOGDEBUG "Reading config ...");
			bool bCollaps = m_pDDConfig->GetBool("collapseHierarchy", false);
			bool bCopyTextures = m_pDDConfig->GetBool("copyTextureMaps", false);
			bool bCopyShaders = m_pDDConfig->GetBool("copyShaders", false);
			bool bExportMaterials = m_pDDConfig->GetBool("exportMaterialsID",true);
			bool bInOneFile = m_pDDConfig->GetBool("exportSingleMaterialFileID",false);
			bool bXMLexport = m_pDDConfig->GetBool("XmlID",false);
			bool bExportColours = m_pDDConfig->GetBool("vertexColorsID", false);
			
			Ogre::String sExtension = "";
			bool bOverrideExtension = m_pDDConfig->GetBool("overrideExtensionID",false);
			if(bOverrideExtension)
			{
				std::vector<faststring> lExtensions;
				lExtensions.push_back(".dds");
				lExtensions.push_back(".png");
				lExtensions.push_back(".tga");
				lExtensions.push_back(".bmp");
				lExtensions.push_back(".jpg");
				lExtensions.push_back(".gif");
				int index = m_pDDConfig->GetInt("extensionID");
				sExtension = lExtensions.at(index).c_str();
			}
			//if(bCollaps)
			//{
			//	pProgressDlg->LocalStep("StaticMesh: Collapsing Mesh");
			//	std::list<std::string> clist;
			//	clist.push_back("position");
			//	clist.push_back("normal");
			//	clist.push_back("uv1");
			//	Ogre::SceneNode* pColNode = CIntermediateBuilder::Get()->CollapseHierarchy(pNode, clist, "Collapsed");
			//	CIntermediateBuilder::Get()->CleanUpHierarchy( pNode );
			//	pNode = pColNode;
			//}
			//else
			//{
				pProgressDlg->LocalStep();
			//}


			m_pIMesh = (CIntermediateMesh*)pNode->getAttachedObject(0);

			LOGINFO "Exporting mesh: %s", m_pIMesh->getName().c_str());

			returnVal = CExportObject::Export(pProgressDlg, bForceAll);	


			// We have some remaining steps

			//bool bInOneFile = false;//true;
			MAT_LIST lMaterials;
			bool bMatRetrieved = CIntermediateBuilder::Get()->GetMaterials(lMaterials);

			pProgressDlg->InitLocal(3+(lMaterials.size()));
			pProgressDlg->LocalStep("StaticMesh: Creating Ogre Mesh..");

			/////////////////////////////////////
			//MessageBox(NULL, "PreMeshCompiler","BREAK!",0);
			/////////////////////////////////////

			pOgreMeshCompiler = new COgreMeshCompiler(m_pIMesh, m_pDDConfig, pProgressDlg);

			const Ogre::String& sOrigFilename = m_pDDConfig->GetString("FileName");
			const Ogre::String& sMeshFilename = FixupFilename(sOrigFilename.c_str(), "mesh");
			const Ogre::String& sMaterialFilename = FixupFilename(sOrigFilename.c_str(), "material");
			const Ogre::String& sTextureFilename = FixupFilename(sOrigFilename.c_str(), "texture");
			const Ogre::String& sShaderFilename = FixupFilename(sOrigFilename.c_str(), "shader");

			Ogre::String sBaseName;
			Ogre::String sMeshPath;
			Ogre::String sMaterialPath;
			Ogre::String sTexturePath;
			Ogre::String sShaderPath;

			Ogre::StringUtil::splitFilename(sMeshFilename, sBaseName, sMeshPath);
			int n = sMeshPath.find("/");
			while(n != Ogre::String::npos)
			{
				sMeshPath.replace(n,1,"\\");
				n = sMeshPath.find("/");
			}

			Ogre::StringUtil::splitFilename(sTextureFilename, sBaseName, sTexturePath);
			n = sTexturePath.find("/");
			while(n != Ogre::String::npos)
			{
				sTexturePath.replace(n,1,"\\");
				n = sTexturePath.find("/");
			}

			Ogre::StringUtil::splitFilename(sMaterialFilename, sBaseName, sMaterialPath);
			n = sMaterialPath.find("/");
			while(n != Ogre::String::npos)
			{
				sMaterialPath.replace(n,1,"\\");
				n = sMaterialPath.find("/");
			}

			Ogre::StringUtil::splitFilename(sShaderFilename, sBaseName, sShaderPath);
			n = sShaderPath.find("/");
			while(n != Ogre::String::npos)
			{
				sShaderPath.replace(n,1,"\\");
				n = sShaderPath.find("/");
			}

			if(!::MakeSureDirectoryPathExists(sMeshPath.c_str()))
			{
				LOGERROR "Error while attempting to create path: %s", sMeshPath.c_str());
				delete pOgreMeshCompiler;
				CIntermediateBuilder::Get()->CleanUpHierarchy( pNode );
				CIntermediateBuilder::Get()->Clear();
				return false;
			}

			if(!::MakeSureDirectoryPathExists(sMaterialPath.c_str()))
			{
				LOGERROR "Error while attempting to create path: %s", sMaterialPath.c_str());
				delete pOgreMeshCompiler;
				CIntermediateBuilder::Get()->CleanUpHierarchy( pNode );
				CIntermediateBuilder::Get()->Clear();
				return false;
			}

			if(!::MakeSureDirectoryPathExists(sTexturePath.c_str()))
			{
				LOGERROR "Error while attempting to create path: %s", sTexturePath.c_str());
				delete pOgreMeshCompiler;
				CIntermediateBuilder::Get()->CleanUpHierarchy( pNode );
				CIntermediateBuilder::Get()->Clear();
				return false;
			}

			if(!::MakeSureDirectoryPathExists(sShaderPath.c_str()))
			{
				LOGERROR "Error while attempting to create path: %s", sShaderPath.c_str());
				delete pOgreMeshCompiler;
				CIntermediateBuilder::Get()->CleanUpHierarchy( pNode );
				CIntermediateBuilder::Get()->Clear();
				return false;
			}

			pProgressDlg->LocalStep("StaticMesh: Writing Ogre Mesh..");
			//LOGINFO "Writing Ogre Mesh (%s) ...", sMeshFilename.c_str());

			/////////////////////////////////////
			//MessageBox(NULL, "PreWrite","BREAK!",0);
			/////////////////////////////////////

			pOgreMeshCompiler->WriteOgreMesh(sMeshFilename,bXMLexport);

			/////////////////////////////////////
			//MessageBox(NULL, "Pre Materials","BREAK!",0);
			/////////////////////////////////////

			
			if(bExportMaterials)
			{
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

						COgreMaterialCompiler matComp( it->second, sExtension, bExportColours );
						if(bInOneFile)
						{
							LOGDEBUG "Queueing material ...");
							matWriter.queueForExport( matComp.GetOgreMaterial() );
						}
						else
						{
							try
							{
								Ogre::MaterialPtr oMatPtr = matComp.GetOgreMaterial();
								Ogre::String matName(oMatPtr->getName()+".material");
								Ogre::String matFile(sMaterialPath+"\\"+matName);
								LOGDEBUG "Exporting individual material: (%s).", matName.c_str());
								matWriter.exportMaterial( oMatPtr, matFile);
							} catch (Ogre::Exception e) 
							{
								LOGERROR "OgreExeception caught: %s", e.getDescription().c_str()); 
							}
						}

						if(bCopyTextures) matComp.CopyTextureMaps( sTexturePath, sExtension );
						if(bCopyShaders) matComp.CopyShaderSources( sShaderPath );
						it++;
					}
					if(bInOneFile)
					{
						try	
						{
							LOGDEBUG "Exporting Global material file...");
							matWriter.exportQueued( Ogre::String(sMaterialFilename+".material") );
						} catch (Ogre::Exception e) 
						{
							LOGERROR "OgreExeception caught: %s",  e.getDescription().c_str()); 
						}
					}
				}
			} // End Material export

		} catch(Ogre::Exception e)
		{
			LOGERROR "OgreException caught: %s", e.getDescription().c_str());
		} catch(...)
		{
			LOGERROR "Caught unhandled exception in CMeshExportObject::Export()");
		}
		
		LOGDEBUG "Cleaning up...");
		delete pOgreMeshCompiler;
		// We should do a general Cleanup from the imtermediateBuilder since there might be several objects in the hierarchy
		CIntermediateBuilder::Get()->CleanUpHierarchy( pNode );
		CIntermediateBuilder::Get()->Clear();

		LOGINFO "..Done!");
	}

	END_PROFILE("CMeshExportObject::Export()");

	return returnVal;

}

CIntermediateMesh* CMeshExportObject::GetIntermediateMesh(void)
{
	return m_pIMesh;
}
