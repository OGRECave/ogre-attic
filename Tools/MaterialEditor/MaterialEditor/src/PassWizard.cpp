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
#include "PassWizard.h"

#include <wx/sizer.h>

#include "MaterialController.h"
#include "PassController.h"
#include "Project.h"
#include "TechniqueController.h"
#include "TechniquePage.h"
#include "Workspace.h"

BEGIN_EVENT_TABLE(PassWizard, wxWizard)
EVT_WIZARD_FINISHED(wxID_ANY, PassWizard::OnFinish)
END_EVENT_TABLE()

PassWizard::PassWizard()
{
}

PassWizard::~PassWizard()
{
}

bool PassWizard::Create(wxWindow* parent, int id /* = -1 */, const wxString& title /* = wxEmptyString */, const wxBitmap& bitmap /* = wxNullBitmap */, const wxPoint& pos /* = wxDefaultPosition */, long style /* = wxDEFAULT_DIALOG_STYLE */)
{
	bool result = wxWizard::Create(parent, id, title, bitmap, pos, style);

	mPassPage = new PassPage(this);

	GetPageAreaSizer()->Add(mPassPage);

	return result;
}

PassPage* PassWizard::getPassPage() const
{
	return mPassPage;
}

void PassWizard::OnFinish(wxWizardEvent& event)
{
	Project* project = mPassPage->getProject();
	MaterialController* mc = mPassPage->getMaterial();
	TechniqueController* tc = mPassPage->getTechnique();

	wxString name;
	mPassPage->getName(name);

	tc->createPass(name.c_str());
}
