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
#include "../debug.h"
#include "server.h"

BEGIN_EVENT_TABLE(PlayerApplication, wxApp)
	EVT_KEY_DOWN(PlayerApplication::OnKeyDown)
END_EVENT_TABLE()

bool PlayerApplication::OnInit()
{
	m_mainFrame = new MainFrame(wxT("TibiaMovie"));
	m_mainFrame->Show(true);
	return true;
}

int PlayerApplication::OnExit()
{
	wxApp::CleanUp();
	return 0;
}

void PlayerApplication::toggleGUI()
{
	if(!m_mainFrame->IsShown()){
		Debug::printf(DEBUG_NOTICE, "show window\n");
		m_mainFrame->Show(true);
	}
	else{
		Debug::printf(DEBUG_NOTICE, "hide window\n");
		m_mainFrame->Show(false);
	}
}

void PlayerApplication::OnKeyDown(wxKeyEvent& event)
{
	if(event.GetKeyCode() == 'S' && event.ControlDown()){
		m_mainFrame->Show(false);
		return;
	}
	event.Skip();
}


DEFINE_EVENT_TYPE(wxEVT_REWIND)
DEFINE_EVENT_TYPE(wxEVT_PLAY)
DEFINE_EVENT_TYPE(wxEVT_PAUSE)
DEFINE_EVENT_TYPE(wxEVT_FORDWARD)
DEFINE_EVENT_TYPE(wxEVT_UPDATE_TIMER)

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_CLOSE(MainFrame::OnCloseWindow)
	EVT_BUTTON(wxEVT_REWIND, MainFrame::OnRewind)
	EVT_BUTTON(wxEVT_PLAY, MainFrame::OnPlay)
	EVT_BUTTON(wxEVT_PAUSE, MainFrame::OnPause)
	EVT_BUTTON(wxEVT_FORDWARD, MainFrame::OnFordward)
	EVT_TIMER(wxEVT_UPDATE_TIMER, MainFrame::onTimer)
END_EVENT_TABLE()

MainFrame::MainFrame(const wxString& title)
	: wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(350, 100),
		wxTAB_TRAVERSAL | wxCAPTION | wxCLIP_CHILDREN | wxSTAY_ON_TOP |
		wxFRAME_TOOL_WINDOW)
{
	wxTimer* timer = new wxTimer(this, wxEVT_UPDATE_TIMER);
	timer->Start(500);

	wxPanel* panel = new wxPanel(this, wxID_ANY);

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer3->SetFlexibleDirection( wxBOTH );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxFlexGridSizer* fgSizer5;
	fgSizer5 = new wxFlexGridSizer( 1, 4, 0, 0 );
	fgSizer5->SetFlexibleDirection( wxBOTH );
	fgSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_button7 = new wxButton( panel, wxEVT_REWIND, wxT("Rewind"), wxDefaultPosition, wxDefaultSize, 0 );
	m_button7->Enable(false);
	fgSizer5->Add( m_button7, 0, wxALL, 5 );

	m_button8 = new wxButton( panel, wxEVT_PLAY, wxT("Play"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer5->Add( m_button8, 0, wxALL, 5 );

	m_button9 = new wxButton( panel, wxEVT_PAUSE, wxT("Pause"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer5->Add( m_button9, 0, wxALL, 5 );

	m_button10 = new wxButton( panel, wxEVT_FORDWARD, wxT("Forward"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer5->Add( m_button10, 0, wxALL, 5 );

	fgSizer3->Add( fgSizer5, 1, wxEXPAND, 5 );

	wxFlexGridSizer* fgSizer6;
	fgSizer6 = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizer6->SetFlexibleDirection( wxBOTH );
	fgSizer6->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_slider2 = new wxSlider( panel, wxID_ANY, 0, 0, 100, wxDefaultPosition, wxSize( 250,-1 ), wxSL_HORIZONTAL );
	fgSizer6->Add( m_slider2, 0, wxALL, 5 );

	m_staticText2 = new wxStaticText( panel, wxID_ANY, wxT("Time"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	fgSizer6->Add( m_staticText2, 0, wxALL, 5 );

	fgSizer3->Add( fgSizer6, 1, wxEXPAND, 5 );

	panel->SetSizer( fgSizer3 );
	panel->Layout();

	Centre();
}

void MainFrame::OnRewind(wxCommandEvent& event)
{
	//
}

void MainFrame::OnPlay(wxCommandEvent& event)
{
	PlayerStatus::getInstance().setPlaySpeed(1.f);
}

void MainFrame::OnPause(wxCommandEvent& event)
{
	PlayerStatus::getInstance().setPlaySpeed(0.f);
}

void MainFrame::OnFordward(wxCommandEvent& event)
{
	PlayerStatus::getInstance().setPlaySpeed(5.f);
}

void MainFrame::onTimer(wxTimerEvent& event)
{
	wxTimeSpan currentTime = wxTimeSpan::Milliseconds(PlayerStatus::getInstance().getCurrentTime());
	wxTimeSpan totalTime = wxTimeSpan::Milliseconds(PlayerStatus::getInstance().getTotalTime());

	wxString label = currentTime.Format("%M:%S") + "/" + totalTime.Format("%M:%S");
	m_staticText2->SetLabel(label);

	if(totalTime.GetMilliseconds().ToLong() != 0){
		m_slider2->SetValue(100*currentTime.GetMilliseconds().ToLong()/totalTime.GetMilliseconds().ToLong());
	}
}

void MainFrame::OnCloseWindow(wxCloseEvent& event)
{
	if(event.CanVeto()){
		event.Veto();
		Hide();
	}
	else{
		Destroy();
	}
}
