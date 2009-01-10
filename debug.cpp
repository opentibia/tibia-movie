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

#include "debug.h"
#include "stdio.h"
#include "stdarg.h"

FILE* g_debug = NULL;

void Debug::start(const char* name)
{
	g_debug = fopen(name, "w");
	fprintf(g_debug, "Start client\n");
}

void Debug::stop()
{
	fprintf(g_debug, "End client\n");
	fclose(g_debug);
}

int Debug::printf(const char* format, ...)
{
	va_list listPtr;
	va_start(listPtr, format);
	int ret =  vfprintf(g_debug, format, listPtr);
	va_end(listPtr);
	return ret;
}

