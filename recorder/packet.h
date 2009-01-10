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

#include "../common.h"
#include "../debug.h"
#include "recorder.h"

const int BUFFER_SIZE = 65535;

class Packet{
public:
	Packet() : m_offset(0), m_size(0), m_crc(0),
		m_isEncrypted(false), m_hasCRC(false), m_recorder(0) {}
	~Packet();

	void addBytes(const char* buffer, int &len);

	void setCrypto(uint32_t* const key);
	void setCRC(){m_hasCRC = true;}

	bool isFinished() const {return (m_offset == m_size + 2);}

	bool record();

	void setRecorder(const RecordOptions& options){
		m_recorder = new Recorder(options);
	}

private:

	unsigned char* const getRaw(int& len);
	void clearBuffer(){ m_offset = 0; m_size = 0; m_crc = 0;}

	int getHeaderSize() const{
		int header = 0;
		if(m_isEncrypted) header +=2;
		if(m_hasCRC) header += 4;
		return header;
	}

	bool XTEA_decrypt(int& len);

	char m_buffer[BUFFER_SIZE + 6];
	uint16_t m_offset;

	uint16_t m_size;
	uint32_t m_crc;

	bool m_isEncrypted;
	bool m_hasCRC;
	uint32_t m_key[4];

	Recorder* m_recorder;
};

#endif
