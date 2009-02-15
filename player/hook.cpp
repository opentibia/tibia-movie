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

#include "hook.h"
#include "server.h"
#include "../common.h"
#include "../debug.h"
#include "../clients.h"
#include "gui.h"
#include "wx/init.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if(fdwReason == DLL_PROCESS_ATTACH){

	}
	else if(fdwReason == DLL_PROCESS_DETACH){
		wxTheApp->OnExit();
	}
	return TRUE;
}

DWORD WINAPI wxThread(LPVOID lpParameter)
{
	wxApp::SetInstance(new PlayerApplication);
	wxEntry(GetModuleHandle(NULL));
	return true;
}

//Global variables
extern RecordOptions g_options;
extern ClientInfo g_info;
Server g_server;
HookPlayer g_hook;
WNDPROC g_oldwndproc;

int WSAAPI sendHook(SOCKET s, const char* buf, int len, int flags)
{
	Debug::printf(DEBUG_NOTICE, "send hook(%d %x %d %d)\n", s, buf, len, flags);
	if(g_info.isEncrypted) g_server.setCrypto(g_info.XTEAKey);
	int r = send(s, buf, len, flags);
	return r;
}

int WSAAPI connectHook(SOCKET s, sockaddr *name, int namelen)
{
	Debug::printf(DEBUG_NOTICE, "connect hook: \n");
	if(namelen == sizeof(sockaddr_in)){
		sockaddr_in* sin = (sockaddr_in*)name;
		sin->sin_port = htons(g_server.getPort());
		sin->sin_addr.s_addr = inet_addr("127.0.0.1");
		Debug::printf(DEBUG_NOTICE, "Changing ip/port\n");
	}
	else{
		Debug::printf(DEBUG_ERROR, "Failed to change ip/port\n");
	}
	return connect(s, name, namelen);;
}

LRESULT CALLBACK WindowHook(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg == WM_KEYDOWN){
		if(wParam == 'S' && (GetAsyncKeyState(VK_CONTROL) & 0x8000)){
			PlayerApplication::getInstance()->toggleGUI();
			return 0;
		}
	}
	return CallWindowProc(g_oldwndproc, hwnd, uMsg, wParam, lParam);
}

//

HookPlayer::HookPlayer()
{
	Debug::start("debug_player.txt");
	//Debug::setDebugLevel(DEBUG_NOTICE);

	getRecordOptions();

	//Install hooks
	writeU32(g_info.hook_send, (uint32_t)&sendHook);
	writeU32(g_info.hook_connect, (uint32_t)&connectHook);
	//
	DWORD pid = GetCurrentProcessId();
	HWND tibiaWindow = NULL;
	while(1){
		tibiaWindow = FindWindowEx(NULL, tibiaWindow, wxT("TibiaClient"), NULL);
		if(tibiaWindow == NULL){
			//Error
			Debug::printf(DEBUG_ERROR, "Client window not found\n");
			break;
		}
		else{
			DWORD process;
			GetWindowThreadProcessId(tibiaWindow, &process);
			if(process == pid){
				Debug::printf(DEBUG_NOTICE, "Installing window hook\n");
				g_oldwndproc = (WNDPROC)GetWindowLong(tibiaWindow, GWL_WNDPROC);
				SetWindowLong(tibiaWindow, GWL_WNDPROC, (LONG)WindowHook);
				break;
			}
		}
	}

	Debug::printf(DEBUG_INFO, "filename: %s\n", g_options.fileName);
	g_server.setFile(g_options.fileName);
	if(g_info.hasCRC) g_server.setCRC();

	//Initialize wxWidgets
	CreateThread(NULL, 0, wxThread, NULL, 0, NULL);

	g_server.startServer();
}

HookPlayer::~HookPlayer()
{
	Debug::stop();
}
