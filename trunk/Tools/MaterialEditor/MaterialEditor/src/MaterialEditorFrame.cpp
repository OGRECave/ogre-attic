/*
-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License (LGPL) as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA or go to
http://www.gnu.org/copyleft/lesser.txt
-------------------------------------------------------------------------
*/
#include "MaterialEditorFrame.h"

#include <wx/bitmap.h>
#include <wx/notebook.h>
#include <wx/treectrl.h>
#include <wx/wxscintilla.h>
#include <wx/wizard.h>
#include <wx/aui/auibook.h>
#include <wx/aui/framemanager.h>
#include <wx/propgrid/manager.h>
#include <wx/propgrid/advprops.h>

#include "OgreCamera.h"
#include "OgreColourValue.h"
#include "OgreMaterial.h"
#include "OgreMaterialManager.h"
#include "OgreRoot.h"
#include "OgreString.h"
#include "OgreStringConverter.h"
#include "OgreVector3.h"

#include "LogPanel.h"
#include "ResourcePanel.h"
#include "wxOgre.h"
#include "CodeEditor.h"
#include "MaterialController.h"
#include "MaterialPropertyGridPage.h"
#include "TechniqueController.h"
#include "TechniquePropertyGridPage.h"
#include "PassController.h"
#include "PassPropertyGridPage.h"
#include "ProjectPage.h"
#include "ProjectWizard.h"
#include "WorkspacePanel.h"

using Ogre::Camera;
using Ogre::ColourValue;
using Ogre::RenderSystemList;
using Ogre::Root;
using Ogre::String;
using Ogre::Vector3;

const long ID_FILE_NEW_MENU = wxNewId();
const long ID_FILE_NEW_MENU_PROJECT = wxNewId();
const long ID_FILE_NEW_MENU_MATERIAL = wxNewId();
const long ID_FILE_MENU_OPEN = wxNewId();
const long ID_FILE_MENU_SAVE = wxNewId();
const long ID_FILE_MENU_SAVE_AS = wxNewId();
const long ID_FILE_MENU_CLOSE = wxNewId();
const long ID_FILE_MENU_EXIT = wxNewId();

const long ID_TOOLS_MENU_RESOURCES = wxNewId();
const long ID_TOOLS_MENU_RESOURCES_MENU_ADD_GROUP = wxNewId();
const long ID_TOOLS_MENU_RESOURCES_MENU_REMOVE_GROUP = wxNewId();
const long ID_TOOLS_MENU_RESOURCES_MENU_ADD = wxNewId();
const long ID_TOOLS_MENU_RESOURCES_MENU_REMOVE = wxNewId();

const long ID_VIEW_MENU_OPENGL = wxNewId();
const long ID_VIEW_MENU_DIRECTX = wxNewId();

BEGIN_EVENT_TABLE(MaterialEditorFrame, wxFrame)
	EVT_ACTIVATE(MaterialEditorFrame::OnActivate)
	EVT_MENU (ID_FILE_NEW_MENU_PROJECT, MaterialEditorFrame::OnNewProject)
	EVT_MENU (ID_FILE_MENU_OPEN,		MaterialEditorFrame::OnFileOpen)
	EVT_MENU (ID_FILE_MENU_EXIT,		MaterialEditorFrame::OnFileExit)
	EVT_MENU (ID_VIEW_MENU_OPENGL ,		MaterialEditorFrame::OnViewOpenGL)
	EVT_MENU (ID_VIEW_MENU_DIRECTX,		MaterialEditorFrame::OnViewDirectX)
END_EVENT_TABLE()

