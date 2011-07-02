/*
    Copyright (C) 2000-2007 Paul Davis

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

#include <utility>
#include <gtkmm2ext/barcontroller.h>

#include "pbd/memento_command.h"
#include "pbd/stacktrace.h"

#include "ardour/automation_control.h"
#include "ardour/event_type_map.h"
#include "ardour/route.h"
#include "ardour/session.h"

#include "ardour_ui.h"
#include "automation_time_axis.h"
#include "automation_streamview.h"
#include "global_signals.h"
#include "gui_thread.h"
#include "route_time_axis.h"
#include "automation_line.h"
#include "public_editor.h"
#include "selection.h"
#include "rgb_macros.h"
#include "point_selection.h"
#include "utils.h"

#include "i18n.h"

using namespace std;
using namespace ARDOUR;
using namespace PBD;
using namespace Gtk;
using namespace Gtkmm2ext;
using namespace Editing;

Pango::FontDescription AutomationTimeAxisView::name_font;
bool AutomationTimeAxisView::have_name_font = false;
const string AutomationTimeAxisView::state_node_name = "AutomationChild";


/** \a a the automatable object this time axis is to display data for.
 * For route/track automation (e.g. gain) pass the route for both \r and \a.
 * For route child (e.g. plugin) automation, pass the child for \a.
 * For region automation (e.g. MIDI CC), pass null for \a.
 */
