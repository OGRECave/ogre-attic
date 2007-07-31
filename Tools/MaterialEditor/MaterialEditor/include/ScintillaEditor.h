#ifndef _SCINTILLAEDITOR_H_
#define _SCINTILLAEDITOR_H_

#include <wx/wxscintilla.h>

#include "Editor.h"

class ScintillaEditor : public wxScintilla, Editor
{
public:
	ScintillaEditor(wxWindow* parent, wxWindowID id = -1,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxVSCROLL
		);

	~ScintillaEditor();

	virtual bool isDirty();

	void OnSize(wxSizeEvent &event);
	void OnMarginClick(wxScintillaEvent &event);
	void OnCharAdded(wxScintillaEvent &event);
	void OnUpdateUI(wxScintillaEvent &event);

protected:
	wxChar GetLastNonWhitespaceChar(int position = -1);
	wxString GetLineIndentString(int line);
	int FindBlockStart(int position, wxChar blockStart, wxChar blockEnd, bool skipNested = true);

	void HighlightBraces();

	void setDirty(bool dirty);

	wxScintilla* mScintilla;
	bool mDirty;

private:
	// File
	wxString mFileName;

	// Margin variables
	int mLineNumID;
	int mLineNumMargin;
	int mFoldingID;
	int mFoldingMargin;
	int mDividerID;

	DECLARE_EVENT_TABLE()
};

#endif // _SCINTILLAEDITOR_H_