MaterialEditorFrame::MaterialEditorFrame(wxWindow* parent)
: wxFrame(parent, - 1, _("Ogre Material Editor"), wxDefaultPosition, wxSize(512, 512), wxDEFAULT_FRAME_STYLE)
{
	mRoot = Ogre::Root::getSingletonPtr();
	mSceneManager = 0;

	// Find Render Systems
	// Testing only, this will be deleted once Projects can tell us
	// which rendering system they want used
	mDirectXRenderSystem = NULL;
	mOpenGLRenderSystem = NULL;
	RenderSystemList *rl = mRoot->getAvailableRenderers();
	if (rl->empty()) 
	{
		wxMessageBox("No render systems found", "Error");
		return;
	}
	for(RenderSystemList::iterator it = rl->begin(); it != rl->end(); ++it)
	{
		if((*it)->getName() == "OpenGL Rendering Subsystem") 
			mOpenGLRenderSystem = *it;
		else
			mDirectXRenderSystem = *it;
	}
	
	createAuiManager();
	createMenuBar();

	CreateToolBar();
	CreateStatusBar();
}

MaterialEditorFrame::~MaterialEditorFrame() 
{
	if(mAuiManager)
	{
		mAuiManager->UnInit();
		delete mAuiManager;
	}
}

void MaterialEditorFrame::createDummyControl()
{
	mDummy = new wxControl(this, wxID_ANY);
	mDummy->Show(false);

	// Grab the current render system from Ogre 
	Ogre::RenderSystem* renderSystem = mRoot->getRenderSystem(); 

	// Create a new parameters list according to compiled OS 
	Ogre::NameValuePairList params; 
	params["externalWindowHandle"] = Ogre::StringConverter::toString((size_t)(mDummy->GetHandle())); 

	// Create the render window (give the name of wxWidget window to Ogre) 
	/* mRenderWindow = */ renderSystem->createRenderWindow("dummy", 10, 10, false, &params); 
}

void MaterialEditorFrame::createAuiManager()
{
	mAuiManager = new wxAuiManager();
	mAuiManager->SetManagedWindow(this);

	createAuiNotebookPane();
	createWorkspacePane();
	createLogPane();
	createPropertiesPane();

	// TEMP
	mRoot->setRenderSystem(mOpenGLRenderSystem);
	mOpenGLRenderSystem->setConfigOption("Full Screen", "No");
	mOpenGLRenderSystem->setConfigOption("VSync", "No");

	char buffer [1024] ;

	wxSize size = wxWindow::GetSize();

	sprintf(buffer, "%d x %d @ 32-bit colour", size.GetWidth(), size.GetHeight());
	mOpenGLRenderSystem->setConfigOption("Video Mode", buffer);

	mRoot->initialise(false);

	//createDummyControl();
	//createOgrePane();

	mAuiManager->Update();
}

void MaterialEditorFrame::createAuiNotebookPane()
{
	mAuiNotebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_DEFAULT_STYLE | wxNO_BORDER);

	wxAuiPaneInfo info;

	info.Floatable(false);
	info.Movable(false);
	info.CenterPane();

	mAuiManager->AddPane(mAuiNotebook, info);
}

void MaterialEditorFrame::createWorkspacePane()
{
	mWorkspaceNotebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);

	mWorkspacePanel = new WorkspacePanel(mWorkspaceNotebook);
	//mMaterialTree = new wxTreeCtrl(mWorkspaceNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
	mWorkspaceNotebook->AddPage(mWorkspacePanel, "Materials");

	mResourcePanel = new ResourcePanel(mWorkspaceNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
	mWorkspaceNotebook->AddPage(mResourcePanel, "Resources");

	wxAuiPaneInfo info;
	info.Caption(_("Workspace"));
	info.MaximizeButton(true);
	info.BestSize(256, 512);
	info.Left();

	mAuiManager->AddPane(mWorkspaceNotebook, info);
}

void MaterialEditorFrame::createLogPane()
{
	mLogPanel = new LogPanel(this);
	mLogPanel->setLog("OGRE");

	wxAuiPaneInfo info;
	info.Caption(_("Log"));
	info.MaximizeButton(true);
	info.BestSize(256, 128);
	info.Bottom();

	mAuiManager->AddPane(mLogPanel, info);
}

