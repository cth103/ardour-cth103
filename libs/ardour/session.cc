/*
    Copyright (C) 1999-2010 Paul Davis

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

#include <stdint.h>

#include <algorithm>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <cstdio> /* sprintf(3) ... grrr */
#include <cmath>
#include <cerrno>
#include <unistd.h>
#include <limits.h>

#include <glibmm/thread.h>
#include <glibmm/miscutils.h>
#include <glibmm/fileutils.h>

#include <boost/algorithm/string/erase.hpp>

#include "pbd/error.h"
#include "pbd/boost_debug.h"
#include "pbd/pathscanner.h"
#include "pbd/stl_delete.h"
#include "pbd/basename.h"
#include "pbd/stacktrace.h"
#include "pbd/file_utils.h"
#include "pbd/convert.h"
#include "pbd/strsplit.h"
#include "pbd/strsplit.h"
#include "pbd/unwind.h"

#include "ardour/amp.h"
#include "ardour/analyser.h"
#include "ardour/audio_buffer.h"
#include "ardour/audio_diskstream.h"
#include "ardour/audio_port.h"
#include "ardour/audio_track.h"
#include "ardour/audioengine.h"
#include "ardour/audiofilesource.h"
#include "ardour/auditioner.h"
#include "ardour/buffer_manager.h"
#include "ardour/buffer_set.h"
#include "ardour/bundle.h"
#include "ardour/butler.h"
#include "ardour/click.h"
#include "ardour/control_protocol_manager.h"
#include "ardour/data_type.h"
#include "ardour/debug.h"
#include "ardour/filename_extensions.h"
#include "ardour/graph.h"
#include "ardour/midi_track.h"
#include "ardour/midi_ui.h"
#include "ardour/named_selection.h"
#include "ardour/operations.h"
#include "ardour/playlist.h"
#include "ardour/plugin.h"
#include "ardour/plugin_insert.h"
#include "ardour/process_thread.h"
#include "ardour/rc_configuration.h"
#include "ardour/recent_sessions.h"
#include "ardour/region.h"
#include "ardour/region_factory.h"
#include "ardour/route_graph.h"
#include "ardour/route_group.h"
#include "ardour/send.h"
#include "ardour/session.h"
#include "ardour/session_directory.h"
#include "ardour/session_playlists.h"
#include "ardour/smf_source.h"
#include "ardour/source_factory.h"
#include "ardour/utils.h"

#include "midi++/port.h"
#include "midi++/jack_midi_port.h"
#include "midi++/mmc.h"
#include "midi++/manager.h"

#include "i18n.h"

namespace ARDOUR {
class MidiSource;
class Processor;
class Speakers;
}

using namespace std;
using namespace ARDOUR;
using namespace PBD;

bool Session::_disable_all_loaded_plugins = false;

PBD::Signal1<void,std::string> Session::Dialog;
PBD::Signal0<int> Session::AskAboutPendingState;
PBD::Signal2<int, framecnt_t, framecnt_t> Session::AskAboutSampleRateMismatch;
PBD::Signal0<void> Session::SendFeedback;
PBD::Signal3<int,Session*,std::string,DataType> Session::MissingFile;

PBD::Signal1<void, framepos_t> Session::StartTimeChanged;
PBD::Signal1<void, framepos_t> Session::EndTimeChanged;
PBD::Signal0<void> Session::AutoBindingOn;
PBD::Signal0<void> Session::AutoBindingOff;
PBD::Signal2<void,std::string, std::string> Session::Exported;
PBD::Signal1<int,boost::shared_ptr<Playlist> > Session::AskAboutPlaylistDeletion;
PBD::Signal0<void> Session::Quit;
PBD::Signal0<void> Session::FeedbackDetected;
PBD::Signal0<void> Session::SuccessfulGraphSort;

static void clean_up_session_event (SessionEvent* ev) { delete ev; }
const SessionEvent::RTeventCallback Session::rt_cleanup (clean_up_session_event);

/** @param snapshot_name Snapshot name, without .ardour prefix */
Session::Session (AudioEngine &eng,
                  const string& fullpath,
                  const string& snapshot_name,
                  BusProfile* bus_profile,
                  string mix_template)
	: _engine (eng)
	, _target_transport_speed (0.0)
	, _requested_return_frame (-1)
	, _session_dir (new SessionDirectory(fullpath))
	, state_tree (0)
	, _state_of_the_state (Clean)
	, _butler (new Butler (*this))
	, _post_transport_work (0)
	, _send_timecode_update (false)
	, _all_route_group (new RouteGroup (*this, "all"))
	, routes (new RouteList)
	, _total_free_4k_blocks (0)
	, _bundles (new BundleList)
	, _bundle_xml_node (0)
	, _current_trans (0)
	, _click_io ((IO*) 0)
	, click_data (0)
	, click_emphasis_data (0)
	, main_outs (0)
	, _have_rec_enabled_track (false)
	, _suspend_timecode_transmission (0)
{
	_locations = new Locations (*this);

	if (how_many_dsp_threads () > 1) {
		/* For now, only create the graph if we are using >1 DSP threads, as
		   it is a bit slower than the old code with 1 thread.
		*/
		_process_graph.reset (new Graph (*this));
	}

	playlists.reset (new SessionPlaylists);

	_all_route_group->set_active (true, this);

	interpolation.add_channel_to (0, 0);

	if (!eng.connected()) {
		throw failed_constructor();
	}

	n_physical_outputs = _engine.n_physical_outputs ();
	n_physical_inputs =  _engine.n_physical_inputs ();

	first_stage_init (fullpath, snapshot_name);

	_is_new = !Glib::file_test (_path, Glib::FileTest (G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR));

	if (_is_new) {
		if (create (mix_template, bus_profile)) {
			destroy ();
			throw failed_constructor ();
		}
	}

	if (second_stage_init ()) {
		destroy ();
		throw failed_constructor ();
	}

	store_recent_sessions(_name, _path);

	bool was_dirty = dirty();

	_state_of_the_state = StateOfTheState (_state_of_the_state & ~Dirty);

	Config->ParameterChanged.connect_same_thread (*this, boost::bind (&Session::config_changed, this, _1, false));
	config.ParameterChanged.connect_same_thread (*this, boost::bind (&Session::config_changed, this, _1, true));

	if (was_dirty) {
		DirtyChanged (); /* EMIT SIGNAL */
	}

	StartTimeChanged.connect_same_thread (*this, boost::bind (&Session::start_time_changed, this, _1));
	EndTimeChanged.connect_same_thread (*this, boost::bind (&Session::end_time_changed, this, _1));

	_is_new = false;
}

Session::~Session ()
{
#ifdef PT_TIMING	
	ST.dump ("ST.dump");
#endif	
	destroy ();
}

void
Session::destroy ()
{
	vector<void*> debug_pointers;

	/* if we got to here, leaving pending capture state around
	   is a mistake.
	*/

	remove_pending_capture_state ();

	_state_of_the_state = StateOfTheState (CannotSave|Deletion);

	_engine.remove_session ();

	/* clear history so that no references to objects are held any more */

	_history.clear ();

	/* clear state tree so that no references to objects are held any more */

	delete state_tree;

	/* reset dynamic state version back to default */

	Stateful::loading_state_version = 0;

	_butler->drop_references ();
	delete _butler;
	delete midi_control_ui;
	delete _all_route_group;

	if (click_data != default_click) {
		delete [] click_data;
	}

	if (click_emphasis_data != default_click_emphasis) {
		delete [] click_emphasis_data;
	}

	clear_clicks ();

	/* clear out any pending dead wood from RCU managed objects */

	routes.flush ();
	_bundles.flush ();

	AudioDiskstream::free_working_buffers();

	/* tell everyone who is still standing that we're about to die */
	drop_references ();

	/* tell everyone to drop references and delete objects as we go */

	DEBUG_TRACE (DEBUG::Destruction, "delete named selections\n");
	named_selections.clear ();

	DEBUG_TRACE (DEBUG::Destruction, "delete regions\n");
	RegionFactory::delete_all_regions ();

	DEBUG_TRACE (DEBUG::Destruction, "delete routes\n");

	/* reset these three references to special routes before we do the usual route delete thing */

	auditioner.reset ();
	_master_out.reset ();
	_monitor_out.reset ();

	{
		RCUWriter<RouteList> writer (routes);
		boost::shared_ptr<RouteList> r = writer.get_copy ();

		for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
			DEBUG_TRACE(DEBUG::Destruction, string_compose ("Dropping for route %1 ; pre-ref = %2\n", (*i)->name(), (*i).use_count()));
			(*i)->drop_references ();
		}

		r->clear ();
		/* writer goes out of scope and updates master */
	}
	routes.flush ();

	DEBUG_TRACE (DEBUG::Destruction, "delete sources\n");
	for (SourceMap::iterator i = sources.begin(); i != sources.end(); ++i) {
		DEBUG_TRACE(DEBUG::Destruction, string_compose ("Dropping for source %1 ; pre-ref = %2\n", i->second->name(), i->second.use_count()));
		i->second->drop_references ();
	}

	sources.clear ();

	DEBUG_TRACE (DEBUG::Destruction, "delete route groups\n");
	for (list<RouteGroup *>::iterator i = _route_groups.begin(); i != _route_groups.end(); ++i) {

		delete *i;
	}

	/* not strictly necessary, but doing it here allows the shared_ptr debugging to work */
	playlists.reset ();

	delete _locations;

	DEBUG_TRACE (DEBUG::Destruction, "Session::destroy() done\n");

#ifdef BOOST_SP_ENABLE_DEBUG_HOOKS
	boost_debug_list_ptrs ();
#endif
}

void
Session::when_engine_running ()
{
	string first_physical_output;

	BootMessage (_("Set block size and sample rate"));

	set_block_size (_engine.frames_per_cycle());
	set_frame_rate (_engine.frame_rate());

	BootMessage (_("Using configuration"));

	boost::function<void (std::string)> ff (boost::bind (&Session::config_changed, this, _1, false));
	boost::function<void (std::string)> ft (boost::bind (&Session::config_changed, this, _1, true));

	Config->map_parameters (ff);
	config.map_parameters (ft);

	/* every time we reconnect, recompute worst case output latencies */

	_engine.Running.connect_same_thread (*this, boost::bind (&Session::initialize_latencies, this));

	if (synced_to_jack()) {
		_engine.transport_stop ();
	}

	if (config.get_jack_time_master()) {
		_engine.transport_locate (_transport_frame);
	}

	_clicking = false;

	try {
		XMLNode* child = 0;

		_click_io.reset (new ClickIO (*this, "click"));
		_click_gain.reset (new Amp (*this));
		_click_gain->activate ();

		if (state_tree && (child = find_named_node (*state_tree->root(), "Click")) != 0) {

			/* existing state for Click */
			int c = 0;

			if (Stateful::loading_state_version < 3000) {
				c = _click_io->set_state_2X (*child->children().front(), Stateful::loading_state_version, false);
			} else {
				const XMLNodeList& children (child->children());
				XMLNodeList::const_iterator i = children.begin();
				if ((c = _click_io->set_state (**i, Stateful::loading_state_version)) == 0) {
					++i;
					if (i != children.end()) {
						c = _click_gain->set_state (**i, Stateful::loading_state_version);
					}
				}
			}
			
			if (c == 0) {
				_clicking = Config->get_clicking ();

			} else {

				error << _("could not setup Click I/O") << endmsg;
				_clicking = false;
			}


		} else {

			/* default state for Click: dual-mono to first 2 physical outputs */

			vector<string> outs;
			_engine.get_physical_outputs (DataType::AUDIO, outs);

			for (uint32_t physport = 0; physport < 2; ++physport) {
				if (outs.size() > physport) {
					if (_click_io->add_port (outs[physport], this)) {
						// relax, even though its an error
					}
				}
			}

			if (_click_io->n_ports () > ChanCount::ZERO) {
				_clicking = Config->get_clicking ();
			}
		}
	}

	catch (failed_constructor& err) {
		error << _("cannot setup Click I/O") << endmsg;
	}

	BootMessage (_("Compute I/O Latencies"));

	if (_clicking) {
		// XXX HOW TO ALERT UI TO THIS ? DO WE NEED TO?
	}

	BootMessage (_("Set up standard connections"));

	vector<string> inputs[DataType::num_types];
	vector<string> outputs[DataType::num_types];
	for (uint32_t i = 0; i < DataType::num_types; ++i) {
		_engine.get_physical_inputs (DataType (DataType::Symbol (i)), inputs[i]);
		_engine.get_physical_outputs (DataType (DataType::Symbol (i)), outputs[i]);
	}

	/* Create a set of Bundle objects that map
	   to the physical I/O currently available.  We create both
	   mono and stereo bundles, so that the common cases of mono
	   and stereo tracks get bundles to put in their mixer strip
	   in / out menus.  There may be a nicer way of achieving that;
	   it doesn't really scale that well to higher channel counts
	*/

	/* mono output bundles */

	for (uint32_t np = 0; np < outputs[DataType::AUDIO].size(); ++np) {
		char buf[32];
		snprintf (buf, sizeof (buf), _("out %" PRIu32), np+1);

		boost::shared_ptr<Bundle> c (new Bundle (buf, true));
		c->add_channel (_("mono"), DataType::AUDIO);
		c->set_port (0, outputs[DataType::AUDIO][np]);

		add_bundle (c);
	}

	/* stereo output bundles */

	for (uint32_t np = 0; np < outputs[DataType::AUDIO].size(); np += 2) {
		if (np + 1 < outputs[DataType::AUDIO].size()) {
			char buf[32];
			snprintf (buf, sizeof(buf), _("out %" PRIu32 "+%" PRIu32), np + 1, np + 2);
                        boost::shared_ptr<Bundle> c (new Bundle (buf, true));
			c->add_channel (_("L"), DataType::AUDIO);
			c->set_port (0, outputs[DataType::AUDIO][np]);
			c->add_channel (_("R"), DataType::AUDIO);
			c->set_port (1, outputs[DataType::AUDIO][np + 1]);

			add_bundle (c);
		}
	}

	/* mono input bundles */

	for (uint32_t np = 0; np < inputs[DataType::AUDIO].size(); ++np) {
		char buf[32];
		snprintf (buf, sizeof (buf), _("in %" PRIu32), np+1);

		boost::shared_ptr<Bundle> c (new Bundle (buf, false));
		c->add_channel (_("mono"), DataType::AUDIO);
		c->set_port (0, inputs[DataType::AUDIO][np]);

		add_bundle (c);
	}

	/* stereo input bundles */

	for (uint32_t np = 0; np < inputs[DataType::AUDIO].size(); np += 2) {
		if (np + 1 < inputs[DataType::AUDIO].size()) {
			char buf[32];
			snprintf (buf, sizeof(buf), _("in %" PRIu32 "+%" PRIu32), np + 1, np + 2);

			boost::shared_ptr<Bundle> c (new Bundle (buf, false));
			c->add_channel (_("L"), DataType::AUDIO);
			c->set_port (0, inputs[DataType::AUDIO][np]);
			c->add_channel (_("R"), DataType::AUDIO);
			c->set_port (1, inputs[DataType::AUDIO][np + 1]);

			add_bundle (c);
		}
	}

	/* MIDI input bundles */

	for (uint32_t np = 0; np < inputs[DataType::MIDI].size(); ++np) {
		string n = inputs[DataType::MIDI][np];
		boost::erase_first (n, X_("alsa_pcm:"));

		boost::shared_ptr<Bundle> c (new Bundle (n, false));
		c->add_channel ("", DataType::MIDI);
		c->set_port (0, inputs[DataType::MIDI][np]);
		add_bundle (c);
	}

	/* MIDI output bundles */

	for (uint32_t np = 0; np < outputs[DataType::MIDI].size(); ++np) {
		string n = outputs[DataType::MIDI][np];
		boost::erase_first (n, X_("alsa_pcm:"));

		boost::shared_ptr<Bundle> c (new Bundle (n, true));
		c->add_channel ("", DataType::MIDI);
		c->set_port (0, outputs[DataType::MIDI][np]);
		add_bundle (c);
	}

	BootMessage (_("Setup signal flow and plugins"));

	/* Reset all panners */

	Delivery::reset_panners ();

	/* this will cause the CPM to instantiate any protocols that are in use
	 * (or mandatory), which will pass it this Session, and then call
	 * set_state() on each instantiated protocol to match stored state.
	 */

	ControlProtocolManager::instance().set_session (this);

	/* This must be done after the ControlProtocolManager set_session above,
	   as it will set states for ports which the ControlProtocolManager creates.
	*/

	MIDI::Manager::instance()->set_port_states (Config->midi_port_states ());

	/* And this must be done after the MIDI::Manager::set_port_states as
	 * it will try to make connections whose details are loaded by set_port_states.
	 */

	hookup_io ();

	/* Let control protocols know that we are now all connected, so they
	 * could start talking to surfaces if they want to.
	 */

	ControlProtocolManager::instance().midi_connectivity_established ();

	if (_is_new && !no_auto_connect()) {
		Glib::Mutex::Lock lm (AudioEngine::instance()->process_lock());
		auto_connect_master_bus ();
	}

	_state_of_the_state = StateOfTheState (_state_of_the_state & ~(CannotSave|Dirty));

        /* update latencies */

        initialize_latencies ();

	/* hook us up to the engine */

	BootMessage (_("Connect to engine"));
	_engine.set_session (this);
}

