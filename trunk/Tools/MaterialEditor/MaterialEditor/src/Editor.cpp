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
#include "Editor.h"

#include <wx/control.h>

#include "EditorInput.h"

Editor::Editor()
: mEditorInput(NULL)
{
	registerEvents();
}

Editor::Editor(EditorInput* input)
: mEditorInput(input)
{
	registerEvents();
}

Editor::~Editor()
{
}

wxControl* Editor::getControl() const
{
	return mControl;
}

void Editor::setControl(wxControl* control)
{
	mControl = control;
}

void Editor::registerEvents()
{
	registerEvent(DirtyStateChanged);
}


EditorInput* Editor::getEditorInput()
{
	return mEditorInput;
}

void Editor::setEditorInput(EditorInput* input)
{
	mEditorInput = input;
}

void Editor::activate()
{
	// Do nothing
}

void Editor::deactivate()
{
	// Do nothing
}

bool Editor::isDirty() const
{
	return false;
}

void Editor::save()
{
	// Do nothing
}

void Editor::saveAs()
{
	// Do nothing
}

bool Editor::isSaveAsAllowed() const
{
	return false;
}

bool Editor::isRedoable() const
{
	return false;
}

void Editor::redo()
{
}

bool Editor::isUndoable() const
{
	return false;
}

void Editor::undo()
{

}

void Editor::cut()
{
	// Do nothing
}

void Editor::copy()
{
	// Do nothing
}

void Editor::paste()
{
	// Do nothing
}