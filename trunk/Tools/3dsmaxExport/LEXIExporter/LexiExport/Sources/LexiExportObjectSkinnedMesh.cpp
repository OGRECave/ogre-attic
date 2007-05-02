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
#include "LexiExportObjectSkinnedMesh.h"
#include "LexiOgreMeshCompiler.h"
#include "LexiOgreMaterialCompiler.h"
#include "LexiIntermediateBuilder.h"
#include "LexiIntermediateMesh.h"
#include "LexiDialogSelectNode.h"

#include <dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")

CObjectPropertiesDlg *CSkinnedMeshExportObject::m_pEditDlg=NULL;
CDDObject* CSkinnedMeshExportObject::m_pDDMetaDesc=NULL;
//

CSkinnedMeshExportObject::CSkinnedMeshExportObject(CDDObject *pConfig) : CExportObject(pConfig)
{
/*	m_iID = 0xffffffff;
	m_sName = "<unnamed>";
	m_sFilename = "<unknown>";
	m_pSceneNode = NULL;
	m_pDDMetaDesc = BuildMetaDesc();	*/	
	//m_pDDMetaDesc = BuildMetaDesc();	
	m_pIMesh = NULL;
}

CSkinnedMeshExportObject::~CSkinnedMeshExportObject()
{
//	if(m_pDDMetaDesc) m_pDDMetaDesc->Release();
	// Free scene node - no longer needed
//	if(m_pSceneNode) delete m_pSceneNode;
}

// Check if ExportObject supports a given ExportObject instance as parent
bool CSkinnedMeshExportObject::SupportsParentType(const CExportObject *pParent) const
{	
	// Meshes can only have root as parent
	if(pParent==NULL || stricmp(pParent->GetType(), "root")!=0) return false;
	return true;
}

//
bool CSkinnedMeshExportObject::SupportsMAXNode(INode *pMAXNode) const
{
	return CIntermediateBuilderSkeleton::QuerySkinModifier(pMAXNode);

	//if(pMAXNode==NULL) return false;
	//SClass_ID nClass = GetClassIDFromNode(pMAXNode);
	//if(nClass == GEOMOBJECT_CLASS_ID) return true;
	//return false;
}

// Get window for editing ExportObject properties
GDI::Window* CSkinnedMeshExportObject::GetEditWindow(GDI::Window *pParent)
{
	if(m_pEditDlg==NULL)
	{
		m_pEditDlg=new CObjectPropertiesDlg(pParent);
		m_pEditDlg->Create();		
		m_pDDMetaDesc=BuildMetaDesc();
		m_pEditDlg->Init(m_pDDMetaDesc, ".mesh");	
	}

	m_pEditDlg->SetInstance(m_pDDConfig, this);
	return m_pEditDlg;
}

