/*
-------------------------------------------------------------------------
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
-------------------------------------------------------------------------
*/
#include "ScintillaEditor.h"

#include "OgreDataStream.h"

using Ogre::DataStream;
using Ogre::DataStreamPtr;
using Ogre::FileStreamDataStream;

BEGIN_EVENT_TABLE(ScintillaEditor, wxScintilla)
	EVT_SIZE (ScintillaEditor::OnSize)
	// Scintilla
	EVT_SCI_MARGINCLICK (-1, ScintillaEditor::OnMarginClick)
	EVT_SCI_CHARADDED (-1,   ScintillaEditor::OnCharAdded)
	EVT_SCI_UPDATEUI(-1, ScintillaEditor::OnUpdateUI)
END_EVENT_TABLE()

ScintillaEditor::ScintillaEditor(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style)
: wxScintilla (parent, id, pos, size, style), mDirty(false)
{
	setControl(this);
	
	mLineNumID = 0;
	mLineNumMargin = TextWidth(wxSCI_STYLE_LINENUMBER, _T("99999"));
	mFoldingID = 1;
	mFoldingMargin = 16;
	mDividerID = 1;

	SetProperty(wxT("fold"), wxT("1"));
	SetFoldFlags(16);
	SetMarginType(mFoldingID, wxSCI_MARGIN_SYMBOL);
	SetMarginMask(mFoldingID, wxSCI_MASK_FOLDERS);
	SetMarginSensitive(mFoldingID, true);
	SetMarginWidth(mFoldingID, mFoldingMargin);

	MarkerDefine(wxSCI_MARKNUM_FOLDEROPEN, wxSCI_MARK_BOXMINUS);
	MarkerSetForeground(wxSCI_MARKNUM_FOLDEROPEN, wxColour(0xff, 0xff, 0xff));
	MarkerSetBackground(wxSCI_MARKNUM_FOLDEROPEN, wxColour(0x80, 0x80, 0x80));
	MarkerDefine(wxSCI_MARKNUM_FOLDER, wxSCI_MARK_BOXPLUS);
	MarkerSetForeground(wxSCI_MARKNUM_FOLDER, wxColour(0xff, 0xff, 0xff));
	MarkerSetBackground(wxSCI_MARKNUM_FOLDER, wxColour(0x80, 0x80, 0x80));
	MarkerDefine(wxSCI_MARKNUM_FOLDERSUB, wxSCI_MARK_VLINE);
	MarkerSetForeground(wxSCI_MARKNUM_FOLDERSUB, wxColour(0xff, 0xff, 0xff));
	MarkerSetBackground(wxSCI_MARKNUM_FOLDERSUB, wxColour(0x80, 0x80, 0x80));
	MarkerDefine(wxSCI_MARKNUM_FOLDERTAIL, wxSCI_MARK_LCORNER);
	MarkerSetForeground(wxSCI_MARKNUM_FOLDERTAIL, wxColour(0xff, 0xff, 0xff));
	MarkerSetBackground(wxSCI_MARKNUM_FOLDERTAIL, wxColour(0x80, 0x80, 0x80));
	MarkerDefine(wxSCI_MARKNUM_FOLDEREND, wxSCI_MARK_BOXPLUSCONNECTED);
	MarkerSetForeground(wxSCI_MARKNUM_FOLDEREND, wxColour(0xff, 0xff, 0xff));
	MarkerSetBackground(wxSCI_MARKNUM_FOLDEREND, wxColour(0x80, 0x80, 0x80));
	MarkerDefine(wxSCI_MARKNUM_FOLDEROPENMID, wxSCI_MARK_BOXMINUSCONNECTED);
	MarkerSetForeground(wxSCI_MARKNUM_FOLDEROPENMID, wxColour(0xff, 0xff, 0xff));
	MarkerSetBackground(wxSCI_MARKNUM_FOLDEROPENMID, wxColour(0x80, 0x80, 0x80));
	MarkerDefine(wxSCI_MARKNUM_FOLDERMIDTAIL, wxSCI_MARK_TCORNER);
	MarkerSetForeground(wxSCI_MARKNUM_FOLDERMIDTAIL, wxColour(0xff, 0xff, 0xff));
	MarkerSetBackground(wxSCI_MARKNUM_FOLDERMIDTAIL, wxColour(0x80, 0x80, 0x80));

	// Set defaults, these should eventually be set via user prefs
	SetViewEOL(false);
	SetIndentationGuides(false);
	SetMarginWidth(mLineNumID, mLineNumMargin);
	//SetMarginWidth(mFoldingID, mFoldingMargin);
	//SetMarginSensitive(mFoldingID, true);
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

	StyleSetBackground(wxSCI_STYLE_BRACELIGHT, wxColour(wxT("YELLOW")));
	StyleSetBold(wxSCI_STYLE_BRACELIGHT, true);

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
	//MarkerDefine(wxSCI_MARKNUM_FOLDER, wxSCI_MARK_BOXPLUS);
	//MarkerSetBackground(wxSCI_MARKNUM_FOLDER, wxColour(_T("BLACK")));
	//MarkerSetForeground(wxSCI_MARKNUM_FOLDER, wxColour(_T("WHITE")));
	//MarkerDefine(wxSCI_MARKNUM_FOLDEROPEN, wxSCI_MARK_BOXMINUS);
	//MarkerSetBackground(wxSCI_MARKNUM_FOLDEROPEN, wxColour(_T("BLACK")));
	//MarkerSetForeground(wxSCI_MARKNUM_FOLDEROPEN, wxColour(_T("WHITE")));
	//MarkerDefine(wxSCI_MARKNUM_FOLDERSUB, wxSCI_MARK_EMPTY);
	//MarkerDefine(wxSCI_MARKNUM_FOLDEREND, wxSCI_MARK_SHORTARROW);
	//MarkerDefine(wxSCI_MARKNUM_FOLDEROPENMID, wxSCI_MARK_ARROWDOWN);
	//MarkerDefine(wxSCI_MARKNUM_FOLDERMIDTAIL, wxSCI_MARK_EMPTY);
	//MarkerDefine(wxSCI_MARKNUM_FOLDERTAIL, wxSCI_MARK_EMPTY);

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

ScintillaEditor::~ScintillaEditor() 
{
}

void ScintillaEditor::activate()
{
	// TODO: Connect to update ui event
}

void ScintillaEditor::deactivate()
{
	// TODO: Disconnect from update ui event
}

bool ScintillaEditor::isDirty()
{
	return mDirty;
}

void ScintillaEditor::save()
{
	//setDirty(false);
}

void ScintillaEditor::saveAs()
{
}

bool ScintillaEditor::isSaveAsAllowed()
{
	return true;
}

bool ScintillaEditor::isRedoable()
{
	return CanRedo();
}

void ScintillaEditor::redo()
{
	if(!CanRedo()) return;
	
	Redo();
}

bool ScintillaEditor::isUndoable()
{
	return CanUndo();
}

void ScintillaEditor::undo()
{
	if(!CanUndo()) return;
	
	Undo();
}

bool ScintillaEditor::isCuttable()
{
	return GetReadOnly() || (GetSelectionEnd() - GetSelectionStart() <= 0);
}

void ScintillaEditor::cut()
{
	if(GetReadOnly() || (GetSelectionEnd() - GetSelectionStart() <= 0)) return;
	
	Cut();
}

bool ScintillaEditor::isCopyable()
{
	return GetReadOnly() || (GetSelectionEnd() - GetSelectionStart() <= 0);
}

void ScintillaEditor::copy()
{
	if(GetSelectionEnd() - GetSelectionStart() <= 0) return;
	
	Copy();
}

bool ScintillaEditor::isPastable()
{
	return CanPaste();
}

void ScintillaEditor::paste()
{
	if(!CanPaste()) return;
	
	Paste();
}

void ScintillaEditor::loadKeywords(wxString& path)
{
	std::ifstream fp;
	fp.open(path, std::ios::in | std::ios::binary);
	if(fp)
	{
		DataStreamPtr stream(new FileStreamDataStream(path.c_str(), &fp, false));
		
		int index = -1;
		String line;
		wxString keywords;
		while(!stream->eof())
		{
			line = stream->getLine();
			
			// Ignore blank lines and comments (comment lines start with '#')
			if(line.length() > 0 && line.at(0) != '#')
			{
				if(line.at(0) == '[')
				{
					if(index != -1)
					{
						SetKeyWords(index, keywords);
						keywords.clear();
					}
					
					++index;
				}
				else
				{
					keywords.Append(line);
					keywords.Append(" ");
				}
			}
		}
		
		SetKeyWords(index, keywords);
	}
}

wxChar ScintillaEditor::GetLastNonWhitespaceChar(int position /* = -1 */)
{
	if (position == -1)
		position = GetCurrentPos();

	int count = 0; // Used to count the number of blank lines
	bool foundlf = false; // For the rare case of CR's without LF's
	while (position)
	{
		wxChar c = GetCharAt(--position);
		int style = GetStyleAt(position);
		bool inComment = style == wxSCI_C_COMMENT ||
			style == wxSCI_C_COMMENTDOC ||
			style == wxSCI_C_COMMENTDOCKEYWORD ||
			style == wxSCI_C_COMMENTDOCKEYWORDERROR ||
			style == wxSCI_C_COMMENTLINE ||
			style == wxSCI_C_COMMENTLINEDOC;
		if (c == wxT('\n'))
		{
			count++;
			foundlf = true;
		}
		else if (c == wxT('\r') && !foundlf)
			count++;
		else
			foundlf = false;
		if (count > 1) return 0; // Don't over-indent
		if (!inComment && c != wxT(' ') && c != wxT('\t') && c != wxT('\n') && c != wxT('\r'))
			return c;
	}

	return 0;
}

wxString ScintillaEditor::GetLineIndentString(int line)
{
	int currLine = (line == -1) ? LineFromPosition(GetCurrentPos()) : line;

	wxString text = GetLine(currLine);
	int length = (int)text.Length();
	wxString indent;
	for (int i = 0; i < length; ++i)
	{
		if (text[i] == wxT(' ') || text[i] == wxT('\t'))
			indent << text[i];
		else
			break;
	}

	return indent;
}

int ScintillaEditor::FindBlockStart(int position, wxChar blockStart, wxChar blockEnd, bool skipNested /* = true */)
{
	int level = 0;
	wxChar ch = GetCharAt(position);
	while (ch)
	{
		if (ch == blockEnd)
			++level;

		else if (ch == blockStart)
		{
			if (level == 0) return position;
			--level;
		}

		--position;

		ch = GetCharAt(position);
	}

	return -1;
}

void ScintillaEditor::HighlightBraces()
{
	int currPos = GetCurrentPos();
	int newPos = BraceMatch(currPos);
	if (newPos == wxSCI_INVALID_POSITION)
	{
		if(currPos > 0)
			newPos = BraceMatch(--currPos);
	}

	wxChar ch = GetCharAt(currPos);
	if (ch == wxT('{') || ch == wxT('[') || ch == wxT('(') ||
		ch == wxT('}') || ch == wxT(']') || ch == wxT(')'))
	{
		if (newPos != wxSCI_INVALID_POSITION)
		{
			BraceHighlight(currPos, newPos);
		}
		else
		{
			BraceBadLight(currPos);
		}
	}
	else BraceHighlight(-1, -1);

	Refresh(false);
}

//----------------------------------------------------------------------------
// Common event handlers
void ScintillaEditor::OnSize(wxSizeEvent& event)
{
	int x = GetClientSize().x;// + GetMarginLeft(); //mLineNumMargin + mFoldingMargin;

	if (x > 0) SetScrollWidth(x);

	event.Skip();
}


void ScintillaEditor::OnMarginClick(wxScintillaEvent &event)
{
	if (event.GetMargin() == 1)
	{
		int lineClick = LineFromPosition(event.GetPosition());
		int levelClick = GetFoldLevel(lineClick);
		if ((levelClick & wxSCI_FOLDLEVELHEADERFLAG) > 0)
		{
			ToggleFold (lineClick);
		}
	}
}

void ScintillaEditor::OnCharAdded(wxScintillaEvent &event)
{
	char ch = event.GetKey();
	int currentLine = GetCurrentLine();
	int pos = GetCurrentPos();

	if (ch == wxT('\n') && currentLine > 0)
	{
		BeginUndoAction();

		wxString indent = GetLineIndentString(currentLine - 1);

		wxChar b = GetLastNonWhitespaceChar();
		if(b == wxT('{'))
		{
			if(GetUseTabs())
				indent << wxT("\t");
			else
				indent << wxT("    ");
		}

		InsertText(pos, indent);
		GotoPos((int)(pos + indent.Length()));
		ChooseCaretX();

		EndUndoAction();
	}
	else if(ch == wxT('}'))
	{
		BeginUndoAction();

		wxString line = GetLine(currentLine);
		line.Trim(false);
		line.Trim(true);
		if(line.Matches(wxT("}")))
		{
			pos = GetCurrentPos() - 2;
			pos = FindBlockStart(pos, wxT('{'), wxT('}'));

			if(pos != -1)
			{
				wxString indent = GetLineIndentString(LineFromPosition(pos));
				indent << wxT('}');
				DelLineLeft();
				DelLineRight();
				pos = GetCurrentPos();
				InsertText(pos, indent);
				GotoPos((int)(pos + indent.Length()));
				ChooseCaretX();
			}
		}

		EndUndoAction();
	}

	setDirty(true);
}

void ScintillaEditor::OnUpdateUI(wxScintillaEvent &event)
{
	HighlightBraces();
}

void ScintillaEditor::setDirty(const bool dirty)
{
	if(mDirty == dirty) return;
	mDirty = dirty;
	//fireEvent(DirtyStateChanged, new EditorEventArgs(this, getEditorInput()));
}
