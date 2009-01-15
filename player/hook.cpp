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


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return TRUE;
}

//Global variables
RecordOptions g_options;
ClientInfo g_info;
Server g_server;
HookPlayer g_hook;

int WSAAPI sendHook(SOCKET s, const char* buf, int len, int flags)
{
	Debug::printf("send hook(%d %x %d %d)\n", s, buf, len, flags);
	if(g_info.isEncrypted) g_server.setCrypto(g_info.XTEAKey);
	int r = send(s, buf, len, flags);
	return r;
}

int WSAAPI connectHook(SOCKET s, sockaddr *name, int namelen)
{
	//if(!g_server.isStarted()) g_server.startServer();
	Debug::printf("connect hook: ");
	if(namelen == sizeof(sockaddr_in)){
		sockaddr_in* sin = (sockaddr_in*)name;
		sin->sin_port = htons(g_server.getPort());
		sin->sin_addr.s_addr = inet_addr("127.0.0.1");
		Debug::printf("changing ip/port\n");
	}
	else{
		Debug::printf("failed to change ip/port\n");
	}
	return connect(s, name, namelen);;
}

HookPlayer::HookPlayer()
{
	Debug::start("debug.txt");

	HANDLE hFileMapping = OpenFileMapping(PAGE_READWRITE, FALSE, "TibiaMovie1");
	LPVOID m_pvData = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);

	memcpy(&g_options, m_pvData, sizeof(g_options));

	UnmapViewOfFile(m_pvData);
	CloseHandle(hFileMapping);

	g_info = getClientInfo(g_options.client);
	Debug::printf("version: %d.%d.%d\n", g_info.major, g_info.minor, g_info.revision);

	writeU32(g_info.hook_send, (uint32_t)&sendHook);
	writeU32(g_info.hook_connect, (uint32_t)&connectHook);

	Debug::printf("filename: %s\n", g_options.fileName);
	g_server.setFile(g_options.fileName);
	if(g_info.hasCRC) g_server.setCRC();
	g_server.startServer();
}

HookPlayer::~HookPlayer()
{
	Debug::stop();
}
