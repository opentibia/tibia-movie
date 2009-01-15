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

bool writeU32(uint32_t* address, const uint32_t value)
{
	MEMORY_BASIC_INFORMATION mbi;
	DWORD old;
	if(!VirtualQuery((void*)address, &mbi, sizeof(mbi))){
		return false;
	}
	if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_READWRITE, &old)){
		return false;
	}
	*address = value;
	return true;
}
