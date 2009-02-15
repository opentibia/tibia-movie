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

#include "recorder.h"
#include "windows.h"
#include "../codecs/codec.h"
#include "../debug.h"

Recorder::Recorder(const RecordOptions& options)
{
	m_options = options;
	m_startTime = 0;
	m_codec = Codec::getCodec(options.codecName, options.codecOptions, &(options.fileName[0]));

	if(m_codec){
		m_codec->setRecordOptions(options);
		if(m_codec) m_codec->start();
	}
}

Recorder::~Recorder()
{
	delete m_codec;
}

bool Recorder::record(unsigned char* const raw, int len)
{
	if(m_startTime == 0) m_startTime = GetTickCount();
	if(m_codec) return m_codec->record(raw, len, GetTickCount() - m_startTime);

	return false;
}
