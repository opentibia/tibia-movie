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

#ifndef __TBMV_CODEC_TMV__H__
#define __TBMV_CODEC_TMV__H__

#include "zlib.h"
#include "codec.h"

#pragma pack(1)

struct CodecHeader{
	uint32_t magic;
	uint32_t options;
};

struct DataHeader{
	uint16_t dataHeaderVersion;
	uint8_t clientMajor;
	uint8_t clientMinor;
	uint8_t clientRevision;
	uint32_t recordStart;
	uint32_t npackets;
	uint32_t totalTime;
	uint32_t fileSize;
};

struct TMV2Header{
	CodecHeader cheader;
	DataHeader dheader;
};

#pragma pack()

class CodecTMV : public Codec {
public:
	CodecTMV(uint32_t options, const char* file);
	virtual ~CodecTMV();

	virtual bool getClientVersion(ClientVersion& version);
	virtual uint32_t getTotalTime();
	virtual bool getFirstPacket();
	virtual bool getNextPacket(char* buffer, uint32_t& len ,uint32_t& timestamp);

	virtual bool record(unsigned char* const raw, int len, uint32_t timestamp);
	virtual void start();
	virtual void stop();

protected:
	enum TMVState{
		TMV_STATE_PLAY,
		TMV_STATE_REC,
		TMV_STATE_NONE
	};

	bool checkHeader(FILE* file);
	bool checkState(TMVState req);

	uint32_t m_npackets;
	uint32_t m_lastTimeStamp;
	long int m_headerOffset;
	long int m_headerSize;
	FILE* m_file;
	char* m_buffer;
	uint32_t m_bufferOffset;
	uint32_t m_currenPacket;
	bool m_fileLoaded;

	TMVState m_state;
};

#endif
