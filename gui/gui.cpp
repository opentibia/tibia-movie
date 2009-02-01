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
#include <wx/filename.h>

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

#include "../debug.h"
#include "../codecs/codec.h"
#include "config.h"

MainFrame::MainFrame(const wxString& title)
       : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(200, 330))
{
	Centre();
}

IMPLEMENT_APP(Application)

bool Application::OnInit()
{
	Debug::start("gui_debug.txt");

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
	gaugeSpeed->SetValue(0);

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

void Application::errorMessage(wxString msg)
{
	wxMessageDialog error(mainFrame, msg, wxT("Error"),
		wxOK  | wxICON_ERROR);
	error.ShowModal();
}

int Application::OnExit(){
	Debug::stop();
	return 0;
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
		wxString fileName = file.GetPath();
		Codec* codec = Codec::getCodec(fileName.c_str());
		if(!codec){
			errorMessage(wxT("It is not a valid movie!"));
			return;
		}
		ClientVersion version;
		codec->getClientVersion(version);

		//Locate the right client version
		wxString tibiaPath = Config::getInstance().getClientPath(version);
		if(tibiaPath == wxEmptyString){
			bool versionCheck = false;
			do{
				wxString caption;
				caption.sprintf("Select Tibia %d.%d.%d directory...", version.major, version.minor, version.revision);
				wxDirDialog dir(mainFrame, caption, wxT(""), wxDD_DIR_MUST_EXIST);
				int result = dir.ShowModal();
				if(result == wxID_OK){

					wxFileName wxTibiaExe(dir.GetPath(), wxT("Tibia.exe"));
					wxString tibiaExe = wxTibiaExe.GetFullPath();

					//** Verify that is the client we need
					DWORD dummy;
					DWORD dwSize = GetFileVersionInfoSize(tibiaExe.c_str(), &dummy);
					if(dwSize){
						char* verData = new char[dwSize+1];
						if(GetFileVersionInfo(tibiaExe.c_str(), 0, dwSize, (LPVOID)verData)){
							VS_FIXEDFILEINFO* verQueryData;
							UINT verQuerySize;
							if(VerQueryValue((LPVOID)verData, "\\", (LPVOID*)&verQueryData, &verQuerySize)){
								if(verQuerySize == sizeof(VS_FIXEDFILEINFO)){
									ClientVersion testVersion;
									testVersion.major = HIWORD(verQueryData->dwFileVersionMS);
									testVersion.minor = LOWORD(verQueryData->dwFileVersionMS);
									testVersion.revision = HIWORD(verQueryData->dwFileVersionLS);
									if(memcmp(&testVersion, &version, sizeof(ClientVersion)) == 0){
										versionCheck = true;
										tibiaPath = dir.GetPath();
										Config::getInstance().setClientPath(version, tibiaPath);
									}
									else{
										errorMessage(wxT("This is not requested version!"));
									}
								}
								else{
									errorMessage(wxT("Internal error. Wrong VS_FIXEDFILEINFO size"));
								}
							}
							else{
								errorMessage(wxT("Internal error. VerQueryValue"));
							}
						}
						else{
							errorMessage(wxT("Tibia.exe not found!"));
						}
						delete[] verData;
					}
					//**
				}
				else{
					//The user could not found the client
					delete codec;
					return;
				}
			}while(!versionCheck);
		}

		uint32_t totalTime = codec->getTotalTime();
		uint32_t codecName = codec->getName();
		delete codec;

		//start client
		PROCESS_INFORMATION PInfo;
		STARTUPINFO SInfo;
		memset(&PInfo, 0, sizeof(PROCESS_INFORMATION));
		memset(&SInfo, 0, sizeof(STARTUPINFO));

		wxFileName wxTibiaExe(tibiaPath, wxT("Tibia.exe"));
		wxString tibiaExe = wxTibiaExe.GetFullPath();
		int created = CreateProcess(NULL, tibiaExe.char_str(), NULL, NULL, FALSE,
							CREATE_SUSPENDED, NULL, tibiaPath.c_str(), &SInfo, &PInfo);
		if(!created){
			errorMessage(wxT("Error starting Tibia.exe!"));
			return;
		}
		HANDLE process = PInfo.hProcess;
		ResumeThread(PInfo.hThread);
		//wait....
		int timeout = 100;
		HWND tibiaWindow = NULL;
		while(1){
			Sleep(100);

			//check client window
			//FIX. This fails when there are more than 1 client running
			tibiaWindow = FindWindow("TibiaClient", "Tibia");
			if(tibiaWindow){
				break;
			}

			//avoid infinite loop
			timeout--;
			if(timeout == 0){
				break;
			}

			//check that the process is still alive
			DWORD exitCode;
			bool ret = GetExitCodeProcess(process, &exitCode);
			if(!ret || exitCode != STILL_ACTIVE){
				break;
			}
		}
		if(!tibiaWindow){
			//something went wrong while waiting
			errorMessage(wxT("Error waiting Tibia.exe"));
			TerminateProcess(process, 0);
			return;
		}
		//suspend before injecting the dll
		Sleep(500);
		SuspendThread(PInfo.hThread);
		SetWindowText(tibiaWindow, "Tibia - TibiaMovie");

		//setup player options
		RecordOptions options;
		options.client = version;
		options.codecName = codecName;
		options.codecOptions = 0; //ignored in player mode
		if(fileName.size() > sizeof(options.fileName)-1){
			errorMessage(wxT("Too long path"));
			TerminateProcess(process, 0);
			return;
		}
		strcpy(options.fileName, fileName.c_str());

		//and copy player options
		HANDLE hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
			0, sizeof(options), "TibiaMovie1");
		if(!hFileMapping){
			errorMessage(wxT("Internal error. CreateFileMapping"));
			TerminateProcess(process, 0);
			return;
		}
		LPVOID m_pvData = MapViewOfFile(hFileMapping, FILE_MAP_WRITE, 0, 0, 0);
		if(!m_pvData){
			errorMessage(wxT("Internal error. MapViewOfFile"));
			CloseHandle(hFileMapping);
			TerminateProcess(process, 0);
			return;
		}
		memcpy(m_pvData, &options, sizeof(options));

		//inject dll
		wxFileName wxTBPlayerDLL(wxGetCwd(), "tbplay.dll");
		wxString library = wxTBPlayerDLL.GetFullPath();
		PVOID mem = VirtualAllocEx(process, NULL, strlen(library.c_str()) + 1, MEM_COMMIT, PAGE_READWRITE);
		if(mem == NULL){
			errorMessage(wxT("Internal error. VirtualAllocEx"));
			TerminateProcess(process, 0);
			UnmapViewOfFile(m_pvData);
			CloseHandle(hFileMapping);
			return ;
		}
		if(WriteProcessMemory(process, mem, (void*)library.char_str(), strlen(library) + 1, NULL) == 0){
			errorMessage(wxT("Internal error. WriteProcessMemory"));
			TerminateProcess(process, 0);
			UnmapViewOfFile(m_pvData);
			CloseHandle(hFileMapping);
			return;
		}

		HANDLE hThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE) GetProcAddress(GetModuleHandle("KERNEL32.DLL"),"LoadLibraryA"), mem, 0, NULL);
		if(hThread == INVALID_HANDLE_VALUE){
			errorMessage(wxT("Internal error. CreateRemoteThread"));
			TerminateProcess(process, 0);
			UnmapViewOfFile(m_pvData);
			CloseHandle(hFileMapping);
			return;
		}
		if(WaitForSingleObject(hThread, 10*1000) != WAIT_OBJECT_0){
			errorMessage(wxT("Internal error. WaitForSingleObject"));
			TerminateProcess(process, 0);
			UnmapViewOfFile(m_pvData);
			CloseHandle(hFileMapping);
			return;
		}

		HANDLE hLibrary = NULL;
		if(!GetExitCodeThread(hThread, (LPDWORD)&hLibrary)){
			errorMessage(wxT("Internal error. loading tbplay.dll - 1"));
			TerminateProcess(process, 0);
			UnmapViewOfFile(m_pvData);
			CloseHandle(hFileMapping);
			return;
		}
		CloseHandle(hThread);
		VirtualFreeEx(process, mem, strlen(library) + 1, MEM_RELEASE);

		//free file mapping
		UnmapViewOfFile(m_pvData);
		CloseHandle(hFileMapping);

		if(!hLibrary){
			errorMessage(wxT("Internal error. loading tbplay.dll - 2"));
			TerminateProcess(process, 0);
			UnmapViewOfFile(m_pvData);
			CloseHandle(hFileMapping);
			return;
		}

		ResumeThread(PInfo.hThread);
	}
}
