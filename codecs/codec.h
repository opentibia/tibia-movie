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

#ifndef __TBMV_CODEC__H__
#define __TBMV_CODEC__H__

#include "../common.h"
#include "stdio.h"
#include <string>

class Codec{
protected:
	Codec(uint32_t options, const char* file);

public:
	virtual ~Codec();

	static Codec* getCodec(uint32_t name, uint32_t options, const char* file);
	static Codec* getCodec(const char* file);

	//play
	virtual bool getClientVersion(ClientVersion& version) = 0;
	virtual bool getFirstPacket() = 0;
	virtual bool getNextPacket(char* buffer, uint32_t& len ,uint32_t& timestamp) = 0;
	virtual uint32_t getTotalTime() = 0;

	//record
	virtual bool record(unsigned char* const raw, int len, uint32_t timestamp) = 0;
	virtual void start() = 0;
	virtual void stop() = 0;

	virtual uint32_t getName() = 0;

	const char* getFileName() const { return m_fileName.c_str();}
	uint32_t getOptions() const { return m_options;}

	void setRecordOptions(const RecordOptions& opt) { m_recOptions = opt;}
	const RecordOptions& getRecordOptions() const {return m_recOptions;}

private:
	RecordOptions m_recOptions;
	std::string m_fileName;
	uint32_t m_options;
};


class CodecText : public Codec {
public:
	CodecText(uint32_t options, const char* file);
	virtual ~CodecText();

	virtual uint32_t getName(){return 'TEXT';}

	virtual bool getClientVersion(ClientVersion& version){ return false; }
	virtual uint32_t getTotalTime(){ return 0;}
	virtual bool getFirstPacket(){ return false; }
	virtual bool getNextPacket(char* buffer, uint32_t& len ,uint32_t& timestamp){ return false; }

	virtual bool record(unsigned char* const raw, int len, uint32_t timestamp);
	virtual void start();
	virtual void stop();

protected:

	FILE* m_file;
};

#endif