void MaterialEditorFrame::createPropertiesPane()
{
	mPropertyGrid = new wxPropertyGridManager(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxPG_BOLD_MODIFIED | wxPG_SPLITTER_AUTO_CENTER | wxPG_TOOLBAR | wxPG_DESCRIPTION | wxPGMAN_DEFAULT_STYLE);

	// Adding a page sets target page to the one added, so
	// we don't have to call SetTargetPage if we are filling
	// it right after adding.
	MaterialController* controller = new MaterialController((MaterialPtr)MaterialManager::getSingletonPtr()->create("Test", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME));
	TechniqueController* tc = controller->createTechnique();
	PassController* pc = tc->createPass();
	PassPropertyGridPage* page = new PassPropertyGridPage(pc);
	//TechniquePropertyGridPage* page = new TechniquePropertyGridPage(tc);
	//MaterialPropertyGridPage* page = new MaterialPropertyGridPage(controller);
	mPropertyGrid->AddPage(wxEmptyString, wxPG_NULL_BITMAP, page);
	page->populate();

	// For total safety, finally reset the target page.
	mPropertyGrid->SetTargetPage(0);

	wxAuiPaneInfo info;
	info.Caption(_("Properties"));
	info.MaximizeButton(true);
	info.BestSize(256, 512);
	info.Left();

	mAuiManager->AddPane(mPropertyGrid, info);
}

void MaterialEditorFrame::createOgrePane()
{
	//mRoot->createRenderWindow("PreviewWindow", 0, 0, false);

	mSceneManager = mRoot->createSceneManager("DefaultSceneManager", "DefaultSceneManager1");

	mSceneManager->setAmbientLight(ColourValue(0.4, 0.3, 0.3));
	
	Camera* camera = mSceneManager->createCamera("PlayerCam");
	camera->setPosition(Vector3(0, 0, 500));
	camera->lookAt(Vector3(0, 0, -300));
	camera->setNearClipDistance(5);

	mOgreControl = new wxOgre(camera, this, wxID_ANY);

	//mOgreControl->Show(false);

	String caption;
	String rs = mRoot->getRenderSystem()->getName();
	if(rs == "OpenGL Rendering Subsystem") caption = "OGRE - OpenGL";
	else caption = "OGRE - DirectX";

	wxAuiPaneInfo info;
	info.Caption("test");
	info.MaximizeButton(true);
	info.MinimizeButton(true);
	info.Float();

	mAuiManager->AddPane(mOgreControl, info);
}

void MaterialEditorFrame::createMenuBar()
{
	mMenuBar = new wxMenuBar();

	createFileMenu();
	createEditMenu();
	createViewMenu();
	createToolsMenu();
	createWindowMenu();
	createHelpMenu();

	SetMenuBar(mMenuBar);
}

void MaterialEditorFrame::createFileMenu()
{	
	mFileMenu = new wxMenu();

	// New sub menu
	wxMenu* newMenu = new wxMenu("");
	wxMenuItem* newProjectItem = newMenu->Append(ID_FILE_NEW_MENU_PROJECT, wxT("&Project..."));
	wxBitmap projectImage;
	if(projectImage.LoadFile("resources/images/project.gif", wxBITMAP_TYPE_GIF))
	{
		newProjectItem->SetBitmap(projectImage);
	}

	wxMenuItem* newMaterialItem = newMenu->Append(ID_FILE_NEW_MENU_MATERIAL, wxT("&Material..."));
	wxBitmap materialImage;
	if(materialImage.LoadFile("resources/images/material.gif", wxBITMAP_TYPE_GIF))
	{
		newProjectItem->SetBitmap(materialImage);
	}

	mFileMenu->AppendSubMenu(newMenu, wxT("&New"));

	mFileMenu->Append(ID_FILE_MENU_OPEN, _("&Open..."));

	wxMenuItem* saveItem = mFileMenu->Append(ID_FILE_MENU_SAVE, _("&Save"));
	wxBitmap saveImage;
	if(saveImage.LoadFile("resources/images/save.gif", wxBITMAP_TYPE_GIF))
	{
		saveItem->SetBitmap(saveImage);
	}

	wxMenuItem* saveAsItem = mFileMenu->Append(ID_FILE_MENU_SAVE_AS, _("Save &As..."));
	wxBitmap saveAsImage;
	if(saveAsImage.LoadFile("resources/images/saveas.gif", wxBITMAP_TYPE_GIF))
	{
		saveAsItem->SetBitmap(saveAsImage);
	}

	mFileMenu->AppendSeparator();

	mFileMenu->Append(ID_FILE_MENU_CLOSE, _("&Close"));
	mFileMenu->AppendSeparator();
	mFileMenu->Append(ID_FILE_MENU_EXIT, _("E&xit"));

	mMenuBar->Append(mFileMenu, _("&File"));
}

