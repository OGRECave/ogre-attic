/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Lasse Tassing

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

#ifndef __NDS_META_DEFINES__
#define __NDS_META_DEFINES__

const int g_iGroupLabelHeight=18;
const int g_iGroupCtrlIndent=2;		// 2 is min - leaves space for lines
const int g_iDefaultCtrlHeight=20;

const int g_iSmallEditWidth=85;
const int g_iColorEditWidth=85;
const int g_iColorPreviewWidth=48;
const int g_iSelectionWidth=150;
const int g_iComboHeight=200;
const int g_iSliderHeight=16;
const int g_iSliderHandleWidth=10;

const int g_iGroupFontHeight=14;
const int g_iCtrlFontHeight=12;

#define g_pszGroupFont "Arial"
#define g_pszCtrlFont "Arial"

#define WM_NOTIFY_MESSAGE_ID	WM_USER+0x0

//

#define GET_SYSTEM_COLOR_BRUSH(x)	GetGDISysColorBrush(x)
#define GET_SYSTEM_COLOR(x)			GetGDISysColor(x)
#define GET_CURRENT_INSTANCE		GetCurrentInstance()

#endif