AutomationTimeAxisView::AutomationTimeAxisView (
	Session* s,
	boost::shared_ptr<Route> r,
	boost::shared_ptr<Automatable> a,
	boost::shared_ptr<AutomationControl> c,
	Evoral::Parameter p,
	PublicEditor& e,
	TimeAxisView& parent,
	bool show_regions,
	Canvas::Canvas& canvas,
	const string & nom,
	const string & nomparent
	)
	: AxisView (s)
	, TimeAxisView (s, e, &parent, canvas)
	, _route (r)
	, _control (c)
	, _automatable (a)
	, _parameter (p)
	, _base_rect (0)
	, _view (show_regions ? new AutomationStreamView (*this) : 0)
	, _name (nom)
	, auto_button (X_("")) /* force addition of a label */
{
	if (!have_name_font) {
		name_font = get_font_for_style (X_("AutomationTrackName"));
		have_name_font = true;
	}

	if (_automatable && _control) {
		_controller = AutomationController::create (_automatable, _control->parameter(), _control);
	}

	automation_menu = 0;
	auto_off_item = 0;
	auto_touch_item = 0;
	auto_write_item = 0;
	auto_play_item = 0;
	mode_discrete_item = 0;
	mode_line_item = 0;

	ignore_state_request = false;
	first_call_to_set_height = true;

	_base_rect = new Canvas::Rectangle (_canvas_display);
	_base_rect->set_x1 (Canvas::COORD_MAX);
	_base_rect->set_outline_color (ARDOUR_UI::config()->canvasvar_AutomationTrackOutline.get());

	/* outline ends and bottom */
	_base_rect->set_outline_what (0x1 | 0x2 | 0x8);
	_base_rect->set_fill_color (ARDOUR_UI::config()->canvasvar_AutomationTrackFill.get());

	_base_rect->set_data ("trackview", this);

	_base_rect->Event.connect (sigc::bind (
			sigc::mem_fun (_editor, &PublicEditor::canvas_automation_track_event),
			_base_rect, this));

	if (!a) {
		_base_rect->lower_to_bottom();
	}

	hide_button.add (*(manage (new Gtk::Image (::get_icon("hide")))));

	auto_button.set_name ("TrackVisualButton");
	hide_button.set_name ("TrackRemoveButton");

	auto_button.unset_flags (Gtk::CAN_FOCUS);
	hide_button.unset_flags (Gtk::CAN_FOCUS);

	controls_table.set_no_show_all();

	ARDOUR_UI::instance()->set_tip(auto_button, _("automation state"));
	ARDOUR_UI::instance()->set_tip(hide_button, _("hide track"));

	/* rearrange the name display */

	/* we never show these for automation tracks, so make
	   life easier and remove them.
	*/

	hide_name_entry();

	/* keep the parameter name short */

	string shortpname = _name;
	int ignore_width;
	shortpname = fit_to_pixels (_name, 60, name_font, ignore_width, true);

	name_label.set_text (shortpname);
	name_label.set_alignment (Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER);
        name_label.set_name (X_("TrackParameterName"));

	string tipname = nomparent;
	if (!tipname.empty()) {
		tipname += ": ";
	}
	tipname += _name;
	ARDOUR_UI::instance()->set_tip(controls_ebox, tipname);

	/* add the buttons */
	controls_table.attach (hide_button, 0, 1, 0, 1, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::EXPAND);

	controls_table.attach (auto_button, 5, 8, 0, 1, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::EXPAND);

	if (_controller) {
		/* add bar controller */
		controls_table.attach (*_controller.get(), 0, 8, 1, 2, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::EXPAND);
	}

	controls_table.show_all ();

	hide_button.signal_clicked().connect (sigc::mem_fun(*this, &AutomationTimeAxisView::hide_clicked));
	auto_button.signal_clicked().connect (sigc::mem_fun(*this, &AutomationTimeAxisView::auto_clicked));

	controls_base_selected_name = X_("AutomationTrackControlsBaseSelected");
	controls_base_unselected_name = X_("AutomationTrackControlsBase");
	controls_ebox.set_name (controls_base_unselected_name);

	XMLNode* xml_node = get_state_node ();

	if (xml_node) {
		set_state (*xml_node, Stateful::loading_state_version);
	}

	/* ask for notifications of any new RegionViews */
	if (show_regions) {

		assert(_view);
		_view->attach ();

	} else {
		/* no regions, just a single line for the entire track (e.g. bus gain) */

		assert (_control);

		boost::shared_ptr<AutomationLine> line (
			new AutomationLine (
				ARDOUR::EventTypeMap::instance().to_symbol(_parameter),
				*this,
				*_canvas_display,
				_control->alist()
				)
			);

		line->set_line_color (ARDOUR_UI::config()->canvasvar_ProcessorAutomationLine.get());
		line->queue_reset ();
		add_line (line);
	}

	/* make sure labels etc. are correct */

	automation_state_changed ();
	ColorsChanged.connect (sigc::mem_fun (*this, &AutomationTimeAxisView::color_handler));

	_route->DropReferences.connect (
		_route_connections, invalidator (*this), ui_bind (&AutomationTimeAxisView::route_going_away, this), gui_context ()
		);
}

AutomationTimeAxisView::~AutomationTimeAxisView ()
{
}

void
AutomationTimeAxisView::route_going_away ()
{
	_route.reset ();
}

void
AutomationTimeAxisView::auto_clicked ()
{
	using namespace Menu_Helpers;

	if (automation_menu == 0) {
		automation_menu = manage (new Menu);
		automation_menu->set_name ("ArdourContextMenu");
		MenuList& items (automation_menu->items());

		items.push_back (MenuElem (_("Manual"), sigc::bind (sigc::mem_fun(*this,
				&AutomationTimeAxisView::set_automation_state), (AutoState) Off)));
		items.push_back (MenuElem (_("Play"), sigc::bind (sigc::mem_fun(*this,
				&AutomationTimeAxisView::set_automation_state), (AutoState) Play)));
		items.push_back (MenuElem (_("Write"), sigc::bind (sigc::mem_fun(*this,
				&AutomationTimeAxisView::set_automation_state), (AutoState) Write)));
		items.push_back (MenuElem (_("Touch"), sigc::bind (sigc::mem_fun(*this,
				&AutomationTimeAxisView::set_automation_state), (AutoState) Touch)));
	}

	automation_menu->popup (1, gtk_get_current_event_time());
}