// End edit
void CSkinnedMeshExportObject::CloseEditWindow()
{
	if(m_pEditDlg!=NULL)
	{
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
bool CSkinnedMeshExportObject::OnCreate(CExporterPropertiesDlg *pPropDialog)
{
	if(GetMAXNodeID()==0xFFFFFFFF)
	{
		CSelectNodeDlg dlg((GDI::Window*)pPropDialog, this);
		if(dlg.DoModal() != IDOK) return false;	// user must select a valid node
	}
	else
	{
		INode* pMaxNode = GetNodeFromID( GetMAXNodeID() );
		if(!SupportsMAXNode( pMaxNode ))
		{
			MessageBox(NULL,"Selected object not of compatible type. Skinned meshes requires a skin modifier.", NDS_EXPORTER_TITLE, MB_ICONERROR);
			return false;
		}
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

void CSkinnedMeshExportObject::Read(CDDObject* pConfig)
{
	m_iID = pConfig->GetInt("id", 0xffffffff);
	m_sName = pConfig->GetString("name", "<unnamed>");
	m_sFilename = pConfig->GetString("filename", "<unknown>");
	CExportObject::Read(pConfig);
}

void CSkinnedMeshExportObject::Write(CDDObject* pConfig) const
{
	CExportObject::Write(pConfig);
}

//

CDDObject* CSkinnedMeshExportObject::GetMetaDesc() const
{
	return m_pDDMetaDesc;
}

CDDObject* CSkinnedMeshExportObject::GetEditMeta() const
{
	return m_pConfig;
}*/

CDDObject* CSkinnedMeshExportObject::BuildMetaDesc( void )
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
	pDDMetaElement->SetBool("Default", true);
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID","vertexColorsID");
	pDDMetaElement->SetString("Type","bool");
	pDDMetaElement->SetString("Group","Export Settings");
	pDDMetaElement->SetString("Caption","Vertex Colors");
	pDDMetaElement->SetString("Help","Export baked vertex colors");
	pDDMetaElement->SetBool("Default", true);
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID","uvID");
	pDDMetaElement->SetString("Type","bool");
	pDDMetaElement->SetString("Group","Export Settings");
	pDDMetaElement->SetString("Caption","Texture Coordinates");
	pDDMetaElement->SetString("Help","Export texture vertex coordinates");
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
	pDDMetaElement->SetString("Help","Optimize Index and Vertex buffer. This will in most cases speed up the export process considerably!");
	pDDMetaElement->SetBool("Default", true);
	lSettings.push_back(pDDMetaElement);

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

bool CSkinnedMeshExportObject::Export(CExportProgressDlg *pProgressDlg, bool bForceAll) 
{
	START_PROFILE("CSkinnedMeshExportObject::Export()");
	bool returnVal = false;
	//CIntermediateMesh* pIMesh = NULL;
	COgreMeshCompiler* pOgreMeshCompiler = NULL;
	COgreSkeletonCompiler* pSkeletonCompiler = NULL;

	if(m_bEnabled || bForceAll)
	{
		try 
		{
			// We have two step to begin with
			pProgressDlg->InitLocal(2);

			LOGDEBUG "Starting...");

			// Inform the builder that is should build the skeleton
			m_pDDConfig->SetBool("SkeletonID", true);

			CIntermediateBuilder::Get()->Clear();
			CIntermediateBuilder::Get()->SetConfig(m_pDDConfig);

			// BREAK THIS METHOD UP INTO SMALLER FUNCTION CALLS

			pProgressDlg->LocalStep("SkinnedMesh: Reading max data");

			LOGDEBUG "Creating Hierarchy...");

			Ogre::SceneNode* pNode = CIntermediateBuilder::Get()->CreateHierarchy(GetMAXNodeID(), true, false);
			if(pNode == NULL)
			{
				LOGERROR "No node with such ID: %d", GetMAXNodeID());
				return false;
			}

			LOGDEBUG "Reading config ...");
			bool bCopyTextures = m_pDDConfig->GetBool("copyTextureMaps", false);
			bool bCopyShaders = m_pDDConfig->GetBool("copyShaders", false);
			bool bExportMaterials = m_pDDConfig->GetBool("exportMaterialsID",true);
			bool bInOneFile = m_pDDConfig->GetBool("exportSingleMaterialFileID",false);

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

			const Ogre::String& sOrigFilename = m_pDDConfig->GetString("FileName");
			const Ogre::String& sMeshFilename = FixupFilename(sOrigFilename.c_str(), "mesh");
			const Ogre::String& sMaterialFilename = FixupFilename(sOrigFilename.c_str(), "material");
			const Ogre::String& sTextureFilename = FixupFilename(sOrigFilename.c_str(), "texture");
			const Ogre::String& sShaderFilename = FixupFilename(sOrigFilename.c_str(), "shader");
			const Ogre::String& sSkeletonFilename = FixupFilename(sOrigFilename.c_str(), "shader");

			Ogre::String sBaseName;
			Ogre::String sMeshPath;
			Ogre::String sMaterialPath;
			Ogre::String sTexturePath;
			Ogre::String sShaderPath;
			Ogre::String sSkeletonPath;

			Ogre::StringUtil::splitFilename(sMeshFilename, sBaseName, sMeshPath);
			int n = sMeshPath.find("/");
			while(n != Ogre::String::npos)
			{
				sMeshPath.replace(n,1,"\\");
				n = sMeshPath.find("/");
			}

			Ogre::StringUtil::splitFilename(sSkeletonFilename, sBaseName, sSkeletonPath);
			n = sSkeletonPath.find("/");
			while(n != Ogre::String::npos)
			{
				sSkeletonPath.replace(n,1,"\\");
				n = sSkeletonPath.find("/");
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
				delete m_pIMesh;
				return false;
			}

			if(!::MakeSureDirectoryPathExists(sSkeletonPath.c_str()))
			{
				LOGERROR "Error while attempting to create path: %s", sMeshPath.c_str());
				delete pOgreMeshCompiler;
				delete m_pIMesh;
				return false;
			}

			if(!::MakeSureDirectoryPathExists(sMaterialPath.c_str()))
			{
				LOGERROR "Error while attempting to create path: %s", sMaterialPath.c_str());
				delete pOgreMeshCompiler;
				delete m_pIMesh;
				return false;
			}

			if(!::MakeSureDirectoryPathExists(sTexturePath.c_str()))
			{
				LOGERROR "Error while attempting to create path: %s", sTexturePath.c_str());
				delete pOgreMeshCompiler;
				delete m_pIMesh;
				return false;
			}

			if(!::MakeSureDirectoryPathExists(sShaderPath.c_str()))
			{
				LOGERROR "Error while attempting to create path: %s", sShaderPath.c_str());
				delete pOgreMeshCompiler;
				delete m_pIMesh;
				return false;
			}

			m_pIMesh = (CIntermediateMesh*)pNode->getAttachedObject(0);

			pProgressDlg->LocalStep("SkinnedMesh: Exporting Mesh");
			LOGINFO "Exporting mesh: %s", m_pIMesh->getName().c_str());

			returnVal = CExportObject::Export(pProgressDlg, bForceAll);	

			// Calculate remaining steps:
			// Materials
			//bool bInOneFile = false;//true;
			MAT_LIST lMaterials;
			bool bMatRetrieved = CIntermediateBuilder::Get()->GetMaterials(lMaterials);
			pProgressDlg->InitLocal(5+(lMaterials.size()));
			pProgressDlg->LocalStep("SkinnedMesh: Creating Ogre Mesh..");
			pOgreMeshCompiler = new COgreMeshCompiler(m_pIMesh, m_pDDConfig, pProgressDlg);

			pProgressDlg->LocalStep("SkinnedMesh: Creating Ogre Skeleton..");
			LOGINFO "Writing Skeleton.");	
			pSkeletonCompiler = new COgreSkeletonCompiler( sSkeletonFilename, pOgreMeshCompiler->GetOgreMesh() );

			pProgressDlg->LocalStep("SkinnedMesh: Writing Ogre Skeleton..");
			if( !pSkeletonCompiler->WriteOgreSkeleton( sSkeletonFilename+".skeleton") )
				LOGERROR "Could not write skeleton.");


			pProgressDlg->LocalStep("SkinnedMesh: Writing Ogre Mesh..");
			LOGINFO "Writing Ogre Mesh (%s) ...", sMeshFilename.c_str());
			pOgreMeshCompiler->WriteOgreMesh(sMeshFilename);

						
			if(bExportMaterials)
			{
				pProgressDlg->LocalStep("SkinnedMesh: Exporting Ogre Materials..");
				LOGINFO "Exporting material(s) ...");
				if ( bMatRetrieved )
				{
					Ogre::MaterialSerializer matWriter;
					MAT_LIST::iterator it = lMaterials.begin();

					while (it != lMaterials.end())
					{
						Ogre::String matDesc("SkinnedMesh: Exporting Ogre Material: ");
						matDesc += it->second->GetName();
						pProgressDlg->LocalStep(matDesc.c_str());

						COgreMaterialCompiler matComp( it->second, sExtension );
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
			}

		} catch(Ogre::Exception e)
		{
			LOGERROR "OgreException caught: %s", e.getDescription().c_str());
		} catch(...)
		{
			LOGERROR "Caught unhandled exception in CSkinnedMeshExportObject::Export()");
		}

		LOGDEBUG "Cleaning up...");
		delete pOgreMeshCompiler;
		delete m_pIMesh;
		m_pIMesh = NULL;
		delete pSkeletonCompiler;

		LOGINFO "..Done!");

	}

	END_PROFILE("CSkinnedMeshExportObject::Export()");
	return returnVal;

}


CIntermediateMesh* CSkinnedMeshExportObject::GetIntermediateMesh(void)
{
	return m_pIMesh;
}