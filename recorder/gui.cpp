//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////

#include "gui.h"

DEFINE_EVENT_TYPE(wxEVT_APP_RECORD)
DEFINE_EVENT_TYPE(wxEVT_APP_STOP)
DEFINE_EVENT_TYPE(wxEVT_APP_SAVEAS)
DEFINE_EVENT_TYPE(wxEVT_APP_OPEN)

BEGIN_EVENT_TABLE(Application, wxApp)
	EVT_BUTTON(wxEVT_APP_STOP, Application::OnStop)
	EVT_BUTTON(wxEVT_APP_RECORD, Application::OnRecord)
	EVT_MENU(wxEVT_APP_OPEN, Application::OnOpen)
	EVT_MENU(wxEVT_APP_SAVEAS, Application::OnSaveAs)
END_EVENT_TABLE()

MainFrame::MainFrame(const wxString& title)
       : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(200, 330))
{
  Centre();
}

IMPLEMENT_APP(Application)

bool Application::OnInit()
{
    mainFrame = new MainFrame(wxT("TibiaMovie"));
	mainFrame->SetSize(wxSize(256, 200));
	long lStyle = mainFrame->GetWindowStyle();
	lStyle &= ~wxRESIZE_BORDER;
	mainFrame->SetWindowStyleFlag(lStyle);

	wxPanel* panel = new wxPanel(mainFrame, wxID_ANY);

	wxFont font(10, wxDEFAULT, wxNORMAL, wxBOLD);

	wxButton* btnRecord = new wxButton(panel, wxEVT_APP_RECORD, wxT("Record"), wxPoint(8, 12), wxSize(120, 28));
	btnRecord->Show(true);
	btnRecord->SetFont(font);

	wxButton* btnClose = new wxButton(panel, wxEVT_APP_STOP, wxT("Stop"), wxPoint(120, 12), wxSize(120, 28));
	btnClose->SetFont(font);
	btnClose->Show(true);

	wxStaticText* txtSpeed = new wxStaticText(panel, wxID_ANY, wxT("Speed:"), wxPoint(8, 60));
	txtSpeed->SetFont(font);
	txtSpeed->Show(true);

	wxGauge* gaugeSpeed = new wxGauge(panel, wxID_ANY, 100, wxPoint(70, 60), wxSize(150, 20));
	gaugeSpeed->Show(true);
	gaugeSpeed->SetValue(20);

	wxStaticText* txtPosition = new wxStaticText(panel, wxID_ANY, wxT("Position:"), wxPoint(8, 80));
	txtPosition->SetFont(font);
	txtPosition->Show(true);

	wxStaticText* txtTime = new wxStaticText(panel, wxID_ANY, wxT("Time:"), wxPoint(8, 100));
	txtTime->SetFont(font);
	txtTime->Show(true);

	//Build menu
    wxMenu* fileMenu = new wxMenu;
    wxMenuItem* openFile = fileMenu->Append(wxEVT_APP_OPEN, wxT("&Open...\tCTRL+O"), wxT("Open movie"));
    wxMenuItem* saveFile = fileMenu->Append(wxEVT_APP_SAVEAS, wxT("&Save...\tCTRL+S"), wxT("Save movie"));

	wxMenu* aboutMenu = new wxMenu;
	wxMenu* optionsMenu = new wxMenu;

	menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, wxT("&File"));
    menuBar->Append(optionsMenu, wxT("&Options"));
    menuBar->Append(aboutMenu, wxT("&About"));

    mainFrame->SetMenuBar(menuBar);

	mainFrame->Show(true);
	return true;
}

void Application::OnRecord(wxCommandEvent& WXUNUSED(event))
{
	//
}

void Application::OnStop(wxCommandEvent& WXUNUSED(event))
{
	///
}

void Application::OnSaveAs(wxCommandEvent& WXUNUSED(event))
{
	/*
	if(mode == MODE_PLAYBACK){
		return false;
	}
	*/

	wxFileDialog file(mainFrame, wxT("Save As..."), wxT(""),wxT(""),wxT("*.tmv"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int result = file.ShowModal();

	if(result == wxID_OK){
		//
	}
}

void Application::OnOpen(wxCommandEvent& WXUNUSED(event))
{
	/*
	if(mode == MODE_RECORDING){
		return false;
	}
	*/

	wxFileDialog file(mainFrame, wxT("Open..."), wxT(""),wxT(""),wxT("*.tmv"), wxFD_OPEN);
	int result = file.ShowModal();

	if(result == wxID_OK){
		//
	}

	//inject dll
}
