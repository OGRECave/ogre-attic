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
//  Max Exporter - GDI Dialog
//
/////////////////////////////////////////////////

#ifndef __NDS_LexiExporter_GDI_Dialog__
#define __NDS_LexiExporter_GDI_Dialog__

//

namespace GDI {

//

class Dialog : public Window {

	private:

		unsigned int m_iTemplate;
		DLGPROC m_fnProc;

	public:

		Dialog(unsigned int iTemplate, DLGPROC fnProc, Window* pParent);
		virtual ~Dialog();

	public:

		HWND m_hParent;

	public:

		// Create
		bool Create(unsigned int nTemplateID, Window* pParent);
		bool Create();

		// Do modal run
		int DoModal();

		// End modal dialog run
		void EndDialog(int iResult);

		// Bind dialog item
		void Bind(unsigned int nID, Window& ctl);

		// Goto dialog control (set focus, etc.)
		void GotoDlgCtrl(unsigned int nID);

};

//

} // namespace GDI

//

#endif // __NDS_LexiExporter_GDI_Dialog__