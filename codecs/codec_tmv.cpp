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

#include "codec_tmv.h"
#include "../debug.h"
#include "time.h"
#include "string.h"

CodecTMV::CodecTMV(uint32_t options, const char* file) :
Codec(options, file)
{
	m_npackets = 0;
	m_lastTimeStamp = 0;

	m_headerOffset = 0;
	m_headerSize = 0;

	m_file = NULL;

	m_buffer = NULL;
	m_bufferOffset = 0;
	m_currenPacket = 0;

	m_fileLoaded = false;
	m_state = TMV_STATE_NONE;
}

CodecTMV::~CodecTMV()
{
	delete[] m_buffer;
	if(m_state == TMV_STATE_REC) stop();
}

bool CodecTMV::checkHeader(FILE* file, TMV2Header& header)
{
	fread(&header, sizeof(header), 1, m_file);
	//Check magic
	if(memcmp(&header.cheader.magic, "TMV2", 4) != 0){
		fclose(file);
		Debug::printf(DEBUG_ERROR, "CodecTMV: Magic test failed. This is not TMV2!\n");
		return false;
	}
	//and check hedaer version
	if(header.dheader.dataHeaderVersion != 1){
		fclose(file);
		Debug::printf(DEBUG_ERROR, "CodecTMV: Header version != 1\n");
		return false;
	}
	return true;
}

bool CodecTMV::getClientVersion(ClientVersion& version)
{
	FILE* file = fopen(getFileName(), "rb");
	if(!file){
		Debug::printf(DEBUG_ERROR, "CodecTMV: Failed to open file %s.\n", getFileName());
		return false;
	}
	TMV2Header header;
	if(!checkHeader(file, header)){
		return false;
	}
	version.major = header.dheader.clientMajor;
	version.minor = header.dheader.clientMinor;
	version.revision = header.dheader.clientRevision;

	fclose(file);
	return true;
}

uint32_t CodecTMV::getTotalTime()
{
	FILE* file = fopen(getFileName(), "rb");
	if(!file){
		Debug::printf(DEBUG_ERROR, "CodecTMV: Failed to open file %s.\n", getFileName());
		return 0;
	}
	TMV2Header header;
	if(!checkHeader(file, header)){
		return 0;
	}

	fclose(file);
	return header.dheader.totalTime;
}

bool CodecTMV::checkState(TMVState req)
{
	if(m_state == TMV_STATE_NONE || m_state == req){
		return true;
	}
	else{
		Debug::printf(DEBUG_ERROR, "CodecTMV: Error checking state %d, req %d\n", m_state, req);
		return false;
	}
}

bool CodecTMV::getFirstPacket()
{
	if(!checkState(TMV_STATE_PLAY)) return false;
	if(!m_fileLoaded){
		m_file = fopen(getFileName(), "rb");
		if(!m_file){
			Debug::printf(DEBUG_ERROR, "CodecTMV: Failed to open file %s.\n",getFileName());
			return false;
		}
		TMV2Header header;
		if(!checkHeader(m_file, header)){
			return false;
		}

		m_npackets = header.dheader.npackets;
		long int dataStart = ftell(m_file);
		fseek(m_file, 0, SEEK_END);
		long int size = ftell(m_file) - dataStart;
		fseek(m_file, dataStart, SEEK_SET);
		char* tmpBuffer = new char[size];
		fread(tmpBuffer, size, 1, m_file);
		if(header.cheader.options & 1){
			//compressed
			if(header.dheader.fileSize > (uint32_t)size*20){
				Debug::printf(DEBUG_ERROR, "CodecTMV: Corrupted file? fileSize(%d) > 20*size(%d)\n", header.dheader.fileSize, 20*size);
				fclose(m_file);
				return false;
			}
			Bytef* uncBuffer = new Bytef[header.dheader.fileSize];
			uLongf destLen = header.dheader.fileSize;
			uncompress(uncBuffer, &destLen, (Bytef*)tmpBuffer, size);
			m_buffer = (char*)uncBuffer;
			delete[] tmpBuffer;
		}
		else{
			m_buffer = tmpBuffer;
		}
		fclose(m_file);
		m_fileLoaded = true;
	}
	m_currenPacket = 0;
	m_bufferOffset = 0;
	m_state = TMV_STATE_PLAY;
	return true;
}

