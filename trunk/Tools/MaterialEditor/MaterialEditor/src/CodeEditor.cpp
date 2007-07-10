/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License (LGPL) as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA or go to
http://www.gnu.org/copyleft/lesser.txt
-------------------------------------------------------------------------*/
// For compilers that support precompilation, includes <wx/wx.h>.
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/file.h>
#include <wx/filename.h>

#include "CodeEditor.h"

#include "SharedDefs.h"

BEGIN_EVENT_TABLE(CodeEditor, wxScintilla)
EVT_SIZE (CodeEditor::OnSize)
// Edit
EVT_MENU (wxID_CLEAR,		CodeEditor::OnEditClear)
EVT_MENU (wxID_CUT,			CodeEditor::OnEditCut)
EVT_MENU (wxID_COPY,		CodeEditor::OnEditCopy)
EVT_MENU (wxID_PASTE,		CodeEditor::OnEditPaste)
EVT_MENU (ID_INDENTINC,		CodeEditor::OnEditIndentInc)
EVT_MENU (ID_INDENTRED,		CodeEditor::OnEditIndentRed)
EVT_MENU (wxID_SELECTALL,	CodeEditor::OnEditSelectAll)
EVT_MENU (ID_SELECTLINE,	CodeEditor::OnEditSelectLine)
EVT_MENU (wxID_REDO,		CodeEditor::OnEditRedo)
EVT_MENU (wxID_UNDO,		CodeEditor::OnEditUndo)
// Find
EVT_MENU (wxID_FIND,		CodeEditor::OnFind)
EVT_MENU (ID_FINDNEXT,		CodeEditor::OnFindNext)
EVT_MENU (ID_REPLACE,		CodeEditor::OnReplace)
EVT_MENU (ID_REPLACENEXT,	CodeEditor::OnReplaceNext)
EVT_MENU (ID_BRACEMATCH,	CodeEditor::OnBraceMatch)
EVT_MENU (ID_GOTO,			CodeEditor::OnGoto)
// View
EVT_MENU_RANGE (ID_HILIGHTFIRST, ID_HILIGHTLAST, CodeEditor::OnHilightLang)
EVT_MENU (ID_DISPLAYEOL,	CodeEditor::OnDisplayEOL)
EVT_MENU (ID_INDENTGUIDE,	CodeEditor::OnIndentGuide)
EVT_MENU (ID_LINENUMBER,	CodeEditor::OnLineNumber)
EVT_MENU (ID_LONGLINEON,	CodeEditor::OnLongLineOn)
EVT_MENU (ID_WHITESPACE,	CodeEditor::OnWhiteSpace)
EVT_MENU (ID_FOLDTOGGLE,	CodeEditor::OnFoldToggle)
EVT_MENU (ID_OVERTYPE,		CodeEditor::OnSetOverType)
EVT_MENU (ID_READONLY,		CodeEditor::OnSetReadOnly)
EVT_MENU (ID_WRAPMODEON,	CodeEditor::OnWrapModeOn)
EVT_MENU (ID_CHARSETANSI,	CodeEditor::OnUseCharset)
EVT_MENU (ID_CHARSETMAC,	CodeEditor::OnUseCharset)
// Extra
EVT_MENU (ID_CHANGELOWER,	CodeEditor::OnChangeCase)
EVT_MENU (ID_CHANGEUPPER,	CodeEditor::OnChangeCase)
EVT_MENU (ID_CONVERTCR,		CodeEditor::OnConvertEOL)
EVT_MENU (ID_CONVERTCRLF,	CodeEditor::OnConvertEOL)
EVT_MENU (ID_CONVERTLF,		CodeEditor::OnConvertEOL)
// scintilla
EVT_SCI_MARGINCLICK (-1, CodeEditor::OnMarginClick)
EVT_SCI_CHARADDED (-1,   CodeEditor::OnCharAdded)
END_EVENT_TABLE()

