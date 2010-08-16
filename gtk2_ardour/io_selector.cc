/*
    Copyright (C) 2002-2007 Paul Davis

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

#include <gtkmm/messagedialog.h>
#include <glibmm/objectbase.h>

#include <gtkmm2ext/doi.h>

#include "ardour/port_insert.h"
#include "ardour/session.h"
#include "ardour/io.h"
#include "ardour/audioengine.h"
#include "ardour/track.h"
#include "ardour/audio_track.h"
#include "ardour/midi_track.h"
#include "ardour/mtdm.h"
#include "ardour/data_type.h"
#include "ardour/port.h"
#include "ardour/bundle.h"

#include "io_selector.h"
#include "utils.h"
#include "gui_thread.h"
#include "i18n.h"

using namespace ARDOUR;
using namespace Gtk;

IOSelector::IOSelector (Gtk::Window* p, ARDOUR::Session* session, boost::shared_ptr<ARDOUR::IO> io)
	: PortMatrix (p, session, DataType::NIL)
	, _io (io)
{
	setup_type ();
	
	/* signal flow from 0 to 1 */

	_find_inputs_for_io_outputs = (_io->direction() == IO::Output);

	if (_find_inputs_for_io_outputs) {
		_other = 1;
		_ours = 0;
	} else {
		_other = 0;
		_ours = 1;
	}

	_port_group.reset (new PortGroup (io->name()));
	_ports[_ours].add_group (_port_group);

	io->changed.connect (_io_connection, invalidator (*this), boost::bind (&IOSelector::io_changed_proxy, this), gui_context ());

	setup_all_ports ();
	init ();
}

void
IOSelector::setup_type ()
{
	/* set type according to what's in the IO */

	int N = 0;
	DataType type_with_ports = DataType::NIL;
	for (DataType::iterator i = DataType::begin(); i != DataType::end(); ++i) {
		if (_io->ports().num_ports (*i)) {
			type_with_ports = *i;
			++N;
		}
	}

	if (N <= 1) {
		set_type (type_with_ports);
	} else {
		set_type (DataType::NIL);
	}
}

void
IOSelector::io_changed_proxy ()
{
	/* The IO's changed signal is emitted from code that holds its route's processor lock,
	   so we can't call setup_all_ports (which results in a call to Route::foreach_processor)
	   without a deadlock unless we break things up with this idle handler.
	*/
	
	Glib::signal_idle().connect_once (sigc::mem_fun (*this, &IOSelector::io_changed));
}

void
IOSelector::io_changed ()
{
	setup_type ();
	setup_all_ports ();
}

void
IOSelector::setup_ports (int dim)
{
	if (!_session) {
		return;
	}

	_ports[dim].suspend_signals ();

	if (dim == _other) {

		_ports[_other].gather (_session, type(), _find_inputs_for_io_outputs, false);

	} else {

		_port_group->clear ();
		_port_group->add_bundle (_io->bundle (), _io);
	}

	_ports[dim].resume_signals ();
}

void
IOSelector::set_state (ARDOUR::BundleChannel c[2], bool s)
{
	ARDOUR::Bundle::PortList const & our_ports = c[_ours].bundle->channel_ports (c[_ours].channel);
	ARDOUR::Bundle::PortList const & other_ports = c[_other].bundle->channel_ports (c[_other].channel);

	for (ARDOUR::Bundle::PortList::const_iterator i = our_ports.begin(); i != our_ports.end(); ++i) {
		for (ARDOUR::Bundle::PortList::const_iterator j = other_ports.begin(); j != other_ports.end(); ++j) {

			Port* f = _session->engine().get_port_by_name (*i);
			if (!f) {
				return;
			}

			if (s) {
				_io->connect (f, *j, 0);
			} else {
				_io->disconnect (f, *j, 0);
			}
		}
	}
}

