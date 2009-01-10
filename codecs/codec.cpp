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
		Debug::printf("Codec: TEXT\n");
		break;
	case 'TMV2':
		codec = new CodecTMV(options, file);
		Debug::printf("Codec: TMV2\n");
		break;
	default:
		codec = NULL;
		Debug::printf("Codec: not known\n");
		break;
	}
	return codec;
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
	Debug::printf("CodecText: record %d\n", len);
	for(int i = 0; i < len; ++i){
		fprintf(m_file, " %02X ", (uint32_t)raw[i]);
	}
	fprintf(m_file, "\n");
	return true;
}

void CodecText::start()
{
	Debug::printf("CodecText: open %s\n", getFileName());
	m_file = fopen(getFileName(), "w");
}

void CodecText::stop()
{
	fclose(m_file);
}