CodeEditor::CodeEditor(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style)
	: wxScintilla (parent, id, pos, size, style)
{
	mFileName = _T("");
	//m_language = NULL;

	mLineNumID = 0;
	mLineNumMargin = TextWidth(wxSCI_STYLE_LINENUMBER, _T("99999"));
	mFoldingID = 1;
	mFoldingMargin = 16;
	mDividerID = 1;

	// Set defaults, these should eventually be set via user prefs
	SetViewEOL(false);
	SetIndentationGuides(false);
	SetMarginWidth(mLineNumID, mLineNumMargin);
	SetEdgeMode(wxSCI_EDGE_LINE);
	//SetViewWhiteSpace(wxSCI_WS_VISIBLEALWAYS);
	SetOvertype(false);
	SetReadOnly(false);
	SetWrapMode(wxSCI_WRAP_NONE);

	wxFont font(10, wxTELETYPE, wxNORMAL, wxNORMAL);
	StyleSetFont(wxSCI_STYLE_DEFAULT, font);
	StyleSetForeground(wxSCI_STYLE_DEFAULT, wxColour(wxT("BLACK")));
	StyleSetBackground(wxSCI_STYLE_DEFAULT, wxColour(wxT("WHITE")));
	StyleSetForeground(wxSCI_STYLE_LINENUMBER, wxColour(wxT("DARK BLUE")));
	StyleSetBackground(wxSCI_STYLE_LINENUMBER, wxColour(wxT("WHITE")));
	StyleSetForeground(wxSCI_STYLE_INDENTGUIDE, wxColour(wxT("DARK GREY")));
	//InitializePrefs(DEFAULT_LANGUAGE);
	SetTabWidth(4);
	SetUseTabs(false);
	SetTabIndents(true);
	SetBackSpaceUnIndents(true);
	SetIndent(4);

	// Set visibility
	SetVisiblePolicy(wxSCI_VISIBLE_STRICT | wxSCI_VISIBLE_SLOP, 1);
	SetXCaretPolicy(wxSCI_CARET_EVEN | wxSCI_VISIBLE_STRICT | wxSCI_CARET_SLOP, 1);
	SetYCaretPolicy(wxSCI_CARET_EVEN | wxSCI_VISIBLE_STRICT | wxSCI_CARET_SLOP, 1);

	SetCaretLineVisible(true);
	SetCaretLineBackground(wxColour(225, 235, 224));

	// Markers
	MarkerDefine(wxSCI_MARKNUM_FOLDER, wxSCI_MARK_BOXPLUS);
	MarkerSetBackground(wxSCI_MARKNUM_FOLDER, wxColour(_T("BLACK")));
	MarkerSetForeground(wxSCI_MARKNUM_FOLDER, wxColour(_T("WHITE")));
	MarkerDefine(wxSCI_MARKNUM_FOLDEROPEN, wxSCI_MARK_BOXMINUS);
	MarkerSetBackground(wxSCI_MARKNUM_FOLDEROPEN, wxColour(_T("BLACK")));
	MarkerSetForeground(wxSCI_MARKNUM_FOLDEROPEN, wxColour(_T("WHITE")));
	MarkerDefine(wxSCI_MARKNUM_FOLDERSUB, wxSCI_MARK_EMPTY);
	MarkerDefine(wxSCI_MARKNUM_FOLDEREND, wxSCI_MARK_SHORTARROW);
	MarkerDefine(wxSCI_MARKNUM_FOLDEROPENMID, wxSCI_MARK_ARROWDOWN);
	MarkerDefine(wxSCI_MARKNUM_FOLDERMIDTAIL, wxSCI_MARK_EMPTY);
	MarkerDefine(wxSCI_MARKNUM_FOLDERTAIL, wxSCI_MARK_EMPTY);

	// Clear wrong default keys
#if !defined(__WXGTK__)
	//CmdKeyClear(wxSCI_KEY_TAB, 0);
	CmdKeyClear(wxSCI_KEY_TAB, wxSCI_SCMOD_SHIFT);
#endif
	CmdKeyClear('A', wxSCI_SCMOD_CTRL);
#if !defined(__WXGTK__)
	CmdKeyClear('C', wxSCI_SCMOD_CTRL);
#endif
	CmdKeyClear('D', wxSCI_SCMOD_CTRL);
	CmdKeyClear('D', wxSCI_SCMOD_SHIFT | wxSCI_SCMOD_CTRL);
	CmdKeyClear('F', wxSCI_SCMOD_ALT | wxSCI_SCMOD_CTRL);
	CmdKeyClear('L', wxSCI_SCMOD_CTRL);
	CmdKeyClear('L', wxSCI_SCMOD_SHIFT | wxSCI_SCMOD_CTRL);
	CmdKeyClear('T', wxSCI_SCMOD_CTRL);
	CmdKeyClear('T', wxSCI_SCMOD_SHIFT | wxSCI_SCMOD_CTRL);
	CmdKeyClear('U', wxSCI_SCMOD_CTRL);
	CmdKeyClear('U', wxSCI_SCMOD_SHIFT | wxSCI_SCMOD_CTRL);
#if !defined(__WXGTK__)
	CmdKeyClear('V', wxSCI_SCMOD_CTRL);
	CmdKeyClear('X', wxSCI_SCMOD_CTRL);
#endif
	CmdKeyClear('Y', wxSCI_SCMOD_CTRL);
#if !defined(__WXGTK__)
	CmdKeyClear('Z', wxSCI_SCMOD_CTRL);
#endif

	UsePopUp(0);
	SetLayoutCache(wxSCI_CACHE_PAGE);
	SetBufferedDraw(1);
}

