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

#include "stdafx.h"

namespace GDI
{

MetaBaseCtrl::MetaBaseCtrl()
{
	m_hWnd=NULL;	
	m_iX=0;
	m_iY=16;
	m_iHeight=g_iDefaultCtrlHeight;
	m_iWidth=100;
	m_bEnabled=true;
}
MetaBaseCtrl::~MetaBaseCtrl()
{

}

bool MetaBaseCtrl::Create(MetaControl *pOwner, MetaGroup *pGroup, const char *pszMetaID, const CDDObject *pMetaKey)
{
	m_pOwner=pOwner;
	m_pGroup=pGroup;
	m_sMetaID=pszMetaID;
	m_pMetaKey=pMetaKey;
	m_sCaption=m_pMetaKey->GetString("Caption", pszMetaID);
	m_sHelp=m_pMetaKey->GetString("Help", "No help available");

	// Initial size and position	
	m_hWnd=::CreateWindow("MetaChildControl", pszMetaID, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN, m_iX, m_iY, m_iWidth, m_iHeight, m_pGroup->m_hWnd, NULL, NULL, NULL);	

	OnCreated();
	pGroup->AddControl(this);
	UpdateData(m_pOwner->GetData());
	return true;
}

void MetaBaseCtrl::Destroy()
{
	if(m_hWnd)
	{
		::DestroyWindow(m_hWnd);
		m_hWnd=NULL;
	}	
}

void MetaBaseCtrl::MoveTo(int iX, int iY)
{
	m_iX=iX;
	m_iY=iY;
	::SetWindowPos(m_hWnd, NULL, m_iX, m_iY, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOREDRAW);
}

int MetaBaseCtrl::GetHeight()
{
	return m_iHeight;
}

void MetaBaseCtrl::SetWidth(int iWidth)
{
	m_iWidth=iWidth;
	::SetWindowPos(m_hWnd, NULL, 0, 0, m_iWidth, GetHeight(), SWP_NOMOVE|SWP_NOZORDER);
	OnLayout();
}

void MetaBaseCtrl::OnPaint()
{
	PAINTSTRUCT ps;
	BeginPaint(m_hWnd, &ps);
	EndPaint(m_hWnd, &ps);
}

// Enable/disable control
void MetaBaseCtrl::SetEnabled(bool bEnabled)
{
	m_bEnabled=bEnabled;	
	::InvalidateRect(m_hWnd, NULL, false);
}

// Evaluate conditions. This function will call SetEnabled() is needed.
void MetaBaseCtrl::CheckConditions()
{
	if(m_pMetaKey->GetKeyType("Condition")!=DD_STRING) return;

	CDDObject *pData=m_pOwner->GetData();
	bool bEnabled=true;
	static char buffer[1024];
	static char func[10];
	char lastoperator=0;
	const char *pszConditionString=m_pMetaKey->GetString("Condition");
	do
	{
		// Find next variable identifier
		pszConditionString=strchr(pszConditionString, '$');
		if(pszConditionString==NULL) break;

		// Skip identifier
		++pszConditionString;
		if(!*pszConditionString) break;

		const char *pszEnd=(char*)pszConditionString;
		// Find token or space seperator
		while(	*pszEnd!=0   && *pszEnd!='=' &&
				*pszEnd!='<' && *pszEnd!='>' &&
				*pszEnd!='!' && *pszEnd!=' ')
			++pszEnd;
		if(*pszEnd==0) break;	// End of string, bail
		
		// Copy data identifier string
		int iLen=(int)(pszEnd-pszConditionString);
		memcpy(buffer, pszConditionString, iLen);
		buffer[iLen]=0;	// Zero terminate

		// Early data type determination - eliminate further check if data is non-existing
		EDDType eKeyType=pData->GetKeyType(buffer);
		
		// Skip spaces
		pszConditionString=pszEnd;
		while(*pszConditionString==' ' && *pszConditionString!=0) 
			++pszConditionString;
		if(!*pszConditionString) break;

		// Copy operator
		int iCount=0;
		while(	(*pszConditionString=='=' || *pszConditionString=='>' ||
				*pszConditionString=='<' || *pszConditionString=='!') && iCount<2)
			func[iCount++]=*(pszConditionString++);
		func[iCount]=0;

		// Skip spaces
		while(*pszConditionString==' ' && *pszConditionString!=0) 
			++pszConditionString;
		if(!*pszConditionString) break;

		// Evaluate each type differently
		bool bEvaluate=true;
		switch(eKeyType)
		{
			case DD_INT:
				{
					// Get data for comparison
					char *pszTemp;
					int iComp1=pData->GetInt(buffer);
					int iComp2=strtol(pszConditionString, &pszTemp, 10);
					pszEnd=pszTemp;
					switch(func[0])
					{
						case '=': bEvaluate=iComp1==iComp2;break;
						case '>': bEvaluate=func[1]=='=' ? iComp1>=iComp2 : iComp1>iComp2;break;
						case '<': bEvaluate=func[1]=='=' ? iComp1<=iComp2 : iComp1<iComp2;break;
						case '!': bEvaluate=iComp1!=iComp2;break;						
					}
				} break;
			case DD_FLOAT:
				{
					// Get data for comparison
					char *pszTemp;
					float fComp1=pData->GetFloat(buffer);
					float fComp2=(float)strtod(pszConditionString, &pszTemp);
					pszEnd=pszTemp;
					switch(func[0])
					{
					case '=': bEvaluate=fComp1==fComp2;break;
					case '>': bEvaluate=func[1]=='=' ? fComp1>=fComp2 : fComp1>fComp2;break;
					case '<': bEvaluate=func[1]=='=' ? fComp1<=fComp2 : fComp1<fComp2;break;
					case '!': bEvaluate=fComp1!=fComp2;break;						
					}
				} break;
			case DD_BOOL:
				{
					// Get data for comparison
					bool bComp1=pData->GetBool(buffer);
					bool bComp2=false;
					// booleans can be expressed as strings and numbers
					switch(_toupper(*pszConditionString))
					{
						// Check numeric value 0 - false. Update pszEnd
						case '0':bComp2=false;pszEnd=pszConditionString+1;break;
						// Check numeric value 1 - false. Update pszEnd
						case '1':bComp2=true;pszEnd=pszConditionString+1;break;						
						// Check string "false" and skip 5 characters
						case 'F':bComp2=false;pszEnd=pszConditionString+5;break;
						// Check string "true" and skip 4 characters
						case 'T':bComp2=true;pszEnd=pszConditionString+4;break;
					}
					if(func[0]=='!') bEvaluate=bComp1!=bComp2;
					else bEvaluate=bComp1==bComp2;
				} break;
			case DD_STRING:
				{
					// Get data for comparison
					const char *pszComp1=pData->GetString(buffer);					
					if(*pszConditionString=='\'') 
					{
						++pszConditionString; // Skip ping

						// Search for end quote
						pszEnd=strchr(pszConditionString, '\'');
						if(!pszEnd) 
						{
							pszEnd=strchr(pszConditionString, '$');
							if(!pszEnd) pszEnd=pszConditionString+strlen(pszConditionString);
							else pszEnd--; // We need the string without $
						} else
						{
							pszEnd--; // We need the string without quote
						}
					} else
					{
						pszEnd=strchr(pszConditionString, ' ');
						if(!pszEnd) 
						{
							pszEnd=strchr(pszConditionString, '$');
							if(!pszEnd) pszEnd=pszConditionString+strlen(pszConditionString);
							else pszEnd--; // We need the string without $
						} else
						{
							pszEnd--; // We need the string without space
						}
					}
					// Calulate length and copy to buffer
					iLen=pszEnd-pszConditionString;
					memcpy(buffer, pszConditionString, iLen);
					buffer[iLen]=0;

					if(func[0]=='!') bEvaluate=_stricmp(pszComp1, buffer)!=0;
					else bEvaluate=_stricmp(pszComp1, buffer)==0;
				} break;
		}		

		// Now we have evaluated the expression to true/false
		switch(lastoperator)
		{
			case 0:bEnabled=bEvaluate;break;
			case '&':bEnabled&=bEvaluate;break;
			case '|':bEnabled|=bEvaluate;break;
		}

		// Search for & or | operator
		pszConditionString=pszEnd;
		while(*pszConditionString!=0 && *pszConditionString!='&' && *pszConditionString!='|')
			++pszConditionString;
		if(!*pszConditionString) break;

		// Store this operator
		lastoperator=*pszConditionString;		
	} while(1);	

	if(bEnabled!=m_bEnabled) SetEnabled(bEnabled);
}
MetaControl* MetaBaseCtrl::GetOwner()
{
	return m_pOwner;
}

const char * MetaBaseCtrl::GetMetaID()
{
	return m_sMetaID.c_str();
}

}