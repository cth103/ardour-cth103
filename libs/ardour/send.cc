/*
    Copyright (C) 2000 Paul Davis 

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

#include <algorithm>

#include "pbd/xml++.h"

#include "ardour/send.h"
#include "ardour/session.h"
#include "ardour/port.h"
#include "ardour/audio_port.h"
#include "ardour/buffer_set.h"
#include "ardour/meter.h"
#include "ardour/panner.h"
#include "ardour/io.h"

#include "i18n.h"

using namespace ARDOUR;
using namespace PBD;

Send::Send (Session& s)
	: IOProcessor (s, string_compose (_("send %1"), (_bitslot = s.next_send_id()) + 1))
{
	_metering = false;
	ProcessorCreated (this); /* EMIT SIGNAL */
}

Send::Send (Session& s, const XMLNode& node)
	: IOProcessor (s, "send")
{
	_metering = false;

	if (set_state (node)) {
		throw failed_constructor();
	}

	ProcessorCreated (this); /* EMIT SIGNAL */
}

Send::~Send ()
{
	GoingAway ();
}

XMLNode&
Send::get_state(void)
{
	return state (true);
}

XMLNode&
Send::state(bool full)
{
	XMLNode& node = IOProcessor::state(full);
	char buf[32];
	node.add_property ("type", "send");
	snprintf (buf, sizeof (buf), "%" PRIu32, _bitslot);
	node.add_property ("bitslot", buf);

	return node;
}

int
Send::set_state(const XMLNode& node)
{
	XMLNodeList nlist = node.children();
	XMLNodeIterator niter;
	const XMLProperty* prop;

	if ((prop = node.property ("bitslot")) == 0) {
		_bitslot = _session.next_send_id();
	} else {
		sscanf (prop->value().c_str(), "%" PRIu32, &_bitslot);
		_session.mark_send_id (_bitslot);
	}

	const XMLNode* insert_node = &node;

	/* Send has regular IO automation (gain, pan) */

	for (niter = nlist.begin(); niter != nlist.end(); ++niter) {
		if ((*niter)->name() == "IOProcessor") {
			insert_node = *niter;
		} else if ((*niter)->name() == X_("Automation")) {
			// _io->set_automation_state (*(*niter), Evoral::Parameter(GainAutomation));
		}
	}
	
	IOProcessor::set_state (*insert_node);

	return 0;
}

void
Send::run_in_place (BufferSet& bufs, nframes_t start_frame, nframes_t end_frame, nframes_t nframes)
{
	if (active()) {

		_io->deliver_output (bufs, start_frame, end_frame, nframes);

		if (_metering) {
			if (_io->effective_gain() == 0) {
				_io->peak_meter().reset();
			} else {
				_io->peak_meter().run_in_place(_io->output_buffers(), start_frame, end_frame, nframes);
			}
		}

	} else {
		_io->silence (nframes);
		
		if (_metering) {
			_io->peak_meter().reset();
		}
	}
}

void
Send::set_metering (bool yn)
{
	_metering = yn;

	if (!_metering) {
		/* XXX possible thread hazard here */
		_io->peak_meter().reset();
	}
}

bool
Send::can_support_io_configuration (const ChanCount& in, ChanCount& out) const
{
	if (_io->input_maximum() == ChanCount::INFINITE
			&& _io->output_maximum() == ChanCount::INFINITE) {

		/* not configured yet, we can support anything */

		out = in;
		return true; /* we can support anything the first time we're asked */

	} else {

		/* for a send, processor input corresponds to IO output */

		if (_io->output_maximum() == in) {
			out = in;
			return true;
		} 
	}

	return false;
}

bool
Send::configure_io (ChanCount in, ChanCount out)
{
	/* we're transparent no matter what.  fight the power. */

	if (out != in) {
		return false;
	}

	/*_io->set_output_maximum (in);
	_io->set_output_minimum (in);
	_io->set_input_maximum (ChanCount::ZERO);
	_io->set_input_minimum (ChanCount::ZERO);*/

	if (_io->ensure_io (ChanCount::ZERO, in, false, this) != 0) {
		return false;
	}

	Processor::configure_io(in, out);
	_io->reset_panner();

	return true;
}

/** Set up the XML description of a send so that its name is unique.
 *  @param state XML send state.
 *  @param session Session.
 */
void
Send::make_unique (XMLNode &state, Session &session)
{
	uint32_t const bitslot = session.next_send_id() + 1;

	char buf[32];
	snprintf (buf, sizeof (buf), "%" PRIu32, bitslot);
	state.property("bitslot")->set_value (buf);

	std::string const name = string_compose (_("send %1"), bitslot);
	
	state.property("name")->set_value (name);

	XMLNode* io = state.child ("IO");
	if (io) {
		io->property("name")->set_value (name);
	}
}
