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
#include "psapi.h"

DEFINE_EVENT_TYPE(wxEVT_APP_RECORD)
DEFINE_EVENT_TYPE(wxEVT_APP_STOP)
DEFINE_EVENT_TYPE(wxEVT_APP_OPEN)

BEGIN_EVENT_TABLE(Application, wxApp)
	EVT_BUTTON(wxEVT_APP_STOP, Application::OnStop)
	EVT_BUTTON(wxEVT_APP_RECORD, Application::OnRecord)
	EVT_MENU(wxEVT_APP_OPEN, Application::OnOpen)
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
	Debug::start("debug_gui.txt");

    mainFrame = new MainFrame(wxT("TibiaMovie"));
	mainFrame->SetSize(wxSize(256, 200));
	long lStyle = mainFrame->GetWindowStyle();
	lStyle &= ~wxRESIZE_BORDER;
	mainFrame->SetWindowStyleFlag(lStyle);

	wxPanel* panel = new wxPanel(mainFrame, wxID_ANY);

	wxFont font(10, wxDEFAULT, wxNORMAL, wxBOLD);

	wxButton* btnRecord = new wxButton(panel, wxEVT_APP_RECORD, wxT("Record"), wxPoint(8, 12), wxSize(120, 28));
	btnRecord->SetFont(font);
	btnRecord->Show(true);

	wxButton* btnClose = new wxButton(panel, wxEVT_APP_STOP, wxT("Stop"), wxPoint(120, 12), wxSize(120, 28));
	btnClose->SetFont(font);
	btnClose->Enable(false);
	btnClose->Show(true);

	//Build menu
    wxMenu* fileMenu = new wxMenu;
    /*wxMenuItem* openFile = */fileMenu->Append(wxEVT_APP_OPEN, wxT("&Open...\tCTRL+O"), wxT("Open movie"));

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

void Application::OnStop(wxCommandEvent& WXUNUSED(event))
{
	//
}