void
Session::auto_connect_master_bus ()
{
	if (!_master_out || !Config->get_auto_connect_standard_busses() || _monitor_out) {
		return;
	}
		
	/* if requested auto-connect the outputs to the first N physical ports.
	 */
	
	uint32_t limit = _master_out->n_outputs().n_total();
	vector<string> outputs[DataType::num_types];
	
	for (uint32_t i = 0; i < DataType::num_types; ++i) {
		_engine.get_physical_outputs (DataType (DataType::Symbol (i)), outputs[i]);
	}
	
	for (uint32_t n = 0; n < limit; ++n) {
		boost::shared_ptr<Port> p = _master_out->output()->nth (n);
		string connect_to;
		if (outputs[p->type()].size() > n) {
			connect_to = outputs[p->type()][n];
		}
		
		if (!connect_to.empty() && p->connected_to (connect_to) == false) {
			if (_master_out->output()->connect (p, connect_to, this)) {
				error << string_compose (_("cannot connect master output %1 to %2"), n, connect_to)
				      << endmsg;
				break;
			}
		}
	}
}

void
Session::remove_monitor_section ()
{
	if (!_monitor_out) {
		return;
	}

	/* force reversion to Solo-In-Place */
	Config->set_solo_control_is_listen_control (false);

	{
		/* Hold process lock while doing this so that we don't hear bits and
		 * pieces of audio as we work on each route.
		 */
		
		Glib::Mutex::Lock lm (AudioEngine::instance()->process_lock ());
		
		/* Connect tracks to monitor section. Note that in an
		   existing session, the internal sends will already exist, but we want the
		   routes to notice that they connect to the control out specifically.
		*/
		
		
		boost::shared_ptr<RouteList> r = routes.reader ();
		PBD::Unwinder<bool> uw (ignore_route_processor_changes, true);
		
		for (RouteList::iterator x = r->begin(); x != r->end(); ++x) {
			
			if ((*x)->is_monitor()) {
				/* relax */
			} else if ((*x)->is_master()) {
				/* relax */
			} else {
				(*x)->remove_aux_or_listen (_monitor_out);
			}
		}
	}

	remove_route (_monitor_out);
	auto_connect_master_bus ();
}

void
Session::add_monitor_section ()
{
	RouteList rl;

	if (_monitor_out || !_master_out) {
		return;
	}

	boost::shared_ptr<Route> r (new Route (*this, _("monitor"), Route::MonitorOut, DataType::AUDIO));

	if (r->init ()) {
		return;
	}

#ifdef BOOST_SP_ENABLE_DEBUG_HOOKS
	// boost_debug_shared_ptr_mark_interesting (r.get(), "Route");
#endif
	{
		Glib::Mutex::Lock lm (AudioEngine::instance()->process_lock ());
		r->input()->ensure_io (_master_out->output()->n_ports(), false, this);
		r->output()->ensure_io (_master_out->output()->n_ports(), false, this);
	}

	rl.push_back (r);
	add_routes (rl, false, false, false);
	
	assert (_monitor_out);

	/* AUDIO ONLY as of june 29th 2009, because listen semantics for anything else
	   are undefined, at best.
	*/
	
	uint32_t limit = _monitor_out->n_inputs().n_audio();
	
	if (_master_out) {
		
		/* connect the inputs to the master bus outputs. this
		 * represents a separate data feed from the internal sends from
		 * each route. as of jan 2011, it allows the monitor section to
		 * conditionally ignore either the internal sends or the normal
		 * input feed, but we should really find a better way to do
		 * this, i think.
		 */

		_master_out->output()->disconnect (this);

		for (uint32_t n = 0; n < limit; ++n) {
			boost::shared_ptr<AudioPort> p = _monitor_out->input()->ports().nth_audio_port (n);
			boost::shared_ptr<AudioPort> o = _master_out->output()->ports().nth_audio_port (n);
			
			if (o) {
				string connect_to = o->name();
				if (_monitor_out->input()->connect (p, connect_to, this)) {
					error << string_compose (_("cannot connect control input %1 to %2"), n, connect_to)
					      << endmsg;
					break;
				}
			}
		}
	}
	
	/* if monitor section is not connected, connect it to physical outs
	 */
	
	if (Config->get_auto_connect_standard_busses() && !_monitor_out->output()->connected ()) {
		
		if (!Config->get_monitor_bus_preferred_bundle().empty()) {
			
			boost::shared_ptr<Bundle> b = bundle_by_name (Config->get_monitor_bus_preferred_bundle());
			
			if (b) {
				_monitor_out->output()->connect_ports_to_bundle (b, this);
			} else {
				warning << string_compose (_("The preferred I/O for the monitor bus (%1) cannot be found"),
							   Config->get_monitor_bus_preferred_bundle())
					<< endmsg;
			}
			
		} else {
			
			/* Monitor bus is audio only */

			uint32_t mod = n_physical_outputs.get (DataType::AUDIO);
			uint32_t limit = _monitor_out->n_outputs().get (DataType::AUDIO);
			vector<string> outputs[DataType::num_types];

			for (uint32_t i = 0; i < DataType::num_types; ++i) {
				_engine.get_physical_outputs (DataType (DataType::Symbol (i)), outputs[i]);
			}
			
			
			if (mod != 0) {
				
				for (uint32_t n = 0; n < limit; ++n) {
					
					boost::shared_ptr<Port> p = _monitor_out->output()->ports().port(DataType::AUDIO, n);
					string connect_to;
					if (outputs[DataType::AUDIO].size() > (n % mod)) {
						connect_to = outputs[DataType::AUDIO][n % mod];
					}
					
					if (!connect_to.empty()) {
						if (_monitor_out->output()->connect (p, connect_to, this)) {
							error << string_compose (
								_("cannot connect control output %1 to %2"),
								n, connect_to)
							      << endmsg;
							break;
						}
					}
				}
			}
		}
	}

	/* Hold process lock while doing this so that we don't hear bits and
	 * pieces of audio as we work on each route.
	 */
	 
	Glib::Mutex::Lock lm (AudioEngine::instance()->process_lock ());

	/* Connect tracks to monitor section. Note that in an
	   existing session, the internal sends will already exist, but we want the
	   routes to notice that they connect to the control out specifically.
	*/


	boost::shared_ptr<RouteList> rls = routes.reader ();

	PBD::Unwinder<bool> uw (ignore_route_processor_changes, true);

	for (RouteList::iterator x = rls->begin(); x != rls->end(); ++x) {
		
		if ((*x)->is_monitor()) {
			/* relax */
		} else if ((*x)->is_master()) {
			/* relax */
		} else {
			(*x)->enable_monitor_send ();
		}
	}
}

void
Session::hookup_io ()
{
	/* stop graph reordering notifications from
	   causing resorts, etc.
	*/

	_state_of_the_state = StateOfTheState (_state_of_the_state | InitialConnecting);

	if (!auditioner) {

		/* we delay creating the auditioner till now because
		   it makes its own connections to ports.
		*/

		try {
			boost::shared_ptr<Auditioner> a (new Auditioner (*this));
			if (a->init()) {
				throw failed_constructor ();
			}
			a->use_new_diskstream ();
			auditioner = a;
		}

		catch (failed_constructor& err) {
			warning << _("cannot create Auditioner: no auditioning of regions possible") << endmsg;
		}
	}

	/* load bundles, which we may have postponed earlier on */
	if (_bundle_xml_node) {
		load_bundles (*_bundle_xml_node);
		delete _bundle_xml_node;
	}

	/* Tell all IO objects to connect themselves together */

	IO::enable_connecting ();
	MIDI::JackMIDIPort::MakeConnections ();

	/* Anyone who cares about input state, wake up and do something */

	IOConnectionsComplete (); /* EMIT SIGNAL */

	_state_of_the_state = StateOfTheState (_state_of_the_state & ~InitialConnecting);

	/* now handle the whole enchilada as if it was one
	   graph reorder event.
	*/

	graph_reordered ();

	/* update the full solo state, which can't be
	   correctly determined on a per-route basis, but
	   needs the global overview that only the session
	   has.
	*/

	update_route_solo_state ();
}

void
Session::track_playlist_changed (boost::weak_ptr<Track> wp)
{
	boost::shared_ptr<Track> track = wp.lock ();
	if (!track) {
		return;
	}

	boost::shared_ptr<Playlist> playlist;

	if ((playlist = track->playlist()) != 0) {
		playlist->RegionAdded.connect_same_thread (*this, boost::bind (&Session::playlist_region_added, this, _1));
		playlist->RangesMoved.connect_same_thread (*this, boost::bind (&Session::playlist_ranges_moved, this, _1));
		playlist->RegionsExtended.connect_same_thread (*this, boost::bind (&Session::playlist_regions_extended, this, _1));
	}
}

bool
Session::record_enabling_legal () const
{
	/* this used to be in here, but survey says.... we don't need to restrict it */
	// if (record_status() == Recording) {
	//	return false;
	// }

	if (Config->get_all_safe()) {
		return false;
	}
	return true;
}

void
Session::set_track_monitor_input_status (bool yn)
{
	boost::shared_ptr<RouteList> rl = routes.reader ();
	for (RouteList::iterator i = rl->begin(); i != rl->end(); ++i) {
		boost::shared_ptr<AudioTrack> tr = boost::dynamic_pointer_cast<AudioTrack> (*i);
		if (tr && tr->record_enabled ()) {
			//cerr << "switching to input = " << !auto_input << __FILE__ << __LINE__ << endl << endl;
			tr->request_jack_monitors_input (yn);
		}
	}
}

void
Session::auto_punch_start_changed (Location* location)
{
	replace_event (SessionEvent::PunchIn, location->start());

	if (get_record_enabled() && config.get_punch_in()) {
		/* capture start has been changed, so save new pending state */
		save_state ("", true);
	}
}

void
Session::auto_punch_end_changed (Location* location)
{
	framepos_t when_to_stop = location->end();
	// when_to_stop += _worst_output_latency + _worst_input_latency;
	replace_event (SessionEvent::PunchOut, when_to_stop);
}

void
Session::auto_punch_changed (Location* location)
{
	framepos_t when_to_stop = location->end();

	replace_event (SessionEvent::PunchIn, location->start());
	//when_to_stop += _worst_output_latency + _worst_input_latency;
	replace_event (SessionEvent::PunchOut, when_to_stop);
}

void
Session::auto_loop_changed (Location* location)
{
	replace_event (SessionEvent::AutoLoop, location->end(), location->start());

	if (transport_rolling() && play_loop) {


		// if (_transport_frame > location->end()) {

		if (_transport_frame < location->start() || _transport_frame > location->end()) {
			// relocate to beginning of loop
			clear_events (SessionEvent::LocateRoll);

			request_locate (location->start(), true);

		}
		else if (Config->get_seamless_loop() && !loop_changing) {

			// schedule a locate-roll to refill the diskstreams at the
			// previous loop end
			loop_changing = true;

			if (location->end() > last_loopend) {
				clear_events (SessionEvent::LocateRoll);
				SessionEvent *ev = new SessionEvent (SessionEvent::LocateRoll, SessionEvent::Add, last_loopend, last_loopend, 0, true);
				queue_event (ev);
			}

		}
	}

	last_loopend = location->end();
}

void
Session::set_auto_punch_location (Location* location)
{
	Location* existing;

	if ((existing = _locations->auto_punch_location()) != 0 && existing != location) {
		punch_connections.drop_connections();
		existing->set_auto_punch (false, this);
		remove_event (existing->start(), SessionEvent::PunchIn);
		clear_events (SessionEvent::PunchOut);
		auto_punch_location_changed (0);
	}

	set_dirty();

	if (location == 0) {
		return;
	}

	if (location->end() <= location->start()) {
		error << _("Session: you can't use that location for auto punch (start <= end)") << endmsg;
		return;
	}

	punch_connections.drop_connections ();

	location->start_changed.connect_same_thread (punch_connections, boost::bind (&Session::auto_punch_start_changed, this, _1));
	location->end_changed.connect_same_thread (punch_connections, boost::bind (&Session::auto_punch_end_changed, this, _1));
	location->changed.connect_same_thread (punch_connections, boost::bind (&Session::auto_punch_changed, this, _1));

	location->set_auto_punch (true, this);

	auto_punch_changed (location);

	auto_punch_location_changed (location);
}

void
Session::set_auto_loop_location (Location* location)
{
	Location* existing;

	if ((existing = _locations->auto_loop_location()) != 0 && existing != location) {
		loop_connections.drop_connections ();
		existing->set_auto_loop (false, this);
		remove_event (existing->end(), SessionEvent::AutoLoop);
		auto_loop_location_changed (0);
	}

	set_dirty();

	if (location == 0) {
		return;
	}

	if (location->end() <= location->start()) {
		error << _("Session: you can't use a mark for auto loop") << endmsg;
		return;
	}

	last_loopend = location->end();

	loop_connections.drop_connections ();

	location->start_changed.connect_same_thread (loop_connections, boost::bind (&Session::auto_loop_changed, this, _1));
	location->end_changed.connect_same_thread (loop_connections, boost::bind (&Session::auto_loop_changed, this, _1));
	location->changed.connect_same_thread (loop_connections, boost::bind (&Session::auto_loop_changed, this, _1));

	location->set_auto_loop (true, this);

	/* take care of our stuff first */

	auto_loop_changed (location);

	/* now tell everyone else */

	auto_loop_location_changed (location);
}

void
Session::locations_added (Location *)
{
	set_dirty ();
}

void
Session::locations_changed ()
{
	_locations->apply (*this, &Session::handle_locations_changed);
}

void
Session::handle_locations_changed (Locations::LocationList& locations)
{
	Locations::LocationList::iterator i;
	Location* location;
	bool set_loop = false;
	bool set_punch = false;

	for (i = locations.begin(); i != locations.end(); ++i) {

		location =* i;

		if (location->is_auto_punch()) {
			set_auto_punch_location (location);
			set_punch = true;
		}
		if (location->is_auto_loop()) {
			set_auto_loop_location (location);
			set_loop = true;
		}

		if (location->is_session_range()) {
			_session_range_location = location;
		}
	}

	if (!set_loop) {
		set_auto_loop_location (0);
	}
	if (!set_punch) {
		set_auto_punch_location (0);
	}

	set_dirty();
}

void
Session::enable_record ()
{
	if (_transport_speed != 0.0 && _transport_speed != 1.0) {
		/* no recording at anything except normal speed */
		return;
	}

	while (1) {
		RecordState rs = (RecordState) g_atomic_int_get (&_record_status);

		if (rs == Recording) {
			break;
		}
		
		if (g_atomic_int_compare_and_exchange (&_record_status, rs, Recording)) {

			_last_record_location = _transport_frame;
			MIDI::Manager::instance()->mmc()->send (MIDI::MachineControlCommand (MIDI::MachineControl::cmdRecordStrobe));

			if (Config->get_monitoring_model() == HardwareMonitoring && config.get_auto_input()) {
				set_track_monitor_input_status (true);
			}

			RecordStateChanged ();
			break;
		}
	}
}

