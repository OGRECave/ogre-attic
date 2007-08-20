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
#include "OgreConfigFile.h"
#include "OgreMaterial.h"
#include "OgreMaterialManager.h"
#include "OgreRoot.h"
#include "OgreString.h"
#include "OgreStringConverter.h"
#include "OgreVector3.h"

#include "CgEditor.h"
#include "DocPanel.h"
#include "EditorManager.h"
#include "LogPanel.h"
#include "MaterialController.h"
#include "MaterialPropertyGridPage.h"
#include "MaterialScriptEditor.h"
#include "MaterialWizard.h"
#include "PropertiesPanel.h"
#include "TechniqueController.h"
#include "TechniquePropertyGridPage.h"
#include "PassController.h"
#include "PassPropertyGridPage.h"
#include "ProjectPage.h"
#include "ProjectWizard.h"
#include "ResourcePanel.h"
#include "WorkspacePanel.h"
#include "wxOgre.h"

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

const long ID_EDIT_MENU_UNDO = wxNewId();
const long ID_EDIT_MENU_REDO = wxNewId();
const long ID_EDIT_MENU_CUT = wxNewId();
const long ID_EDIT_MENU_COPY = wxNewId();
const long ID_EDIT_MENU_PASTE = wxNewId();

const long ID_TOOLS_MENU_RESOURCES = wxNewId();
const long ID_TOOLS_MENU_RESOURCES_MENU_ADD_GROUP = wxNewId();
const long ID_TOOLS_MENU_RESOURCES_MENU_REMOVE_GROUP = wxNewId();
const long ID_TOOLS_MENU_RESOURCES_MENU_ADD = wxNewId();
const long ID_TOOLS_MENU_RESOURCES_MENU_REMOVE = wxNewId();

const long ID_VIEW_MENU_OPENGL = wxNewId();
const long ID_VIEW_MENU_DIRECTX = wxNewId();

BEGIN_EVENT_TABLE(MaterialEditorFrame, wxFrame)
	EVT_ACTIVATE(MaterialEditorFrame::OnActivate)
	// File Menu
	EVT_MENU (ID_FILE_NEW_MENU_PROJECT,  MaterialEditorFrame::OnNewProject)
	EVT_MENU (ID_FILE_NEW_MENU_MATERIAL, MaterialEditorFrame::OnNewMaterial)
	EVT_MENU (ID_FILE_MENU_OPEN,		 MaterialEditorFrame::OnFileOpen)
	EVT_MENU (ID_FILE_MENU_EXIT,		 MaterialEditorFrame::OnFileExit)
	// Edit Menu
	EVT_MENU (ID_EDIT_MENU_UNDO,  MaterialEditorFrame::OnEditUndo)
	EVT_MENU (ID_EDIT_MENU_REDO,  MaterialEditorFrame::OnEditRedo)
	EVT_MENU (ID_EDIT_MENU_CUT,	  MaterialEditorFrame::OnEditCut)
	EVT_MENU (ID_EDIT_MENU_COPY,  MaterialEditorFrame::OnEditCopy)
	EVT_MENU (ID_EDIT_MENU_PASTE, MaterialEditorFrame::OnEditPaste)
	// View Menu
	EVT_MENU (ID_VIEW_MENU_OPENGL , MaterialEditorFrame::OnViewOpenGL)
	EVT_MENU (ID_VIEW_MENU_DIRECTX, MaterialEditorFrame::OnViewDirectX)
END_EVENT_TABLE()

MaterialEditorFrame::MaterialEditorFrame(wxWindow* parent)
: mFileMenu(NULL), mEditMenu(NULL), mToolsMenu(NULL),
  wxFrame(parent, - 1, _("Ogre Material Editor"), wxDefaultPosition, wxSize(512, 512), wxDEFAULT_FRAME_STYLE)
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
	mAuiManager->SetFlags(wxAUI_MGR_DEFAULT | wxAUI_MGR_ALLOW_ACTIVE_PANE | wxAUI_MGR_TRANSPARENT_DRAG);
	mAuiManager->SetManagedWindow(this);

	wxAuiDockArt* art = mAuiManager->GetArtProvider();
	art->SetMetric(wxAUI_DOCKART_PANE_BORDER_SIZE, 1);
	art->SetMetric(wxAUI_DOCKART_SASH_SIZE, 4);
	art->SetMetric(wxAUI_DOCKART_CAPTION_SIZE, 17);
	art->SetColour(wxAUI_DOCKART_ACTIVE_CAPTION_COLOUR, wxColour(49, 106, 197));
	art->SetColour(wxAUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR, wxColour(90, 135, 208));
	art->SetColour(wxAUI_DOCKART_ACTIVE_CAPTION_TEXT_COLOUR, wxColour(255, 255, 255));
	art->SetColour(wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR, wxColour(200, 198, 183));
	art->SetColour(wxAUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR, wxColour(228, 226, 209));
	art->SetColour(wxAUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR, wxColour(0, 0, 0));

	createAuiNotebookPane();
	createManagementPane();
	createInformationPane();
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

	// Create EditorManager singleton
	new EditorManager(mAuiNotebook);

	wxAuiPaneInfo info;

	info.Floatable(false);
	info.Movable(false);
	info.CenterPane();

	mAuiManager->AddPane(mAuiNotebook, info);
}

