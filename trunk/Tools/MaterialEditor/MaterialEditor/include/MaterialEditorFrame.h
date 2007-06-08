#ifndef _MATERIALEDITORFRAME_H_
#define _MATERIALEDITORFRAME_H_

#include <wx/wx.h>

#include "OgreRenderSystem.h"
#include "OgreRoot.h"

class wxAuiManager;
class wxAuiNotebook;
class wxNotebook;
class wxPropertyGrid;
class wxTreeCtrl;

namespace
{
	class RenderSystem;
	class Root;
}

class LogPanel;
class ResourcePanel;
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
	void createWorkspacePane(void);
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

	void onActivate(wxActivateEvent& event);

	void onFileOpen(wxCommandEvent& event);
	void onFileExit(wxCommandEvent& event);
	void onViewOpenGL(wxCommandEvent& event);
	void onViewDirectX(wxCommandEvent& event);

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
	wxNotebook* mWorkspaceNotebook;
	wxTreeCtrl* mMaterialTree;
	ResourcePanel* mResourcePanel;
	wxPropertyGrid* mPropertyGrid;

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