void
Session::disable_record (bool rt_context, bool force)
{
	RecordState rs;

	if ((rs = (RecordState) g_atomic_int_get (&_record_status)) != Disabled) {

		if ((!Config->get_latched_record_enable () && !play_loop) || force) {
			g_atomic_int_set (&_record_status, Disabled);
			MIDI::Manager::instance()->mmc()->send (MIDI::MachineControlCommand (MIDI::MachineControl::cmdRecordExit));
		} else {
			if (rs == Recording) {
				g_atomic_int_set (&_record_status, Enabled);
			}
		}

		if (Config->get_monitoring_model() == HardwareMonitoring && config.get_auto_input()) {
			set_track_monitor_input_status (false);
		}

		RecordStateChanged (); /* emit signal */

		if (!rt_context) {
			remove_pending_capture_state ();
		}
	}
}

void
Session::step_back_from_record ()
{
	if (g_atomic_int_compare_and_exchange (&_record_status, Recording, Enabled)) {

		if (Config->get_monitoring_model() == HardwareMonitoring && config.get_auto_input()) {
			set_track_monitor_input_status (false);
		}

		RecordStateChanged (); /* emit signal */
	}
}

void
Session::maybe_enable_record ()
{
	if (_step_editors > 0) {
		return;
	}

	g_atomic_int_set (&_record_status, Enabled);

	/* This function is currently called from somewhere other than an RT thread.
	   This save_state() call therefore doesn't impact anything.  Doing it here
	   means that we save pending state of which sources the next record will use,
	   which gives us some chance of recovering from a crash during the record.
	*/

	save_state ("", true);

	if (_transport_speed) {
		if (!config.get_punch_in()) {
			enable_record ();
		}
	} else {
		MIDI::Manager::instance()->mmc()->send (MIDI::MachineControlCommand (MIDI::MachineControl::cmdRecordPause));
		RecordStateChanged (); /* EMIT SIGNAL */
	}

	set_dirty();
}

framepos_t
Session::audible_frame () const
{
	framepos_t ret;
	framepos_t tf;
	framecnt_t offset;

	/* the first of these two possible settings for "offset"
	   mean that the audible frame is stationary until
	   audio emerges from the latency compensation
	   "pseudo-pipeline".

	   the second means that the audible frame is stationary
	   until audio would emerge from a physical port
	   in the absence of any plugin latency compensation
	*/

	offset = worst_playback_latency ();

	if (offset > current_block_size) {
		offset -= current_block_size;
	} else {
		/* XXX is this correct? if we have no external
		   physical connections and everything is internal
		   then surely this is zero? still, how
		   likely is that anyway?
		*/
		offset = current_block_size;
	}

	if (synced_to_jack()) {
		tf = _engine.transport_frame();
	} else {
		tf = _transport_frame;
	}

	ret = tf;

	if (!non_realtime_work_pending()) {

		/* MOVING */

		/* Check to see if we have passed the first guaranteed
		   audible frame past our last start position. if not,
		   return that last start point because in terms
		   of audible frames, we have not moved yet.

		   `Start position' in this context means the time we last
		   either started or changed transport direction.
		*/

		if (_transport_speed > 0.0f) {

			if (!play_loop || !have_looped) {
				if (tf < _last_roll_or_reversal_location + offset) {
					return _last_roll_or_reversal_location;
				}
			}


			/* forwards */
			ret -= offset;

		} else if (_transport_speed < 0.0f) {

			/* XXX wot? no backward looping? */

			if (tf > _last_roll_or_reversal_location - offset) {
				return _last_roll_or_reversal_location;
			} else {
				/* backwards */
				ret += offset;
			}
		}
	}

	return ret;
}

void
Session::set_frame_rate (framecnt_t frames_per_second)
{
	/** \fn void Session::set_frame_size(framecnt_t)
		the AudioEngine object that calls this guarantees
		that it will not be called while we are also in
		::process(). Its fine to do things that block
		here.
	*/

	_base_frame_rate = frames_per_second;

	sync_time_vars();

	Automatable::set_automation_interval (ceil ((double) frames_per_second * (0.001 * Config->get_automation_interval())));

	clear_clicks ();

	// XXX we need some equivalent to this, somehow
	// SndFileSource::setup_standard_crossfades (frames_per_second);

	set_dirty();

	/* XXX need to reset/reinstantiate all LADSPA plugins */
}

void
Session::set_block_size (pframes_t nframes)
{
	/* the AudioEngine guarantees
	   that it will not be called while we are also in
	   ::process(). It is therefore fine to do things that block
	   here.
	*/
	
	{
		current_block_size = nframes;

		ensure_buffers ();

		boost::shared_ptr<RouteList> r = routes.reader ();

		for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
			(*i)->set_block_size (nframes);
		}

		boost::shared_ptr<RouteList> rl = routes.reader ();
		for (RouteList::iterator i = rl->begin(); i != rl->end(); ++i) {
			boost::shared_ptr<Track> tr = boost::dynamic_pointer_cast<Track> (*i);
			if (tr) {
				tr->set_block_size (nframes);
			}
		}

		set_worst_io_latencies ();
	}
}


static void
trace_terminal (boost::shared_ptr<Route> r1, boost::shared_ptr<Route> rbase)
{
	boost::shared_ptr<Route> r2;

	if (r1->feeds (rbase) && rbase->feeds (r1)) {
		info << string_compose(_("feedback loop setup between %1 and %2"), r1->name(), rbase->name()) << endmsg;
		return;
	}

	/* make a copy of the existing list of routes that feed r1 */

	Route::FedBy existing (r1->fed_by());

	/* for each route that feeds r1, recurse, marking it as feeding
	   rbase as well.
	*/

	for (Route::FedBy::iterator i = existing.begin(); i != existing.end(); ++i) {
		if (!(r2 = i->r.lock ())) {
			/* (*i) went away, ignore it */
			continue;
		}

		/* r2 is a route that feeds r1 which somehow feeds base. mark
		   base as being fed by r2
		*/

		rbase->add_fed_by (r2, i->sends_only);

		if (r2 != rbase) {

			/* 2nd level feedback loop detection. if r1 feeds or is fed by r2,
			   stop here.
			*/

			if (r1->feeds (r2) && r2->feeds (r1)) {
				continue;
			}

			/* now recurse, so that we can mark base as being fed by
			   all routes that feed r2
			*/

			trace_terminal (r2, rbase);
		}

	}
}

void
Session::resort_routes ()
{
	/* don't do anything here with signals emitted
	   by Routes during initial setup or while we
	   are being destroyed.
	*/

	if (_state_of_the_state & (InitialConnecting | Deletion)) {
		return;
	}

	{
		RCUWriter<RouteList> writer (routes);
		boost::shared_ptr<RouteList> r = writer.get_copy ();
		resort_routes_using (r);
		/* writer goes out of scope and forces update */
	}

#ifndef NDEBUG
	boost::shared_ptr<RouteList> rl = routes.reader ();
	for (RouteList::iterator i = rl->begin(); i != rl->end(); ++i) {
		DEBUG_TRACE (DEBUG::Graph, string_compose ("%1 fed by ...\n", (*i)->name()));

		const Route::FedBy& fb ((*i)->fed_by());

		for (Route::FedBy::const_iterator f = fb.begin(); f != fb.end(); ++f) {
			boost::shared_ptr<Route> sf = f->r.lock();
			if (sf) {
				DEBUG_TRACE (DEBUG::Graph, string_compose ("\t%1 (sends only ? %2)\n", sf->name(), f->sends_only));
			}
		}
	}
#endif

}

/** This is called whenever we need to rebuild the graph of how we will process
 *  routes.
 *  @param r List of routes, in any order.
 */

void
Session::resort_routes_using (boost::shared_ptr<RouteList> r)
{
	/* We are going to build a directed graph of our routes;
	   this is where the edges of that graph are put.
	*/
	
	GraphEdges edges;

	/* Go through all routes doing two things:
	 *
	 * 1. Collect the edges of the route graph.  Each of these edges
	 *    is a pair of routes, one of which directly feeds the other
	 *    either by a JACK connection or by an internal send.
	 *
	 * 2. Begin the process of making routes aware of which other
	 *    routes directly or indirectly feed them.  This information
	 *    is used by the solo code.
	 */
	   
	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {

		/* Clear out the route's list of direct or indirect feeds */
		(*i)->clear_fed_by ();

		for (RouteList::iterator j = r->begin(); j != r->end(); ++j) {

			bool via_sends_only;

			/* See if this *j feeds *i according to the current state of the JACK
			   connections and internal sends.
			*/
			if ((*j)->direct_feeds_according_to_reality (*i, &via_sends_only)) {
				/* add the edge to the graph (part #1) */
				edges.add (*j, *i, via_sends_only);
				/* tell the route (for part #2) */
				(*i)->add_fed_by (*j, via_sends_only);
			}
		}
	}

	/* Attempt a topological sort of the route graph */
	boost::shared_ptr<RouteList> sorted_routes = topological_sort (r, edges);
	
	if (sorted_routes) {
		/* We got a satisfactory topological sort, so there is no feedback;
		   use this new graph.

		   Note: the process graph rechain does not require a
		   topologically-sorted list, but hey ho.
		*/
		if (_process_graph) {
			_process_graph->rechain (sorted_routes, edges);
		}
		
		_current_route_graph = edges;

		/* Complete the building of the routes' lists of what directly
		   or indirectly feeds them.
		*/
		for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
			trace_terminal (*i, *i);
		}

		r = sorted_routes;

#ifndef NDEBUG
		DEBUG_TRACE (DEBUG::Graph, "Routes resorted, order follows:\n");
		for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
			DEBUG_TRACE (DEBUG::Graph, string_compose ("\t%1 signal order %2\n",
								   (*i)->name(), (*i)->order_key ("signal")));
		}
#endif

		SuccessfulGraphSort (); /* EMIT SIGNAL */

	} else {
		/* The topological sort failed, so we have a problem.  Tell everyone
		   and stick to the old graph; this will continue to be processed, so
		   until the feedback is fixed, what is played back will not quite
		   reflect what is actually connected.  Note also that we do not
		   do trace_terminal here, as it would fail due to an endless recursion,
		   so the solo code will think that everything is still connected
		   as it was before.
		*/
		
		FeedbackDetected (); /* EMIT SIGNAL */
	}

}

/** Find a route name starting with \a base, maybe followed by the
 *  lowest \a id.  \a id will always be added if \a definitely_add_number
 *  is true on entry; otherwise it will only be added if required
 *  to make the name unique.
 *
 *  Names are constructed like e.g. "Audio 3" for base="Audio" and id=3.
 *  The available route name with the lowest ID will be used, and \a id
 *  will be set to the ID.
 *
 *  \return false if a route name could not be found, and \a track_name
 *  and \a id do not reflect a free route name.
 */
bool
Session::find_route_name (string const & base, uint32_t& id, char* name, size_t name_len, bool definitely_add_number)
{
	if (!definitely_add_number && route_by_name (base) == 0) {
		/* juse use the base */
		snprintf (name, name_len, "%s", base.c_str());
		return true;
	}

	do {
		snprintf (name, name_len, "%s %" PRIu32, base.c_str(), id);

		if (route_by_name (name) == 0) {
			return true;
		}

		++id;

	} while (id < (UINT_MAX-1));

	return false;
}

/** Count the total ins and outs of all non-hidden tracks in the session and return them in in and out */
void
Session::count_existing_track_channels (ChanCount& in, ChanCount& out)
{
	in  = ChanCount::ZERO;
	out = ChanCount::ZERO;

	boost::shared_ptr<RouteList> r = routes.reader ();

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
                boost::shared_ptr<Track> tr = boost::dynamic_pointer_cast<Track> (*i);
		if (tr && !tr->is_hidden()) {
			in  += tr->n_inputs();
			out += tr->n_outputs();
		}
	}
}

/** Caller must not hold process lock
 *  @param name_template string to use for the start of the name, or "" to use "MIDI".
 *  @param instrument plugin info for the instrument to insert pre-fader, if any
 */
list<boost::shared_ptr<MidiTrack> >
Session::new_midi_track (boost::shared_ptr<PluginInfo> instrument, TrackMode mode, RouteGroup* route_group, uint32_t how_many, string name_template)
{
	char track_name[32];
	uint32_t track_id = 0;
	string port;
	RouteList new_routes;
	list<boost::shared_ptr<MidiTrack> > ret;
	uint32_t control_id;

	control_id = next_control_id ();

	bool const use_number = (how_many != 1) || name_template.empty () || name_template == _("MIDI");

	while (how_many) {
		if (!find_route_name (name_template.empty() ? _("MIDI") : name_template, ++track_id, track_name, sizeof(track_name), use_number)) {
			error << "cannot find name for new midi track" << endmsg;
			goto failed;
		}

		boost::shared_ptr<MidiTrack> track;

		try {
			track.reset (new MidiTrack (*this, track_name, Route::Flag (0), mode));

			if (track->init ()) {
				goto failed;
			}

			track->use_new_diskstream();

#ifdef BOOST_SP_ENABLE_DEBUG_HOOKS
			// boost_debug_shared_ptr_mark_interesting (track.get(), "Track");
#endif
			{
				Glib::Mutex::Lock lm (AudioEngine::instance()->process_lock ());
				if (track->input()->ensure_io (ChanCount(DataType::MIDI, 1), false, this)) {
					error << "cannot configure 1 in/1 out configuration for new midi track" << endmsg;
					goto failed;
				}

				if (track->output()->ensure_io (ChanCount(DataType::MIDI, 1), false, this)) {
					error << "cannot configure 1 in/1 out configuration for new midi track" << endmsg;
					goto failed;
				}
			}

			track->non_realtime_input_change();

			if (route_group) {
				route_group->add (track);
			}

			track->DiskstreamChanged.connect_same_thread (*this, boost::bind (&Session::resort_routes, this));
			track->set_remote_control_id (control_id);

			new_routes.push_back (track);
			ret.push_back (track);
		}

		catch (failed_constructor &err) {
			error << _("Session: could not create new midi track.") << endmsg;
			goto failed;
		}

		catch (AudioEngine::PortRegistrationFailure& pfe) {

			error << string_compose (_("No more JACK ports are available. You will need to stop %1 and restart JACK with ports if you need this many tracks."), PROGRAM_NAME) << endmsg;
			goto failed;
		}

		--how_many;
	}

  failed:
	if (!new_routes.empty()) {
		add_routes (new_routes, true, true, true);

		if (instrument) {
			for (RouteList::iterator r = new_routes.begin(); r != new_routes.end(); ++r) {
				PluginPtr plugin = instrument->load (*this);
				boost::shared_ptr<Processor> p (new PluginInsert (*this, plugin));
				(*r)->add_processor (p, PreFader);
				
			}
		}
	}

	return ret;
}

void
Session::midi_output_change_handler (IOChange change, void * /*src*/, boost::weak_ptr<Route> wmt)
{
        boost::shared_ptr<Route> midi_track (wmt.lock());

        if (!midi_track) {
                return;
        }

	if ((change.type & IOChange::ConfigurationChanged) && Config->get_output_auto_connect() != ManualConnect) {

                if (change.after.n_audio() <= change.before.n_audio()) {
                        return;
                }

                /* new audio ports: make sure the audio goes somewhere useful,
                   unless the user has no-auto-connect selected.

                   The existing ChanCounts don't matter for this call as they are only
                   to do with matching input and output indices, and we are only changing
                   outputs here.
                */

                ChanCount dummy;

                auto_connect_route (midi_track, dummy, dummy, false, false, ChanCount(), change.before);
        }
}

/** @param connect_inputs true to connect inputs as well as outputs, false to connect just outputs.
 *  @param input_start Where to start from when auto-connecting inputs; e.g. if this is 0, auto-connect starting from input 0.
 *  @param output_start As \a input_start, but for outputs.
 */