void MaterialEditorFrame::createManagementPane()
{
	mManagementNotebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxNO_BORDER);

	mWorkspacePanel = new WorkspacePanel(mManagementNotebook);
	//mMaterialTree = new wxTreeCtrl(mWorkspaceNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
	mManagementNotebook->AddPage(mWorkspacePanel, "Materials");

	mResourcePanel = new ResourcePanel(mManagementNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
	mManagementNotebook->AddPage(mResourcePanel, "Resources");

	wxAuiPaneInfo info;
	info.Caption(_("Management"));
	info.MaximizeButton(true);
	info.BestSize(256, 512);
	info.Left();
	info.Layer(1);

	mAuiManager->AddPane(mManagementNotebook, info);
}

void MaterialEditorFrame::createInformationPane()
{
	mInformationNotebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxNO_BORDER);

	mLogPanel = new LogPanel(mInformationNotebook);
	mInformationNotebook->AddPage(mLogPanel, "Log");

	mDocPanel = new DocPanel(mInformationNotebook);
	mInformationNotebook->AddPage(mDocPanel, "Documentation");

	wxAuiPaneInfo info;
	info.Caption(_("Information"));
	info.MaximizeButton(true);
	info.BestSize(256, 128);
	info.Bottom();

	mAuiManager->AddPane(mInformationNotebook, info);
}

void MaterialEditorFrame::createPropertiesPane()
{
	mPropertiesPanel = new PropertiesPanel(this);

	wxAuiPaneInfo info;
	info.Caption(_("Properties"));
	info.MaximizeButton(true);
	info.BestSize(256, 512);
	info.Left();
	info.Layer(1);

	mAuiManager->AddPane(mPropertiesPanel, info);
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
	wxMenu* newMenu = new wxMenu();

	wxMenuItem* newProjectItem = newMenu->Append(ID_FILE_NEW_MENU_PROJECT, wxT("&Project..."));
	wxBitmap projectImage;
	if(projectImage.LoadFile("resources/images/project.png", wxBITMAP_TYPE_PNG))
	{
		newProjectItem->SetBitmap(projectImage);
	}

	wxMenuItem* newMaterialItem = newMenu->Append(ID_FILE_NEW_MENU_MATERIAL, wxT("&Material..."));
	wxBitmap materialImage;
	if(materialImage.LoadFile("resources/images/material.png", wxBITMAP_TYPE_PNG))
	{
		newMaterialItem->SetBitmap(materialImage);
	}

	mFileMenu->AppendSubMenu(newMenu, wxT("&New"));

	mFileMenu->Append(ID_FILE_MENU_OPEN, _("&Open..."));

	wxMenuItem* saveItem = mFileMenu->Append(ID_FILE_MENU_SAVE, wxT("&Save"));
	wxBitmap saveImage;
	if(saveImage.LoadFile("resources/images/save.png", wxBITMAP_TYPE_PNG))
	{
		saveItem->SetBitmap(saveImage);
	}

	wxMenuItem* saveAsItem = mFileMenu->Append(ID_FILE_MENU_SAVE_AS, wxT("Save &As..."));
	//wxBitmap saveAsImage;
	//if(saveAsImage.LoadFile("resources/images/save.png", wxBITMAP_TYPE_PNG))
	//{
		saveAsItem->SetBitmap(saveImage);
	//}

	mFileMenu->AppendSeparator();

	mFileMenu->Append(ID_FILE_MENU_CLOSE, _("&Close"));
	mFileMenu->AppendSeparator();
	mFileMenu->Append(ID_FILE_MENU_EXIT, _("E&xit"));

	mFileMenu->UpdateUI();

	mMenuBar->Append(mFileMenu, _("&File"));
}

