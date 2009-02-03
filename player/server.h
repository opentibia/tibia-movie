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

#ifndef __TBMV_SERVER_H__
#define __TBMV_SERVER_H__

#include <string>
#include "../packet.h"
#include "winsock2.h"
#include "windows.h"

class Codec;

class Server{
public:
	Server();
	~Server();

	void setFile(const char* file) {m_fileName = file;}
	void setPlaySpeed(float speed) {m_playSpeed = speed;}

	void setCrypto(uint32_t* const key);
	void setCRC(){m_packet.setCRC();}

	bool isStarted(){ return m_isStarted;}
	bool startServer();

	uint16_t getPort(){ return m_port;}

private:

	static DWORD WINAPI serverThread(Server* this_ptr);

	bool sendPacket();

	std::string m_fileName;
	float m_playSpeed;

	enum ServerState{
		SERVER_STATE_LOGIN,
		SERVER_STATE_GAME
	};
	ServerState m_state;
	bool m_isStarted;

	bool m_isEncrypted;
	bool m_hasCRC;

	Codec* m_codec;
	Packet m_packet;

	uint16_t m_port;
	SOCKET m_clientSock;
};

#endif