void
Session::auto_connect_route (boost::shared_ptr<Route> route, ChanCount& existing_inputs, ChanCount& existing_outputs,
                             bool with_lock, bool connect_inputs, ChanCount input_start, ChanCount output_start)
{
	if (!IO::connecting_legal) {
		return;
	}

	Glib::Mutex::Lock lm (AudioEngine::instance()->process_lock (), Glib::NOT_LOCK);

	if (with_lock) {
		lm.acquire ();
	}

	/* If both inputs and outputs are auto-connected to physical ports,
	   use the max of input and output offsets to ensure auto-connected
	   port numbers always match up (e.g. the first audio input and the
	   first audio output of the route will have the same physical
	   port number).  Otherwise just use the lowest input or output
	   offset possible.
	*/

	DEBUG_TRACE (DEBUG::Graph,
	             string_compose("Auto-connect: existing in = %1 out = %2\n",
	                            existing_inputs, existing_outputs));

	const bool in_out_physical =
		(Config->get_input_auto_connect() & AutoConnectPhysical)
		&& (Config->get_output_auto_connect() & AutoConnectPhysical)
		&& connect_inputs;

	const ChanCount in_offset = in_out_physical
		? ChanCount::max(existing_inputs, existing_outputs)
                : existing_inputs;

	const ChanCount out_offset = in_out_physical
		? ChanCount::max(existing_inputs, existing_outputs)
		: existing_outputs;

	for (DataType::iterator t = DataType::begin(); t != DataType::end(); ++t) {
		vector<string> physinputs;
		vector<string> physoutputs;

		_engine.get_physical_outputs (*t, physoutputs);
		_engine.get_physical_inputs (*t, physinputs);

		if (!physinputs.empty() && connect_inputs) {
			uint32_t nphysical_in = physinputs.size();

			DEBUG_TRACE (DEBUG::Graph,
			             string_compose("There are %1 physical inputs of type %2\n",
			                            nphysical_in, *t));

			for (uint32_t i = input_start.get(*t); i < route->n_inputs().get(*t) && i < nphysical_in; ++i) {
				string port;

				if (Config->get_input_auto_connect() & AutoConnectPhysical) {
					DEBUG_TRACE (DEBUG::Graph,
					             string_compose("Get index %1 + %2 % %3 = %4\n",
					                            in_offset.get(*t), i, nphysical_in,
					                            (in_offset.get(*t) + i) % nphysical_in));
					port = physinputs[(in_offset.get(*t) + i) % nphysical_in];
				}

				DEBUG_TRACE (DEBUG::Graph,
				             string_compose("Connect route %1 IN to %2\n",
				                            route->name(), port));

				if (!port.empty() && route->input()->connect (route->input()->ports().port(*t, i), port, this)) {
					break;
				}

                                ChanCount one_added (*t, 1);
                                existing_inputs += one_added;
			}
		}

		if (!physoutputs.empty()) {
			uint32_t nphysical_out = physoutputs.size();
			for (uint32_t i = output_start.get(*t); i < route->n_outputs().get(*t); ++i) {
				string port;

				if ((*t) == DataType::MIDI || Config->get_output_auto_connect() & AutoConnectPhysical) {
					port = physoutputs[(out_offset.get(*t) + i) % nphysical_out];
				} else if ((*t) == DataType::AUDIO && Config->get_output_auto_connect() & AutoConnectMaster) {
                                        /* master bus is audio only */
					if (_master_out && _master_out->n_inputs().get(*t) > 0) {
						port = _master_out->input()->ports().port(*t,
								i % _master_out->input()->n_ports().get(*t))->name();
					}
				}

				DEBUG_TRACE (DEBUG::Graph,
				             string_compose("Connect route %1 OUT to %2\n",
				                            route->name(), port));

				if (!port.empty() && route->output()->connect (route->output()->ports().port(*t, i), port, this)) {
					break;
				}

                                ChanCount one_added (*t, 1);
                                existing_outputs += one_added;
			}
		}
	}
}

/** Caller must not hold process lock
 *  @param name_template string to use for the start of the name, or "" to use "Audio".
 */
list< boost::shared_ptr<AudioTrack> >
Session::new_audio_track (int input_channels, int output_channels, TrackMode mode, RouteGroup* route_group, 
			  uint32_t how_many, string name_template)
{
	char track_name[32];
	uint32_t track_id = 0;
	string port;
	RouteList new_routes;
	list<boost::shared_ptr<AudioTrack> > ret;
	uint32_t control_id;

	control_id = next_control_id ();

	bool const use_number = (how_many != 1) || name_template.empty () || name_template == _("Audio");

	while (how_many) {
		if (!find_route_name (name_template.empty() ? _("Audio") : name_template, ++track_id, track_name, sizeof(track_name), use_number)) {
			error << "cannot find name for new audio track" << endmsg;
			goto failed;
		}

		boost::shared_ptr<AudioTrack> track;

		try {
			track.reset (new AudioTrack (*this, track_name, Route::Flag (0), mode));

			if (track->init ()) {
				goto failed;
			}

			track->use_new_diskstream();

#ifdef BOOST_SP_ENABLE_DEBUG_HOOKS
			// boost_debug_shared_ptr_mark_interesting (track.get(), "Track");
#endif
			{
				Glib::Mutex::Lock lm (AudioEngine::instance()->process_lock ());

				if (track->input()->ensure_io (ChanCount(DataType::AUDIO, input_channels), false, this)) {
					error << string_compose (
						_("cannot configure %1 in/%2 out configuration for new audio track"),
						input_channels, output_channels)
					      << endmsg;
					goto failed;
				}

				if (track->output()->ensure_io (ChanCount(DataType::AUDIO, output_channels), false, this)) {
					error << string_compose (
						_("cannot configure %1 in/%2 out configuration for new audio track"),
						input_channels, output_channels)
					      << endmsg;
					goto failed;
				}
			}

			if (route_group) {
				route_group->add (track);
			}

			track->non_realtime_input_change();

			track->DiskstreamChanged.connect_same_thread (*this, boost::bind (&Session::resort_routes, this));
			track->set_remote_control_id (control_id);
			++control_id;

			new_routes.push_back (track);
			ret.push_back (track);
		}

		catch (failed_constructor &err) {
			error << _("Session: could not create new audio track.") << endmsg;
			goto failed;
		}

		catch (AudioEngine::PortRegistrationFailure& pfe) {

			error << pfe.what() << endmsg;
			goto failed;
		}

		--how_many;
	}

  failed:
	if (!new_routes.empty()) {
		add_routes (new_routes, true, true, true);
	}

	return ret;
}

void
Session::set_remote_control_ids ()
{
	RemoteModel m = Config->get_remote_model();
	bool emit_signal = false;

	boost::shared_ptr<RouteList> r = routes.reader ();

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		if (MixerOrdered == m) {
			int32_t order = (*i)->order_key(N_("signal"));
			(*i)->set_remote_control_id (order+1, false);
			emit_signal = true;
		} else if (EditorOrdered == m) {
			int32_t order = (*i)->order_key(N_("editor"));
			(*i)->set_remote_control_id (order+1, false);
			emit_signal = true;
		} else if (UserOrdered == m) {
			//do nothing ... only changes to remote id's are initiated by user
		}
	}

	if (emit_signal) {
		Route::RemoteControlIDChange();
	}
}

/** Caller must not hold process lock.
 *  @param name_template string to use for the start of the name, or "" to use "Bus".
 */
RouteList
Session::new_audio_route (int input_channels, int output_channels, RouteGroup* route_group, uint32_t how_many, string name_template)
{
	char bus_name[32];
	uint32_t bus_id = 0;
	string port;
	RouteList ret;
	uint32_t control_id;

	control_id = next_control_id ();

	bool const use_number = (how_many != 1) || name_template.empty () || name_template == _("Bus");
	
	while (how_many) {
		if (!find_route_name (name_template.empty () ? _("Bus") : name_template, ++bus_id, bus_name, sizeof(bus_name), use_number)) {
			error << "cannot find name for new audio bus" << endmsg;
			goto failure;
		}

		try {
			boost::shared_ptr<Route> bus (new Route (*this, bus_name, Route::Flag(0), DataType::AUDIO));

			if (bus->init ()) {
				goto failure;
			}

#ifdef BOOST_SP_ENABLE_DEBUG_HOOKS
			// boost_debug_shared_ptr_mark_interesting (bus.get(), "Route");
#endif
			{
				Glib::Mutex::Lock lm (AudioEngine::instance()->process_lock ());

				if (bus->input()->ensure_io (ChanCount(DataType::AUDIO, input_channels), false, this)) {
					error << string_compose (_("cannot configure %1 in/%2 out configuration for new audio track"),
								 input_channels, output_channels)
					      << endmsg;
					goto failure;
				}


				if (bus->output()->ensure_io (ChanCount(DataType::AUDIO, output_channels), false, this)) {
					error << string_compose (_("cannot configure %1 in/%2 out configuration for new audio track"),
								 input_channels, output_channels)
					      << endmsg;
					goto failure;
				}
			}

			if (route_group) {
				route_group->add (bus);
			}
			bus->set_remote_control_id (control_id);
			++control_id;

			bus->add_internal_return ();

			ret.push_back (bus);
		}


		catch (failed_constructor &err) {
			error << _("Session: could not create new audio route.") << endmsg;
			goto failure;
		}

		catch (AudioEngine::PortRegistrationFailure& pfe) {
			error << pfe.what() << endmsg;
			goto failure;
		}


		--how_many;
	}

  failure:
	if (!ret.empty()) {
		add_routes (ret, false, true, true); // autoconnect outputs only
	}

	return ret;

}

RouteList
Session::new_route_from_template (uint32_t how_many, const std::string& template_path)
{
	RouteList ret;
	uint32_t control_id;
	XMLTree tree;
	uint32_t number = 0;

	if (!tree.read (template_path.c_str())) {
		return ret;
	}

	XMLNode* node = tree.root();

	IO::disable_connecting ();

	control_id = next_control_id ();

	while (how_many) {

		XMLNode node_copy (*node);

		/* Remove IDs of everything so that new ones are used */
		node_copy.remove_property_recursively (X_("id"));

		try {
			string const route_name = node_copy.property(X_("name"))->value ();
			
			/* generate a new name by adding a number to the end of the template name */
			char name[32];
			if (!find_route_name (route_name.c_str(), ++number, name, sizeof(name), true)) {
				fatal << _("Session: UINT_MAX routes? impossible!") << endmsg;
				/*NOTREACHED*/
			}

			/* set this name in the XML description that we are about to use */
			Route::set_name_in_state (node_copy, name);

			/* trim bitslots from listen sends so that new ones are used */
			XMLNodeList children = node_copy.children ();
			for (XMLNodeList::iterator i = children.begin(); i != children.end(); ++i) {
				if ((*i)->name() == X_("Processor")) {
					XMLProperty* role = (*i)->property (X_("role"));
					if (role && role->value() == X_("Listen")) {
						(*i)->remove_property (X_("bitslot"));
					}
				}
			}
			
			boost::shared_ptr<Route> route (XMLRouteFactory (node_copy, 3000));

			if (route == 0) {
				error << _("Session: cannot create track/bus from template description") << endmsg;
				goto out;
			}

			if (boost::dynamic_pointer_cast<Track>(route)) {
				/* force input/output change signals so that the new diskstream
				   picks up the configuration of the route. During session
				   loading this normally happens in a different way.
				*/

				Glib::Mutex::Lock lm (AudioEngine::instance()->process_lock ());

				IOChange change (IOChange::Type (IOChange::ConfigurationChanged | IOChange::ConnectionsChanged));
				change.after = route->input()->n_ports();
				route->input()->changed (change, this);
				change.after = route->output()->n_ports();
				route->output()->changed (change, this);
			}

			route->set_remote_control_id (control_id);
			++control_id;

			ret.push_back (route);
		}

		catch (failed_constructor &err) {
			error << _("Session: could not create new route from template") << endmsg;
			goto out;
		}

		catch (AudioEngine::PortRegistrationFailure& pfe) {
			error << pfe.what() << endmsg;
			goto out;
		}

		--how_many;
	}

  out:
	if (!ret.empty()) {
		add_routes (ret, true, true, true);
		IO::enable_connecting ();
	}

	return ret;
}

void
Session::add_routes (RouteList& new_routes, bool input_auto_connect, bool output_auto_connect, bool save)
{
        ChanCount existing_inputs;
        ChanCount existing_outputs;

        count_existing_track_channels (existing_inputs, existing_outputs);

	{
		RCUWriter<RouteList> writer (routes);
		boost::shared_ptr<RouteList> r = writer.get_copy ();
		r->insert (r->end(), new_routes.begin(), new_routes.end());

		/* if there is no control out and we're not in the middle of loading,
		   resort the graph here. if there is a control out, we will resort
		   toward the end of this method. if we are in the middle of loading,
		   we will resort when done.
		*/

		if (!_monitor_out && IO::connecting_legal) {
			resort_routes_using (r);
		}
	}

	for (RouteList::iterator x = new_routes.begin(); x != new_routes.end(); ++x) {

		boost::weak_ptr<Route> wpr (*x);
		boost::shared_ptr<Route> r (*x);

		r->listen_changed.connect_same_thread (*this, boost::bind (&Session::route_listen_changed, this, _1, wpr));
		r->solo_changed.connect_same_thread (*this, boost::bind (&Session::route_solo_changed, this, _1, _2, wpr));
		r->solo_isolated_changed.connect_same_thread (*this, boost::bind (&Session::route_solo_isolated_changed, this, _1, wpr));
		r->mute_changed.connect_same_thread (*this, boost::bind (&Session::route_mute_changed, this, _1));
		r->output()->changed.connect_same_thread (*this, boost::bind (&Session::set_worst_io_latencies_x, this, _1, _2));
		r->processors_changed.connect_same_thread (*this, boost::bind (&Session::route_processors_changed, this, _1));
		r->order_key_changed.connect_same_thread (*this, boost::bind (&Session::route_order_key_changed, this));

		if (r->is_master()) {
			_master_out = r;
		}

		if (r->is_monitor()) {
			_monitor_out = r;
		}

		boost::shared_ptr<Track> tr = boost::dynamic_pointer_cast<Track> (r);
		if (tr) {
			tr->PlaylistChanged.connect_same_thread (*this, boost::bind (&Session::track_playlist_changed, this, boost::weak_ptr<Track> (tr)));
			track_playlist_changed (boost::weak_ptr<Track> (tr));
			tr->RecordEnableChanged.connect_same_thread (*this, boost::bind (&Session::update_have_rec_enabled_track, this));

			boost::shared_ptr<MidiTrack> mt = boost::dynamic_pointer_cast<MidiTrack> (tr);
			if (mt) {
				mt->StepEditStatusChange.connect_same_thread (*this, boost::bind (&Session::step_edit_status_change, this, _1));
                                mt->output()->changed.connect_same_thread (*this, boost::bind (&Session::midi_output_change_handler, this, _1, _2, boost::weak_ptr<Route>(mt)));
			}
		}

		if (input_auto_connect || output_auto_connect) {
			auto_connect_route (r, existing_inputs, existing_outputs, true, input_auto_connect);
		}
	}

	if (_monitor_out && IO::connecting_legal) {

		{
			Glib::Mutex::Lock lm (_engine.process_lock());		
			
			for (RouteList::iterator x = new_routes.begin(); x != new_routes.end(); ++x) {
				if ((*x)->is_monitor()) {
					/* relax */
				} else if ((*x)->is_master()) {
					/* relax */
				} else {
					(*x)->enable_monitor_send ();
				}
			}
		}

		resort_routes ();
	}

	set_dirty();

	if (save) {
		save_state (_current_snapshot_name);
	}

	RouteAdded (new_routes); /* EMIT SIGNAL */
	Route::RemoteControlIDChange (); /* EMIT SIGNAL */
}

void
Session::globally_set_send_gains_to_zero (boost::shared_ptr<Route> dest)
{
	boost::shared_ptr<RouteList> r = routes.reader ();
	boost::shared_ptr<Send> s;

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		if ((s = (*i)->internal_send_for (dest)) != 0) {
			s->amp()->gain_control()->set_value (0.0);
		}
	}
}

void
Session::globally_set_send_gains_to_unity (boost::shared_ptr<Route> dest)
{
	boost::shared_ptr<RouteList> r = routes.reader ();
	boost::shared_ptr<Send> s;

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		if ((s = (*i)->internal_send_for (dest)) != 0) {
			s->amp()->gain_control()->set_value (1.0);
		}
	}
}

