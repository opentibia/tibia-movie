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
#include "packet_rec.h"
#include "../common.h"
#include "../debug.h"
#include "../clients.h"


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return TRUE;
}

//Global variables
extern RecordOptions g_options;
extern ClientInfo g_info;
bool g_keySet = false;
const uint32_t* gameStateAddr = NULL;
PacketRecord* g_packet = NULL;

HookRecord g_hook;

int WSAAPI recvHook(SOCKET s, char* buf, int len, int flags)
{
	Debug::printf(DEBUG_NOTICE, "recv hook(%d %x %d %d)\n", s, buf, len, flags);

	int r = recv(s, buf, len, flags);

	Debug::printf(DEBUG_NOTICE, "recv(%d) mode: %d\n", r, *gameStateAddr);

	if(*gameStateAddr > g_info.minGameState){
		Debug::printf(DEBUG_NOTICE, "Game server\n");
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
			g_packet->recordBytes(buf + bufOffset, writeLen);
			Debug::printf(DEBUG_NOTICE, "write len: %d\n",writeLen);
			if(g_packet->isFinished()){
				Debug::printf(DEBUG_NOTICE, "Finished packet\n");
				g_packet->record();
				Debug::printf(DEBUG_NOTICE, "------------\n");
			}
			bufOffset += writeLen;
			tmplen -= writeLen;
		}
	}

	return r;
}

HookRecord::HookRecord()
{
	Debug::start("debug.txt");
	getRecordOptions();

	g_packet = new PacketRecord;
	g_packet->setRecorder(g_options);

	writeU32(g_info.hook_recv, (uint32_t)&recvHook);
	gameStateAddr = g_info.gameState;
}

HookRecord::~HookRecord()
{
	delete g_packet;
	Debug::stop();
}