PortMatrixNode::State
IOSelector::get_state (ARDOUR::BundleChannel c[2]) const
{
	ARDOUR::Bundle::PortList const & our_ports = c[_ours].bundle->channel_ports (c[_ours].channel);
	ARDOUR::Bundle::PortList const & other_ports = c[_other].bundle->channel_ports (c[_other].channel);

	if (!_session || our_ports.empty() || other_ports.empty()) {
		/* we're looking at a bundle with no parts associated with this channel,
		   so nothing to connect */
		return PortMatrixNode::NOT_ASSOCIATED;
	}

	for (ARDOUR::Bundle::PortList::const_iterator i = our_ports.begin(); i != our_ports.end(); ++i) {
		for (ARDOUR::Bundle::PortList::const_iterator j = other_ports.begin(); j != other_ports.end(); ++j) {

			Port* f = _session->engine().get_port_by_name (*i);

			/* since we are talking about an IO, our ports should all have an associated Port *,
			   so the above call should never fail */
			assert (f);

			if (!f->connected_to (*j)) {
				/* if any one thing is not connected, all bets are off */
				return PortMatrixNode::NOT_ASSOCIATED;
			}
		}
	}

	return PortMatrixNode::ASSOCIATED;
}

uint32_t
IOSelector::n_io_ports () const
{
	if (!_find_inputs_for_io_outputs) {
		return _io->n_ports().get (_io->default_type());
	} else {
		return _io->n_ports().get (_io->default_type());
	}
}

bool
IOSelector::list_is_global (int dim) const
{
	return (dim == _other);
}

string
IOSelector::disassociation_verb () const
{
	return _("Disconnect");
}

string
IOSelector::channel_noun () const
{
	return _("port");
}

IOSelectorWindow::IOSelectorWindow (ARDOUR::Session* session, boost::shared_ptr<ARDOUR::IO> io, bool /*can_cancel*/)
	: _selector (this, session, io)
{
	set_name ("IOSelectorWindow2");
	set_title (_("I/O selector"));

	add (_selector);

	set_position (Gtk::WIN_POS_MOUSE);

	io_name_changed (this);

	show_all ();

	signal_delete_event().connect (sigc::mem_fun (*this, &IOSelectorWindow::wm_delete));
}

bool
IOSelectorWindow::wm_delete (GdkEventAny* /*event*/)
{
	_selector.Finished (IOSelector::Accepted);
	hide ();
	return true;
}


void
IOSelectorWindow::on_map ()
{
	_selector.setup_all_ports ();
	Window::on_map ();
}

void
IOSelectorWindow::on_show ()
{
	Gtk::Window::on_show ();
	pair<uint32_t, uint32_t> const pm_max = _selector.max_size ();
	resize_window_to_proportion_of_monitor (this, pm_max.first, pm_max.second);
}

void
IOSelectorWindow::io_name_changed (void* src)
{
	ENSURE_GUI_THREAD (*this, &IOSelectorWindow::io_name_changed, src)

	string title;

	if (!_selector.find_inputs_for_io_outputs()) {
		title = string_compose(_("%1 input"), _selector.io()->name());
	} else {
		title = string_compose(_("%1 output"), _selector.io()->name());
	}

	set_title (title);
}

PortInsertUI::PortInsertUI (Gtk::Window* parent, ARDOUR::Session* sess, boost::shared_ptr<ARDOUR::PortInsert> pi)
        : _pi (pi)
        , latency_button (_("Measure Latency"))
        , input_selector (parent, sess, pi->input())
        , output_selector (parent, sess, pi->output())
{
        latency_hbox.pack_start (latency_button, false, false);
        latency_hbox.pack_start (latency_display, false, false);
        latency_frame.add (latency_hbox);
        
	output_selector.set_min_height_divisor (2);
	input_selector.set_min_height_divisor (2);
        
        pack_start (latency_frame);
	pack_start (output_selector, true, true);
	pack_start (input_selector, true, true);

        update_latency_display ();

        latency_button.signal_toggled().connect (mem_fun (*this, &PortInsertUI::latency_button_toggled));
}


void
PortInsertUI::update_latency_display ()
{
        nframes_t sample_rate = input_selector.session()->engine().frame_rate();
        if (sample_rate == 0) {
                latency_display.set_text (_("Disconnected from audio engine"));
        } else {
                char buf[64];
                snprintf (buf, sizeof (buf), "%10.3lf frames %10.3lf ms", 
                          (float)_pi->latency(), (float)_pi->latency() * 1000.0f/sample_rate);
                latency_display.set_text(buf);
        }
}