void
Session::globally_set_send_gains_from_track(boost::shared_ptr<Route> dest)
{
	boost::shared_ptr<RouteList> r = routes.reader ();
	boost::shared_ptr<Send> s;

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		if ((s = (*i)->internal_send_for (dest)) != 0) {
			s->amp()->gain_control()->set_value ((*i)->gain_control()->get_value());
		}
	}
}

/** @param include_buses true to add sends to buses and tracks, false for just tracks */
void
Session::globally_add_internal_sends (boost::shared_ptr<Route> dest, Placement p, bool include_buses)
{
	boost::shared_ptr<RouteList> r = routes.reader ();
	boost::shared_ptr<RouteList> t (new RouteList);

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		/* no MIDI sends because there are no MIDI busses yet */
		if (include_buses || boost::dynamic_pointer_cast<AudioTrack>(*i)) {
			t->push_back (*i);
		}
	}

	add_internal_sends (dest, p, t);
}

void
Session::add_internal_sends (boost::shared_ptr<Route> dest, Placement p, boost::shared_ptr<RouteList> senders)
{
	for (RouteList::iterator i = senders->begin(); i != senders->end(); ++i) {
		add_internal_send (dest, (*i)->before_processor_for_placement (p), *i);
	}
}

void
Session::add_internal_send (boost::shared_ptr<Route> dest, int index, boost::shared_ptr<Route> sender)
{
	add_internal_send (dest, sender->before_processor_for_index (index), sender);
}

void
Session::add_internal_send (boost::shared_ptr<Route> dest, boost::shared_ptr<Processor> before, boost::shared_ptr<Route> sender)
{
	if (sender->is_monitor() || sender->is_master() || sender == dest || dest->is_monitor() || dest->is_master()) {
		return;
	}

	if (!dest->internal_return()) {
		dest->add_internal_return ();
	}

	sender->add_aux_send (dest, before);

	graph_reordered ();
}

void
Session::remove_route (boost::shared_ptr<Route> route)
{
	if (route == _master_out) {
		return;
	}

	route->set_solo (false, this);

	{
		RCUWriter<RouteList> writer (routes);
		boost::shared_ptr<RouteList> rs = writer.get_copy ();

		rs->remove (route);

		/* deleting the master out seems like a dumb
		   idea, but its more of a UI policy issue
		   than our concern.
		*/

		if (route == _master_out) {
			_master_out = boost::shared_ptr<Route> ();
		}

		if (route == _monitor_out) {
			_monitor_out.reset ();
		}

		/* writer goes out of scope, forces route list update */
	}

	update_route_solo_state ();

	// We need to disconnect the route's inputs and outputs

	route->input()->disconnect (0);
	route->output()->disconnect (0);

	/* if the route had internal sends sending to it, remove them */
	if (route->internal_return()) {

		boost::shared_ptr<RouteList> r = routes.reader ();
		for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
			boost::shared_ptr<Send> s = (*i)->internal_send_for (route);
			if (s) {
				(*i)->remove_processor (s);
			}
		}
	}

	boost::shared_ptr<MidiTrack> mt = boost::dynamic_pointer_cast<MidiTrack> (route);
	if (mt && mt->step_editing()) {
		if (_step_editors > 0) {
			_step_editors--;
		}
	}

	update_latency_compensation ();
	set_dirty();

	/* Re-sort routes to remove the graph's current references to the one that is
	 * going away, then flush old references out of the graph.
	 */

	resort_routes ();
	if (_process_graph) {
		_process_graph->clear_other_chain ();
	}

	/* get rid of it from the dead wood collection in the route list manager */

	/* XXX i think this is unsafe as it currently stands, but i am not sure. (pd, october 2nd, 2006) */

	routes.flush ();

	/* try to cause everyone to drop their references */

	route->drop_references ();

	sync_order_keys (N_("session"));

	Route::RemoteControlIDChange(); /* EMIT SIGNAL */

	/* save the new state of the world */

	if (save_state (_current_snapshot_name)) {
		save_history (_current_snapshot_name);
	}
}

void
Session::route_mute_changed (void* /*src*/)
{
	set_dirty ();
}

void
Session::route_listen_changed (void* /*src*/, boost::weak_ptr<Route> wpr)
{
	boost::shared_ptr<Route> route = wpr.lock();
	if (!route) {
		error << string_compose (_("programming error: %1"), X_("invalid route weak ptr passed to route_solo_changed")) << endmsg;
		return;
	}

	if (route->listening_via_monitor ()) {

		if (Config->get_exclusive_solo()) {
			/* new listen: disable all other listen */
			boost::shared_ptr<RouteList> r = routes.reader ();
			for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
				if ((*i) == route || (*i)->solo_isolated() || (*i)->is_master() || (*i)->is_monitor() || (*i)->is_hidden()) {
					continue;
				}
				(*i)->set_listen (false, this);
			}
		}

		_listen_cnt++;

	} else if (_listen_cnt > 0) {

		_listen_cnt--;
	}

	update_route_solo_state ();
}
void
Session::route_solo_isolated_changed (void* /*src*/, boost::weak_ptr<Route> wpr)
{
	boost::shared_ptr<Route> route = wpr.lock ();

	if (!route) {
		/* should not happen */
		error << string_compose (_("programming error: %1"), X_("invalid route weak ptr passed to route_solo_changed")) << endmsg;
		return;
	}

	bool send_changed = false;

	if (route->solo_isolated()) {
		if (_solo_isolated_cnt == 0) {
			send_changed = true;
		}
		_solo_isolated_cnt++;
	} else if (_solo_isolated_cnt > 0) {
		_solo_isolated_cnt--;
		if (_solo_isolated_cnt == 0) {
			send_changed = true;
		}
	}

	if (send_changed) {
		IsolatedChanged (); /* EMIT SIGNAL */
	}
}

void
Session::route_solo_changed (bool self_solo_change, void* /*src*/, boost::weak_ptr<Route> wpr)
{
	DEBUG_TRACE (DEBUG::Solo, string_compose ("route solo change, self = %1\n", self_solo_change));

	if (!self_solo_change) {
		// session doesn't care about changes to soloed-by-others
		return;
	}

	if (solo_update_disabled) {
		// We know already
		DEBUG_TRACE (DEBUG::Solo, "solo update disabled - changed ignored\n");
		return;
	}

	boost::shared_ptr<Route> route = wpr.lock ();
	assert (route);

	boost::shared_ptr<RouteList> r = routes.reader ();
	int32_t delta;

	if (route->self_soloed()) {
		delta = 1;
	} else {
		delta = -1;
	}

	RouteGroup* rg = route->route_group ();
	bool leave_group_alone = (rg && rg->is_active() && rg->is_solo());

	if (delta == 1 && Config->get_exclusive_solo()) {
		
		/* new solo: disable all other solos, but not the group if its solo-enabled */

		for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
			if ((*i) == route || (*i)->solo_isolated() || (*i)->is_master() || (*i)->is_monitor() || (*i)->is_hidden() ||
			    (leave_group_alone && ((*i)->route_group() == rg))) {
				continue;
			}
			(*i)->set_solo (false, this);
		}
	}

	DEBUG_TRACE (DEBUG::Solo, string_compose ("propagate solo change, delta = %1\n", delta));

	solo_update_disabled = true;

	RouteList uninvolved;

	DEBUG_TRACE (DEBUG::Solo, string_compose ("%1\n", route->name()));

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		bool via_sends_only;
		bool in_signal_flow;

		if ((*i) == route || (*i)->solo_isolated() || (*i)->is_master() || (*i)->is_monitor() || (*i)->is_hidden() ||
		    (leave_group_alone && ((*i)->route_group() == rg))) {
			continue;
		}

		in_signal_flow = false;

		DEBUG_TRACE (DEBUG::Solo, string_compose ("check feed from %1\n", (*i)->name()));
		
		if ((*i)->feeds (route, &via_sends_only)) {
			if (!via_sends_only) {
				if (!route->soloed_by_others_upstream()) {
					(*i)->mod_solo_by_others_downstream (delta);
				}
			}
			in_signal_flow = true;
		} else {
			DEBUG_TRACE (DEBUG::Solo, "\tno feed from\n");
		}
		
		DEBUG_TRACE (DEBUG::Solo, string_compose ("check feed to %1\n", (*i)->name()));

		if (route->feeds (*i, &via_sends_only)) {
			/* propagate solo upstream only if routing other than
			   sends is involved, but do consider the other route
			   (*i) to be part of the signal flow even if only
			   sends are involved.
			*/
			DEBUG_TRACE (DEBUG::Solo, string_compose ("%1 feeds %2 via sends only %3 sboD %4 sboU %5\n",
								  route->name(),
								  (*i)->name(),
								  via_sends_only,
								  route->soloed_by_others_downstream(),
								  route->soloed_by_others_upstream()));
			if (!via_sends_only) {
				if (!route->soloed_by_others_downstream()) {
					DEBUG_TRACE (DEBUG::Solo, string_compose ("\tmod %1 by %2\n", (*i)->name(), delta));
					(*i)->mod_solo_by_others_upstream (delta);
				}
			}
			in_signal_flow = true;
		} else {
			DEBUG_TRACE (DEBUG::Solo, "\tno feed to\n");
		}

		if (!in_signal_flow) {
			uninvolved.push_back (*i);
		}
	}

	solo_update_disabled = false;
	DEBUG_TRACE (DEBUG::Solo, "propagation complete\n");

	update_route_solo_state (r);

	/* now notify that the mute state of the routes not involved in the signal
	   pathway of the just-solo-changed route may have altered.
	*/

	for (RouteList::iterator i = uninvolved.begin(); i != uninvolved.end(); ++i) {
		DEBUG_TRACE (DEBUG::Solo, string_compose ("mute change for %1\n", (*i)->name()));
		(*i)->mute_changed (this);
	}

	SoloChanged (); /* EMIT SIGNAL */
	set_dirty();
}

void
Session::update_route_solo_state (boost::shared_ptr<RouteList> r)
{
	/* now figure out if anything that matters is soloed (or is "listening")*/

	bool something_soloed = false;
	uint32_t listeners = 0;
	uint32_t isolated = 0;

	if (!r) {
		r = routes.reader();
	}

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		if (!(*i)->is_master() && !(*i)->is_monitor() && !(*i)->is_hidden() && (*i)->self_soloed()) {
			something_soloed = true;
		}

		if (!(*i)->is_hidden() && (*i)->listening_via_monitor()) {
			if (Config->get_solo_control_is_listen_control()) {
				listeners++;
			} else {
				(*i)->set_listen (false, this);
			}
		}

		if ((*i)->solo_isolated()) {
			isolated++;
		}
	}

	if (something_soloed != _non_soloed_outs_muted) {
		_non_soloed_outs_muted = something_soloed;
		SoloActive (_non_soloed_outs_muted); /* EMIT SIGNAL */
	}

	_listen_cnt = listeners;

	if (isolated != _solo_isolated_cnt) {
		_solo_isolated_cnt = isolated;
		IsolatedChanged (); /* EMIT SIGNAL */
	}
}

boost::shared_ptr<RouteList>
Session::get_routes_with_internal_returns() const
{
	boost::shared_ptr<RouteList> r = routes.reader ();
	boost::shared_ptr<RouteList> rl (new RouteList);

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		if ((*i)->internal_return ()) {
			rl->push_back (*i);
		}
	}
	return rl;
}

bool
Session::io_name_is_legal (const std::string& name)
{
	boost::shared_ptr<RouteList> r = routes.reader ();

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		if ((*i)->name() == name) {
			return false;
		}

		if ((*i)->has_io_processor_named (name)) {
			return false;
		}
	}

	return true;
}

void
Session::set_exclusive_input_active (boost::shared_ptr<Route> rt, bool /*others_on*/)
{
	RouteList rl;
	vector<string> connections;

	PortSet& ps (rt->input()->ports());

	for (PortSet::iterator p = ps.begin(); p != ps.end(); ++p) {
		p->get_connections (connections);
	}

	for (vector<string>::iterator s = connections.begin(); s != connections.end(); ++s) {
		routes_using_input_from (*s, rl);
	}

	/* scan all relevant routes to see if others are on or off */

	bool others_are_already_on = false;

	for (RouteList::iterator r = rl.begin(); r != rl.end(); ++r) {
		if ((*r) != rt) {
			boost::shared_ptr<MidiTrack> mt = boost::dynamic_pointer_cast<MidiTrack> (*r);
			if (mt) {
				if (mt->input_active()) {
					others_are_already_on = true;
					break;
				}
			}
		}
	}

	/* globally reverse other routes */

	for (RouteList::iterator r = rl.begin(); r != rl.end(); ++r) {
		if ((*r) != rt) {
			boost::shared_ptr<MidiTrack> mt = boost::dynamic_pointer_cast<MidiTrack> (*r);
			if (mt) {
				mt->set_input_active (!others_are_already_on);
			}
		}
	}
}

void
Session::routes_using_input_from (const string& str, RouteList& rl)
{
	boost::shared_ptr<RouteList> r = routes.reader ();

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		if ((*i)->input()->connected_to (str)) {
			rl.push_back (*i);
		}
	}
}

boost::shared_ptr<Route>
Session::route_by_name (string name)
{
	boost::shared_ptr<RouteList> r = routes.reader ();

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		if ((*i)->name() == name) {
			return *i;
		}
	}

	return boost::shared_ptr<Route> ((Route*) 0);
}

boost::shared_ptr<Route>
Session::route_by_id (PBD::ID id)
{
	boost::shared_ptr<RouteList> r = routes.reader ();

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		if ((*i)->id() == id) {
			return *i;
		}
	}

	return boost::shared_ptr<Route> ((Route*) 0);
}

boost::shared_ptr<Track>
Session::track_by_diskstream_id (PBD::ID id)
{
	boost::shared_ptr<RouteList> r = routes.reader ();

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		boost::shared_ptr<Track> t = boost::dynamic_pointer_cast<Track> (*i);
		if (t && t->using_diskstream_id (id)) {
			return t;
		}
	}

	return boost::shared_ptr<Track> ();
}

boost::shared_ptr<Route>
Session::route_by_remote_id (uint32_t id)
{
	boost::shared_ptr<RouteList> r = routes.reader ();

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		if ((*i)->remote_control_id() == id) {
			return *i;
		}
	}

	return boost::shared_ptr<Route> ((Route*) 0);
}

void
Session::playlist_region_added (boost::weak_ptr<Region> w)
{
	boost::shared_ptr<Region> r = w.lock ();
	if (!r) {
		return;
	}

	/* These are the operations that are currently in progress... */
	list<GQuark> curr = _current_trans_quarks;
	curr.sort ();

	/* ...and these are the operations during which we want to update
	   the session range location markers.
	*/
	list<GQuark> ops;
	ops.push_back (Operations::capture);
	ops.push_back (Operations::paste);
	ops.push_back (Operations::duplicate_region);
	ops.push_back (Operations::insert_file);
	ops.push_back (Operations::insert_region);
	ops.push_back (Operations::drag_region_brush);
	ops.push_back (Operations::region_drag);
	ops.push_back (Operations::selection_grab);
	ops.push_back (Operations::region_fill);
	ops.push_back (Operations::fill_selection);
	ops.push_back (Operations::create_region);
	ops.push_back (Operations::region_copy);
	ops.push_back (Operations::fixed_time_region_copy);
	ops.sort ();

	/* See if any of the current operations match the ones that we want */
	list<GQuark> in;
	set_intersection (_current_trans_quarks.begin(), _current_trans_quarks.end(), ops.begin(), ops.end(), back_inserter (in));

	/* If so, update the session range markers */
	if (!in.empty ()) {
		maybe_update_session_range (r->position (), r->last_frame ());
	}
}

/** Update the session range markers if a is before the current start or
 *  b is after the current end.
 */
void
Session::maybe_update_session_range (framepos_t a, framepos_t b)
{
	if (_state_of_the_state & Loading) {
		return;
	}

	if (_session_range_location == 0) {

		add_session_range_location (a, b);

	} else {

		if (a < _session_range_location->start()) {
			_session_range_location->set_start (a);
		}

		if (b > _session_range_location->end()) {
			_session_range_location->set_end (b);
		}
	}
}