void MaterialEditorFrame::createEditMenu()
{
	mEditMenu = new wxMenu("");
	mMenuBar->Append(mEditMenu, _("&Edit"));
}

void MaterialEditorFrame::createViewMenu()
{
	mViewMenu = new wxMenu("");
	mViewMenu->Append(ID_VIEW_MENU_OPENGL, _("OpenGL"));
	mViewMenu->Append(ID_VIEW_MENU_DIRECTX, _("DirectX"));
	mMenuBar->Append(mViewMenu, _("&View"));
}

void MaterialEditorFrame::createToolsMenu()
{
	mToolsMenu = new wxMenu("");
	wxMenu* resourceMenu = new wxMenu("");
	resourceMenu->Append(ID_TOOLS_MENU_RESOURCES_MENU_ADD_GROUP, _("Add Group"));
	resourceMenu->Append(ID_TOOLS_MENU_RESOURCES_MENU_REMOVE_GROUP, _("Remove Group"));
	resourceMenu->Append(ID_TOOLS_MENU_RESOURCES_MENU_ADD, _("Add"));
	resourceMenu->Append(ID_TOOLS_MENU_RESOURCES_MENU_REMOVE, _("Remove"));
	mToolsMenu->AppendSubMenu(resourceMenu, _("Resources"));
	mMenuBar->Append(mToolsMenu, _("&Tools"));
}

void MaterialEditorFrame::createWindowMenu()
{
	mWindowMenu = new wxMenu("");
	mMenuBar->Append(mWindowMenu, _("&Window"));
}

void MaterialEditorFrame::createHelpMenu()
{
	mHelpMenu = new wxMenu("");
	mMenuBar->Append(mHelpMenu, _("&Help"));
}

void MaterialEditorFrame::OnActivate(wxActivateEvent& event)
{
	//if(mOgreControl) mOgreControl->initOgre();
}

