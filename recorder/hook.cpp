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
#include "packet.h"
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
bool g_keySet = false;
const uint32_t* gameStateAddr = NULL;
Packet* g_packet = NULL;

Hook g_hook;

int WSAAPI recvHook(SOCKET s, char* buf, int len, int flags)
{
	Debug::printf("recv hook(%d %x %d %d)\n", s, buf, len, flags);

	int r = recv(s, buf, len, flags);

	Debug::printf("recv(%d) mode: %d\n", r, *gameStateAddr);

	if(*gameStateAddr > g_info.minGameState){
		Debug::printf("Game server\n");
		//Game server
		if(g_info.isEncrypted && !g_keySet){
			g_packet->setCrypto(g_info.XTEAKey);
			if(g_info.hasCRC) g_packet->setCRC();
			g_keySet = true;
		}
		int bufOffset = 0;
		int tmplen = r;
		while(tmplen != 0){
			int writeLen = r - bufOffset;
			g_packet->addBytes(buf + bufOffset, writeLen);
			Debug::printf("write len: %d\n",writeLen);
			if(g_packet->isFinished()){
				Debug::printf("Finished packet\n");
				g_packet->record();
				Debug::printf("------------\n");
			}
			bufOffset += writeLen;
			tmplen -= writeLen;
		}
	}

	return r;
}

bool writeU32(uint32_t* address, const uint32_t value)
{
	MEMORY_BASIC_INFORMATION mbi;
	DWORD old;
	if(!VirtualQuery((void*)address, &mbi, sizeof(mbi))){
		return false;
	}
	if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_READWRITE, &old)){
		return false;
	}
	*address = value;
	return true;
}

Hook::Hook()
{
	Debug::start("debug.txt");

	HANDLE hFileMapping = OpenFileMapping(PAGE_READWRITE, FALSE, "TibiaMovie1");
	LPVOID m_pvData = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);

	memcpy(&g_options, m_pvData, sizeof(g_options));

	UnmapViewOfFile(m_pvData);
	CloseHandle(hFileMapping);

	g_info = getClientInfo(g_options.client);
	Debug::printf("version: %d.%d.%d\n", g_info.major, g_info.minor, g_info.revision);

	g_packet = new Packet;
	g_packet->setRecorder(g_options);

	writeU32(g_info.hook_recv, (uint32_t)&recvHook);
	gameStateAddr = g_info.gameState;
}

Hook::~Hook()
{
	delete g_packet;
	Debug::stop();
}