void MaterialEditorFrame::createEditMenu()
{
	mEditMenu = new wxMenu("");
	mEditMenu->Append(ID_EDIT_MENU_UNDO, wxT("Undo"));
	mEditMenu->Append(ID_EDIT_MENU_REDO, wxT("Redo"));
	mEditMenu->AppendSeparator();
	mEditMenu->Append(ID_EDIT_MENU_CUT, wxT("Cut"));
	mEditMenu->Append(ID_EDIT_MENU_COPY, wxT("Copy"));
	mEditMenu->Append(ID_EDIT_MENU_PASTE, wxT("Paste"));
	
	mMenuBar->Append(mEditMenu, wxT("&Edit"));
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

void MaterialEditorFrame::OnActiveEditorChanged(EventArgs& args)
{
	EditorEventArgs eea = dynamic_cast<EditorEventArgs&>(args);
	Editor* editor = eea.getEditor();

	// TODO: Update menu item enablement
}

void MaterialEditorFrame::OnNewProject(wxCommandEvent& event)
{
	//wxBitmap projectImage;
	//projectImage.LoadFile("resources/images/new_project.gif", wxBITMAP_TYPE_GIF);

	ProjectWizard* wizard = new ProjectWizard();
	wizard->Create(this, wxID_ANY, wxT("New Project"), wxNullBitmap, wxDefaultPosition, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	wizard->RunWizard(wizard->getProjectPage()); // This seems unnatural, seems there must be a better way to deal with wizards

	wizard->Destroy();
}

void MaterialEditorFrame::OnNewMaterial(wxCommandEvent& event)
{
	//wxBitmap materialImage;
	//materialImage.LoadFile("resources/images/new_material.gif", wxBITMAP_TYPE_GIF);

	MaterialWizard* wizard = new MaterialWizard();
	wizard->Create(this, wxID_ANY, wxT("New Material"), wxNullBitmap, wxDefaultPosition, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	wizard->RunWizard(wizard->getMaterialPage());// This seems unnatural, seems there must be a better way to deal with wizards

	wizard->Destroy();
}

void MaterialEditorFrame::OnFileOpen(wxCommandEvent& event)
{
	wxFileDialog * openDialog = new wxFileDialog(this, _("Choose a file to open"), wxEmptyString, wxEmptyString, _("Material Files (*.material)|*.material|Program Files (*.program)|*.program|Cg Files (*.cg)|*.cg|GLSL Files(*.vert; *.frag)|*.vert;*.frag|All Files (*.*)|*.*"));

	if(openDialog->ShowModal() == wxID_OK)
	{
		wxString path = openDialog->GetPath();
		if(path.EndsWith(wxT(".material")) || path.EndsWith(wxT(".program")))
		{
			MaterialScriptEditor* editor = new MaterialScriptEditor(EditorManager::getSingletonPtr()->getEditorNotebook());
			editor->loadFile(path);
			int index = (int)path.find_last_of('\\');
			if(index == -1) index = (int)path.find_last_of('/');
			editor->setName((index != -1) ? path.substr(index + 1, path.Length()) : path);

			EditorManager::getSingletonPtr()->openEditor(editor);
		}
		else if(path.EndsWith(wxT(".cg")))
		{
			CgEditor* editor = new CgEditor(EditorManager::getSingletonPtr()->getEditorNotebook());
			editor->loadFile(path);
			int index = (int)path.find_last_of('\\');
			if(index == -1) index = (int)path.find_last_of('/');
			editor->setName((index != -1) ? path.substr(index + 1, path.Length()) : path);

			EditorManager::getSingletonPtr()->openEditor(editor);
		}
	}
}

void MaterialEditorFrame::OnFileExit(wxCommandEvent& event)
{
	Close();
}

void MaterialEditorFrame::OnEditUndo(wxCommandEvent& event)
{
	Editor* editor = EditorManager::getSingletonPtr()->getActiveEditor();
	if(editor != NULL) editor->undo();
}

void MaterialEditorFrame::OnEditRedo(wxCommandEvent& event)
{
	Editor* editor = EditorManager::getSingletonPtr()->getActiveEditor();
	if(editor != NULL) editor->redo();
}

void MaterialEditorFrame::OnEditCut(wxCommandEvent& event)
{
	Editor* editor = EditorManager::getSingletonPtr()->getActiveEditor();
	if(editor != NULL) editor->cut();
}

void MaterialEditorFrame::OnEditCopy(wxCommandEvent& event)
{
	Editor* editor = EditorManager::getSingletonPtr()->getActiveEditor();
	if(editor != NULL) editor->copy();
}

void MaterialEditorFrame::OnEditPaste(wxCommandEvent& event)
{
	Editor* editor = EditorManager::getSingletonPtr()->getActiveEditor();
	if(editor != NULL) editor->paste();
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