CodeEditor::~CodeEditor() 
{

}

//----------------------------------------------------------------------------
// Common event handlers
void CodeEditor::OnSize(wxSizeEvent& event)
{
	int x = GetClientSize().x + mLineNumMargin + mFoldingMargin;

	if (x > 0) SetScrollWidth(x);

	event.Skip();
}

// Edit event handlers
void CodeEditor::OnEditRedo(wxCommandEvent &WXUNUSED(event))
{
	if (!CanRedo()) return;

	Redo();
}

void CodeEditor::OnEditUndo(wxCommandEvent &WXUNUSED(event))
{
	if (!CanUndo()) return;

	Undo();
}

void CodeEditor::OnEditClear(wxCommandEvent &WXUNUSED(event))
{
	if (GetReadOnly()) return;

	Clear();
}

void CodeEditor::OnEditCut(wxCommandEvent &WXUNUSED(event))
{
	if (GetReadOnly() || (GetSelectionEnd()-GetSelectionStart() <= 0)) return;

	Cut();
}

void CodeEditor::OnEditCopy(wxCommandEvent &WXUNUSED(event))
{
	if (GetSelectionEnd()-GetSelectionStart() <= 0) return;

	Copy();
}

void CodeEditor::OnEditPaste(wxCommandEvent &WXUNUSED(event))
{
	if(!CanPaste()) return;

	Paste();
}

void CodeEditor::OnFind(wxCommandEvent &WXUNUSED(event))
{
}

void CodeEditor::OnFindNext(wxCommandEvent &WXUNUSED(event))
{
}

void CodeEditor::OnReplace(wxCommandEvent &WXUNUSED(event))
{
}

void CodeEditor::OnReplaceNext(wxCommandEvent &WXUNUSED(event))
{
}

void CodeEditor::OnBraceMatch(wxCommandEvent &WXUNUSED(event))
{
	int min = GetCurrentPos();
	int max = BraceMatch(min);
	if (max > (min+1))
	{
		BraceHighlight(min + 1, max);
		SetSelection(min + 1, max);
	}
	else
	{
		BraceBadLight(min);
	}
}

void CodeEditor::OnGoto(wxCommandEvent &WXUNUSED(event))
{
}

void CodeEditor::OnEditIndentInc(wxCommandEvent &WXUNUSED(event))
{
	CmdKeyExecute(wxSCI_CMD_TAB);
}

void CodeEditor::OnEditIndentRed(wxCommandEvent &WXUNUSED(event))
{
	CmdKeyExecute(wxSCI_CMD_DELETEBACK);
}

void CodeEditor::OnEditSelectAll(wxCommandEvent &WXUNUSED(event))
{
	SetSelection(0, GetLength());
}

void CodeEditor::OnEditSelectLine(wxCommandEvent &WXUNUSED(event))
{
	int lineStart = PositionFromLine(GetCurrentLine());
	int lineEnd = PositionFromLine(GetCurrentLine() + 1);
	SetSelection(lineStart, lineEnd);
}

void CodeEditor::OnHilightLang(wxCommandEvent &event)
{
	//InitializePrefs(g_LanguagePrefs [event.GetId() - myID_HILIGHTFIRST].name);
}