void
AutomationTimeAxisView::set_automation_state (AutoState state)
{
	if (ignore_state_request) {
		return;
	}

	if (_automatable) {
		_automatable->set_parameter_automation_state (_parameter, state);
	}

	if (_view) {
		_view->set_automation_state (state);

		/* AutomationStreamViews don't signal when their automation state changes, so handle
		   our updates `manually'.
		*/
		automation_state_changed ();
	}
}

void
AutomationTimeAxisView::automation_state_changed ()
{
	AutoState state;

	/* update button label */

	if (_view) {
		state = _view->automation_state ();
	} else if (_line) {
		assert (_control);
		state = _control->alist()->automation_state ();
	} else {
		state = Off;
	}

	switch (state & (Off|Play|Touch|Write)) {
	case Off:
		auto_button.set_label (_("Manual"));
		if (auto_off_item) {
			ignore_state_request = true;
			auto_off_item->set_active (true);
			auto_play_item->set_active (false);
			auto_touch_item->set_active (false);
			auto_write_item->set_active (false);
			ignore_state_request = false;
		}
		break;
	case Play:
		auto_button.set_label (_("Play"));
		if (auto_play_item) {
			ignore_state_request = true;
			auto_play_item->set_active (true);
			auto_off_item->set_active (false);
			auto_touch_item->set_active (false);
			auto_write_item->set_active (false);
			ignore_state_request = false;
		}
		break;
	case Write:
		auto_button.set_label (_("Write"));
		if (auto_write_item) {
			ignore_state_request = true;
			auto_write_item->set_active (true);
			auto_off_item->set_active (false);
			auto_play_item->set_active (false);
			auto_touch_item->set_active (false);
			ignore_state_request = false;
		}
		break;
	case Touch:
		auto_button.set_label (_("Touch"));
		if (auto_touch_item) {
			ignore_state_request = true;
			auto_touch_item->set_active (true);
			auto_off_item->set_active (false);
			auto_play_item->set_active (false);
			auto_write_item->set_active (false);
			ignore_state_request = false;
		}
		break;
	default:
		auto_button.set_label (_("???"));
		break;
	}
}

/** The interpolation style of our AutomationList has changed, so update */
void
AutomationTimeAxisView::interpolation_changed (AutomationList::InterpolationStyle s)
{
	if (mode_line_item && mode_discrete_item) {
		if (s == AutomationList::Discrete) {
			mode_discrete_item->set_active(true);
			mode_line_item->set_active(false);
		} else {
			mode_line_item->set_active(true);
			mode_discrete_item->set_active(false);
		}
	}
}

/** A menu item has been selected to change our interpolation mode */
void
AutomationTimeAxisView::set_interpolation (AutomationList::InterpolationStyle style)
{
	/* Tell our view's list, if we have one, otherwise tell our own.
	 * Everything else will be signalled back from that.
	 */

	if (_view) {
		_view->set_interpolation (style);
	} else {
		assert (_control);
		_control->list()->set_interpolation (style);
	}
}

void
AutomationTimeAxisView::clear_clicked ()
{
	assert (_line || _view);

	_session->begin_reversible_command (_("clear automation"));

	if (_line) {
		_line->clear ();
	} else if (_view) {
		_view->clear ();
	}

	_session->commit_reversible_command ();
	_session->set_dirty ();
}

