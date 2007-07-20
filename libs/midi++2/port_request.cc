/*
    Copyright (C) 2000 Paul Barton-Davis 

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    $Id$
*/

#include <midi++/port.h>
#include <midi++/port_request.h>
#include <midi++/factory.h>

using namespace std;
using namespace MIDI;

PortRequest::PortRequest (const string &xdev, 
			  const string &xtag, 
			  const string &xmode,
			  const string &xtype)

{
	status = OK;
	
	devname = strdup (xdev.c_str());
	tagname = strdup (xtag.c_str());
	mode = PortFactory::string_to_mode (xmode);
	type = PortFactory::string_to_type (xtype);
}