void CodeEditor::OnDisplayEOL(wxCommandEvent &WXUNUSED(event))
{
	SetViewEOL (!GetViewEOL());
}

void CodeEditor::OnIndentGuide(wxCommandEvent &WXUNUSED(event))
{
	SetIndentationGuides(!GetIndentationGuides());
}

void CodeEditor::OnLineNumber(wxCommandEvent &WXUNUSED(event))
{
	SetMarginWidth(mLineNumID, GetMarginWidth(mLineNumID) == 0 ? mLineNumMargin : 0);
}

void CodeEditor::OnLongLineOn(wxCommandEvent &WXUNUSED(event))
{
	SetEdgeMode(GetEdgeMode() == 0? wxSCI_EDGE_LINE: wxSCI_EDGE_NONE);
}

void CodeEditor::OnWhiteSpace(wxCommandEvent &WXUNUSED(event))
{
	SetViewWhiteSpace (GetViewWhiteSpace() == 0 ? wxSCI_WS_VISIBLEALWAYS : wxSCI_WS_INVISIBLE);
}

void CodeEditor::OnFoldToggle(wxCommandEvent &WXUNUSED(event))
{
	ToggleFold(GetFoldParent(GetCurrentLine()));
}

void CodeEditor::OnSetOverType(wxCommandEvent &WXUNUSED(event))
{
	SetOvertype(!GetOvertype());
}

void CodeEditor::OnSetReadOnly(wxCommandEvent &WXUNUSED(event))
{
	SetReadOnly(!GetReadOnly());
}

void CodeEditor::OnWrapModeOn(wxCommandEvent &WXUNUSED(event))
{
	SetWrapMode(GetWrapMode() == 0 ? wxSCI_WRAP_WORD : wxSCI_WRAP_NONE);
}

void CodeEditor::OnUseCharset(wxCommandEvent &event)
{
	/*
	int Nr;
	int charset = GetCodePage();

	switch (event.GetId())
	{
		case ID_CHARSETANSI: { charset = wxSCI_CHARSET_ANSI; break; }
		case ID_CHARSETMAC: { charset = wxSCI_CHARSET_ANSI; break; }
	}

	for (Nr = 0; Nr < wxSCI_STYLE_LASTPREDEFINED; Nr++)
	{
		StyleSetCharacterSet(Nr, charset);
	}

	SetCodePage(charset);
	*/
}

void CodeEditor::OnChangeCase(wxCommandEvent &event)
{
	int id = event.GetId();
	if(id == ID_CHANGELOWER) CmdKeyExecute(wxSCI_CMD_LOWERCASE);
	else if(id == ID_CHANGEUPPER) CmdKeyExecute(wxSCI_CMD_UPPERCASE);
}

void CodeEditor::OnConvertEOL(wxCommandEvent &event)
{
	int eolMode = GetEOLMode();

	const int id = event.GetId();
	if(id == ID_CONVERTCR) eolMode = wxSCI_EOL_CR;
	else if(id == ID_CONVERTCRLF) eolMode = wxSCI_EOL_CRLF;
	else if(id == ID_CONVERTLF) eolMode = wxSCI_EOL_LF;

	ConvertEOLs(eolMode);
	SetEOLMode(eolMode);
}

void CodeEditor::OnMarginClick(wxScintillaEvent &event)
{
	if (event.GetMargin() == 2)
	{
		int lineClick = LineFromPosition(event.GetPosition());
		int levelClick = GetFoldLevel(lineClick);
		if ((levelClick & wxSCI_FOLDLEVELHEADERFLAG) > 0)
		{
			ToggleFold (lineClick);
		}
	}
}

void CodeEditor::OnCharAdded(wxScintillaEvent &event)
{
	char chr = event.GetKey();
	int currentLine = GetCurrentLine();
	// Change this if support for mac files with \r is needed
	if (chr == '\n')
	{
		int lineInd = 0;
		if (currentLine > 0)
		{
			lineInd = GetLineIndentation(currentLine - 1);
		}

		if (lineInd == 0) return;

		SetLineIndentation (currentLine, lineInd);
		GotoPos(PositionFromLine (currentLine) + lineInd);
	}
}