void
Session::playlist_ranges_moved (list<Evoral::RangeMove<framepos_t> > const & ranges)
{
	for (list<Evoral::RangeMove<framepos_t> >::const_iterator i = ranges.begin(); i != ranges.end(); ++i) {
		maybe_update_session_range (i->to, i->to + i->length);
	}
}

void
Session::playlist_regions_extended (list<Evoral::Range<framepos_t> > const & ranges)
{
	for (list<Evoral::Range<framepos_t> >::const_iterator i = ranges.begin(); i != ranges.end(); ++i) {
		maybe_update_session_range (i->from, i->to);
	}
}

/* Region management */

boost::shared_ptr<Region>
Session::find_whole_file_parent (boost::shared_ptr<Region const> child) const
{
	const RegionFactory::RegionMap& regions (RegionFactory::regions());
	RegionFactory::RegionMap::const_iterator i;
	boost::shared_ptr<Region> region;

	Glib::Mutex::Lock lm (region_lock);

	for (i = regions.begin(); i != regions.end(); ++i) {

		region = i->second;

		if (region->whole_file()) {

			if (child->source_equivalent (region)) {
				return region;
			}
		}
	}

	return boost::shared_ptr<Region> ();
}

int
Session::destroy_sources (list<boost::shared_ptr<Source> > srcs)
{
	set<boost::shared_ptr<Region> > relevant_regions;

	for (list<boost::shared_ptr<Source> >::iterator s = srcs.begin(); s != srcs.end(); ++s) {
		RegionFactory::get_regions_using_source (*s, relevant_regions);
	}

	for (set<boost::shared_ptr<Region> >::iterator r = relevant_regions.begin(); r != relevant_regions.end(); ) {
		set<boost::shared_ptr<Region> >::iterator tmp;

		tmp = r;
		++tmp;

		playlists->destroy_region (*r);
		RegionFactory::map_remove (*r);

		(*r)->drop_sources ();
		(*r)->drop_references ();

		relevant_regions.erase (r);

		r = tmp;
	}

	for (list<boost::shared_ptr<Source> >::iterator s = srcs.begin(); s != srcs.end(); ) {

		{
			Glib::Mutex::Lock ls (source_lock);
			/* remove from the main source list */
			sources.erase ((*s)->id());
		}

		(*s)->mark_for_remove ();
		(*s)->drop_references ();

		s = srcs.erase (s);
	}

	return 0;
}

int
Session::remove_last_capture ()
{
	list<boost::shared_ptr<Source> > srcs;

	boost::shared_ptr<RouteList> rl = routes.reader ();
	for (RouteList::iterator i = rl->begin(); i != rl->end(); ++i) {
		boost::shared_ptr<Track> tr = boost::dynamic_pointer_cast<Track> (*i);
		if (!tr) {
			continue;
		}

		list<boost::shared_ptr<Source> >& l = tr->last_capture_sources();

		if (!l.empty()) {
			srcs.insert (srcs.end(), l.begin(), l.end());
			l.clear ();
		}
	}

	destroy_sources (srcs);

	save_state (_current_snapshot_name);

	return 0;
}

/* Source Management */

void
Session::add_source (boost::shared_ptr<Source> source)
{
	pair<SourceMap::key_type, SourceMap::mapped_type> entry;
	pair<SourceMap::iterator,bool> result;

	entry.first = source->id();
	entry.second = source;

	{
		Glib::Mutex::Lock lm (source_lock);
		result = sources.insert (entry);
	}

	if (result.second) {

		/* yay, new source */

		boost::shared_ptr<FileSource> fs = boost::dynamic_pointer_cast<FileSource> (source);
		
		if (fs) {
			if (!fs->within_session()) {
				ensure_search_path_includes (Glib::path_get_dirname (fs->path()), fs->type());
			}
		}
		
		set_dirty();

		boost::shared_ptr<AudioFileSource> afs;

		if ((afs = boost::dynamic_pointer_cast<AudioFileSource>(source)) != 0) {
			if (Config->get_auto_analyse_audio()) {
				Analyser::queue_source_for_analysis (source, false);
			}
		}

		source->DropReferences.connect_same_thread (*this, boost::bind (&Session::remove_source, this, boost::weak_ptr<Source> (source)));
	}
}

void
Session::remove_source (boost::weak_ptr<Source> src)
{
	if (_state_of_the_state & Deletion) {
		return;
	}

	SourceMap::iterator i;
	boost::shared_ptr<Source> source = src.lock();

	if (!source) {
		return;
	}

	{
		Glib::Mutex::Lock lm (source_lock);

		if ((i = sources.find (source->id())) != sources.end()) {
			sources.erase (i);
		}
	}

	if (!(_state_of_the_state & InCleanup)) {

		/* save state so we don't end up with a session file
		   referring to non-existent sources.
		*/

		save_state (_current_snapshot_name);
	}
}

boost::shared_ptr<Source>
Session::source_by_id (const PBD::ID& id)
{
	Glib::Mutex::Lock lm (source_lock);
	SourceMap::iterator i;
	boost::shared_ptr<Source> source;

	if ((i = sources.find (id)) != sources.end()) {
		source = i->second;
	}

	return source;
}

boost::shared_ptr<Source>
Session::source_by_path_and_channel (const string& path, uint16_t chn)
{
	Glib::Mutex::Lock lm (source_lock);

	for (SourceMap::iterator i = sources.begin(); i != sources.end(); ++i) {
		boost::shared_ptr<AudioFileSource> afs
			= boost::dynamic_pointer_cast<AudioFileSource>(i->second);

		if (afs && afs->path() == path && chn == afs->channel()) {
			return afs;
		}
	}
	return boost::shared_ptr<Source>();
}

uint32_t
Session::count_sources_by_origin (const string& path)
{
	uint32_t cnt = 0;
	Glib::Mutex::Lock lm (source_lock);

	for (SourceMap::iterator i = sources.begin(); i != sources.end(); ++i) {
		boost::shared_ptr<FileSource> fs
			= boost::dynamic_pointer_cast<FileSource>(i->second);

		if (fs && fs->origin() == path) {
			++cnt;
		}
	}

	return cnt;
}


string
Session::change_source_path_by_name (string path, string oldname, string newname, bool destructive)
{
	string look_for;
	string old_basename = PBD::basename_nosuffix (oldname);
	string new_legalized = legalize_for_path (newname);

	/* note: we know (or assume) the old path is already valid */

	if (destructive) {

		/* destructive file sources have a name of the form:

		    /path/to/Tnnnn-NAME(%[LR])?.wav

		    the task here is to replace NAME with the new name.
		*/

		string dir;
		string prefix;
		string::size_type dash;

		dir = Glib::path_get_dirname (path);
		path = Glib::path_get_basename (path);

		/* '-' is not a legal character for the NAME part of the path */

		if ((dash = path.find_last_of ('-')) == string::npos) {
			return "";
		}

		prefix = path.substr (0, dash);

		path += prefix;
		path += '-';
		path += new_legalized;
		path += native_header_format_extension (config.get_native_file_header_format(), DataType::AUDIO);
		path = Glib::build_filename (dir, path);

	} else {

		/* non-destructive file sources have a name of the form:

		    /path/to/NAME-nnnnn(%[LR])?.ext

		    the task here is to replace NAME with the new name.
		*/

		string dir;
		string suffix;
		string::size_type dash;
		string::size_type postfix;

		dir = Glib::path_get_dirname (path);
		path = Glib::path_get_basename (path);

		/* '-' is not a legal character for the NAME part of the path */

		if ((dash = path.find_last_of ('-')) == string::npos) {
			return "";
		}

		suffix = path.substr (dash+1);

		// Suffix is now everything after the dash. Now we need to eliminate
		// the nnnnn part, which is done by either finding a '%' or a '.'

		postfix = suffix.find_last_of ("%");
		if (postfix == string::npos) {
			postfix = suffix.find_last_of ('.');
		}

		if (postfix != string::npos) {
			suffix = suffix.substr (postfix);
		} else {
			error << "Logic error in Session::change_source_path_by_name(), please report" << endl;
			return "";
		}

		const uint32_t limit = 10000;
		char buf[PATH_MAX+1];

		for (uint32_t cnt = 1; cnt <= limit; ++cnt) {

			snprintf (buf, sizeof(buf), "%s-%u%s", newname.c_str(), cnt, suffix.c_str());

			if (!matching_unsuffixed_filename_exists_in (dir, buf)) {
				path = Glib::build_filename (dir, buf);
				break;
			}

			path = "";
		}

		if (path.empty()) {
			fatal << string_compose (_("FATAL ERROR! Could not find a suitable version of %1 for a rename"),
			                         newname) << endl;
			/*NOTREACHED*/
		}
	}

	return path;
}

/** Return the full path (in some session directory) for a new within-session source.
 * \a name must be a session-unique name that does not contain slashes
 *         (e.g. as returned by new_*_source_name)
 */
string
Session::new_source_path_from_name (DataType type, const string& name)
{
	assert(name.find("/") == string::npos);

	SessionDirectory sdir(get_best_session_directory_for_new_source());

	sys::path p;
	if (type == DataType::AUDIO) {
		p = sdir.sound_path();
	} else if (type == DataType::MIDI) {
		p = sdir.midi_path();
	} else {
		error << "Unknown source type, unable to create file path" << endmsg;
		return "";
	}

	p /= name;
	return p.to_string();
}

string
Session::peak_path (string base) const
{
	sys::path peakfile_path(_session_dir->peak_path());
	peakfile_path /= base + peakfile_suffix;
	return peakfile_path.to_string();
}

/** Return a unique name based on \a base for a new internal audio source */
string
Session::new_audio_source_name (const string& base, uint32_t nchan, uint32_t chan, bool destructive)
{
	uint32_t cnt;
	char buf[PATH_MAX+1];
	const uint32_t limit = 10000;
	string legalized;
	string ext = native_header_format_extension (config.get_native_file_header_format(), DataType::AUDIO);

	buf[0] = '\0';
	legalized = legalize_for_path (base);

	// Find a "version" of the base name that doesn't exist in any of the possible directories.
	for (cnt = (destructive ? ++destructive_index : 1); cnt <= limit; ++cnt) {

		vector<space_and_path>::iterator i;
		uint32_t existing = 0;

		for (i = session_dirs.begin(); i != session_dirs.end(); ++i) {

			if (destructive) {

				if (nchan < 2) {
					snprintf (buf, sizeof(buf), "T%04d-%s%s",
					          cnt, legalized.c_str(), ext.c_str());
				} else if (nchan == 2) {
					if (chan == 0) {
						snprintf (buf, sizeof(buf), "T%04d-%s%%L%s",
						          cnt, legalized.c_str(), ext.c_str());
					} else {
						snprintf (buf, sizeof(buf), "T%04d-%s%%R%s",
						          cnt, legalized.c_str(), ext.c_str());
					}
				} else if (nchan < 26) {
					snprintf (buf, sizeof(buf), "T%04d-%s%%%c%s",
					          cnt, legalized.c_str(), 'a' + chan, ext.c_str());
				} else {
					snprintf (buf, sizeof(buf), "T%04d-%s%s",
					          cnt, legalized.c_str(), ext.c_str());
				}

			} else {

				if (nchan < 2) {
					snprintf (buf, sizeof(buf), "%s-%u%s", legalized.c_str(), cnt, ext.c_str());
				} else if (nchan == 2) {
					if (chan == 0) {
						snprintf (buf, sizeof(buf), "%s-%u%%L%s", legalized.c_str(), cnt, ext.c_str());
					} else {
						snprintf (buf, sizeof(buf), "%s-%u%%R%s", legalized.c_str(), cnt, ext.c_str());
					}
				} else if (nchan < 26) {
					snprintf (buf, sizeof(buf), "%s-%u%%%c%s", legalized.c_str(), cnt, 'a' + chan, ext.c_str());
				} else {
					snprintf (buf, sizeof(buf), "%s-%u%s", legalized.c_str(), cnt, ext.c_str());
				}
			}

			SessionDirectory sdir((*i).path);

			string spath = sdir.sound_path().to_string();

			/* note that we search *without* the extension so that
			   we don't end up both "Audio 1-1.wav" and "Audio 1-1.caf"
			   in the event that this new name is required for
			   a file format change.
			*/

			if (matching_unsuffixed_filename_exists_in (spath, buf)) {
				existing++;
				break;
			}
		}

		if (existing == 0) {
			break;
		}

		if (cnt > limit) {
			error << string_compose(
					_("There are already %1 recordings for %2, which I consider too many."),
					limit, base) << endmsg;
			destroy ();
			throw failed_constructor();
		}
	}

	return Glib::path_get_basename (buf);
}

/** Create a new within-session audio source */
boost::shared_ptr<AudioFileSource>
Session::create_audio_source_for_session (size_t n_chans, string const & n, uint32_t chan, bool destructive)
{
	const string name    = new_audio_source_name (n, n_chans, chan, destructive);
	const string path    = new_source_path_from_name(DataType::AUDIO, name);

	return boost::dynamic_pointer_cast<AudioFileSource> (
		SourceFactory::createWritable (DataType::AUDIO, *this, path, string(), destructive, frame_rate()));
}

/** Return a unique name based on \a base for a new internal MIDI source */
string
Session::new_midi_source_name (const string& base)
{
	uint32_t cnt;
	char buf[PATH_MAX+1];
	const uint32_t limit = 10000;
	string legalized;

	buf[0] = '\0';
	legalized = legalize_for_path (base);

	// Find a "version" of the file name that doesn't exist in any of the possible directories.
	for (cnt = 1; cnt <= limit; ++cnt) {

		vector<space_and_path>::iterator i;
		uint32_t existing = 0;

		for (i = session_dirs.begin(); i != session_dirs.end(); ++i) {

			SessionDirectory sdir((*i).path);

			sys::path p = sdir.midi_path();
			p /= legalized;

			snprintf (buf, sizeof(buf), "%s-%u.mid", p.to_string().c_str(), cnt);

			if (sys::exists (buf)) {
				existing++;
			}
		}

		if (existing == 0) {
			break;
		}

		if (cnt > limit) {
			error << string_compose(
					_("There are already %1 recordings for %2, which I consider too many."),
					limit, base) << endmsg;
			destroy ();
			throw failed_constructor();
		}
	}

	return Glib::path_get_basename(buf);
}


/** Create a new within-session MIDI source */
boost::shared_ptr<MidiSource>
Session::create_midi_source_for_session (Track* track, string const & n)
{
	/* try to use the existing write source for the track, to keep numbering sane
	 */

	if (track) {
		/*MidiTrack* mt = dynamic_cast<Track*> (track);
		  assert (mt);
		*/

		list<boost::shared_ptr<Source> > l = track->steal_write_sources ();

		if (!l.empty()) {
			assert (boost::dynamic_pointer_cast<MidiSource> (l.front()));
			return boost::dynamic_pointer_cast<MidiSource> (l.front());
		}
	}

	const string name = new_midi_source_name (n);
	const string path = new_source_path_from_name (DataType::MIDI, name);

	return boost::dynamic_pointer_cast<SMFSource> (
		SourceFactory::createWritable (
			DataType::MIDI, *this, path, string(), false, frame_rate()));
}


void
Session::add_playlist (boost::shared_ptr<Playlist> playlist, bool unused)
{
	if (playlist->hidden()) {
		return;
	}

	playlists->add (playlist);

	if (unused) {
		playlist->release();
	}

	set_dirty();
}

void
Session::remove_playlist (boost::weak_ptr<Playlist> weak_playlist)
{
	if (_state_of_the_state & Deletion) {
		return;
	}

	boost::shared_ptr<Playlist> playlist (weak_playlist.lock());

	if (!playlist) {
		return;
	}

	playlists->remove (playlist);

	set_dirty();
}

void
Session::set_audition (boost::shared_ptr<Region> r)
{
	pending_audition_region = r;
	add_post_transport_work (PostTransportAudition);
	_butler->schedule_transport_work ();
}

void
Session::audition_playlist ()
{
	SessionEvent* ev = new SessionEvent (SessionEvent::Audition, SessionEvent::Add, SessionEvent::Immediate, 0, 0.0);
	ev->region.reset ();
	queue_event (ev);
}

