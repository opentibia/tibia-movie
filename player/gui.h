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

#ifndef __TBMV_GUIPLAY__H__
#define __TBMV_GUIPLAY__H__

#include "../common.h"
#include <wx/wx.h>

class MainFrame;

class PlayerApplication : public wxApp
{
public:
    virtual bool OnInit();
    virtual int OnExit();
    void OnKeyDown(wxKeyEvent& event);

    void toggleGUI();

	static PlayerApplication* getInstance(){
		return static_cast<PlayerApplication*>(wxTheApp);
	}

private:
	MainFrame* m_mainFrame;

	DECLARE_EVENT_TABLE()
};

class MainFrame : public wxFrame
{
public:
    MainFrame(const wxString& title);

	void OnCloseWindow(wxCloseEvent& event);

	void OnRewind(wxCommandEvent& event);
	void OnPlay(wxCommandEvent& event);
	void OnPause(wxCommandEvent& event);
	void OnFordward(wxCommandEvent& event);

	void onTimer(wxTimerEvent& event);

protected:
	wxButton* m_button7;
	wxButton* m_button8;
	wxButton* m_button9;
	wxButton* m_button10;
	wxSlider* m_slider2;
	wxStaticText* m_staticText2;

private:
	DECLARE_EVENT_TABLE()
};

#endif