//////////////////////////////////////////////////////////////////////////////
// private
//////////////////////////////////////////////////////////////////////////////
/*
wxString CodeEditor::DeterminePrefs(const wxString &filename) {

	LanguageInfo const* curInfo;

	// determine language from filepatterns
	int languageNr;
	for (languageNr = 0; languageNr < g_LanguagePrefsSize; languageNr++)
	{
		curInfo = &g_LanguagePrefs [languageNr];
		wxString filepattern = curInfo->filepattern;
		filepattern.Lower();
		while (!filepattern.IsEmpty())
		{
			wxString cur = filepattern.BeforeFirst(';');
			if ((cur == filename) ||
				(cur == (filename.BeforeLast('.') + _T(".*"))) ||
				(cur == (_T("*.") + filename.AfterLast('.')))) {
					return curInfo->name;
			}
			filepattern = filepattern.AfterFirst(';');
		}
	}

	return wxEmptyString;
}

bool CodeEditor::InitializePrefs(const wxString &name)
{
	// initialize styles
	StyleClearAll();
	LanguageInfo const* curInfo = NULL;

	// determine language
	bool found = false;
	int languageNr;
	for (languageNr = 0; languageNr < g_LanguagePrefsSize; languageNr++)
	{
		curInfo = &g_LanguagePrefs[languageNr];
		if (curInfo->name == name)
		{
			found = true;
			break;
		}
	}

	if (!found) return false;

	// set lexer and language
	SetLexer (curInfo->lexer);
	m_language = curInfo;

	// set margin for line numbers
	SetMarginType (mLineNumID, wxSCI_MARGIN_NUMBER);
	StyleSetForeground (wxSCI_STYLE_LINENUMBER, wxColour (_T("DARK GREY")));
	StyleSetBackground (wxSCI_STYLE_LINENUMBER, wxColour (_T("WHITE")));
	SetMarginWidth (mLineNumID,
		g_CommonPrefs.lineNumberEnable? mLineNumMargin: 0);

	// set common styles
	StyleSetForeground (wxSCI_STYLE_DEFAULT, wxColour (_T("DARK GREY")));
	StyleSetForeground (wxSCI_STYLE_INDENTGUIDE, wxColour (_T("DARK GREY")));

	// initialize settings
	if (g_CommonPrefs.syntaxEnable)
	{
		int keywordnr = 0;
		int Nr;
		for (Nr = 0; Nr < STYLE_TYPES_COUNT; Nr++)
		{
			if (curInfo->styles[Nr].type == -1) continue;

			const StyleInfo &curType = g_StylePrefs[curInfo->styles[Nr].type];
			wxFont font(curType.fontsize, wxTELETYPE, wxNORMAL, wxNORMAL, false,curType.fontname);
			StyleSetFont(Nr, font);
			if (curType.foreground)
			{
				StyleSetForeground (Nr, wxColour(curType.foreground));
			}
			if (curType.background)
			{
				StyleSetBackground (Nr, wxColour(curType.background));
			}

			StyleSetBold(Nr, (curType.fontstyle & TOKEN_STYLE_BOLD) > 0);
			StyleSetItalic(Nr, (curType.fontstyle & TOKEN_STYLE_ITALIC) > 0);
			StyleSetUnderline(Nr, (curType.fontstyle & TOKEN_STYLE_UNDERL) > 0);
			StyleSetVisible(Nr, (curType.fontstyle & TOKEN_STYLE_HIDDEN) == 0);
			StyleSetCase(Nr, curType.lettercase);
			const wxChar *pwords = curInfo->styles[Nr].words;
			if (pwords) 
			{
				SetKeyWords(keywordnr, pwords);
				keywordnr += 1;
			}
		}
	}

	// set margin as unused
	SetMarginType(mDividerID, wxSCI_MARGIN_SYMBOL);
	SetMarginWidth(mDividerID, 8);
	SetMarginSensitive(mDividerID, false);

	// folding
	SetMarginType(mFoldingID, wxSCI_MARGIN_SYMBOL);
	SetMarginMask(mFoldingID, wxSCI_MASK_FOLDERS);
	StyleSetBackground(mFoldingID, wxColour(_T("WHITE")));
	SetMarginWidth(mFoldingID, 0);
	SetMarginSensitive(mFoldingID, false);
	if (g_CommonPrefs.foldEnable)
	{
		SetMarginWidth(mFoldingID, curInfo->folds != 0? mFoldingMargin: 0);
		SetMarginSensitive(mFoldingID, curInfo->folds != 0);
		SetProperty(_T("fold"), curInfo->folds != 0? _T("1"): _T("0"));
		SetProperty(_T("fold.comment"), (curInfo->folds & FOLD_TYPE_COMMENT) > 0? _T("1"): _T("0"));
		SetProperty (_T("fold.compact"), (curInfo->folds & FOLD_TYPE_COMPACT) > 0? _T("1"): _T("0"));
		SetProperty (_T("fold.preprocessor"), (curInfo->folds & FOLD_TYPE_PREPROC) > 0? _T("1"): _T("0"));
		SetProperty (_T("fold.html"), (curInfo->folds & FOLD_TYPE_HTML) > 0? _T("1"): _T("0"));
		SetProperty (_T("fold.html.preprocessor"),(curInfo->folds & FOLD_TYPE_HTMLPREP) > 0? _T("1"): _T("0"));
		SetProperty (_T("fold.comment.python"), (curInfo->folds & FOLD_TYPE_COMMENTPY) > 0? _T("1"): _T("0"));
		SetProperty (_T("fold.quotes.python"), (curInfo->folds & FOLD_TYPE_QUOTESPY) > 0? _T("1"): _T("0"));
	}

	SetFoldFlags (wxSCI_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSCI_FOLDFLAG_LINEAFTER_CONTRACTED);

	// set spaces and indention
	SetTabWidth(4);
	SetUseTabs(false);
	SetTabIndents(true);
	SetBackSpaceUnIndents(true);
	SetIndent(g_CommonPrefs.indentEnable ? 4 : 0);

	// others
	SetViewEOL(g_CommonPrefs.displayEOLEnable);
	SetIndentationGuides(g_CommonPrefs.indentGuideEnable);
	SetEdgeColumn(80);
	SetEdgeMode(g_CommonPrefs.longLineOnEnable? wxSCI_EDGE_LINE: wxSCI_EDGE_NONE);
	SetViewWhiteSpace (g_CommonPrefs.whiteSpaceEnable ? wxSCI_WS_VISIBLEALWAYS : wxSCI_WS_INVISIBLE);
	SetOvertype(g_CommonPrefs.overTypeInitial);
	SetReadOnly(g_CommonPrefs.readOnlyInitial);
	SetWrapMode(g_CommonPrefs.wrapModeInitial ? wxSCI_WRAP_WORD : wxSCI_WRAP_NONE);

	return true;
}
*/