void
AutomationTimeAxisView::set_height (uint32_t h)
{
	bool const changed = (height != (uint32_t) h) || first_call_to_set_height;
	uint32_t const normal = preset_height (HeightNormal);
	bool const changed_between_small_and_normal = ( (height < normal && h >= normal) || (height >= normal || h < normal) );

	TimeAxisView* state_parent = get_parent_with_state ();
	assert(state_parent);
	XMLNode* xml_node = 0;

	if (_control) {
		xml_node = _control->extra_xml ("GUI");
	} else {
		/* XXX we need somewhere to store GUI info for per-region
		 * automation 
		 */
	}

	TimeAxisView::set_height (h);
	_base_rect->set_y1 (h);

	if (_line) {
		_line->set_height(h);
	}

	if (_view) {
		_view->set_height(h);
		_view->update_contents_height();
	}

	char buf[32];
	snprintf (buf, sizeof (buf), "%u", height);
	if (xml_node) {
		xml_node->add_property ("height", buf);
	}

	if (changed_between_small_and_normal || first_call_to_set_height) {

		first_call_to_set_height = false;

		if (h >= preset_height (HeightNormal)) {
			hide_name_entry ();
			show_name_label ();
			name_hbox.show_all ();

			auto_button.show();
			hide_button.show_all();

		} else if (h >= preset_height (HeightSmall)) {
			controls_table.hide_all ();
			hide_name_entry ();
			show_name_label ();
			name_hbox.show_all ();

			auto_button.hide();
			hide_button.hide();
		}
	} else if (h >= preset_height (HeightNormal)) {
		cerr << "track grown, but neither changed_between_small_and_normal nor first_call_to_set_height set!" << endl;
	}

	if (changed) {
		if (_canvas_display->visible () && _route) {
			/* only emit the signal if the height really changed and we were visible */
			_route->gui_changed ("visible_tracks", (void *) 0); /* EMIT_SIGNAL */
		}
	}
}

void
AutomationTimeAxisView::set_frames_per_pixel (double fpp)
{
	TimeAxisView::set_frames_per_pixel (fpp);

	if (_line) {
		_line->reset ();
	}

	if (_view) {
		_view->set_frames_per_pixel (fpp);
	}
}

void
AutomationTimeAxisView::hide_clicked ()
{
	// LAME fix for refreshing the hide button
	hide_button.set_sensitive(false);

	set_marked_for_display (false);
	hide ();

	hide_button.set_sensitive(true);
}

void
AutomationTimeAxisView::build_display_menu ()
{
	using namespace Menu_Helpers;

	/* prepare it */

	TimeAxisView::build_display_menu ();

	/* now fill it with our stuff */

	MenuList& items = display_menu->items();

	items.push_back (MenuElem (_("Hide"), sigc::mem_fun(*this, &AutomationTimeAxisView::hide_clicked)));
	items.push_back (SeparatorElem());
	items.push_back (MenuElem (_("Clear"), sigc::mem_fun(*this, &AutomationTimeAxisView::clear_clicked)));
	items.push_back (SeparatorElem());

	/* state menu */

	Menu* auto_state_menu = manage (new Menu);
	auto_state_menu->set_name ("ArdourContextMenu");
	MenuList& as_items = auto_state_menu->items();

	as_items.push_back (CheckMenuElem (_("Manual"), sigc::bind (
			sigc::mem_fun(*this, &AutomationTimeAxisView::set_automation_state),
			(AutoState) Off)));
	auto_off_item = dynamic_cast<CheckMenuItem*>(&as_items.back());

	as_items.push_back (CheckMenuElem (_("Play"), sigc::bind (
			sigc::mem_fun(*this, &AutomationTimeAxisView::set_automation_state),
			(AutoState) Play)));
	auto_play_item = dynamic_cast<CheckMenuItem*>(&as_items.back());

	as_items.push_back (CheckMenuElem (_("Write"), sigc::bind (
			sigc::mem_fun(*this, &AutomationTimeAxisView::set_automation_state),
			(AutoState) Write)));
	auto_write_item = dynamic_cast<CheckMenuItem*>(&as_items.back());

	as_items.push_back (CheckMenuElem (_("Touch"), sigc::bind (
			sigc::mem_fun(*this, &AutomationTimeAxisView::set_automation_state),
			(AutoState) Touch)));
	auto_touch_item = dynamic_cast<CheckMenuItem*>(&as_items.back());

	items.push_back (MenuElem (_("State"), *auto_state_menu));

	/* mode menu */

	/* current interpolation state */
	AutomationList::InterpolationStyle const s = _view ? _view->interpolation() : _control->list()->interpolation ();

	if (EventTypeMap::instance().is_midi_parameter(_parameter)) {

		Menu* auto_mode_menu = manage (new Menu);
		auto_mode_menu->set_name ("ArdourContextMenu");
		MenuList& am_items = auto_mode_menu->items();

		RadioMenuItem::Group group;

		am_items.push_back (RadioMenuElem (group, _("Discrete"), sigc::bind (
				sigc::mem_fun(*this, &AutomationTimeAxisView::set_interpolation),
				AutomationList::Discrete)));
		mode_discrete_item = dynamic_cast<CheckMenuItem*>(&am_items.back());
		mode_discrete_item->set_active (s == AutomationList::Discrete);

		am_items.push_back (RadioMenuElem (group, _("Linear"), sigc::bind (
				sigc::mem_fun(*this, &AutomationTimeAxisView::set_interpolation),
				AutomationList::Linear)));
		mode_line_item = dynamic_cast<CheckMenuItem*>(&am_items.back());
		mode_line_item->set_active (s == AutomationList::Linear);

		items.push_back (MenuElem (_("Mode"), *auto_mode_menu));
	}

	/* make sure the automation menu state is correct */

	automation_state_changed ();
	interpolation_changed (s);
}