void Application::OnRecord(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog file(mainFrame, wxT("Save As..."), wxT(""),wxT(""),wxT("*.tmv"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int result = file.ShowModal();

	if(result == wxID_OK){

		wxString fileName = file.GetPath();

		//Find any running client
		HWND tibiaWindow = FindWindow(wxT("TibiaClient"), wxT("Tibia"));
		if(!tibiaWindow){
			errorMessage(wxT("Not running tibia client found"));
			return;
		}
		DWORD processId;
		DWORD threadId = GetWindowThreadProcessId(tibiaWindow, &processId);

		HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, 0,processId);
		if(!process){
			errorMessage(wxT("Cannot open tibia process."));
			return;
		}
		TCHAR buffer[1024];
		if(GetModuleFileNameEx(process, NULL, buffer, 1023) == 0){
			errorMessage(wxT("Cannot determine exe file path."));
			CloseHandle(process);
			return;
		}
		ClientVersion version = getFileVersion(buffer);
		if(version.major == -1){
			CloseHandle(process);
			return;
		}

		HANDLE thread = OpenThread(THREAD_ALL_ACCESS, 0, threadId);
		if(!thread){
			errorMessage(wxT("Cannot open tibia thread."));
			CloseHandle(process);
			return;
		}
		SuspendThread(thread);
		SetWindowText(tibiaWindow, wxT("Tibia - TibiaMovie.Rec"));

		//setup player options
		RecordOptions options;
		options.client = version;
		options.codecName = 'TMV2';
		options.codecOptions = 0; //not compressed
		if(fileName.size() > sizeof(options.fileName)-1){
			errorMessage(wxT("Too long path"));
			CloseHandle(thread);
			CloseHandle(process);
			return;
		}
		strcpy(options.fileName, fileName.mb_str());

		// inject the dll
		if(!injectDll(wxT("tbrecord.dll"), options, process)){
			CloseHandle(thread);
			CloseHandle(process);
			return;
		}
		//and continue
		ResumeThread(thread);
		CloseHandle(thread);
		CloseHandle(process);

		wxMessageDialog error(mainFrame, "Recording..", wxT("Info"),
			wxOK  | wxICON_INFORMATION);
		error.ShowModal();
	}
}

ClientVersion Application::getFileVersion(LPCTSTR file)
{
	ClientVersion version;
	DWORD dummy;
	DWORD dwSize = GetFileVersionInfoSize(file, &dummy);
	if(dwSize){
		char* verData = new char[dwSize+1];
		if(GetFileVersionInfo(file, 0, dwSize, (LPVOID)verData)){
			VS_FIXEDFILEINFO* verQueryData;
			UINT verQuerySize;
			if(VerQueryValue((LPVOID)verData, wxT("\\"), (LPVOID*)&verQueryData, &verQuerySize)){
				if(verQuerySize == sizeof(VS_FIXEDFILEINFO)){
					version.major = HIWORD(verQueryData->dwFileVersionMS);
					version.minor = LOWORD(verQueryData->dwFileVersionMS);
					version.revision = HIWORD(verQueryData->dwFileVersionLS);
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
			errorMessage(wxT("Exe file not found!"));
		}
		delete[] verData;
	}
	return version;
}

bool Application::injectDll(const wxString& dllName, const RecordOptions& options, const HANDLE process)
{
	HANDLE hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
		0, sizeof(options), wxT("TibiaMovie1"));
	if(!hFileMapping){
		errorMessage(wxT("Internal error. CreateFileMapping"));
		return false;
	}
	LPVOID m_pvData = MapViewOfFile(hFileMapping, FILE_MAP_WRITE, 0, 0, 0);
	if(!m_pvData){
		errorMessage(wxT("Internal error. MapViewOfFile"));
		CloseHandle(hFileMapping);
		return false;
	}
	memcpy(m_pvData, &options, sizeof(options));

	//inject dll
	wxFileName wxTBPlayerDLL(wxGetCwd(), dllName);
	std::string library = std::string(wxTBPlayerDLL.GetFullPath().mb_str());

	PVOID mem = VirtualAllocEx(process, NULL, strlen(library.c_str()) + 1, MEM_COMMIT, PAGE_READWRITE);
	if(mem == NULL){
		errorMessage(wxT("Internal error. VirtualAllocEx"));
		UnmapViewOfFile(m_pvData);
		CloseHandle(hFileMapping);
		return false;
	}
	if(WriteProcessMemory(process, mem, (void*)library.c_str(), strlen(library.c_str()) + 1, NULL) == 0){
		errorMessage(wxT("Internal error. WriteProcessMemory"));
		UnmapViewOfFile(m_pvData);
		CloseHandle(hFileMapping);
		return false;
	}

	HANDLE hThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE) GetProcAddress(GetModuleHandle(wxT("KERNEL32.DLL")), (LPCSTR)wxT("LoadLibraryA")), mem, 0, NULL);
	if(hThread == INVALID_HANDLE_VALUE){
		errorMessage(wxT("Internal error. CreateRemoteThread"));
		UnmapViewOfFile(m_pvData);
		CloseHandle(hFileMapping);
		return false;
	}
	if(WaitForSingleObject(hThread, 10*1000) != WAIT_OBJECT_0){
		errorMessage(wxT("Internal error. WaitForSingleObject"));
		UnmapViewOfFile(m_pvData);
		CloseHandle(hFileMapping);
		return false;
	}

	HANDLE hLibrary = NULL;
	if(!GetExitCodeThread(hThread, (LPDWORD)&hLibrary)){
		errorMessage(wxT("Internal error. loading tbplay.dll - 1"));
		UnmapViewOfFile(m_pvData);
		CloseHandle(hFileMapping);
		return false;
	}
	CloseHandle(hThread);
	VirtualFreeEx(process, mem, strlen(library.c_str()) + 1, MEM_RELEASE);

	//free file mapping
	UnmapViewOfFile(m_pvData);
	CloseHandle(hFileMapping);

	if(!hLibrary){
		errorMessage(wxT("Internal error. loading tbplay.dll - 2"));
		return false;
	}
	return true;
}

