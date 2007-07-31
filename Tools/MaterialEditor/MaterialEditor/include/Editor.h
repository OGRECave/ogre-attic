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
#ifndef _EDITOR_H_
#define _EDITOR_H_

#include "OgreString.h"

#include "EventContainer.h"

class wxControl;

class EditorInput;

using Ogre::String;

class Editor : public EventContainer
{
public:
	enum EditorEvent
	{
		DirtyStateChanged
	};

	Editor();
	Editor(EditorInput* input);
	virtual ~Editor();

	wxControl* getControl() const;
	void setControl(wxControl* control);

	EditorInput* getEditorInput();
	void setEditorInput(EditorInput* input);

	virtual void activate();
	virtual void deactivate();

	virtual bool isDirty() const;
	virtual void save();
	virtual void saveAs();
	virtual bool isSaveAsAllowed() const;

	virtual bool isRedoable() const;
	virtual void redo();
	virtual bool isUndoable() const;
	virtual void undo();

	virtual void cut();
	virtual void copy();
	virtual void paste();

private:
	void registerEvents();

	EditorInput* mEditorInput;
	wxControl* mControl;
};

#endif // _EDITORINPUT_H_