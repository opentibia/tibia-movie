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

#ifndef __TBMV_PACKET_H__
#define __TBMV_PACKET_H__

#include "common.h"
#include "debug.h"

const int BUFFER_SIZE = 65535;

class Packet{
public:
	Packet() : m_offset(0), m_size(0),
		m_isEncrypted(false), m_hasCRC(false),m_rawMode(false) {}
	~Packet() {}

	void setCrypto(uint32_t* const key);
	void setCRC(){m_hasCRC = true;}
	void setRawMode(bool mode){m_rawMode = mode;}

	void addBytes(const char* buffer, int len);
	void addU8(uint8_t d){addUx(d);}
	void addU16(uint16_t d){addUx(d);}
	void addU32(uint32_t d){addUx(d);}
	void addString(const char* d);


	void clearSendBuffer(){ m_offset = getHeaderSize(); m_size = 0;}
	void getSendRaw(const char*& buffer, int& size);


	char* getBuffer(){ return m_buffer + getHeaderSize();}
	uint32_t& getSize(){ return m_size;}

	void recordBytes(const char* buffer, int &len);

protected:

	unsigned char* const getRecRaw(int& len);
	void clearRecBuffer(){ m_offset = 0; m_size = 0; m_crc = 0;}

	template<typename T>
	void addUx(T d){
		const int sT = sizeof(T);
		*(T*)(m_buffer + m_offset) = (T)d;
		m_size += sT; m_offset += sT;
	}

	uint32_t getHeaderSize(bool raw = false) const{
		int header = 0;
		if(!m_rawMode && !raw) header += 2;
		if(m_isEncrypted) header +=2;
		if(m_hasCRC) header += 4;
		return header;
	}

	void XTEA_encrypt();
	bool XTEA_decrypt(int& len);

	char m_buffer[BUFFER_SIZE + 6];
	uint16_t m_offset;

	uint32_t m_size;
	uint32_t m_crc;

	bool m_isEncrypted;
	bool m_hasCRC;
	bool m_rawMode;
	uint32_t m_key[4];
};


#endif