void
AutomationTimeAxisView::add_automation_event (Canvas::Item* /*item*/, GdkEvent* /*event*/, framepos_t when, double y)
{
	if (!_line) {
		return;
	}

	double x = 0;

	_canvas_display->canvas_to_item (x, y);

	/* compute vertical fractional position */

	y = 1.0 - (y / height);

	/* map using line */

	_line->view_to_model_coord (x, y);

	boost::shared_ptr<AutomationList> list = _line->the_list ();

	_session->begin_reversible_command (_("add automation event"));
	XMLNode& before = list->get_state();

	list->add (when, y);

	XMLNode& after = list->get_state();
	_session->commit_reversible_command (new MementoCommand<ARDOUR::AutomationList> (*list, &before, &after));
	_session->set_dirty ();
}

void
AutomationTimeAxisView::cut_copy_clear (Selection& selection, CutCopyOp op)
{
	list<boost::shared_ptr<AutomationLine> > lines;
	if (_line) {
		lines.push_back (_line);
	} else if (_view) {
		lines = _view->get_lines ();
	}

	for (list<boost::shared_ptr<AutomationLine> >::iterator i = lines.begin(); i != lines.end(); ++i) {
		cut_copy_clear_one (**i, selection, op);
	}
}

void
AutomationTimeAxisView::cut_copy_clear_one (AutomationLine& line, Selection& selection, CutCopyOp op)
{
	boost::shared_ptr<Evoral::ControlList> what_we_got;
	boost::shared_ptr<AutomationList> alist (line.the_list());

	XMLNode &before = alist->get_state();

	/* convert time selection to automation list model coordinates */
	const Evoral::TimeConverter<double, ARDOUR::framepos_t>& tc = line.time_converter ();
	double const start = tc.from (selection.time.front().start - tc.origin_b ());
	double const end = tc.from (selection.time.front().end - tc.origin_b ());

	switch (op) {
	case Delete:
		if (alist->cut (start, end) != 0) {
			_session->add_command(new MementoCommand<AutomationList>(*alist.get(), &before, &alist->get_state()));
		}
		break;

	case Cut:

		if ((what_we_got = alist->cut (start, end)) != 0) {
			_editor.get_cut_buffer().add (what_we_got);
			_session->add_command(new MementoCommand<AutomationList>(*alist.get(), &before, &alist->get_state()));
		}
		break;
	case Copy:
		if ((what_we_got = alist->copy (start, end)) != 0) {
			_editor.get_cut_buffer().add (what_we_got);
		}
		break;

	case Clear:
		if ((what_we_got = alist->cut (start, end)) != 0) {
			_session->add_command(new MementoCommand<AutomationList>(*alist.get(), &before, &alist->get_state()));
		}
		break;
	}

	if (what_we_got) {
		for (AutomationList::iterator x = what_we_got->begin(); x != what_we_got->end(); ++x) {
			double when = (*x)->when;
			double val  = (*x)->value;
			line.model_to_view_coord (when, val);
			(*x)->when = when;
			(*x)->value = val;
		}
	}
}