void
Session::non_realtime_set_audition ()
{
	assert (pending_audition_region);
	auditioner->audition_region (pending_audition_region);
	pending_audition_region.reset ();
	AuditionActive (true); /* EMIT SIGNAL */
}

void
Session::audition_region (boost::shared_ptr<Region> r)
{
	SessionEvent* ev = new SessionEvent (SessionEvent::Audition, SessionEvent::Add, SessionEvent::Immediate, 0, 0.0);
	ev->region = r;
	queue_event (ev);
}

void
Session::cancel_audition ()
{
	if (auditioner->auditioning()) {
		auditioner->cancel_audition ();
		AuditionActive (false); /* EMIT SIGNAL */
	}
}

bool
Session::RoutePublicOrderSorter::operator() (boost::shared_ptr<Route> a, boost::shared_ptr<Route> b)
{
	if (a->is_monitor()) {
		return true;
	}
	if (b->is_monitor()) {
		return false;
	}
	return a->order_key(N_("signal")) < b->order_key(N_("signal"));
}

bool
Session::is_auditioning () const
{
	/* can be called before we have an auditioner object */
	if (auditioner) {
		return auditioner->auditioning();
	} else {
		return false;
	}
}

void
Session::graph_reordered ()
{
	/* don't do this stuff if we are setting up connections
	   from a set_state() call or creating new tracks. Ditto for deletion.
	*/

	if (_state_of_the_state & (InitialConnecting|Deletion)) {
		return;
	}

	/* every track/bus asked for this to be handled but it was deferred because
	   we were connecting. do it now.
	*/

	request_input_change_handling ();

	resort_routes ();

	/* force all diskstreams to update their capture offset values to
	   reflect any changes in latencies within the graph.
	*/

	boost::shared_ptr<RouteList> rl = routes.reader ();
	for (RouteList::iterator i = rl->begin(); i != rl->end(); ++i) {
		boost::shared_ptr<Track> tr = boost::dynamic_pointer_cast<Track> (*i);
		if (tr) {
			tr->set_capture_offset ();
		}
	}
}

framecnt_t
Session::available_capture_duration ()
{
	float sample_bytes_on_disk = 4.0; // keep gcc happy

	switch (config.get_native_file_data_format()) {
	case FormatFloat:
		sample_bytes_on_disk = 4.0;
		break;

	case FormatInt24:
		sample_bytes_on_disk = 3.0;
		break;

	case FormatInt16:
		sample_bytes_on_disk = 2.0;
		break;

	default:
		/* impossible, but keep some gcc versions happy */
		fatal << string_compose (_("programming error: %1"),
					 X_("illegal native file data format"))
		      << endmsg;
		/*NOTREACHED*/
	}

	double scale = 4096.0 / sample_bytes_on_disk;

	if (_total_free_4k_blocks * scale > (double) max_framecnt) {
		return max_framecnt;
	}

	return (framecnt_t) floor (_total_free_4k_blocks * scale);
}

void
Session::add_bundle (boost::shared_ptr<Bundle> bundle)
{
	{
		RCUWriter<BundleList> writer (_bundles);
		boost::shared_ptr<BundleList> b = writer.get_copy ();
		b->push_back (bundle);
	}

	BundleAdded (bundle); /* EMIT SIGNAL */

	set_dirty();
}

void
Session::remove_bundle (boost::shared_ptr<Bundle> bundle)
{
	bool removed = false;

	{
		RCUWriter<BundleList> writer (_bundles);
		boost::shared_ptr<BundleList> b = writer.get_copy ();
		BundleList::iterator i = find (b->begin(), b->end(), bundle);

		if (i != b->end()) {
			b->erase (i);
			removed = true;
		}
	}

	if (removed) {
		 BundleRemoved (bundle); /* EMIT SIGNAL */
	}

	set_dirty();
}

boost::shared_ptr<Bundle>
Session::bundle_by_name (string name) const
{
	boost::shared_ptr<BundleList> b = _bundles.reader ();

	for (BundleList::const_iterator i = b->begin(); i != b->end(); ++i) {
		if ((*i)->name() == name) {
			return* i;
		}
	}

	return boost::shared_ptr<Bundle> ();
}

void
Session::tempo_map_changed (const PropertyChange&)
{
	clear_clicks ();

	playlists->update_after_tempo_map_change ();

	_locations->apply (*this, &Session::update_locations_after_tempo_map_change);

	set_dirty ();
}

void
Session::update_locations_after_tempo_map_change (Locations::LocationList& loc)
{
	for (Locations::LocationList::iterator i = loc.begin(); i != loc.end(); ++i) {
		(*i)->recompute_frames_from_bbt ();
	}
}

/** Ensures that all buffers (scratch, send, silent, etc) are allocated for
 * the given count with the current block size.
 */
void
Session::ensure_buffers (ChanCount howmany)
{
	BufferManager::ensure_buffers (howmany);
}

void
Session::ensure_buffer_set(BufferSet& buffers, const ChanCount& count)
{
	for (DataType::iterator t = DataType::begin(); t != DataType::end(); ++t) {
		buffers.ensure_buffers(*t, count.get(*t), _engine.raw_buffer_size(*t));
	}
}

uint32_t
Session::next_insert_id ()
{
	/* this doesn't really loop forever. just think about it */

	while (true) {
		for (boost::dynamic_bitset<uint32_t>::size_type n = 0; n < insert_bitset.size(); ++n) {
			if (!insert_bitset[n]) {
				insert_bitset[n] = true;
				return n;

			}
		}

		/* none available, so resize and try again */

		insert_bitset.resize (insert_bitset.size() + 16, false);
	}
}

uint32_t
Session::next_send_id ()
{
	/* this doesn't really loop forever. just think about it */

	while (true) {
		for (boost::dynamic_bitset<uint32_t>::size_type n = 0; n < send_bitset.size(); ++n) {
			if (!send_bitset[n]) {
				send_bitset[n] = true;
				return n;

			}
		}

		/* none available, so resize and try again */

		send_bitset.resize (send_bitset.size() + 16, false);
	}
}

uint32_t
Session::next_aux_send_id ()
{
	/* this doesn't really loop forever. just think about it */

	while (true) {
		for (boost::dynamic_bitset<uint32_t>::size_type n = 0; n < aux_send_bitset.size(); ++n) {
			if (!aux_send_bitset[n]) {
				aux_send_bitset[n] = true;
				return n;

			}
		}

		/* none available, so resize and try again */

		aux_send_bitset.resize (aux_send_bitset.size() + 16, false);
	}
}

uint32_t
Session::next_return_id ()
{
	/* this doesn't really loop forever. just think about it */

	while (true) {
		for (boost::dynamic_bitset<uint32_t>::size_type n = 0; n < return_bitset.size(); ++n) {
			if (!return_bitset[n]) {
				return_bitset[n] = true;
				return n;

			}
		}

		/* none available, so resize and try again */

		return_bitset.resize (return_bitset.size() + 16, false);
	}
}

void
Session::mark_send_id (uint32_t id)
{
	if (id >= send_bitset.size()) {
		send_bitset.resize (id+16, false);
	}
	if (send_bitset[id]) {
		warning << string_compose (_("send ID %1 appears to be in use already"), id) << endmsg;
	}
	send_bitset[id] = true;
}

void
Session::mark_aux_send_id (uint32_t id)
{
	if (id >= aux_send_bitset.size()) {
		aux_send_bitset.resize (id+16, false);
	}
	if (aux_send_bitset[id]) {
		warning << string_compose (_("aux send ID %1 appears to be in use already"), id) << endmsg;
	}
	aux_send_bitset[id] = true;
}

void
Session::mark_return_id (uint32_t id)
{
	if (id >= return_bitset.size()) {
		return_bitset.resize (id+16, false);
	}
	if (return_bitset[id]) {
		warning << string_compose (_("return ID %1 appears to be in use already"), id) << endmsg;
	}
	return_bitset[id] = true;
}

void
Session::mark_insert_id (uint32_t id)
{
	if (id >= insert_bitset.size()) {
		insert_bitset.resize (id+16, false);
	}
	if (insert_bitset[id]) {
		warning << string_compose (_("insert ID %1 appears to be in use already"), id) << endmsg;
	}
	insert_bitset[id] = true;
}

void
Session::unmark_send_id (uint32_t id)
{
	if (id < send_bitset.size()) {
		send_bitset[id] = false;
	}
}

void
Session::unmark_aux_send_id (uint32_t id)
{
	if (id < aux_send_bitset.size()) {
		aux_send_bitset[id] = false;
	}
}

void
Session::unmark_return_id (uint32_t id)
{
	if (id < return_bitset.size()) {
		return_bitset[id] = false;
	}
}

void
Session::unmark_insert_id (uint32_t id)
{
	if (id < insert_bitset.size()) {
		insert_bitset[id] = false;
	}
}


/* Named Selection management */

boost::shared_ptr<NamedSelection>
Session::named_selection_by_name (string name)
{
	Glib::Mutex::Lock lm (named_selection_lock);
	for (NamedSelectionList::iterator i = named_selections.begin(); i != named_selections.end(); ++i) {
		if ((*i)->name == name) {
			return *i;
		}
	}
	return boost::shared_ptr<NamedSelection>();
}

void
Session::add_named_selection (boost::shared_ptr<NamedSelection> named_selection)
{
	{
		Glib::Mutex::Lock lm (named_selection_lock);
		named_selections.insert (named_selections.begin(), named_selection);
	}

	set_dirty();

	NamedSelectionAdded (); /* EMIT SIGNAL */
}

void
Session::remove_named_selection (boost::shared_ptr<NamedSelection> named_selection)
{
	bool removed = false;

	{
		Glib::Mutex::Lock lm (named_selection_lock);

		NamedSelectionList::iterator i = find (named_selections.begin(), named_selections.end(), named_selection);

		if (i != named_selections.end()) {
			named_selections.erase (i);
			set_dirty();
			removed = true;
		}
	}

	if (removed) {
		 NamedSelectionRemoved (); /* EMIT SIGNAL */
	}
}

void
Session::reset_native_file_format ()
{
	boost::shared_ptr<RouteList> rl = routes.reader ();
	for (RouteList::iterator i = rl->begin(); i != rl->end(); ++i) {
		boost::shared_ptr<Track> tr = boost::dynamic_pointer_cast<Track> (*i);
		if (tr) {
			/* don't save state as we do this, there's no point
			 */

			_state_of_the_state = StateOfTheState (_state_of_the_state|InCleanup);
			tr->reset_write_sources (false);
			_state_of_the_state = StateOfTheState (_state_of_the_state & ~InCleanup);
		}
	}
}

bool
Session::route_name_unique (string n) const
{
	boost::shared_ptr<RouteList> r = routes.reader ();

	for (RouteList::const_iterator i = r->begin(); i != r->end(); ++i) {
		if ((*i)->name() == n) {
			return false;
		}
	}

	return true;
}

bool
Session::route_name_internal (string n) const
{
	if (auditioner && auditioner->name() == n) {
		return true;
	}

	if (_click_io && _click_io->name() == n) {
		return true;
	}

	return false;
}

int
Session::freeze_all (InterThreadInfo& itt)
{
	boost::shared_ptr<RouteList> r = routes.reader ();

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {

		boost::shared_ptr<Track> t;

		if ((t = boost::dynamic_pointer_cast<Track>(*i)) != 0) {
			/* XXX this is wrong because itt.progress will keep returning to zero at the start
			   of every track.
			*/
			t->freeze_me (itt);
		}
	}

	return 0;
}

boost::shared_ptr<Region>
Session::write_one_track (AudioTrack& track, framepos_t start, framepos_t end,
			  bool /*overwrite*/, vector<boost::shared_ptr<Source> >& srcs,
			  InterThreadInfo& itt, 
			  boost::shared_ptr<Processor> endpoint, bool include_endpoint,
			  bool for_export)
{
	boost::shared_ptr<Region> result;
	boost::shared_ptr<Playlist> playlist;
	boost::shared_ptr<AudioFileSource> fsource;
	uint32_t x;
	char buf[PATH_MAX+1];
	ChanCount diskstream_channels (track.n_channels());
	framepos_t position;
	framecnt_t this_chunk;
	framepos_t to_do;
	BufferSet buffers;
	SessionDirectory sdir(get_best_session_directory_for_new_source ());
	const string sound_dir = sdir.sound_path().to_string();
	framepos_t len = end - start;
	bool need_block_size_reset = false;
	string ext;
	ChanCount const max_proc = track.max_processor_streams ();

	if (end <= start) {
		error << string_compose (_("Cannot write a range where end <= start (e.g. %1 <= %2)"),
					 end, start) << endmsg;
		return result;
	}

	const framecnt_t chunk_size = (256 * 1024)/4;

	// block all process callback handling

	block_processing ();

	/* call tree *MUST* hold route_lock */

	if ((playlist = track.playlist()) == 0) {
		goto out;
	}

	ext = native_header_format_extension (config.get_native_file_header_format(), DataType::AUDIO);

	for (uint32_t chan_n = 0; chan_n < diskstream_channels.n_audio(); ++chan_n) {

		for (x = 0; x < 99999; ++x) {
			snprintf (buf, sizeof(buf), "%s/%s-%d-bounce-%" PRIu32 "%s", sound_dir.c_str(), playlist->name().c_str(), chan_n, x+1, ext.c_str());
			if (!Glib::file_test (buf, Glib::FILE_TEST_EXISTS)) {
				break;
			}
		}

		if (x == 99999) {
			error << string_compose (_("too many bounced versions of playlist \"%1\""), playlist->name()) << endmsg;
			goto out;
		}

		try {
			fsource = boost::dynamic_pointer_cast<AudioFileSource> (
				SourceFactory::createWritable (DataType::AUDIO, *this, buf, string(), false, frame_rate()));
		}

		catch (failed_constructor& err) {
			error << string_compose (_("cannot create new audio file \"%1\" for %2"), buf, track.name()) << endmsg;
			goto out;
		}

		srcs.push_back (fsource);
	}

	/* tell redirects that care that we are about to use a much larger
	 * blocksize. this will flush all plugins too, so that they are ready
	 * to be used for this process.
	 */

	need_block_size_reset = true;
	track.set_block_size (chunk_size);

	position = start;
	to_do = len;

	/* create a set of reasonably-sized buffers */
	buffers.ensure_buffers (DataType::AUDIO, max_proc.n_audio(), chunk_size);
	buffers.set_count (max_proc);

	for (vector<boost::shared_ptr<Source> >::iterator src = srcs.begin(); src != srcs.end(); ++src) {
		boost::shared_ptr<AudioFileSource> afs = boost::dynamic_pointer_cast<AudioFileSource>(*src);
		if (afs)
			afs->prepare_for_peakfile_writes ();
	}

	while (to_do && !itt.cancel) {

		this_chunk = min (to_do, chunk_size);

		if (track.export_stuff (buffers, start, this_chunk, endpoint, include_endpoint, for_export)) {
			goto out;
		}

		uint32_t n = 0;
		for (vector<boost::shared_ptr<Source> >::iterator src=srcs.begin(); src != srcs.end(); ++src, ++n) {
			boost::shared_ptr<AudioFileSource> afs = boost::dynamic_pointer_cast<AudioFileSource>(*src);

			if (afs) {
				if (afs->write (buffers.get_audio(n).data(), this_chunk) != this_chunk) {
					goto out;
				}
			}
		}

		start += this_chunk;
		to_do -= this_chunk;

		itt.progress = (float) (1.0 - ((double) to_do / len));

	}

	if (!itt.cancel) {

		time_t now;
		struct tm* xnow;
		time (&now);
		xnow = localtime (&now);

		for (vector<boost::shared_ptr<Source> >::iterator src=srcs.begin(); src != srcs.end(); ++src) {
			boost::shared_ptr<AudioFileSource> afs = boost::dynamic_pointer_cast<AudioFileSource>(*src);

			if (afs) {
				afs->update_header (position, *xnow, now);
				afs->flush_header ();
			}
		}

		/* construct a region to represent the bounced material */

		PropertyList plist;

		plist.add (Properties::start, 0);
		plist.add (Properties::length, srcs.front()->length(srcs.front()->timeline_position()));
		plist.add (Properties::name, region_name_from_path (srcs.front()->name(), true));

		result = RegionFactory::create (srcs, plist);

	}

  out:
	if (!result) {
		for (vector<boost::shared_ptr<Source> >::iterator src = srcs.begin(); src != srcs.end(); ++src) {
			boost::shared_ptr<AudioFileSource> afs = boost::dynamic_pointer_cast<AudioFileSource>(*src);

			if (afs) {
				afs->mark_for_remove ();
			}

			(*src)->drop_references ();
		}

	} else {
		for (vector<boost::shared_ptr<Source> >::iterator src = srcs.begin(); src != srcs.end(); ++src) {
			boost::shared_ptr<AudioFileSource> afs = boost::dynamic_pointer_cast<AudioFileSource>(*src);

			if (afs)
				afs->done_with_peakfile_writes ();
		}
	}


	if (need_block_size_reset) {
		track.set_block_size (get_block_size());
	}

	unblock_processing ();

	return result;
}