bool CodeEditor::LoadFile() 
{
	// Get filname
	if (!mFileName)
	{
		wxFileDialog dlg (this, _T("Open file"), _T(""), _T(""),
			_T("Any file (*)|*"), wxOPEN | wxFILE_MUST_EXIST | wxCHANGE_DIR);
		if (dlg.ShowModal() != wxID_OK) return false;
		mFileName = dlg.GetPath();
	}

	// Load file
	return LoadFile(mFileName);
}

bool CodeEditor::LoadFile(const wxString &filename)
{
	// Load file in edit and clear undo
	if (!filename.IsEmpty()) mFileName = filename;
	if (!wxScintilla::LoadFile(mFileName)) return false;

	// Determine lexer language
	//wxFileName fname(mFileName);
	//InitializePrefs(DeterminePrefs(fname.GetFullName()));

	return true;
}

bool CodeEditor::SaveFile()
{
	// Return if no change
	if (!Modified()) return true;

	// Get file name
	if (!mFileName)
	{
		wxFileDialog dlg (this, _T("Save file"), _T(""), _T(""), _T("Any file (*)|*"),
			wxSAVE | wxOVERWRITE_PROMPT);
		if (dlg.ShowModal() != wxID_OK) return false;
		mFileName = dlg.GetPath();
	}

	// Save file
	return SaveFile(mFileName);
}

bool CodeEditor::SaveFile(const wxString &filename)
{
	// Return if no change
	if (!Modified()) return true;

	return wxScintilla::SaveFile(filename);
}

