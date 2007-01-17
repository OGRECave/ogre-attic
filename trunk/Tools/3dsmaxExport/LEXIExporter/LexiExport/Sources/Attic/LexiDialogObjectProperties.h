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

#ifndef __NDS_LexiExporter_ObjectPropertiesDialog__
#define __NDS_LexiExporter_ObjectPropertiesDialog__

//

class CObjectPropertiesDlg : public GDI::Dialog, public IDDNotify {

	private:

		static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		CDDObject* ExtractAnimationMeta( CDDObject* pDDObj );

		CDDObject* m_pDummyDD;
		std::vector< std::string > m_lAnimNames;
		fastmap< CDDObject* > m_lAnimMap;

	public:

		CObjectPropertiesDlg(Window* pParent, CExportObject* pObj);

		CExportObject* m_pObj;

		HWND m_hMetaWnd;
		GDI::MetaControl* m_pMetaCtrl;
		GDI::ListCtrl m_ItemList;
		HWND m_hAnimMetaWnd;
		GDI::MetaControl* m_pAnimMetaCtrl;

		GDI::Button m_ButtonRemove;

		std::string m_sTitle;
		std::string m_sGBTitle;

		std::string m_sName;
		unsigned int m_iID;
		std::string m_sFilename;

		unsigned int m_iInitFromSelected;


		// Called when data object has changed
		void	OnChanged(const CDDObject *pInstance, const char *pszKey);
		
		//

		void ForceUpdateMembers();
		void BrowseNodeAndAdd();

		void PopulateItemList();
		void UpdateItemList(void);
		void FillItemList(void);
		void DoAnimationSelected();


	protected:

		virtual void OnInitDialog();
		void OnOK();
		void OnCancel();
		void BrowseNode();
		void AddAnimation();

		CDDObject* FindAnimationDataFromIndex( int index );
		CDDObject* FindAnimationData( faststring animName );
		CDDObject* FindOrCreateAnimationData( unsigned int index );
		bool RemoveAnimationData( void );

		void LoadAnimationData( fastvector< const CDDObject* > lAnimList);


	public:

};

//

#endif // __NDS_LexiExporter_ObjectPropertiesDialog__