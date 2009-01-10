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

	m_fileLoaded = false;
	m_state = TMV_STATE_NONE;
}

CodecTMV::~CodecTMV()
{
	delete[] m_buffer;
	if(m_state == TMV_STATE_REC) stop();
}

bool CodecTMV::checkHeader(FILE* file)
{
	char magic[5];
	fread(&magic, 4, 1, file); magic[4] = 0;
	if(strcmp(magic, "TMV2") == 0){
		return true;
	}
	else{
		return false;
	}
}

bool CodecTMV::getClientVersion(ClientVersion& version)
{
	FILE* file = fopen(getFileName(), "rb");
	if(!checkHeader(file)){
		fclose(file);
		return false;
	}
	uint8_t tmp;
	fread(&tmp, sizeof(tmp), 1, file);
	version.major = tmp;
	fread(&tmp, sizeof(tmp), 1, file);
	version.minor = tmp;
	fread(&tmp, sizeof(tmp), 1, file);
	version.revision = tmp;

	fclose(file);
	return true;
}

uint32_t CodecTMV::getTotalTime()
{
	FILE* file = fopen(getFileName(), "rb");
	if(!checkHeader(file)){
		fclose(file);
		return 0;
	}
	uint32_t tmp;
	fseek(file, 19, SEEK_SET);
	fread(&tmp, sizeof(tmp), 1, file);

	fclose(file);
	return true;
}

bool CodecTMV::checkState(TMVState req)
{
	if(m_state == TMV_STATE_NONE || m_state == req){
		return true;
	}
	else{
		Debug::printf("CodecTMV: Error checking state %d, req %d\n", m_state, req);
		return false;
	}
}

bool CodecTMV::getFirstPacket()
{
	if(!checkState(TMV_STATE_PLAY)) return false;
	if(!m_fileLoaded){
		m_file = fopen(getFileName(), "rb");
		if(!checkHeader(m_file)){
			fclose(m_file);
			return false;
		}
		fseek(m_file, 0, SEEK_SET);
		TMV2Header header;
		fread(&header, sizeof(header), 1, m_file);
		if(header.dheader.dataHeaderVersion != 1){
			fclose(m_file);
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
	m_bufferOffset = 0;
	m_state = TMV_STATE_PLAY;
	return true;
}

bool CodecTMV::getNextPacket(char* buffer, uint32_t& len ,uint32_t& timestamp)
{
	if(!checkState(TMV_STATE_PLAY)) return false;
	len = *(uint16_t*)(m_buffer + m_bufferOffset);
	timestamp = *(uint32_t*)(m_buffer + m_bufferOffset + 2);
	memcpy(buffer, m_buffer + m_bufferOffset + 6, len);
	m_bufferOffset += 6 + len;
	return true;
}

bool CodecTMV::record(unsigned char* const raw, int len, uint32_t timestamp)
{
	if(!checkState(TMV_STATE_REC)) return false;
	Debug::printf("CodecTMV: record %d\n", len);
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
	Debug::printf("CodecTMV: open %s\n", getFileName());
	m_file = fopen(getFileName(), "wb");

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