void Application::terminateProcess(const PROCESS_INFORMATION& PInfo)
{
	TerminateProcess(PInfo.hProcess, 0);
	CloseHandle(PInfo.hProcess);
	CloseHandle(PInfo.hThread);
}

void Application::OnOpen(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog file(mainFrame, wxT("Open..."), wxT(""),wxT(""),wxT("*.tmv"), wxFD_OPEN);
	int result = file.ShowModal();

	if(result == wxID_OK){
		wxString fileName = file.GetPath();
		Codec* codec = Codec::getCodec(fileName.mb_str());
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
				caption.sprintf(wxT("Select Tibia %d.%d.%d directory..."), version.major, version.minor, version.revision);
				wxDirDialog dir(mainFrame, caption, wxT(""), wxDD_DIR_MUST_EXIST);
				int result = dir.ShowModal();
				if(result == wxID_OK){

					wxFileName wxTibiaExe(dir.GetPath(), wxT("Tibia.exe"));
					wxString tibiaExe = wxTibiaExe.GetFullPath();

					//** Verify that is the client we need
					ClientVersion testVersion = getFileVersion(tibiaExe);
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
					//The user could not found the client
					delete codec;
					return;
				}
			}while(!versionCheck);
		}

		//uint32_t totalTime = codec->getTotalTime();
		uint32_t codecName = codec->getName();
		delete codec;

		//start client
		PROCESS_INFORMATION PInfo;
		STARTUPINFO SInfo;
		memset(&PInfo, 0, sizeof(PROCESS_INFORMATION));
		memset(&SInfo, 0, sizeof(STARTUPINFO));

		wxFileName wxTibiaExe(tibiaPath, wxT("Tibia.exe"));
		wxString tibiaExe = wxTibiaExe.GetFullPath();
		int created = CreateProcess(NULL, (LPSTR)(tibiaExe.c_str()), NULL, NULL, FALSE,
					CREATE_SUSPENDED, NULL, (LPCSTR)(tibiaPath.c_str()), &SInfo, &PInfo);
		if(!created){
			errorMessage(wxT("Error starting Tibia.exe!"));
			return;
		}
		HANDLE process = PInfo.hProcess;
		ResumeThread(PInfo.hThread);
		//wait....
		int timeout = 100;
		HWND tibiaWindow = NULL;
		while(!tibiaWindow){
			Sleep(100);

			//check client window
			do{
				tibiaWindow = FindWindowEx(NULL, tibiaWindow, wxT("TibiaClient"), NULL);
				if(tibiaWindow){
					DWORD pid;
					GetWindowThreadProcessId(tibiaWindow, &pid);
					if(pid == PInfo.dwProcessId){
						break;
					}
				}
			}while(tibiaWindow);

			//avoid infinite loop
			timeout--;
			if(timeout == 0){
				break;
			}

			//check that the process is still alive
			DWORD exitCode;
			bool ret = GetExitCodeProcess(process, &exitCode) != 0;
			if(!ret || exitCode != STILL_ACTIVE){
				break;
			}
		}
		Sleep(1000);
		if(!tibiaWindow){
			//something went wrong while waiting
			errorMessage(wxT("Error waiting Tibia.exe"));
			terminateProcess(PInfo);
			return;
		}
		//suspend before injecting the dll
		SuspendThread(PInfo.hThread);
		SetWindowText(tibiaWindow, wxT("Tibia - TibiaMovie.Play"));

		//setup player options
		RecordOptions options;
		options.client = version;
		options.codecName = codecName;
		options.codecOptions = 0; //ignored in player mode
		if(fileName.size() > sizeof(options.fileName)-1){
			errorMessage(wxT("Too long path"));
			terminateProcess(PInfo);
			return;
		}
		strcpy(options.fileName, fileName.c_str());

		// inject the dll
		if(!injectDll(wxT("tbplay.dll"), options, process)){
			terminateProcess(PInfo);
			return;
		}
		//and continue
		ResumeThread(PInfo.hThread);
		CloseHandle(PInfo.hProcess);
		CloseHandle(PInfo.hThread);
	}
}