bool
PortInsertUI::check_latency_measurement ()
{
        MTDM* mtdm = _pi->mtdm ();

        if (mtdm->resolve () < 0) {
                latency_display.set_text (_("No signal detected"));
                return true;
        }

        if (mtdm->err () > 0.3) {
                mtdm->invert ();
                mtdm->resolve ();
        }

        char buf[128];
        nframes_t sample_rate = AudioEngine::instance()->frame_rate();

        if (sample_rate == 0) {
                latency_display.set_text (_("Disconnected from audio engine"));
                _pi->stop_latency_detection ();
                return false;
        }

        snprintf (buf, sizeof (buf), "%10.3lf frames %10.3lf ms", mtdm->del (), mtdm->del () * 1000.0f/sample_rate);

        bool solid = true;

        if (mtdm->err () > 0.2) {
                strcat (buf, " ??");
                solid = false;
        }

        if (mtdm->inv ()) {
                strcat (buf, " (Inv)");
                solid = false;
        }

        if (solid) {
                _pi->set_measured_latency ((nframes_t) rint (mtdm->del()));
                latency_button.set_active (false);
                strcat (buf, " (set)");
        }

        latency_display.set_text (buf);
        return true;
}

void
PortInsertUI::latency_button_toggled ()
{
        if (latency_button.get_active ()) {

                _pi->start_latency_detection ();
                latency_display.set_text (_("Detecting ..."));
                latency_timeout = Glib::signal_timeout().connect (mem_fun (*this, &PortInsertUI::check_latency_measurement), 250);

        } else {
                _pi->stop_latency_detection ();
                latency_timeout.disconnect ();
        }
}


void
PortInsertUI::redisplay ()
{
	input_selector.setup_ports (input_selector.other());
	output_selector.setup_ports (output_selector.other());
}

void
PortInsertUI::finished (IOSelector::Result r)
{
	input_selector.Finished (r);
	output_selector.Finished (r);
}


PortInsertWindow::PortInsertWindow (ARDOUR::Session* sess, boost::shared_ptr<ARDOUR::PortInsert> pi, bool can_cancel)
	: ArdourDialog ("port insert dialog"),
	  _portinsertui (this, sess, pi),
	  ok_button (can_cancel ? _("OK"): _("Close")),
	  cancel_button (_("Cancel"))
{

	set_name ("IOSelectorWindow");
	string title = _("Port Insert ");
	title += pi->name();
	set_title (title);

	ok_button.set_name ("IOSelectorButton");
	if (!can_cancel) {
		ok_button.set_image (*Gtk::manage (new Gtk::Image (Gtk::Stock::CLOSE, Gtk::ICON_SIZE_BUTTON)));
	}
	cancel_button.set_name ("IOSelectorButton");

	if (can_cancel) {
		cancel_button.set_image (*Gtk::manage (new Gtk::Image (Gtk::Stock::CANCEL, Gtk::ICON_SIZE_BUTTON)));
		get_action_area()->pack_start (cancel_button, false, false);
	} else {
		cancel_button.hide();
	}
	get_action_area()->pack_start (ok_button, false, false);

	get_vbox()->pack_start (_portinsertui);

	ok_button.signal_clicked().connect (sigc::mem_fun (*this, &PortInsertWindow::accept));
	cancel_button.signal_clicked().connect (sigc::mem_fun (*this, &PortInsertWindow::cancel));

	signal_delete_event().connect (sigc::mem_fun (*this, &PortInsertWindow::wm_delete), false);

	pi->DropReferences.connect (going_away_connection, invalidator (*this), boost::bind (&PortInsertWindow::plugin_going_away, this), gui_context());
}

bool
PortInsertWindow::wm_delete (GdkEventAny* /*event*/)
{
	accept ();
	return true;
}

void
PortInsertWindow::plugin_going_away ()
{
	ENSURE_GUI_THREAD (*this, &PortInsertWindow::plugin_going_away)

	going_away_connection.disconnect ();
	delete_when_idle (this);
}

void
PortInsertWindow::on_map ()
{
	_portinsertui.redisplay ();
	Window::on_map ();
}


void
PortInsertWindow::cancel ()
{
	_portinsertui.finished (IOSelector::Cancelled);
	hide ();
}

void
PortInsertWindow::accept ()
{
	_portinsertui.finished (IOSelector::Accepted);
	hide ();
}