void MaterialEditorFrame::OnNewProject(wxCommandEvent& event)
{
	wxBitmap projectImage;
	projectImage.LoadFile("resources/images/new_project.gif", wxBITMAP_TYPE_GIF);

	ProjectWizard* wizard = new ProjectWizard();
	wizard->Create(this, wxID_ANY, wxT("New Project"), projectImage, wxDefaultPosition, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	wizard->RunWizard(wizard->getProjectPage()); // This seems unnatural, seems there must be a better way to deal with wizards

	/*
	wxBitmap projectImage;
	projectImage.LoadFile("resources/images/new_project.gif", wxBITMAP_TYPE_GIF);

	wxWizard* wizard = new wxWizard(this, wxID_ANY, wxT("New Project"), projectImage, wxDefaultPosition, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	ProjectPage* page = new ProjectPage(wizard);
	wizard->GetPageAreaSizer()->Add(page);
	if(wizard->RunWizard(page))
	{

	}

	wizard->Destroy();
	*/
}

void MaterialEditorFrame::OnFileOpen(wxCommandEvent& event)
{
	wxFileDialog * openDialog = new wxFileDialog(this, _("Choose a Material file to open"), wxEmptyString, wxEmptyString, _("Material Files (*.material)|*.material|All Files (*.*)|*.*"));

	if(openDialog->ShowModal() == wxID_OK)
	{
		wxString path = openDialog->GetPath();

		// TESTING
		// Create Editor
		CodeEditor* ce = new CodeEditor(mAuiNotebook, wxID_ANY);
		ce->StyleClearAll();
		ce->SetIndentationGuides(false);
		ce->SetWrapMode(wxSCI_WRAP_NONE);

		ce->SetLexer(wxSCI_LEX_OMS);
		ce->SetKeyWords(0, "material technique pass texture_unit vertex_program vertex_program_ref fragment_program fragment_program_ref");
		ce->SetKeyWords(1, "diffuse depth_check lighting ambient texture colour_op_ex colour_op_multipass_fallback tex_address_mode env_map cull_hardware colout_op_multipass_fallback source target param_named param_named_auto param_indexed param_indexed_auto scene_blend tex_address_mode");
		ce->SetKeyWords(2, "add on off true false one src_texture src_current spherical mirror alpha_blend clamp none");
		ce->SetKeyWords(3, ":");
		ce->StyleSetForeground(wxSCI_OMS_DEFAULT, wxColour(0, 0, 0));
		ce->StyleSetFontAttr(wxSCI_OMS_DEFAULT, 10, "Courier New", false, false, false);
		ce->StyleSetForeground(wxSCI_OMS_COMMENT, wxColour(0, 128, 0));
		ce->StyleSetFontAttr(wxSCI_OMS_COMMENT, 10, "Courier New", false, false, false);
		ce->StyleSetForeground(wxSCI_OMS_PRIMARY, wxColour(0, 0, 255));
		ce->StyleSetFontAttr(wxSCI_OMS_PRIMARY, 10, "Courier New", true, false, false);
		ce->StyleSetForeground(wxSCI_OMS_ATTRIBUTE, wxColour(136, 0, 0));
		ce->StyleSetFontAttr(wxSCI_OMS_ATTRIBUTE, 10, "Courier New", true, false, false);
		ce->StyleSetForeground(wxSCI_OMS_VALUE, wxColour(160, 0, 160));
		ce->StyleSetFontAttr(wxSCI_OMS_VALUE, 10, "Courier New", false, false, false);
		ce->StyleSetForeground(wxSCI_OMS_NUMBER, wxColour(0, 0, 128));
		ce->StyleSetFontAttr(wxSCI_OMS_NUMBER, 10, "Courier New", false, false, false);
		ce->LoadFile(path);

		mAuiNotebook->AddPage(ce, path.SubString(path.find_last_of('\\') + 1, path.length() - 1), true);
	}
}

void MaterialEditorFrame::OnFileExit(wxCommandEvent& event)
{
	Close();
}

void MaterialEditorFrame::OnViewOpenGL(wxCommandEvent& event)
{
	/*
	if(mOpenGLRenderSystem == NULL)
	{
		wxMessageBox("OpenGL Render System not found", "Error");
		return;
	}
                                                                                   
	mOgreControl->SetRenderSystem(mOpenGLRenderSystem);

	wxAuiPaneInfo info = mAuiManager->GetPane(mOgreControl);
	if(!info.IsOk())
	{
		info.MaximizeButton(true);
		info.MinimizeButton(true);
		info.Float();

		mAuiManager->AddPane(mOgreControl, info);
	}

	info.Caption(_("OGRE - OpenGL"));

	mAuiManager->Update();
	*/
}

void MaterialEditorFrame::OnViewDirectX(wxCommandEvent& event)
{
	/*
	if(mDirectXRenderSystem == NULL)
	{
		wxMessageBox("DirectX Render System not found", "Error");
		return;
	}

	mOgreControl->SetRenderSystem(mDirectXRenderSystem);

	wxAuiPaneInfo info = mAuiManager->GetPane(mOgreControl);
	if(!info.IsOk())
	{
		info.MaximizeButton(true);
		info.MinimizeButton(true);
		info.Float();

		mAuiManager->AddPane(mOgreControl, info);
	}

	info.Caption(_("OGRE - DirectX"));

	mAuiManager->Update();
	*/
}