void
AutomationTimeAxisView::reset_objects (PointSelection& selection)
{
	list<boost::shared_ptr<AutomationLine> > lines;
	if (_line) {
		lines.push_back (_line);
	} else if (_view) {
		lines = _view->get_lines ();
	}

	for (list<boost::shared_ptr<AutomationLine> >::iterator i = lines.begin(); i != lines.end(); ++i) {
		reset_objects_one (**i, selection);
	}
}

void
AutomationTimeAxisView::reset_objects_one (AutomationLine& line, PointSelection& selection)
{
	boost::shared_ptr<AutomationList> alist(line.the_list());

	_session->add_command (new MementoCommand<AutomationList>(*alist.get(), &alist->get_state(), 0));

	for (PointSelection::iterator i = selection.begin(); i != selection.end(); ++i) {

		if ((*i).track != this) {
			continue;
		}

		alist->reset_range ((*i).start, (*i).end);
	}
}

void
AutomationTimeAxisView::cut_copy_clear_objects (PointSelection& selection, CutCopyOp op)
{
	list<boost::shared_ptr<AutomationLine> > lines;
	if (_line) {
		lines.push_back (_line);
	} else if (_view) {
		lines = _view->get_lines ();
	}

	for (list<boost::shared_ptr<AutomationLine> >::iterator i = lines.begin(); i != lines.end(); ++i) {
		cut_copy_clear_objects_one (**i, selection, op);
	}
}

void
AutomationTimeAxisView::cut_copy_clear_objects_one (AutomationLine& line, PointSelection& selection, CutCopyOp op)
{
	boost::shared_ptr<Evoral::ControlList> what_we_got;
	boost::shared_ptr<AutomationList> alist(line.the_list());

	XMLNode &before = alist->get_state();

	for (PointSelection::iterator i = selection.begin(); i != selection.end(); ++i) {

		if ((*i).track != this) {
			continue;
		}

		switch (op) {
		case Delete:
			if (alist->cut ((*i).start, (*i).end) != 0) {
				_session->add_command (new MementoCommand<AutomationList>(*alist.get(), new XMLNode (before), &alist->get_state()));
			}
			break;
		case Cut:
			if ((what_we_got = alist->cut ((*i).start, (*i).end)) != 0) {
				_editor.get_cut_buffer().add (what_we_got);
				_session->add_command (new MementoCommand<AutomationList>(*alist.get(), new XMLNode (before), &alist->get_state()));
			}
			break;
		case Copy:
			if ((what_we_got = alist->copy ((*i).start, (*i).end)) != 0) {
				_editor.get_cut_buffer().add (what_we_got);
			}
			break;

		case Clear:
			if ((what_we_got = alist->cut ((*i).start, (*i).end)) != 0) {
				_session->add_command (new MementoCommand<AutomationList>(*alist.get(), new XMLNode (before), &alist->get_state()));
			}
			break;
		}
	}

	delete &before;

	if (what_we_got) {
		for (AutomationList::iterator x = what_we_got->begin(); x != what_we_got->end(); ++x) {
			double when = (*x)->when;
			double val  = (*x)->value;
			line.model_to_view_coord (when, val);
			(*x)->when = when;
			(*x)->value = val;
		}
	}
}

