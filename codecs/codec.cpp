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

#include "codec.h"
#include "codec_tmv.h"
#include "string.h"
#include "../debug.h"

Codec* Codec::getCodec(uint32_t name, uint32_t options, const char* file)
{
	Codec* codec;
	switch(name){
	case 'TEXT':
		codec = new CodecText(options, file);
		Debug::printf(DEBUG_INFO, "Codec: TEXT\n");
		break;
	case 'TMV2':
		codec = new CodecTMV(options, file);
		Debug::printf(DEBUG_INFO,"Codec: TMV2\n");
		break;
	default:
		codec = NULL;
		Debug::printf(DEBUG_ERROR,"Codec: not known\n");
		break;
	}
	return codec;
}

Codec* Codec::getCodec(const char* file)
{
	FILE* f = fopen(file, "rb");
	if(!f){
		Debug::printf(DEBUG_ERROR, "getCodec: error opening %s\n", file);
		return NULL;
	}

	Codec* ret = NULL;
	char magic[4];
	fread(&magic, 4, 1, f);
	if(memcmp(magic, "TMV2", 4) == 0){
		ret = new CodecTMV(0, file);
		Debug::printf(DEBUG_INFO, "Codec: play TMV2\n");
	}
	else{
		Debug::printf(DEBUG_ERROR, "Codec: play not known\n");
	}
	fclose(f);
	return ret;
}

Codec::Codec(uint32_t options, const char* file)
{
	m_options = options;
	m_fileName = file;
}

Codec::~Codec()
{
	//
}

//********* CodecText ****************

CodecText::CodecText(uint32_t options, const char* file) :
Codec(options, file)
{
	m_file = NULL;
}

CodecText::~CodecText()
{
	stop();
}

bool CodecText::record(unsigned char* const raw, int len, uint32_t timestamp)
{
	if(!m_file) return false;
	Debug::printf(DEBUG_NOTICE, "CodecText: record %d\n", len);
	for(int i = 0; i < len; ++i){
		fprintf(m_file, " %02X ", (uint32_t)raw[i]);
	}
	fprintf(m_file, "\n");
	return true;
}

void CodecText::start()
{
	Debug::printf(DEBUG_INFO, "CodecText: open %s\n", getFileName());
	m_file = fopen(getFileName(), "w");
	if(!m_file){
		Debug::printf(DEBUG_ERROR, "CodecText: error opening %s\n", getFileName());
	}
}

void CodecText::stop()
{
	fclose(m_file);
}
