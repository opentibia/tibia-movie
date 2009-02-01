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

#include "config.h"
#include "../debug.h"

#include <wx/filename.h>

Config::Config() :
wxFileConfig("tibiamovie", wxEmptyString, "./tibiamovie.cfg",
	wxEmptyString, wxCONFIG_USE_RELATIVE_PATH)
{
	//
}

Config::~Config()
{
	//
}

wxString Config::getClientPath(ClientVersion version)
{
	//save previous path to restore it later
	wxString oldPath = GetPath();

	SetPath("clients");
	wxString clientStr;
	clientStr.sprintf("%d.%d.%d", version.major, version.minor, version.revision);
	if(Exists(clientStr)){
		wxString wxPath;
		if(Read(clientStr, &wxPath)){
			//restore old path
			SetPath(oldPath);
			return wxPath;
		}
	}
	//restore old path
	SetPath(oldPath);
	return wxEmptyString;
}


void Config::setClientPath(ClientVersion version, const wxString& path)
{
	//save previous path to restore it later
	wxString oldPath = GetPath();

	SetPath("clients");
	wxString clientStr, wxPath;

	clientStr.sprintf("%d.%d.%d", version.major, version.minor, version.revision);
	Write(clientStr, path);

	//restore path
	SetPath(oldPath);
}
