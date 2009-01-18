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

#ifndef __TBMV_PACKET_REC_H__
#define __TBMV_PACKET_REC_H__

#include "../common.h"
#include "../debug.h"
#include "../packet.h"
#include "recorder.h"

class PacketRecord : public Packet{
public:
	PacketRecord() : m_recorder(0) {setRawMode(true);}
	~PacketRecord() {delete m_recorder;};

	bool isFinished() const {return (m_offset == m_size + 2);}

	bool record();

	void setRecorder(const RecordOptions& options){
		m_recorder = new Recorder(options);
	}

private:

	Recorder* m_recorder;
};

#endif