bool CodecTMV::getNextPacket(char* buffer, uint32_t& len ,uint32_t& timestamp)
{
	if(!checkState(TMV_STATE_PLAY)) return false;
	if(m_currenPacket >= m_npackets) return false;
	len = *(uint16_t*)(m_buffer + m_bufferOffset);
	timestamp = *(uint32_t*)(m_buffer + m_bufferOffset + 2);
	memcpy(buffer, m_buffer + m_bufferOffset + 6, len);
	m_bufferOffset += 6 + len;
	m_currenPacket++;
	return true;
}

bool CodecTMV::record(unsigned char* const raw, int len, uint32_t timestamp)
{
	if(!checkState(TMV_STATE_REC)) return false;
	if(!m_file) return false;
	Debug::printf(DEBUG_NOTICE, "CodecTMV: record %d\n", len);
	uint16_t u16len = len;
	m_npackets++;
	m_lastTimeStamp = timestamp;
	fwrite(&u16len, sizeof(u16len), 1, m_file);
	fwrite(&timestamp, sizeof(timestamp), 1, m_file);
	fwrite(raw, len, 1, m_file);
	return true;
}

void CodecTMV::start()
{
	if(!checkState(TMV_STATE_REC)) return;
	Debug::printf(DEBUG_INFO, "CodecTMV: open %s\n", getFileName());
	m_file = fopen(getFileName(), "wb");
	if(!m_file){
		Debug::printf(DEBUG_ERROR, "CodecTMV: error opening %s\n", getFileName());
		return;
	}

	//-- Codec header --
	fwrite("TMV2", 4, 1, m_file);
	uint32_t tmp32 = getOptions();
	fwrite(&tmp32, sizeof(tmp32), 1, m_file);

	//-- Data header --
	//Data hader version
	uint16_t tmp16 = 1;
	fwrite(&tmp16, sizeof(tmp16), 1, m_file);
	//client version
	const RecordOptions& recOptions = getRecordOptions();
	uint8_t tmp8 = recOptions.client.major;
	fwrite(&tmp8, sizeof(tmp8), 1, m_file);
	tmp8 = recOptions.client.minor;
	fwrite(&tmp8, sizeof(tmp8), 1, m_file);
	tmp8 = recOptions.client.revision;
	fwrite(&tmp8, sizeof(tmp8), 1, m_file);
	//date & time
	tmp32 = time(NULL);
	fwrite(&tmp32, sizeof(tmp32), 1, m_file);

	//**allocate space for info that have to
	// be written at the end**
	m_headerOffset = ftell(m_file);
	tmp32 = 0;

	//number of packets
	fwrite(&tmp32, sizeof(tmp32), 1, m_file);
	//total time(ms)
	fwrite(&tmp32, sizeof(tmp32), 1, m_file);
	//uncompressed file size
	fwrite(&tmp32, sizeof(tmp32), 1, m_file);
	//...
	//**

	//--------

	m_headerSize = ftell(m_file);
	m_state = TMV_STATE_REC;
}

void CodecTMV::stop()
{
	if(!checkState(TMV_STATE_REC)) return;
	//go back to header and write
	uint32_t fileSize = ftell(m_file);
	fseek(m_file, m_headerOffset, SEEK_SET);
	fwrite(&m_npackets, sizeof(uint32_t), 1, m_file);
	fwrite(&m_lastTimeStamp, sizeof(uint32_t), 1, m_file);
	fwrite(&fileSize, sizeof(uint32_t), 1, m_file);
	fclose(m_file);

	//compress if required
	if(getOptions() & 1){
		FILE* file = fopen(getFileName(), "rb");
		if(!file){
			Debug::printf(DEBUG_ERROR, "CodecTMV: error compressing %s\n", getFileName());
			return;
		}
		fseek(file, 0, SEEK_END);
		long int size = ftell(file);
		fseek(file, 0, SEEK_SET);
		Bytef* inBuffer = new Bytef[size];
		uLongf outSize = (uLongf)(size*1.001 + 12.);
		Bytef* outBuffer = new Bytef[outSize];
		fread(inBuffer, size, 1, file);
		fclose(file);

		compress(outBuffer, &outSize, inBuffer + m_headerSize, size - m_headerSize);

		file = fopen(getFileName(), "wb");
		fwrite(inBuffer, m_headerSize, 1, file);
		fwrite(outBuffer, outSize, 1, file);
		fclose(file);

		delete[] inBuffer;
		delete[] outBuffer;
	}
}
