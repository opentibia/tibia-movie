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

#include <wx/wx.h>

extern const wxEventType wxEVT_APP_RECORD;
extern const wxEventType wxEVT_APP_STOP;
extern const wxEventType wxEVT_APP_SAVEAS;
extern const wxEventType wxEVT_APP_OPEN;
extern const wxEventType wxEVT_MAIN_FRAME_SAVE_FILE;

class MainFrame;

class Application : public wxApp
{
public:
    virtual bool OnInit();

	void OnRecord(wxCommandEvent&);
	void OnStop(wxCommandEvent&);
	void OnOpen(wxCommandEvent& WXUNUSED(event));
	void OnSaveAs(wxCommandEvent& WXUNUSED(event));

private:
	MainFrame* mainFrame;
	wxMenuBar* menuBar;

	DECLARE_EVENT_TABLE()
};

class MainFrame : public wxFrame
{
public:
    MainFrame(const wxString& title);

};
