/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Bo Krohn

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
-----------------------------------------------------------------------------
*/

#ifndef __GDI_META_LIB__
#define __GDI_META_LIB__

#include <commctrl.h>

#include "GDIWindow.h"
#include "GDIButton.h"
#include "GDIEdit.h"
#include "GDIComboBox.h"
#include "GDIListCtrl.h"
#include "GDIListBox.h"
#include "GDITreeCtrl.h"
#include "GDIDialog.h"
#include "GDIFolderDialog.h"
#include "GDIAlphaFiller.h"

#include "MetaDefs.h"
#include "MetaControl.h"
#include "MetaGroup.h"
#include "MetaBaseCtrl.h"
#include "MetaInt.h"
#include "MetaFloat.h"
#include "MetaBool.h"
#include "MetaSelection.h"
#include "MetaString.h"
#include "MetaColor.h"
#include "MetaVec3.h"
#include "MetaVec4.h"

extern HBRUSH	GetGDISysColorBrush(int iIndex);
extern DWORD	GetGDISysColor(int iIndex);
extern HINSTANCE GetCurrentInstance(void);

#endif