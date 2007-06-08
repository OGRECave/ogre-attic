#include "LogPanel.h"

#include <wx/sizer.h>
#include <wx/textctrl.h>

#include "OgreString.h"
#include "LogToTextRedirector.h"


LogPanel::LogPanel(wxWindow* parent, wxWindowID id /* = wxID_ANY */, const wxPoint& pos /* = wxDefaultPosition */, const wxSize& size /* = wxDeafultSize */, long style /* = wxTAB_TRAVERSAL */, const wxString& name /* =  */)
: wxPanel(parent, id, pos, size, style, name)
{
	mBoxSizer = new wxBoxSizer(wxVERTICAL);
	
	//mToolPanel = new wxPanel(this);
	//mBoxSizer->Add(mToolPanel, 0, wxEXPAND | wxALL, 5);
    //mLogComboBox = new wxComboBox(mToolPanel, wxID_ANY);
	//mMenuButton = new wxBitmapButton(mToolPanel, wxID_ANY, wxNullBitmap);
	//mClearButton = new wxBitmapButton(mToolPanel, wxID_ANY, wxNullBitmap);
	//mWordWrapButton = new wxBitmapButton(mToolPanel, wxID_ANY, wxNullBitmap);
	
	mTextControl = new wxTextCtrl(this, -1, _(""), wxDefaultPosition, wxSize(200,150), wxNO_BORDER | wxTE_MULTILINE);
	mTextControl->SetEditable(false);
	mBoxSizer->Add(mTextControl, 1, wxEXPAND | wxALL, 5);

	SetSizer(mBoxSizer);
	Layout();

	mRedirector = NULL;
}

LogPanel::~LogPanel()
{
	if(mRedirector)
	{
		delete mRedirector;
		mRedirector = NULL;
	}
}

void LogPanel::setLog(const String& logName)
{
	mTextControl->Clear();

	if(mRedirector) delete mRedirector;
	mRedirector = new LogToTextRedirector(mTextControl, logName);
}

/*
void LogPanel::addLog(const String& logName)
{
	if(mRedirector) delete mRedirector;
	
	mRedirector = new LogToTextRedirector(mTextControl, logName);
	mTextControl->Clear();
}

void LogPanel::removeLog(const String& logName)
{

}
*/