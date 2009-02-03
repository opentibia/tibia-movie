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

#include "packet.h"
#include "string.h"
#include <algorithm>
#include "debug.h"

#define min(a, b)  (a < b) ? a : b;

#define MOD_ADLER 65521
uint32_t adlerChecksum(uint8_t *data, int32_t len)
{
	if(len < 0){
		return 0;
	}

    uint32_t a = 1, b = 0;
    while (len > 0)
    {
        size_t tlen = len > 5552 ? 5552 : len;
        len -= tlen;
        do
        {
            a += *data++;
            b += a;
        } while (--tlen);

        a %= MOD_ADLER;
        b %= MOD_ADLER;
    }

    return (b << 16) | a;
}

void Packet::addBytes(const char* buffer, int len)
{
	memcpy(m_buffer + m_offset, buffer, len);
	m_size += len;
	m_offset += len;
}

void Packet::addString(const char* d)
{
	int len = strlen(d);
	addU16(len);
	addBytes(d, len);
}

void Packet::setCrypto(uint32_t* const key)
{
	m_isEncrypted = true;
	memcpy(&m_key, key, sizeof(uint32_t)*4);
	Debug::printf(DEBUG_NOTICE, "Packet: key set %X %X %X %X\n", m_key[0], m_key[1], m_key[2], m_key[3]);
}

void Packet::XTEA_encrypt()
{
	uint32_t k[4];
	k[0] = m_key[0]; k[1] = m_key[1]; k[2] = m_key[2]; k[3] = m_key[3];

	int32_t messageLength = m_size;

	//add bytes until reach 8 multiple
	uint32_t n;
	if((messageLength % 8) != 0){
		n = 8 - (messageLength % 8);
		m_size += n;
		messageLength = messageLength + n;
	}

	int read_pos = 0;
	uint32_t* buffer = (uint32_t*)(m_buffer + getHeaderSize(true));
	while(read_pos < messageLength/4){
		uint32_t v0 = buffer[read_pos], v1 = buffer[read_pos + 1];
		uint32_t delta = 0x61C88647;
		uint32_t sum = 0;

		for(int32_t i = 0; i < 32; i++) {
			v0 += ((v1 << 4 ^ v1 >> 5) + v1) ^ (sum + k[sum & 3]);
			sum -= delta;
			v1 += ((v0 << 4 ^ v0 >> 5) + v0) ^ (sum + k[sum>>11 & 3]);
		}
		buffer[read_pos] = v0; buffer[read_pos + 1] = v1;
		read_pos = read_pos + 2;
	}
	*(uint16_t*)m_buffer = m_size + getHeaderSize(true) - 2;
	Debug::printf(DEBUG_NOTICE, "encrypted size set to %d\n", m_size);
}

bool Packet::XTEA_decrypt(int& len)
{
	if((m_size + 2 - getHeaderSize()) % 8 != 0){
		Debug::printf(DEBUG_ERROR, "XTEA: wrong size %d(%d)\n", m_size , getHeaderSize());
		return false;
	}
	//
	uint32_t k[4];
	k[0] = m_key[0]; k[1] = m_key[1]; k[2] = m_key[2]; k[3] = m_key[3];

	uint32_t* buffer = (uint32_t*)(m_buffer + getHeaderSize());
	int read_pos = 0;
	int32_t messageLength = m_size;
	while(read_pos < messageLength/4){
		uint32_t v0 = buffer[read_pos], v1 = buffer[read_pos + 1];
		uint32_t delta = 0x61C88647;
		uint32_t sum = 0xC6EF3720;

		for(int32_t i = 0; i < 32; i++) {
			v1 -= ((v0 << 4 ^ v0 >> 5) + v0) ^ (sum + k[sum>>11 & 3]);
			sum += delta;
			v0 -= ((v1 << 4 ^ v1 >> 5) + v1) ^ (sum + k[sum & 3]);
		}
		buffer[read_pos] = v0; buffer[read_pos + 1] = v1;
		read_pos = read_pos + 2;
	}
	//
	len = *(uint16_t*)buffer + 2;
	Debug::printf(DEBUG_NOTICE, "XTEA: decoded size %d\n", len);

	return true;
}

void Packet::getSendRaw(const char*& buffer, int& size)
{
	if(!m_rawMode){
		Debug::printf(DEBUG_NOTICE, "packtet size set to %d\n", m_size);
		*(uint16_t*)(m_buffer + getHeaderSize() - 2) = m_size;
		m_size = m_size + 2;
	}
	if(m_isEncrypted){
		XTEA_encrypt();
		if(m_hasCRC){
			*(uint32_t*)(m_buffer + 2) = ((uint32_t)(adlerChecksum((uint8_t*)(m_buffer + getHeaderSize(true)), m_size)));
		}
	}
	buffer = m_buffer;
	size = m_size + getHeaderSize(true);
}

void Packet::recordBytes(const char* buffer, int &len)
{
	Debug::printf(DEBUG_NOTICE, "Packet: size is %d\n", m_size);
	if(m_size == 0){
		m_size = *(uint16_t*)buffer;
		Debug::printf(DEBUG_NOTICE, "Packet: size set %d\n", m_size);
		if(m_hasCRC){
			m_crc = *(uint32_t*)(buffer + 2);
			Debug::printf(DEBUG_NOTICE, "Packet: crc set %X\n", m_crc);
		}
	}

	int bytes = min(len, (int)(m_size + getHeaderSize() - m_offset));
	memcpy(m_buffer + m_offset, buffer, bytes);
	m_offset += bytes;
	len = bytes;
	Debug::printf(DEBUG_NOTICE, "Packet: Copy %d\n", bytes);
}

unsigned char* const Packet::getRecRaw(int& len)
{
	len = m_size + 2;
	if(m_isEncrypted) XTEA_decrypt(len);
	return (unsigned char*)(m_buffer + getHeaderSize());
}
