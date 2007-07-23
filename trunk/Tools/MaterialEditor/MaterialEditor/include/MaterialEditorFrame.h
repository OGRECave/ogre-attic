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
#ifndef _MATERIALEDITORFRAME_H_
#define _MATERIALEDITORFRAME_H_

#include <wx/wx.h>

#include "OgreRenderSystem.h"
#include "OgreRoot.h"

class wxAuiManager;
class wxAuiNotebook;
class wxNotebook;
class wxPropertyGridManager;
class wxTreeCtrl;

namespace
{
	class RenderSystem;
	class Root;
}

class LogPanel;
class PropertiesPanel;
class ResourcePanel;
class WorkspacePanel;
class wxOgre;

class MaterialEditorFrame : public wxFrame
{
public:
	MaterialEditorFrame(wxWindow* parent = NULL);
	~MaterialEditorFrame();

protected:
	void createDummyControl(void);
	void createAuiManager(void);
	void createAuiNotebookPane(void);
	void createManagementPane(void);
	void createLogPane(void);
	void createPropertiesPane();
	void createOgrePane(void);

	void createMenuBar(void);
	void createFileMenu(void);
	void createEditMenu(void);
	void createViewMenu(void);
	void createToolsMenu(void);
	void createWindowMenu(void);
	void createHelpMenu(void);

	void OnActivate(wxActivateEvent& event);

	void OnNewProject(wxCommandEvent& event);
	void OnNewMaterial(wxCommandEvent& event);
	void OnFileOpen(wxCommandEvent& event);
	void OnFileExit(wxCommandEvent& event);
	void OnViewOpenGL(wxCommandEvent& event);
	void OnViewDirectX(wxCommandEvent& event);

private:
	wxMenuBar* mMenuBar;
	wxMenu* mFileMenu;
	wxMenu* mEditMenu;
	wxMenu* mViewMenu;
	wxMenu* mToolsMenu;
	wxMenu* mWindowMenu;
	wxMenu* mHelpMenu;

	wxAuiManager* mAuiManager;

	wxAuiNotebook* mAuiNotebook;
	wxAuiNotebook* mManagementNotebook;
	WorkspacePanel* mWorkspacePanel;
	ResourcePanel* mResourcePanel;
	PropertiesPanel* mPropertiesPanel;

	wxControl* mDummy;

	Ogre::Root* mRoot;

	LogPanel* mLogPanel;
	wxOgre* mOgreControl;

	Ogre::SceneManager* mSceneManager;
	Ogre::RenderSystem* mDirectXRenderSystem;
	Ogre::RenderSystem* mOpenGLRenderSystem;

	DECLARE_EVENT_TABLE();
};

#endif // _MATERIALEDITORFRAME_H_