bool CodeEditor::Modified()
{
	// Return modified state
	return (GetModify() && !GetReadOnly());
}

/*
EditProperties::EditProperties (Edit *edit, long style)
	: wxDialog (edit, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, style | wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	// sets the application title
	SetTitle (_("Properties"));
	wxString text;

	// fullname
	wxBoxSizer *fullname = new wxBoxSizer (wxHORIZONTAL);
	fullname->Add(10, 0);
	fullname->Add(new wxStaticText(this, -1, _("Full filename"),
		wxDefaultPosition, wxSize(80, -1)), 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);

	fullname->Add(new wxStaticText(this, -1, edit->GetFilename()), 0,
		wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

	// text info
	wxGridSizer *textinfo = new wxGridSizer(4, 0, 2);
	textinfo->Add(new wxStaticText(this, -1, _("Language"), wxDefaultPosition, wxSize(80, -1)),
		0, wxALIGN_LEFT |wxALIGN_CENTER_VERTICAL|wxLEFT, 4);
	textinfo->Add (new wxStaticText(this, -1, edit->m_language->name),
		0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	textinfo->Add(new wxStaticText(this, -1, _("Lexer-ID: "), wxDefaultPosition, wxSize(80, -1)),
		0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
	text = wxString::Format(_T("%d"), edit->GetLexer());
	textinfo->Add(new wxStaticText (this, -1, text), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);

	wxString EOLtype = _T("");
	switch (edit->GetEOLMode())
	{
		case wxSCI_EOL_CR: { EOLtype = _T("CR (Unix)"); break; }
		case wxSCI_EOL_CRLF: { EOLtype = _T("CRLF (Windows)"); break; }
		case wxSCI_EOL_LF: { EOLtype = _T("CR (Macintosh)"); break; }
	}

	textinfo->Add(new wxStaticText(this, -1, _("Line endings"), wxDefaultPosition, wxSize(80, -1)),
		0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT, 4);
	textinfo->Add (new wxStaticText (this, -1, EOLtype), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);

	// text info box
	wxStaticBoxSizer *textinfos = new wxStaticBoxSizer(new wxStaticBox(this, -1, _("Informations")), wxVERTICAL);
	textinfos->Add (textinfo, 0, wxEXPAND);
	textinfos->Add (0, 6);

	// statistic
	wxGridSizer *statistic = new wxGridSizer(4, 0, 2);
	statistic->Add (new wxStaticText(this, -1, _("Total lines"), wxDefaultPosition, wxSize(80, -1)),
		0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT, 4);
	text = wxString::Format(_T("%d"), edit->GetLineCount());
	statistic->Add (new wxStaticText (this, -1, text),
		0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	statistic->Add (new wxStaticText (this, -1, _("Total chars"),
		wxDefaultPosition, wxSize(80, -1)),
		0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT, 4);
	text = wxString::Format (_T("%d"), edit->GetTextLength());
	statistic->Add (new wxStaticText (this, -1, text),
		0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	statistic->Add (new wxStaticText (this, -1, _("Current line"),
		wxDefaultPosition, wxSize(80, -1)),
		0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT, 4);
	text = wxString::Format (_T("%d"), edit->GetCurrentLine());
	statistic->Add (new wxStaticText (this, -1, text),
		0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	statistic->Add (new wxStaticText (this, -1, _("Current pos"),
		wxDefaultPosition, wxSize(80, -1)),
		0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT, 4);
	text = wxString::Format (_T("%d"), edit->GetCurrentPos());
	statistic->Add (new wxStaticText (this, -1, text),
		0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);

	// char/line statistics
	wxStaticBoxSizer *statistics = new wxStaticBoxSizer (
		new wxStaticBox (this, -1, _("Statistics")), wxVERTICAL);

	statistics->Add (statistic, 0, wxEXPAND);
	statistics->Add (0, 6);

	// total pane
	wxBoxSizer *totalpane = new wxBoxSizer (wxVERTICAL);
	totalpane->Add (fullname, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
	totalpane->Add (0, 6);
	totalpane->Add (textinfos, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
	totalpane->Add (0, 10);
	totalpane->Add (statistics, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
	totalpane->Add (0, 6);
	wxButton *okButton = new wxButton (this, wxID_OK, _("OK"));
	okButton->SetDefault();
	totalpane->Add (okButton, 0, wxALIGN_CENTER | wxALL, 10);

	SetSizerAndFit (totalpane);

	ShowModal();
}
-------------------------------------------------------------------------
EditPrint::EditPrint (CodeEditor *editor, wxChar *title)
	: wxPrintout(title)
{
	m_edit = edit;
	m_printed = 0;
}

bool EditPrint::OnPrintPage (int page) {

	wxDC *dc = GetDC();
	if (!dc) return false;

	// scale DC
	PrintScaling (dc);

	// print page
	if (page == 1) m_printed = 0;
	m_printed = m_edit->FormatRange (1, m_printed, m_edit->GetLength(),
		dc, dc, m_printRect, m_pageRect);

	return true;
}

bool EditPrint::OnBeginDocument (int startPage, int endPage) {

	if (!wxPrintout::OnBeginDocument (startPage, endPage)) {
		return false;
	}

	return true;
}

void EditPrint::GetPageInfo (int *minPage, int *maxPage, int *selPageFrom, int *selPageTo) {

	// initialize values
	*minPage = 0;
	*maxPage = 0;
	*selPageFrom = 0;
	*selPageTo = 0;

	// scale DC if possible
	wxDC *dc = GetDC();
	if (!dc) return;
	PrintScaling (dc);

	// get print page informations and convert to printer pixels
	wxSize ppiScr;
	GetPPIScreen (&ppiScr.x, &ppiScr.y);
	wxSize page = g_pageSetupData->GetPaperSize();
	page.x = static_cast<int> (page.x * ppiScr.x / 25.4);
	page.y = static_cast<int> (page.y * ppiScr.y / 25.4);
	m_pageRect = wxRect (0,
		0,
		page.x,
		page.y);

	// get margins informations and convert to printer pixels
	int  top = 25; // default 25
	int  bottom = 25; // default 25
	int  left = 20; // default 20
	int  right = 20; // default 20
	wxPoint (top, left) = g_pageSetupData->GetMarginTopLeft();
	wxPoint (bottom, right) = g_pageSetupData->GetMarginBottomRight();
	top = static_cast<int> (top * ppiScr.y / 25.4);
	bottom = static_cast<int> (bottom * ppiScr.y / 25.4);
	left = static_cast<int> (left * ppiScr.x / 25.4);
	right = static_cast<int> (right * ppiScr.x / 25.4);
	m_printRect = wxRect (left,
		top,
		page.x - (left + right),
		page.y - (top + bottom));

	// count pages
	while (HasPage (*maxPage)) {
		m_printed = m_edit->FormatRange (0, m_printed, m_edit->GetLength(),
			dc, dc, m_printRect, m_pageRect);
		*maxPage += 1;
	}
	if (*maxPage > 0) *minPage = 1;
	*selPageFrom = *minPage;
	*selPageTo = *maxPage;
	m_printed = 0;
}

bool EditPrint::HasPage (int WXUNUSED(page)) {

	return (m_printed < m_edit->GetLength());
}

bool EditPrint::PrintScaling (wxDC *dc){

	// check for dc, return if none
	if (!dc) return false;

	// get printer and screen sizing values
	wxSize ppiScr;
	GetPPIScreen (&ppiScr.x, &ppiScr.y);
	if (ppiScr.x == 0) { // most possible guess 96 dpi
		ppiScr.x = 96;
		ppiScr.y = 96;
	}
	wxSize ppiPrt;
	GetPPIPrinter (&ppiPrt.x, &ppiPrt.y);
	if (ppiPrt.x == 0) { // scaling factor to 1
		ppiPrt.x = ppiScr.x;
		ppiPrt.y = ppiScr.y;
	}
	wxSize dcSize = dc->GetSize();
	wxSize pageSize;
	GetPageSizePixels (&pageSize.x, &pageSize.y);

	// set user scale
	float scale_x = (float)(ppiPrt.x * dcSize.x) /
		(float)(ppiScr.x * pageSize.x);
	float scale_y = (float)(ppiPrt.y * dcSize.y) /
		(float)(ppiScr.y * pageSize.y);
	dc->SetUserScale (scale_x, scale_y);

	return true;
}
*/