gain_t*
Session::gain_automation_buffer() const
{
	return ProcessThread::gain_automation_buffer ();
}

pan_t**
Session::pan_automation_buffer() const
{
	return ProcessThread::pan_automation_buffer ();
}

BufferSet&
Session::get_silent_buffers (ChanCount count)
{
	return ProcessThread::get_silent_buffers (count);
}

BufferSet&
Session::get_scratch_buffers (ChanCount count)
{
	return ProcessThread::get_scratch_buffers (count);
}

BufferSet&
Session::get_mix_buffers (ChanCount count)
{
	return ProcessThread::get_mix_buffers (count);
}

uint32_t
Session::ntracks () const
{
	uint32_t n = 0;
	boost::shared_ptr<RouteList> r = routes.reader ();

	for (RouteList::const_iterator i = r->begin(); i != r->end(); ++i) {
		if (boost::dynamic_pointer_cast<Track> (*i)) {
			++n;
		}
	}

	return n;
}

uint32_t
Session::nbusses () const
{
	uint32_t n = 0;
	boost::shared_ptr<RouteList> r = routes.reader ();

	for (RouteList::const_iterator i = r->begin(); i != r->end(); ++i) {
		if (boost::dynamic_pointer_cast<Track>(*i) == 0) {
			++n;
		}
	}

	return n;
}

void
Session::add_automation_list(AutomationList *al)
{
	automation_lists[al->id()] = al;
}

void
Session::sync_order_keys (std::string const & base)
{
	if (deletion_in_progress()) {
		return;
	}

	if (!Config->get_sync_all_route_ordering()) {
		/* leave order keys as they are */
		return;
	}

	boost::shared_ptr<RouteList> r = routes.reader ();

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		(*i)->sync_order_keys (base);
	}

	Route::SyncOrderKeys (base); // EMIT SIGNAL

	/* this might not do anything */

	set_remote_control_ids ();
}

/** @return true if there is at least one record-enabled track, otherwise false */
bool
Session::have_rec_enabled_track () const
{
	return g_atomic_int_get (&_have_rec_enabled_track) == 1;
}

/** Update the state of our rec-enabled tracks flag */
void
Session::update_have_rec_enabled_track ()
{
	boost::shared_ptr<RouteList> rl = routes.reader ();
	RouteList::iterator i = rl->begin();
	while (i != rl->end ()) {

		boost::shared_ptr<Track> tr = boost::dynamic_pointer_cast<Track> (*i);
		if (tr && tr->record_enabled ()) {
			break;
		}

		++i;
	}

	int const old = g_atomic_int_get (&_have_rec_enabled_track);

	g_atomic_int_set (&_have_rec_enabled_track, i != rl->end () ? 1 : 0);

	if (g_atomic_int_get (&_have_rec_enabled_track) != old) {
		RecordStateChanged (); /* EMIT SIGNAL */
	}
}

void
Session::listen_position_changed ()
{
	boost::shared_ptr<RouteList> r = routes.reader ();

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		(*i)->listen_position_changed ();
	}
}

void
Session::solo_control_mode_changed ()
{
	/* cancel all solo or all listen when solo control mode changes */

	if (soloing()) {
		set_solo (get_routes(), false);
	} else if (listening()) {
		set_listen (get_routes(), false);
	}
}

/** Called when a property of one of our route groups changes */
void
Session::route_group_property_changed (RouteGroup* rg)
{
	RouteGroupPropertyChanged (rg); /* EMIT SIGNAL */
}

/** Called when a route is added to one of our route groups */
void
Session::route_added_to_route_group (RouteGroup* rg, boost::weak_ptr<Route> r)
{
	RouteAddedToRouteGroup (rg, r);
}

/** Called when a route is removed from one of our route groups */
void
Session::route_removed_from_route_group (RouteGroup* rg, boost::weak_ptr<Route> r)
{
	RouteRemovedFromRouteGroup (rg, r);
}

vector<SyncSource>
Session::get_available_sync_options () const
{
	vector<SyncSource> ret;

	ret.push_back (JACK);
	ret.push_back (MTC);
	ret.push_back (MIDIClock);

	return ret;
}

boost::shared_ptr<RouteList>
Session::get_routes_with_regions_at (framepos_t const p) const
{
	boost::shared_ptr<RouteList> r = routes.reader ();
	boost::shared_ptr<RouteList> rl (new RouteList);

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		boost::shared_ptr<Track> tr = boost::dynamic_pointer_cast<Track> (*i);
		if (!tr) {
			continue;
		}

		boost::shared_ptr<Playlist> pl = tr->playlist ();
		if (!pl) {
			continue;
		}

		if (pl->has_region_at (p)) {
			rl->push_back (*i);
		}
	}

	return rl;
}

void
Session::goto_end ()
{
	if (_session_range_location) {
		request_locate (_session_range_location->end(), false);
	} else {
		request_locate (0, false);
	}
}

void
Session::goto_start ()
{
	if (_session_range_location) {
		request_locate (_session_range_location->start(), false);
	} else {
		request_locate (0, false);
	}
}

framepos_t
Session::current_start_frame () const
{
	return _session_range_location ? _session_range_location->start() : 0;
}

framepos_t
Session::current_end_frame () const
{
	return _session_range_location ? _session_range_location->end() : 0;
}

void
Session::add_session_range_location (framepos_t start, framepos_t end)
{
	_session_range_location = new Location (*this, start, end, _("session"), Location::IsSessionRange);
	_locations->add (_session_range_location);
}

/** Called when one of our routes' order keys has changed */
void
Session::route_order_key_changed ()
{
	RouteOrderKeyChanged (); /* EMIT SIGNAL */
}

void
Session::step_edit_status_change (bool yn)
{
	bool send = false;

	bool val = false;
	if (yn) {
		send = (_step_editors == 0);
		val = true;

		_step_editors++;
	} else {
		send = (_step_editors == 1);
		val = false;

		if (_step_editors > 0) {
			_step_editors--;
		}
	}

	if (send) {
		StepEditStatusChange (val);
	}
}


void
Session::start_time_changed (framepos_t old)
{
	/* Update the auto loop range to match the session range
	   (unless the auto loop range has been changed by the user)
	*/

	Location* s = _locations->session_range_location ();
	if (s == 0) {
		return;
	}

	Location* l = _locations->auto_loop_location ();

	if (l && l->start() == old) {
		l->set_start (s->start(), true);
	}
}

void
Session::end_time_changed (framepos_t old)
{
	/* Update the auto loop range to match the session range
	   (unless the auto loop range has been changed by the user)
	*/

	Location* s = _locations->session_range_location ();
	if (s == 0) {
		return;
	}

	Location* l = _locations->auto_loop_location ();

	if (l && l->end() == old) {
		l->set_end (s->end(), true);
	}
}

string
Session::source_search_path (DataType type) const
{
	vector<string> s;

	if (session_dirs.size() == 1) {
		switch (type) {
		case DataType::AUDIO:
			s.push_back ( _session_dir->sound_path().to_string());
			break;
		case DataType::MIDI:
			s.push_back (_session_dir->midi_path().to_string());
			break;
		}
	} else {
		for (vector<space_and_path>::const_iterator i = session_dirs.begin(); i != session_dirs.end(); ++i) {
			SessionDirectory sdir (i->path);
			switch (type) {
			case DataType::AUDIO:
				s.push_back (sdir.sound_path().to_string());
				break;
			case DataType::MIDI:
			        s.push_back (sdir.midi_path().to_string());
				break;
			}
		}
	}

	/* now check the explicit (possibly user-specified) search path
	 */

	vector<string> dirs;

	switch (type) {
	case DataType::AUDIO:
		split (config.get_audio_search_path (), dirs, ':');
		break;
	case DataType::MIDI:
		split (config.get_midi_search_path (), dirs, ':');
		break;
	}

	for (vector<string>::iterator i = dirs.begin(); i != dirs.end(); ++i) {

		vector<string>::iterator si;

		for (si = s.begin(); si != s.end(); ++si) {
			if ((*si) == *i) {
				break;
			}
		}

		if (si == s.end()) {
			s.push_back (*i);
		}
	}
	
	string search_path;

	for (vector<string>::iterator si = s.begin(); si != s.end(); ++si) {
		if (!search_path.empty()) {
			search_path += ':';
		}
		search_path += *si;
	}

	return search_path;
}

void
Session::ensure_search_path_includes (const string& path, DataType type)
{
	string search_path;
	vector<string> dirs;

	if (path == ".") {
		return;
	}

	switch (type) {
	case DataType::AUDIO:
		search_path = config.get_audio_search_path ();
		break;
	case DataType::MIDI:
		search_path = config.get_midi_search_path ();
		break;
	}

	split (search_path, dirs, ':');

	for (vector<string>::iterator i = dirs.begin(); i != dirs.end(); ++i) {
		/* No need to add this new directory if it has the same inode as
		   an existing one; checking inode rather than name prevents duplicated
		   directories when we are using symlinks.

		   On Windows, I think we could just do if (*i == path) here.
		*/
		if (PBD::sys::inodes_same (*i, path)) {
			return;
		}
	}

	if (!search_path.empty()) {
		search_path += ':';
	}

	search_path += path;

	switch (type) {
	case DataType::AUDIO:
		config.set_audio_search_path (search_path);
		break;
	case DataType::MIDI:
		config.set_midi_search_path (search_path);
		break;
	}
}

boost::shared_ptr<Speakers>
Session::get_speakers()
{
	return _speakers;
}

list<string>
Session::unknown_processors () const
{
	list<string> p;

	boost::shared_ptr<RouteList> r = routes.reader ();
	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		list<string> t = (*i)->unknown_processors ();
		copy (t.begin(), t.end(), back_inserter (p));
	}

	p.sort ();
	p.unique ();

	return p;
}

void
Session::update_latency (bool playback)
{
	DEBUG_TRACE (DEBUG::Latency, string_compose ("JACK latency callback: %1\n", (playback ? "PLAYBACK" : "CAPTURE")));

	if (_state_of_the_state & (InitialConnecting|Deletion)) {
		return;
	}

	boost::shared_ptr<RouteList> r = routes.reader ();
	framecnt_t max_latency = 0;

	if (playback) {
		/* reverse the list so that we work backwards from the last route to run to the first */
                RouteList* rl = routes.reader().get();
                r.reset (new RouteList (*rl));
		reverse (r->begin(), r->end());
	}

	/* compute actual latency values for the given direction and store them all in per-port
	   structures. this will also publish the same values (to JACK) so that computation of latency
	   for routes can consistently use public latency values.
	*/

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		max_latency = max (max_latency, (*i)->set_private_port_latencies (playback));
	}

        /* because we latency compensate playback, our published playback latencies should
           be the same for all output ports - all material played back by ardour has
           the same latency, whether its caused by plugins or by latency compensation. since
           these may differ from the values computed above, reset all playback port latencies
           to the same value.
        */

        DEBUG_TRACE (DEBUG::Latency, string_compose ("Set public port latencies to %1\n", max_latency));

        for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
                (*i)->set_public_port_latencies (max_latency, playback);
        }

	if (playback) {

		post_playback_latency ();

	} else {

		post_capture_latency ();
	}

	DEBUG_TRACE (DEBUG::Latency, "JACK latency callback: DONE\n");
}

void
Session::post_playback_latency ()
{
	set_worst_playback_latency ();

	boost::shared_ptr<RouteList> r = routes.reader ();

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		if (!(*i)->is_hidden() && ((*i)->active())) {
			_worst_track_latency = max (_worst_track_latency, (*i)->update_signal_latency ());
		}
	}

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		(*i)->set_latency_compensation (_worst_track_latency);
	}
}

void
Session::post_capture_latency ()
{
	set_worst_capture_latency ();

	/* reflect any changes in capture latencies into capture offsets
	 */

	boost::shared_ptr<RouteList> rl = routes.reader();
	for (RouteList::iterator i = rl->begin(); i != rl->end(); ++i) {
		boost::shared_ptr<Track> tr = boost::dynamic_pointer_cast<Track> (*i);
		if (tr) {
			tr->set_capture_offset ();
		}
	}
}

void
Session::initialize_latencies ()
{
        {
                Glib::Mutex::Lock lm (_engine.process_lock());
                update_latency (false);
                update_latency (true);
        }

        set_worst_io_latencies ();
}

void
Session::set_worst_io_latencies ()
{
	set_worst_playback_latency ();
	set_worst_capture_latency ();
}

void
Session::set_worst_playback_latency ()
{
	if (_state_of_the_state & (InitialConnecting|Deletion)) {
		return;
	}

	_worst_output_latency = 0;

	if (!_engine.connected()) {
		return;
	}

	boost::shared_ptr<RouteList> r = routes.reader ();

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		_worst_output_latency = max (_worst_output_latency, (*i)->output()->latency());
	}

	DEBUG_TRACE (DEBUG::Latency, string_compose ("Worst output latency: %1\n", _worst_output_latency));
}

void
Session::set_worst_capture_latency ()
{
	if (_state_of_the_state & (InitialConnecting|Deletion)) {
		return;
	}

	_worst_input_latency = 0;

	if (!_engine.connected()) {
		return;
	}

	boost::shared_ptr<RouteList> r = routes.reader ();

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		_worst_input_latency = max (_worst_input_latency, (*i)->input()->latency());
	}

        DEBUG_TRACE (DEBUG::Latency, string_compose ("Worst input latency: %1\n", _worst_input_latency));
}

void
Session::update_latency_compensation (bool force_whole_graph)
{
	bool some_track_latency_changed = false;

	if (_state_of_the_state & (InitialConnecting|Deletion)) {
		return;
	}

	DEBUG_TRACE(DEBUG::Latency, "---------------------------- update latency compensation\n\n");

	_worst_track_latency = 0;

	boost::shared_ptr<RouteList> r = routes.reader ();

	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		if (!(*i)->is_hidden() && ((*i)->active())) {
			framecnt_t tl;
			if ((*i)->signal_latency () != (tl = (*i)->update_signal_latency ())) {
				some_track_latency_changed = true;
			}
			_worst_track_latency = max (tl, _worst_track_latency);
		}
	}

	DEBUG_TRACE (DEBUG::Latency, string_compose ("worst signal processing latency: %1 (changed ? %2)\n", _worst_track_latency,
	                                             (some_track_latency_changed ? "yes" : "no")));

	DEBUG_TRACE(DEBUG::Latency, "---------------------------- DONE update latency compensation\n\n");
	
	if (some_track_latency_changed || force_whole_graph)  {
		_engine.update_latencies ();
	}


	for (RouteList::iterator i = r->begin(); i != r->end(); ++i) {
		boost::shared_ptr<Track> tr = boost::dynamic_pointer_cast<Track> (*i);
		if (!tr) {
			continue;
		}
		tr->set_capture_offset ();
	}
}

char
Session::session_name_is_legal (const string& path)
{
	char illegal_chars[] = { '/', '\\', ':', ';', '\0' };

	for (int i = 0; illegal_chars[i]; ++i) {
		if (path.find (illegal_chars[i]) != string::npos) {
			return illegal_chars[i];
		}
	}

	return 0;
}

uint32_t 
Session::next_control_id () const
{
	return ntracks() + nbusses() + 1;
}