/** Paste a selection.
 *  @param pos Position to paste to (session frames).
 *  @param times Number of times to paste.
 *  @param selection Selection to paste.
 *  @param nth Index of the AutomationList within the selection to paste from.
 */
bool
AutomationTimeAxisView::paste (framepos_t pos, float times, Selection& selection, size_t nth)
{
	boost::shared_ptr<AutomationLine> line;

	if (_line) {
		line = _line;
	} else if (_view) {
		line = _view->paste_line (pos);
	}

	if (!line) {
		return false;
	}

	return paste_one (*line, pos, times, selection, nth);
}

bool
AutomationTimeAxisView::paste_one (AutomationLine& line, framepos_t pos, float times, Selection& selection, size_t nth)
{
	AutomationSelection::iterator p;
	boost::shared_ptr<AutomationList> alist(line.the_list());

	for (p = selection.lines.begin(); p != selection.lines.end() && nth; ++p, --nth) {}

	if (p == selection.lines.end()) {
		return false;
	}

	/* Make a copy of the list because we have to scale the
	   values from view coordinates to model coordinates, and we're
	   not supposed to modify the points in the selection.
	*/

	AutomationList copy (**p);

	for (AutomationList::iterator x = copy.begin(); x != copy.end(); ++x) {
		double when = (*x)->when;
		double val  = (*x)->value;
		line.view_to_model_coord (when, val);
		(*x)->when = when;
		(*x)->value = val;
	}

	double const model_pos = line.time_converter().from (pos - line.time_converter().origin_b ());

	XMLNode &before = alist->get_state();
	alist->paste (copy, model_pos, times);
	_session->add_command (new MementoCommand<AutomationList>(*alist.get(), &before, &alist->get_state()));

	return true;
}

void
AutomationTimeAxisView::get_selectables (framepos_t start, framepos_t end, double top, double bot, list<Selectable*>& results)
{
	if (!_line && !_view) {
		return;
	}

	if (touched (top, bot)) {

		/* remember: this is X Window - coordinate space starts in upper left and moves down.
		   _y_position is the "origin" or "top" of the track.
		*/

		/* bottom of our track */
		double const mybot = _y_position + height;

		double topfrac;
		double botfrac;

		if (_y_position >= top && mybot <= bot) {

			/* _y_position is below top, mybot is above bot, so we're fully
			   covered vertically.
			*/

			topfrac = 1.0;
			botfrac = 0.0;

		} else {

			/* top and bot are within _y_position .. mybot */

			topfrac = 1.0 - ((top - _y_position) / height);
			botfrac = 1.0 - ((bot - _y_position) / height);

		}

		if (_line) {
			_line->get_selectables (start, end, botfrac, topfrac, results);
		} else if (_view) {
			_view->get_selectables (start, end, botfrac, topfrac, results);
		}
	}
}

void
AutomationTimeAxisView::get_inverted_selectables (Selection& sel, list<Selectable*>& result)
{
	if (_line) {
		_line->get_inverted_selectables (sel, result);
	}
}

void
AutomationTimeAxisView::set_selected_points (PointSelection& points)
{
	if (_line) {
		_line->set_selected_points (points);
	} else if (_view) {
		_view->set_selected_points (points);
	}
}

void
AutomationTimeAxisView::clear_lines ()
{
	_line.reset();
	_list_connections.drop_connections ();
}

void
AutomationTimeAxisView::add_line (boost::shared_ptr<AutomationLine> line)
{
	assert(line);
	assert(!_line);
	if (_control) {
		assert(line->the_list() == _control->list());
		
		_control->alist()->automation_state_changed.connect (
			_list_connections, invalidator (*this), boost::bind (&AutomationTimeAxisView::automation_state_changed, this), gui_context()
			);
		
		_control->alist()->InterpolationChanged.connect (
			_list_connections, invalidator (*this), boost::bind (&AutomationTimeAxisView::interpolation_changed, this, _1), gui_context()
			);
	}

	_line = line;

	line->set_height (height);

	/* pick up the current state */
	automation_state_changed ();

	line->show();
}

