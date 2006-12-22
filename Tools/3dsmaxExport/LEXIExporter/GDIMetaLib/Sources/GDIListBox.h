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

/////////////////////////////////////////////////
//
//  Max Exporter - GDI ListBox
//
/////////////////////////////////////////////////

#ifndef __NDS_LexiExporter_GDI_ListBox__
#define __NDS_LexiExporter_GDI_ListBox__

//

namespace GDI {

//

class ListBox : public Window {

	public:

		ListBox();
		virtual ~ListBox();

	public:

		//
		//  General Operations
		//

		// Get Count

		// Get Horizontal Extent

		// Get Item Data

		// Get Item Data Ptr

		// Get Item Height

		// Get Item Rect

		// Get List Box Info

		// Get Locale

		// Get Sel

		// Get Text

		// Get Text Len

		// Get Top Index

		// Item From Point

		// Set Column Width

		// Set Horizontal Extent

		// Set Item Data

		// Set Item Data Ptr

		// Set Item Height

		// Set Locale

		// Set Tab Stops

		// Set Top Index
 
		//
		//  Single-Selection Operations
		//

		// Get Cur Sel

		// Set Cur Sel

		//
		//  Multiple-Selection Operations
		//

		// Get Anchor Index

		// Get Caret Index

		// Get Sel Count

		// Get Sel Items

		// Sel Item Range

		// Set Anchor Index

		// Set Caret Index

		// Set Sel

		//
		//  String Operations
		//

		// Add String

		// Delete String

		// Dir

		// Find String

		// Find String Exact

		// Insert String

		// Reset Content

		// SelectString

};

//

} // namespace GDI

//

#endif // __NDS_LexiExporter_GDI_ListBox__