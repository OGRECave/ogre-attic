/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Mark Folkenberg,
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

#ifndef __NDS_LexiExporter_Dialog_ExporterProperties__
#define __NDS_LexiExporter_Dialog_ExporterProperties__

//

class CExporterPropertiesDlg : public GDI::Dialog {

	private:

		Interface* m_pMax;
		IUtil* m_pMaxUtil;

		ExportObjectList* m_pConfig;
		CDDObject* m_pSettings;

		std::vector<unsigned int> m_SelectionList;

		//

		static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	public:

		CExporterPropertiesDlg(Window* pParent, Interface* pMax, IUtil* pMaxUtil, ExportObjectList* pConfig, CDDObject* pSettings);
		~CExporterPropertiesDlg();

		void GetLastSelection(std::vector<unsigned int>& selectionlist) const;

	protected:

		virtual void OnInitDialog();

		void OnButtonAdd(GDI::Button* pButton, unsigned int iSelectedID);

	public:

		bool m_bChanges;

		GDI::ListCtrl m_ItemList;
		HIMAGELIST m_hImageList;
		std::map<std::string, unsigned int> m_ImageListMap;

		GDI::Button m_ButtonAdd;
		GDI::Button m_ButtonAddSelected;
		GDI::Button m_ButtonRemove;
		GDI::Button m_ButtonProperties;
		GDI::Button m_ButtonSettings;
		GDI::Button m_ButtonExport;
		GDI::Button m_ButtonExportSelected;

		//

		void OnConfigButtonAdd();
		void OnConfigButtonAddSelected();
		void OnConfigButtonRemove();
		void OnConfigButtonProperties();
		void OnConfigButtonSettings();
		void OnConfigButtonExport();
		void OnConfigButtonExportSelected();

		void UpdateStuff();

		void PopulateItemList();
		void AddToItemList(unsigned int iConfigIndex);
		void UpdateItemInList(int iIndex);

};

//

#endif // __NDS_LexiExporter_Dialog_ExporterProperties__