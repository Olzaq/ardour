/*
  Copyright (C) 2004 Paul Davis 
  Copyright (C) 2004 Grame 

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

*/

#include <fcntl.h>
#include <cerrno>

#include <midi++/coremidi_midiport.h>
#include <midi++/types.h>
#include <midi++/port_request.h>
#include <mach/mach_time.h>

#include <pbd/pthread_utils.h>

using namespace std;
using namespace MIDI;

MIDITimeStamp CoreMidi_MidiPort::MIDIGetCurrentHostTime()
{
	return mach_absolute_time();
}

CoreMidi_MidiPort::CoreMidi_MidiPort (PortRequest &req) : Port (req)
{
	firstrecv = true;
	int err;
	if (0 == (err = Open(req))) {
		_ok = true;
		req.status = PortRequest::OK;
	} else
		req.status = PortRequest::Unknown;
}

CoreMidi_MidiPort::~CoreMidi_MidiPort () {Close();}

XMLNode&
CoreMidi::MidiPort::get_state() const
{
	XMLNode& node (Port::get_state());
	node.add_property ("type", "coremidi");
	return node;
}


void CoreMidi_MidiPort::Close ()
{
 	if (midi_destination) MIDIEndpointDispose(midi_destination);
	if (midi_source) MIDIEndpointDispose(midi_source);
	if (midi_client) MIDIClientDispose(midi_client);
}

int CoreMidi_MidiPort::write (byte *msg, size_t msglen)	
{
	OSStatus err;
    MIDIPacketList* pktlist = (MIDIPacketList*)midi_buffer;
	MIDIPacket* packet = MIDIPacketListInit(pktlist);
	packet = MIDIPacketListAdd(pktlist,sizeof(midi_buffer),packet,MIDIGetCurrentHostTime(),msglen,msg);
	
	if (packet) {
		
		err = MIDIReceived(midi_source,pktlist);
		if (err != noErr) {
			//error << "MIDIReceived error" << err << endmsg.
		}
		
		bytes_written += msglen;
		return msglen;
	}else{
		return 0;
	}
}

int CoreMidi_MidiPort::Open (PortRequest &req)
{
	OSStatus err;
	CFStringRef coutputStr;
	string str;

	coutputStr = CFStringCreateWithCString(0, req.devname, CFStringGetSystemEncoding());
	err = MIDIClientCreate(coutputStr, 0, 0, &midi_client);
	CFRelease(coutputStr);
    if (!midi_client) {
		//error << "Cannot open CoreMidi client : " << err << endmsg.
        goto error;
    }
  	
	str = req.tagname + string("_in");
	coutputStr = CFStringCreateWithCString(0, str.c_str(), CFStringGetSystemEncoding());
	err = MIDIDestinationCreate(midi_client, coutputStr, read_proc, this, &midi_destination);
	CFRelease(coutputStr);
	if (!midi_destination) {
		//error << "Cannot create CoreMidi destination : " << err << endmsg.
		goto error;
	}
	
	str = req.tagname + string("_out");
	coutputStr = CFStringCreateWithCString(0, str.c_str(), CFStringGetSystemEncoding());
	err = MIDISourceCreate(midi_client, coutputStr, &midi_source);
	CFRelease(coutputStr);
	if (!midi_source) {
		//error << "Cannot create CoreMidi source : " << err << endmsg.
		goto error;
	}	
   
    return err;
    
error:
    Close();
	return err;
}

void CoreMidi_MidiPort::read_proc (const MIDIPacketList *pktlist, void *refCon, void *connRefCon)
{
    CoreMidi_MidiPort* driver = (CoreMidi_MidiPort*)refCon;
    MIDIPacket *packet = (MIDIPacket *)pktlist->packet;	

    if (driver->firstrecv) {
	    driver->firstrecv = false;
	    PBD::ThreadCreated (pthread_self(), "COREMIDI");
    }

    for (unsigned int i = 0; i < pktlist->numPackets; ++i) {
    
        driver->bytes_read += packet->length;
		
	    if (driver->input_parser) {
			driver->input_parser->raw_preparse (*driver->input_parser, packet->data, packet->length);
			for (int i = 0; i < packet->length; i++) {
				driver->input_parser->scanner (packet->data[i]);
			}	
			driver->input_parser->raw_postparse (*driver->input_parser, packet->data, packet->length);
		}
                 
        packet = MIDIPacketNext(packet);
    }
}

int
CoreMidi_MidiPort::discover (vector<PortSet>& ports)
{
	/* XXX do dynamic port discovery here */

	return 0;
}
