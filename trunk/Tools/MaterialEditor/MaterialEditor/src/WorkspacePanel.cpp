/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

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

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "WorkspacePanel.h"

#include <boost/bind.hpp>

#include <wx/button.h>
#include <wx/menu.h>
#include <wx/sizer.h>

#include "OgreMaterial.h"
#include "OgrePass.h"
#include "OgreTechnique.h"

#include "EventArgs.h"
#include "MaterialController.h"
#include "MaterialEventArgs.h"
#include "PassController.h"
#include "Project.h"
#include "ProjectEventArgs.h"
#include "TechniqueController.h"
#include "TechniqueEventArgs.h"
#include "Workspace.h"
#include "WorkspaceEventArgs.h"

const long ID_TREE_CTRL = wxNewId();
const long ID_MENU_NEW = wxNewId();
const long ID_MENU_NEW_PROJECT = wxNewId();
const long ID_MENU_NEW_MATERIAL = wxNewId();
const long ID_MENU_NEW_TECHNIQUE = wxNewId();
const long ID_MENU_NEW_PASS = wxNewId();
const long ID_MENU_PASS_ENABLED = wxNewId();
const long ID_MENU_DELETE = wxNewId();

BEGIN_EVENT_TABLE(WorkspacePanel, wxPanel)
	EVT_TREE_ITEM_RIGHT_CLICK(ID_TREE_CTRL, WorkspacePanel::OnRightClick)
END_EVENT_TABLE()

WorkspacePanel::WorkspacePanel(wxWindow* parent,
			   wxWindowID id /* = wxID_ANY */,
			   const wxPoint& pos /* = wxDefaultPosition */,
			   const wxSize& size /* = wxDefaultSize */,
			   long style /* = wxTAB_TRAVERSAL | wxNO_BORDER */,
			   const wxString& name /* = wxT("Workspace Panel")) */)
			   : wxPanel(parent, id, pos, size, style, name)
{
	createPanel();

	Workspace::getSingletonPtr()->subscribe(Workspace::ProjectAdded, boost::bind(&WorkspacePanel::projectAdded, this, _1));
	Workspace::getSingletonPtr()->subscribe(Workspace::ProjectRemoved, boost::bind(&WorkspacePanel::projectRemoved, this, _1));
}

WorkspacePanel::~WorkspacePanel()
{
}

void WorkspacePanel::createPanel()
{
	mSizer = new wxFlexGridSizer(1, 1, 0, 0);
	mSizer->AddGrowableCol(0);
	mSizer->AddGrowableRow(0);
	mSizer->SetFlexibleDirection(wxVERTICAL);
	mSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	//mToolBarPanel = new wxPanel(this);
	//mSizer->Add(mToolBarPanel, 1, wxALL | wxEXPAND, 0);

	mTreeCtrl = new wxTreeCtrl(this, ID_TREE_CTRL, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTR_FULL_ROW_HIGHLIGHT | wxTR_HAS_BUTTONS | wxTR_SINGLE);
	mRootId = mTreeCtrl->AddRoot(wxT("Workspace"));

	mSizer->Add(mTreeCtrl, 0, wxALL | wxEXPAND, 0);

	SetSizer(mSizer);
	Layout();
}

void WorkspacePanel::showContextMenu(wxPoint point, wxTreeItemId id)
{
	wxMenu* contextMenu = new wxMenu();
	appendNewMenu(contextMenu);
	contextMenu->AppendSeparator();
	if(isProject(id)) appendProjectMenuItems(contextMenu);
	else if(isMaterial(id)) appendMaterialMenuItems(contextMenu);
	else if(isTechnique(id)) appendTechniqueMenuItems(contextMenu);
	else appendPassMenuItems(contextMenu);

	PopupMenu(contextMenu, point);

	contextMenu->Remove(ID_MENU_NEW);

	delete contextMenu;
}

void WorkspacePanel::appendNewMenu(wxMenu* menu)
{
	if(mNewMenu == NULL)
	{
		mNewMenu = new wxMenu(wxEmptyString);
		mNewMenu->Append(ID_MENU_NEW_PROJECT, wxT("Project"));
		mNewMenu->Append(ID_MENU_NEW_MATERIAL, wxT("Material"));
		mNewMenu->Append(ID_MENU_NEW_TECHNIQUE, wxT("Technique"));
		mNewMenu->Append(ID_MENU_NEW_PASS, wxT("Pass"));
	}

	menu->AppendSubMenu(mNewMenu, wxT("New"));
}

void WorkspacePanel::appendProjectMenuItems(wxMenu* menu)
{
}

void WorkspacePanel::appendMaterialMenuItems(wxMenu* menu)
{
}

void WorkspacePanel::appendTechniqueMenuItems(wxMenu* menu)
{
}

void WorkspacePanel::appendPassMenuItems(wxMenu* menu)
{
	menu->AppendCheckItem(ID_MENU_PASS_ENABLED, wxT("Enabled"));
}

bool WorkspacePanel::isProject(wxTreeItemId id)
{
	for(ProjectIdMap::iterator it = mProjectIdMap.begin(); it != mProjectIdMap.end(); ++it)
	{
		if(it->first == id) return true;
	}

	return false;
}

