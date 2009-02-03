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

#ifndef __TBMV_CLIENTS_H__
#define __TBMV_CLIENTS_H__

#include "common.h"

struct ClientInfo{
	int major;
	int minor;
	int revision;

	//packet type
	bool isEncrypted;
	bool hasCRC;

	//game mode
	uint32_t* gameState;
	uint32_t minGameState;

	//XTEA
	uint32_t* XTEAKey;

	//hooks address
	uint32_t* hook_recv;
	uint32_t* hook_send;
	uint32_t* hook_connect;
};

const ClientInfo& getClientInfo(const ClientVersion& version);


#endif