void
AutomationTimeAxisView::entered()
{
	if (_line) {
		_line->track_entered();
	}
}

void
AutomationTimeAxisView::exited ()
{
	if (_line) {
		_line->track_exited();
	}
}

void
AutomationTimeAxisView::color_handler ()
{
	if (_line) {
		_line->set_colors();
	}
}

int
AutomationTimeAxisView::set_state (const XMLNode& node, int version)
{
	TimeAxisView::set_state (node, version);

	if (version < 3000) {
		return set_state_2X (node, version);
	}

	XMLProperty const * prop = node.property ("shown");

	if (prop) {
		set_visibility (string_is_affirmative (prop->value()));
	} else {
		set_visibility (false);
	}

	return 0;
}

int
AutomationTimeAxisView::set_state_2X (const XMLNode& node, int /*version*/)
{
	if (node.name() == X_("gain") && _parameter == Evoral::Parameter (GainAutomation)) {
		XMLProperty const * shown = node.property (X_("shown"));
		if (shown && string_is_affirmative (shown->value ())) {
			set_marked_for_display (true);
			_canvas_display->show (); /* FIXME: necessary? show_at? */
		}
	}

	if (!_marked_for_display) {
		hide ();
	}

	return 0;
}

XMLNode*
AutomationTimeAxisView::get_state_node ()
{
	if (_control) {
		return _control->extra_xml ("GUI", true);
	}
	return 0;
}

void
AutomationTimeAxisView::update_extra_xml_shown (bool shown)
{
	XMLNode* xml_node = get_state_node();
	if (xml_node) {
		xml_node->add_property ("shown", shown ? "yes" : "no");
	}
}

void
AutomationTimeAxisView::what_has_visible_automation (const boost::shared_ptr<Automatable>& automatable, set<Evoral::Parameter>& visible)
{
	/* this keeps "knowledge" of how we store visibility information
	   in XML private to this class.
	*/

	assert (automatable);

	Automatable::Controls& controls (automatable->controls());
	
	for (Automatable::Controls::iterator i = controls.begin(); i != controls.end(); ++i) {
		
		boost::shared_ptr<AutomationControl> ac = boost::dynamic_pointer_cast<AutomationControl> (i->second);

		if (ac) {
			
			const XMLNode* gui_node = ac->extra_xml ("GUI");
			
			if (gui_node) {
				const XMLProperty* prop = gui_node->property ("shown");
				if (prop) {
					if (string_is_affirmative (prop->value())) {
						visible.insert (i->first);
					}
				}
			}
		}
	}
}

guint32
AutomationTimeAxisView::show_at (double y, int& nth, Gtk::VBox *parent)
{
	if (!canvas_item_visible (_canvas_display)) {
		update_extra_xml_shown (true);
	}

	return TimeAxisView::show_at (y, nth, parent);
}

void
AutomationTimeAxisView::show ()
{
	if (!canvas_item_visible (_canvas_display)) {
		update_extra_xml_shown (true);
	}

	return TimeAxisView::show ();
}

void
AutomationTimeAxisView::hide ()
{
	if (canvas_item_visible (_canvas_display)) {
		update_extra_xml_shown (false);
	}

	TimeAxisView::hide ();
}

/** @return true if this view has any automation data to display */
bool
AutomationTimeAxisView::has_automation () const
{
	return ( (_line && _line->npoints() > 0) || (_view && _view->has_automation()) );
}

list<boost::shared_ptr<AutomationLine> >
AutomationTimeAxisView::lines () const
{
	list<boost::shared_ptr<AutomationLine> > lines;

	if (_line) {
		lines.push_back (_line);
	} else if (_view) {
		lines = _view->get_lines ();
	}

	return lines;
}
