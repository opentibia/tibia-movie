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

#include "common.h"
#include "windows.h"
#include "debug.h"
#include "clients.h"
#include "wx/wx.h"

RecordOptions g_options;
ClientInfo g_info;

bool writeU32(uint32_t* address, const uint32_t value)
{
	MEMORY_BASIC_INFORMATION mbi;
	DWORD old;
	if(!VirtualQuery((void*)address, &mbi, sizeof(mbi))){
		Debug::printf(DEBUG_ERROR, "VirtualQuery error\n");
		return false;
	}
	if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_READWRITE, &old)){
		Debug::printf(DEBUG_ERROR, "VirtualProtect error\n");
		return false;
	}
	*address = value;
	return true;
}

bool getRecordOptions()
{
	HANDLE hFileMapping = OpenFileMapping(PAGE_READWRITE, FALSE, wxT("TibiaMovie1"));
	if(!hFileMapping){
		Debug::printf(DEBUG_ERROR, ("Error opening file mapping\n"));
		return false;
	}
	LPVOID m_pvData = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
	if(!m_pvData){
		Debug::printf(DEBUG_ERROR, "Error creating map view\n");
		CloseHandle(hFileMapping);
		return false;
	}

	memcpy(&g_options, m_pvData, sizeof(g_options));

	UnmapViewOfFile(m_pvData);
	CloseHandle(hFileMapping);

	g_info = getClientInfo(g_options.client);
	Debug::printf(DEBUG_INFO, "version: %d.%d.%d\n", g_info.major, g_info.minor, g_info.revision);

	return true;
}
