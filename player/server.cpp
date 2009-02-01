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

#include <algorithm>
#include "server.h"
#include "../codecs/codec.h"
#include "../debug.h"

#define min(x,y) ((x < y) ? (x) : (y))

Server::Server()
{
	m_playSpeed = 1.f;
	m_isEncrypted = false;
	m_hasCRC = false;
	m_state = SERVER_STATE_LOGIN;
	m_isStarted = false;
	m_codec = NULL;

	WSADATA wsadata;
	WSAStartup(MAKEWORD(1,1), &wsadata);
	m_clientSock = INVALID_SOCKET;
	srand(time(NULL));
	m_port = (rand() % 5000) + 10000;
}

void Server::setCrypto(uint32_t* const key)
{
	m_packet.setCrypto(key);
}

Server::~Server()
{
	WSACleanup();
}

bool Server::startServer()
{
	Debug::printf(DEBUG_INFO, "Starting server file: %s\n", m_fileName.c_str());
	Debug::printf(DEBUG_INFO, "port set to %d\n", getPort());
	m_codec = Codec::getCodec(m_fileName.c_str());
	if(!m_codec){
		Debug::printf(DEBUG_ERROR, "Codec not found!\n");
		return false;
	}
	HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&Server::serverThread, this, 0, NULL);
	CloseHandle(thread);
	Sleep(100);
	m_isStarted = true;
	Debug::printf(DEBUG_INFO, "server started\n");
	return true;
}

DWORD WINAPI Server::serverThread(Server* this_ptr)
{
	Debug::printf(DEBUG_NOTICE, "serverThread (%X)\n", this_ptr);
	if(this_ptr->m_state != SERVER_STATE_LOGIN){
		return 1;
	}
	//open listen socket
	SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(listenSock == INVALID_SOCKET){
		Debug::printf(DEBUG_ERROR, "Failed to create listen socket\n");
		return 1;
	}
	Debug::printf(DEBUG_NOTICE, "created listen socket\n");
	SOCKADDR_IN sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr("127.0.0.1");
	sin.sin_port = htons(this_ptr->getPort());
	if(bind(listenSock, (SOCKADDR*)&sin, sizeof(sin)) == SOCKET_ERROR){
		closesocket(listenSock);
		Debug::printf(DEBUG_ERROR, "Failed bind listen socket\n");
		return 1;
	}

	if(listen(listenSock, 3) == SOCKET_ERROR){
		Debug::printf(DEBUG_ERROR, "Failed listen socket\n");
		closesocket(listenSock);
		return 1;
	}

	//wait client login
	SOCKADDR_IN addr_accept;
	int slen = sizeof(addr_accept);
	Debug::printf(DEBUG_NOTICE, "Waiting login...\n");
	this_ptr->m_clientSock = accept(listenSock,(SOCKADDR*)&addr_accept, &slen);
	uint32_t tmp;
	recv(this_ptr->m_clientSock, (char*)&tmp, 4, 0);
	Debug::printf(DEBUG_NOTICE, "login client\n");

	//send dummy login info
	this_ptr->m_packet.clearSendBuffer();
	this_ptr->m_packet.addU8(0x64); //char list
	this_ptr->m_packet.addU8(1); //1 "char"
	this_ptr->m_packet.addString(this_ptr->m_fileName.c_str()); // file name
	this_ptr->m_packet.addString("TibiaMovie"); //server
	this_ptr->m_packet.addU32(inet_addr("127.0.0.1")); //ip
	this_ptr->m_packet.addU16(htons(27591)); //port
	this_ptr->m_packet.addU16(1); //premium days
	this_ptr->sendPacket();
	SOCKET oldsocket = this_ptr->m_clientSock;

	this_ptr->m_state = SERVER_STATE_GAME;

	//wait client game
	Debug::printf(DEBUG_NOTICE, "waiting game\n");
	this_ptr->m_clientSock = accept(listenSock,(SOCKADDR*)&addr_accept, &slen);
	closesocket(oldsocket);
	recv(this_ptr->m_clientSock, (char*)&tmp, 4, 0);
	Debug::printf(DEBUG_NOTICE, "game client\n");

	this_ptr->m_packet.setRawMode(true);
	this_ptr->m_codec->getFirstPacket();

	uint32_t lasttimestamp = 0;
	uint32_t timestamp;
	while(this_ptr->m_codec->getNextPacket(this_ptr->m_packet.getBuffer(),
				this_ptr->m_packet.getSize(), timestamp)){
		Debug::printf(DEBUG_NOTICE, "packet time: %d\n", timestamp);
		if(lasttimestamp != 0) Sleep(timestamp - lasttimestamp);
		this_ptr->sendPacket();
		lasttimestamp = timestamp;
	}
	closesocket(this_ptr->m_clientSock);

	return 0;
}

bool Server::sendPacket()
{
	const char* buffer;
	int size;
	m_packet.getSendRaw(buffer, size);
	Debug::printf(DEBUG_NOTICE, "sending packet %d\n", size);

	bool ret = true;
	long retry = 0;
	long sentBytes = 0;
	do{
		int b = send(m_clientSock, buffer + sentBytes, min(size - sentBytes, 1024), 0);
		if(b <= 0){
			b = 0;
			int errnum = WSAGetLastError();
			if(errnum == WSAEWOULDBLOCK){
				Sleep(10);
				retry++;
				if(retry == 10){
					ret = false;
				}
			}
			else{
				m_packet.clearSendBuffer();
				return false;
			}
		}
		sentBytes += b;
	}while(sentBytes < size);
	m_packet.clearSendBuffer();
	return ret;
}
