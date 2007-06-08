#ifndef _LOGPANEL_H_
#define _LOGPANEL_H_

//#include <wx/bmpbuttn.h>
#include <wx/panel.h>

#include "OgreString.h"

class wxBoxSizer;
class wxTextCtrl;
class LogToTextRedirector;

using Ogre::String;

class LogPanel : public wxPanel
{
public:
	LogPanel(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = "LogPanel");
	~LogPanel();

	void setLog(const String& logName);

protected:
	wxBoxSizer* mBoxSizer;
	//wxPanel* mToolPanel;
	//wxComboBox* mLogComboBox;
	//wxBitmapButton* mMenuButton;
	//wxBitmapButton* mClearButton;
	//wxBitmapButton* mWordWrapButton;
	wxTextCtrl* mTextControl;
	LogToTextRedirector* mRedirector;
};

#endif // _LOGPANEL_H_