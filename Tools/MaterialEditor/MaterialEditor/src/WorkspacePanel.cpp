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
#include <wx/sizer.h>

#include "EventArgs.h"
#include "Project.h"
#include "Workspace.h"
#include "WorkspaceEventArgs.h"

WorkspacePanel::WorkspacePanel(wxWindow* parent,
			   wxWindowID id /* = wxID_ANY */,
			   const wxPoint& pos /* = wxDefaultPosition */,
			   const wxSize& size /* = wxDefaultSize */,
			   long style /* = wxTAB_TRAVERSAL | wxNO_BORDER */,
			   const wxString& name /* = wxT("Workspace Panel")) */)
			   : wxPanel(parent, id, pos, size, style, name)
{
	createPanel();

	Workspace::getSingletonPtr()->subscribe(WE_ProjectAdded, boost::bind(&WorkspacePanel::projectAdded, this, _1));
	Workspace::getSingletonPtr()->subscribe(WE_ProjectRemoved, boost::bind(&WorkspacePanel::projectRemoved, this, _1));
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

	mTreeCtrl = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTR_FULL_ROW_HIGHLIGHT | wxTR_HAS_BUTTONS | wxTR_SINGLE);
	mRootId = mTreeCtrl->AddRoot(wxT("Workspace"));

	mSizer->Add(mTreeCtrl, 0, wxALL | wxEXPAND, 0);

	SetSizer(mSizer);
	Layout();
}

void WorkspacePanel::projectAdded(EventArgs& args)
{
	WorkspaceEventArgs wea = dynamic_cast<WorkspaceEventArgs&>(args);
	Project* project = wea.getProject();

	mTreeCtrl->AppendItem(mRootId, project->getName().c_str());
}

void WorkspacePanel::projectRemoved(EventArgs& args)
{
}
