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
#include "TechniquePropertyGridPage.h"

#include <wx/propgrid/advprops.h>

#include "TechniqueController.h"

BEGIN_EVENT_TABLE(TechniquePropertyGridPage, wxPropertyGridPage)
	EVT_PG_CHANGED(1, TechniquePropertyGridPage::propertyChange)
END_EVENT_TABLE()

TechniquePropertyGridPage::TechniquePropertyGridPage(TechniqueController* controller)
: mController(controller)
{
}

TechniquePropertyGridPage::~TechniquePropertyGridPage()
{
}

void TechniquePropertyGridPage::createPage()
{
	Append(wxStringProperty(wxT("Name"), wxPG_LABEL, mController->getTechnique()->getName()));
	Append(wxStringProperty(wxT("Scheme Name"), wxPG_LABEL, mController->getTechnique()->getSchemeName()));
	Append(wxIntProperty(wxT("LOD Index"), wxPG_LABEL, mController->getTechnique()->getLodIndex()));
}

void TechniquePropertyGridPage::propertyChanged(wxPropertyGridEvent& event)
{
}
