bool WorkspacePanel::isMaterial(wxTreeItemId id)
{
	for(MaterialIdMap::iterator it = mMaterialIdMap.begin(); it != mMaterialIdMap.end(); ++it)
	{
		if(it->first == id) return true;
	}

	return false;
}

bool WorkspacePanel::isTechnique(wxTreeItemId id)
{
	for(TechniqueIdMap::iterator it = mTechniqueIdMap.begin(); it != mTechniqueIdMap.end(); ++it)
	{
		if(it->first == id) return true;
	}

	return false;
}

void WorkspacePanel::subscribe(Project* project)
{
	project->subscribe(Project::NameChanged, boost::bind(&WorkspacePanel::projectNameChanged, this, _1));
	project->subscribe(Project::MaterialAdded, boost::bind(&WorkspacePanel::projectMaterialAdded, this, _1));
	project->subscribe(Project::MaterialRemoved, boost::bind(&WorkspacePanel::projectMaterialRemoved, this, _1));
}

void WorkspacePanel::subscribe(MaterialController* material)
{
	material->subscribe(MaterialController::NameChanged, boost::bind(&WorkspacePanel::projectNameChanged, this, _1));
	material->subscribe(MaterialController::TechniqueAdded, boost::bind(&WorkspacePanel::projectMaterialAdded, this, _1));
	material->subscribe(MaterialController::TechniqueRemoved, boost::bind(&WorkspacePanel::projectMaterialRemoved, this, _1));
}

void WorkspacePanel::subscribe(TechniqueController* technique)
{
	//technique->subscribe(TechniqueController::NameChanged, boost::bind(&WorkspacePanel::projectNameChanged, this, _1));
	technique->subscribe(TechniqueController::PassAdded, boost::bind(&WorkspacePanel::techniquePassAdded, this, _1));
	technique->subscribe(TechniqueController::PassRemoved, boost::bind(&WorkspacePanel::techniquePassRemoved, this, _1));
}

void WorkspacePanel::OnRightClick(wxTreeEvent& event)
{
	showContextMenu(event.GetPoint(), event.GetItem());
}

void WorkspacePanel::projectAdded(EventArgs& args)
{
	WorkspaceEventArgs wea = dynamic_cast<WorkspaceEventArgs&>(args);
	Project* project = wea.getProject();
	subscribe(project);

	wxTreeItemId id = mTreeCtrl->AppendItem(mRootId, project->getName().c_str());
	mTreeCtrl->SelectItem(id, true);

	mProjectIdMap[project] = id;
}

void WorkspacePanel::projectRemoved(EventArgs& args)
{
	// TODO: Implement projectRemoved
}

void WorkspacePanel::projectNameChanged(EventArgs& args)
{
	ProjectEventArgs pea = dynamic_cast<ProjectEventArgs&>(args);
	Project* project = pea.getProject();

	wxTreeItemId projectId = mProjectIdMap[project];
	mTreeCtrl->SetItemText(projectId, project->getName().c_str());
}

void WorkspacePanel::projectMaterialAdded(EventArgs& args)
{
	ProjectEventArgs pea = dynamic_cast<ProjectEventArgs&>(args);
	Project* project = pea.getProject();
	MaterialController* material = pea.getMaterial();

	wxTreeItemId projectId = mProjectIdMap[project];
	wxTreeItemId id = mTreeCtrl->AppendItem(projectId, material->getMaterial()->getName().c_str());
	mTreeCtrl->SelectItem(id, true);

	mMaterialIdMap[material] = id;
}

void WorkspacePanel::projectMaterialRemoved(EventArgs& args)
{
	// TODO: Implement projectMaterialRemoved
}

void WorkspacePanel::materialNameChanged(EventArgs& args)
{
	MaterialEventArgs mea = dynamic_cast<MaterialEventArgs&>(args);
	MaterialController* mc = mea.getMaterialController();

	wxTreeItemId materialId = mMaterialIdMap[mc];
	mTreeCtrl->SetItemText(materialId, mc->getMaterial()->getName().c_str());
}

void WorkspacePanel::materialTechniqueAdded(EventArgs& args)
{
	MaterialEventArgs mea = dynamic_cast<MaterialEventArgs&>(args);
	MaterialController* mc = mea.getMaterialController();
	TechniqueController* tc = mea.getTechniqueController();

	wxTreeItemId materialId = mMaterialIdMap[mc];
	wxTreeItemId id = mTreeCtrl->AppendItem(materialId, tc->getTechnique()->getName().c_str());
	mTreeCtrl->SelectItem(id, true);
}

void WorkspacePanel::materialTechniqueRemoved(EventArgs& args)
{
	// TODO: Implement materialTechniqueRemoved
}

void WorkspacePanel::techniquePassAdded(EventArgs& args)
{
	TechniqueEventArgs tea = dynamic_cast<TechniqueEventArgs&>(args);
	TechniqueController* tc = tea.getTechniqueController();
	PassController* pc = tea.getPassController();

	wxTreeItemId techniqueId = mTechniqueIdMap[tc];
	wxTreeItemId id = mTreeCtrl->AppendItem(techniqueId, pc->getPass()->getName().c_str());
	mTreeCtrl->SelectItem(id, true);
}

void WorkspacePanel::techniquePassRemoved(EventArgs& args)
{
	// TODO: Implement materialTechniqueRemoved
}