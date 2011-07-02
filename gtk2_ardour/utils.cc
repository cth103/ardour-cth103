/*
    Copyright (C) 2003 Paul Davis

    This program is free software; you an redistribute it and/or modify
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

#ifdef WAF_BUILD
#include "gtk2ardour-config.h"
#endif

#include <pango/pangoft2.h> // for fontmap resolution control for GnomeCanvas
#include <pango/pangocairo.h> // for fontmap resolution control for GnomeCanvas

#include <cstdlib>
#include <cctype>
#include <fstream>
#include <sys/stat.h>
#include <gtkmm/rc.h>
#include <gtkmm/window.h>
#include <gtkmm/combo.h>
#include <gtkmm/label.h>
#include <gtkmm/paned.h>
#include <gtk/gtkpaned.h>

#include "pbd/file_utils.h"

#include <gtkmm2ext/utils.h>
#include "ardour/configuration.h"
#include "ardour/rc_configuration.h"
#include "ardour/filesystem_paths.h"
#include "canvas/item.h"

#include "ardour_ui.h"
#include "debug.h"
#include "public_editor.h"
#include "keyboard.h"
#include "utils.h"
#include "i18n.h"
#include "rgb_macros.h"
#include "gui_thread.h"

using namespace std;
using namespace Gtk;
using namespace Glib;
using namespace PBD;
using Gtkmm2ext::Keyboard;

sigc::signal<void>  DPIReset;

int
pixel_width (const string& str, Pango::FontDescription& font)
{
	Label foo;
	Glib::RefPtr<Pango::Layout> layout = foo.create_pango_layout ("");

	layout->set_font_description (font);
	layout->set_text (str);

	int width, height;
	Gtkmm2ext::get_ink_pixel_size (layout, width, height);
	return width;
}

string
fit_to_pixels (const string& str, int pixel_width, Pango::FontDescription& font, int& actual_width, bool with_ellipses)
{
	Label foo;
	Glib::RefPtr<Pango::Layout> layout = foo.create_pango_layout ("");
	string::size_type shorter_by = 0;
	string txt;

	layout->set_font_description (font);

	actual_width = 0;

	string ustr = str;
	string::iterator last = ustr.end();
	--last; /* now points at final entry */

	txt = ustr;

	while (!ustr.empty()) {

		layout->set_text (txt);

		int width, height;
		Gtkmm2ext::get_ink_pixel_size (layout, width, height);

		if (width < pixel_width) {
			actual_width = width;
			break;
		}

		ustr.erase (last--);
		shorter_by++;

		if (with_ellipses && shorter_by > 3) {
			txt = ustr;
			txt += "...";
		} else {
			txt = ustr;
		}
	}

	return txt;
}

/** Try to fit a string into a given horizontal space by ellipsizing it.
 *  @param cr Cairo context in which the text will be plotted.
 *  @param name Text.
 *  @param avail Available horizontal space.
 *  @return (Text, possibly ellipsized) and (horizontal size of text)
 */

std::pair<std::string, double>
fit_to_pixels (cairo_t* cr, std::string name, double avail)
{
	/* XXX hopefully there exists a more efficient way of doing this */

	bool abbreviated = false;
	uint32_t width = 0;

	while (1) {
		cairo_text_extents_t ext;
		cairo_text_extents (cr, name.c_str(), &ext);

		if (ext.width < avail || name.length() <= 4) {
			width = ext.width;
			break;
		}

		if (abbreviated) {
			name = name.substr (0, name.length() - 4) + "...";
		} else {
			name = name.substr (0, name.length() - 3) + "...";
			abbreviated = true;
		}
	}

	return std::make_pair (name, width);
}


/** Add an element to a menu, settings its sensitivity.
 * @param m Menu to add to.
 * @param e Element to add.
 * @param s true to make sensitive, false to make insensitive
 */
void
add_item_with_sensitivity (Menu_Helpers::MenuList& m, Menu_Helpers::MenuElem e, bool s)
{
	m.push_back (e);
	if (!s) {
		m.back().set_sensitive (false);
	}
}


gint
just_hide_it (GdkEventAny */*ev*/, Gtk::Window *win)
{
	win->hide ();
	return 0;
}

/* xpm2rgb copied from nixieclock, which bore the legend:

    nixieclock - a nixie desktop timepiece
    Copyright (C) 2000 Greg Ercolano, erco@3dsite.com

    and was released under the GPL.
*/

unsigned char*
xpm2rgb (const char** xpm, uint32_t& w, uint32_t& h)
{
	static long vals[256], val;
	uint32_t t, x, y, colors, cpp;
	unsigned char c;
	unsigned char *savergb, *rgb;

	// PARSE HEADER

	if ( sscanf(xpm[0], "%u%u%u%u", &w, &h, &colors, &cpp) != 4 ) {
		error << string_compose (_("bad XPM header %1"), xpm[0])
		      << endmsg;
		return 0;
	}

	savergb = rgb = (unsigned char*) malloc (h * w * 3);

	// LOAD XPM COLORMAP LONG ENOUGH TO DO CONVERSION
	for (t = 0; t < colors; ++t) {
		sscanf (xpm[t+1], "%c c #%lx", &c, &val);
		vals[c] = val;
	}

	// COLORMAP -> RGB CONVERSION
	//    Get low 3 bytes from vals[]
	//

	const char *p;
	for (y = h-1; y > 0; --y) {

		for (p = xpm[1+colors+(h-y-1)], x = 0; x < w; x++, rgb += 3) {
			val = vals[(int)*p++];
			*(rgb+2) = val & 0xff; val >>= 8;  // 2:B
			*(rgb+1) = val & 0xff; val >>= 8;  // 1:G
			*(rgb+0) = val & 0xff;             // 0:R
		}
	}

	return (savergb);
}

unsigned char*
xpm2rgba (const char** xpm, uint32_t& w, uint32_t& h)
{
	static long vals[256], val;
	uint32_t t, x, y, colors, cpp;
	unsigned char c;
	unsigned char *savergb, *rgb;
	char transparent;

	// PARSE HEADER

	if ( sscanf(xpm[0], "%u%u%u%u", &w, &h, &colors, &cpp) != 4 ) {
		error << string_compose (_("bad XPM header %1"), xpm[0])
		      << endmsg;
		return 0;
	}

	savergb = rgb = (unsigned char*) malloc (h * w * 4);

	// LOAD XPM COLORMAP LONG ENOUGH TO DO CONVERSION

	if (strstr (xpm[1], "None")) {
		sscanf (xpm[1], "%c", &transparent);
		t = 1;
	} else {
		transparent = 0;
		t = 0;
	}

	for (; t < colors; ++t) {
		sscanf (xpm[t+1], "%c c #%lx", &c, &val);
		vals[c] = val;
	}

	// COLORMAP -> RGB CONVERSION
	//    Get low 3 bytes from vals[]
	//

	const char *p;
	for (y = h-1; y > 0; --y) {

		char alpha;

		for (p = xpm[1+colors+(h-y-1)], x = 0; x < w; x++, rgb += 4) {

			if (transparent && (*p++ == transparent)) {
				alpha = 0;
				val = 0;
			} else {
				alpha = 255;
				val = vals[(int)*p];
			}

			*(rgb+3) = alpha;                  // 3: alpha
			*(rgb+2) = val & 0xff; val >>= 8;  // 2:B
			*(rgb+1) = val & 0xff; val >>= 8;  // 1:G
			*(rgb+0) = val & 0xff;             // 0:R
		}
	}

	return (savergb);
}

Canvas::Points*
get_canvas_points (string /*who*/, uint32_t npoints)
{
	// cerr << who << ": wants " << npoints << " canvas points" << endl;
#ifdef TRAP_EXCESSIVE_POINT_REQUESTS
	if (npoints > (uint32_t) gdk_screen_width() + 4) {
		abort ();
	}
#endif
	return new Canvas::Points (npoints);
}

Pango::FontDescription
get_font_for_style (string widgetname)
{
	Gtk::Window window (WINDOW_TOPLEVEL);
	Gtk::Label foobar;
	Glib::RefPtr<Gtk::Style> style;

	window.add (foobar);
	foobar.set_name (widgetname);
	foobar.ensure_style();

	style = foobar.get_style ();

	Glib::RefPtr<const Pango::Layout> layout = foobar.get_layout();

	PangoFontDescription *pfd = (PangoFontDescription *)pango_layout_get_font_description((PangoLayout *)layout->gobj());

	if (!pfd) {

		/* layout inherited its font description from a PangoContext */

		PangoContext* ctxt = (PangoContext*) pango_layout_get_context ((PangoLayout*) layout->gobj());
		pfd =  pango_context_get_font_description (ctxt);
		return Pango::FontDescription (pfd); /* make a copy */
	}

	return Pango::FontDescription (pfd); /* make a copy */
}

uint32_t
rgba_from_style (string style, uint32_t r, uint32_t g, uint32_t b, uint32_t a, string attr, int state, bool rgba)
{
	/* In GTK+2, styles aren't set up correctly if the widget is not
	   attached to a toplevel window that has a screen pointer.
	*/

	static Gtk::Window* window = 0;

	if (window == 0) {
		window = new Window (WINDOW_TOPLEVEL);
	}

	Gtk::Label foo;

	window->add (foo);

	foo.set_name (style);
	foo.ensure_style ();

	GtkRcStyle* rc = foo.get_style()->gobj()->rc_style;

	if (rc) {
		if (attr == "fg") {
			r = rc->fg[state].red / 257;
			g = rc->fg[state].green / 257;
			b = rc->fg[state].blue / 257;

			/* what a hack ... "a" is for "active" */
			if (state == Gtk::STATE_NORMAL && rgba) {
				a = rc->fg[GTK_STATE_ACTIVE].red / 257;
			}
		} else if (attr == "bg") {
			r = g = b = 0;
			r = rc->bg[state].red / 257;
			g = rc->bg[state].green / 257;
			b = rc->bg[state].blue / 257;
		} else if (attr == "base") {
			r = rc->base[state].red / 257;
			g = rc->base[state].green / 257;
			b = rc->base[state].blue / 257;
		} else if (attr == "text") {
			r = rc->text[state].red / 257;
			g = rc->text[state].green / 257;
			b = rc->text[state].blue / 257;
		}
	} else {
		warning << string_compose (_("missing RGBA style for \"%1\""), style) << endl;
	}

	window->remove ();

	if (state == Gtk::STATE_NORMAL && rgba) {
		return (uint32_t) RGBA_TO_UINT(r,g,b,a);
	} else {
		return (uint32_t) RGB_TO_UINT(r,g,b);
	}
}


Gdk::Color
color_from_style (string widget_style_name, int state, string attr)
{
	GtkStyle* style;

	style = gtk_rc_get_style_by_paths (gtk_settings_get_default(),
					   widget_style_name.c_str(),
					   0, G_TYPE_NONE);

	if (!style) {
		error << string_compose (_("no style found for %1, using red"), style) << endmsg;
		return Gdk::Color ("red");
	}

	if (attr == "fg") {
		return Gdk::Color (&style->fg[state]);
	}

	if (attr == "bg") {
		return Gdk::Color (&style->bg[state]);
	}

	if (attr == "light") {
		return Gdk::Color (&style->light[state]);
	}

	if (attr == "dark") {
		return Gdk::Color (&style->dark[state]);
	}

	if (attr == "mid") {
		return Gdk::Color (&style->mid[state]);
	}

	if (attr == "text") {
		return Gdk::Color (&style->text[state]);
	}

	if (attr == "base") {
		return Gdk::Color (&style->base[state]);
	}

	if (attr == "text_aa") {
		return Gdk::Color (&style->text_aa[state]);
	}

	error << string_compose (_("unknown style attribute %1 requested for color; using \"red\""), attr) << endmsg;
	return Gdk::Color ("red");
}

Glib::RefPtr<Gdk::GC>
gc_from_style (string widget_style_name, int state, string attr)
{
        GtkStyle* style;

        style = gtk_rc_get_style_by_paths (gtk_settings_get_default(),
                                           widget_style_name.c_str(),
                                           0, G_TYPE_NONE);

        if (!style) {
                error << string_compose (_("no style found for %1, using red"), style) << endmsg;
		Glib::RefPtr<Gdk::GC> ret = Gdk::GC::create();
		ret->set_rgb_fg_color(Gdk::Color("red"));
                return ret;
        }

        if (attr == "fg") {
                return Glib::wrap(style->fg_gc[state]);
        }

        if (attr == "bg") {
                return Glib::wrap(style->bg_gc[state]);
        }

        if (attr == "light") {
                return Glib::wrap(style->light_gc[state]);
        }

        if (attr == "dark") {
                return Glib::wrap(style->dark_gc[state]);
        }

        if (attr == "mid") {
                return Glib::wrap(style->mid_gc[state]);
        }

        if (attr == "text") {
                return Glib::wrap(style->text_gc[state]);
        }

        if (attr == "base") {
                return Glib::wrap(style->base_gc[state]);
        }

        if (attr == "text_aa") {
                return Glib::wrap(style->text_aa_gc[state]);
        }

        error << string_compose (_("unknown style attribute %1 requested for color; using \"red\""), attr) << endmsg;
	Glib::RefPtr<Gdk::GC> ret = Gdk::GC::create();
	ret->set_rgb_fg_color(Gdk::Color("red"));
        return ret;
}

void
set_color (Gdk::Color& c, int rgb)
{
	c.set_rgb((rgb >> 16)*256, ((rgb & 0xff00) >> 8)*256, (rgb & 0xff)*256);
}

bool
relay_key_press (GdkEventKey* ev, Gtk::Window* win)
{
	if (!key_press_focus_accelerator_handler (*win, ev)) {
		return PublicEditor::instance().on_key_press_event(ev);
	} else {
		return true;
	}
}

bool
forward_key_press (GdkEventKey* ev)
{
        return PublicEditor::instance().on_key_press_event(ev);
}

#ifdef GTKOSX
static guint
osx_keyval_without_alt (guint accent_keyval)
{
	switch (accent_keyval) {
	case GDK_oe:
		return GDK_q;
	case GDK_registered:
		return GDK_r;
	case GDK_dagger:
		return GDK_t;
	case GDK_yen:
		return GDK_y;
	case GDK_diaeresis:
		return GDK_u;
	case GDK_oslash:
		return GDK_o;
	case GDK_Greek_pi:
		return GDK_p;
	case GDK_leftdoublequotemark:
		return GDK_bracketleft;
	case GDK_leftsinglequotemark:
		return GDK_bracketright;
	case GDK_guillemotleft:
		return GDK_backslash;
	case GDK_aring:
		return GDK_a;
	case GDK_ssharp:
		return GDK_s;
	case GDK_partialderivative:
		return GDK_d;
	case GDK_function:
		return GDK_f;
	case GDK_copyright:
		return GDK_g;
	case GDK_abovedot:
		return GDK_h;
	case GDK_notsign:
		return GDK_l;
	case GDK_ellipsis:
		return GDK_semicolon;
	case GDK_ae:
		return GDK_apostrophe;
	case GDK_Greek_OMEGA:
		return GDK_z;
	case GDK_ccedilla:
		return GDK_c;
	case GDK_radical:
		return GDK_v;
	case GDK_integral:
		return GDK_b;
	case GDK_mu:
		return GDK_m;
	case GDK_lessthanequal:
		return GDK_comma;
	case GDK_greaterthanequal:
		return GDK_period;
	case GDK_division:
		return GDK_slash;
	default:
		break;
	}

	return GDK_VoidSymbol;
}
#endif

bool
key_press_focus_accelerator_handler (Gtk::Window& window, GdkEventKey* ev)
{
	GtkWindow* win = window.gobj();
	GtkWidget* focus = gtk_window_get_focus (win);
	bool special_handling_of_unmodified_accelerators = false;
	bool allow_activating = true;

	if (focus) {
		if (GTK_IS_ENTRY(focus) || Keyboard::some_magic_widget_has_focus()) {
			special_handling_of_unmodified_accelerators = true;
		}
	}

#ifdef GTKOSX
	/* should this be universally true? */
	if (Keyboard::some_magic_widget_has_focus ()) {
		allow_activating = false;
	}
#endif


        DEBUG_TRACE (DEBUG::Accelerators, string_compose ("Win = %1 Key event: code = %2  state = %3 special handling ? %4 magic widget focus ? %5 allow_activation ? %6\n",
                                                          win,
                                                          ev->keyval,
                                                          ev->state,
                                                          special_handling_of_unmodified_accelerators,
                                                          Keyboard::some_magic_widget_has_focus(),
                                                          allow_activating));

	/* This exists to allow us to override the way GTK handles
	   key events. The normal sequence is:

	   a) event is delivered to a GtkWindow
	   b) accelerators/mnemonics are activated
	   c) if (b) didn't handle the event, propagate to
	       the focus widget and/or focus chain

	   The problem with this is that if the accelerators include
	   keys without modifiers, such as the space bar or the
	   letter "e", then pressing the key while typing into
	   a text entry widget results in the accelerator being
	   activated, instead of the desired letter appearing
	   in the text entry.

	   There is no good way of fixing this, but this
	   represents a compromise. The idea is that
	   key events involving modifiers (not Shift)
	   get routed into the activation pathway first, then
	   get propagated to the focus widget if necessary.

	   If the key event doesn't involve modifiers,
	   we deliver to the focus widget first, thus allowing
	   it to get "normal text" without interference
	   from acceleration.

	   Of course, this can also be problematic: if there
	   is a widget with focus, then it will swallow
	   all "normal text" accelerators.
	*/

#ifdef GTKOSX
	if (!special_handling_of_unmodified_accelerators) {
		if (ev->state & GDK_MOD1_MASK) {
			/* we're not in a text entry or "magic focus" widget so we don't want OS X "special-character"
			   text-style handling of alt-<key>. change the keyval back to what it would be without
			   the alt key. this way, we see <alt>-v rather than <alt>-radical and so on.
			*/
			guint keyval_without_alt = osx_keyval_without_alt (ev->keyval);

			if (keyval_without_alt != GDK_VoidSymbol) {
                                DEBUG_TRACE (DEBUG::Accelerators, string_compose ("Remapped %1 to %2\n", gdk_keyval_name (ev->keyval), gdk_keyval_name (keyval_without_alt)));
				ev->keyval = keyval_without_alt;
			}
		}
	}
#endif

	if (!special_handling_of_unmodified_accelerators) {

		/* pretend that certain key events that GTK does not allow
		   to be used as accelerators are actually something that
		   it does allow.
		*/

		uint32_t fakekey = ev->keyval;

		if (Gtkmm2ext::possibly_translate_keyval_to_make_legal_accelerator (fakekey)) {
			if (allow_activating && gtk_accel_groups_activate(G_OBJECT(win), fakekey, GdkModifierType(ev->state))) {
				return true;
			}
		}
	}

	/* consider all relevant modifiers but not LOCK or SHIFT */

	guint mask = (Keyboard::RelevantModifierKeyMask & ~(Gdk::SHIFT_MASK|Gdk::LOCK_MASK));

	if (!special_handling_of_unmodified_accelerators || (ev->state & mask)) {

		/* no special handling or there are modifiers in effect: accelerate first */

                DEBUG_TRACE (DEBUG::Accelerators, "\tactivate, then propagate\n");

		if (allow_activating) {
			if (gtk_window_activate_key (win, ev)) {
				return true;
			}
		}

                DEBUG_TRACE (DEBUG::Accelerators, "\tnot accelerated, now propagate\n");

		return gtk_window_propagate_key_event (win, ev);
	}

	/* no modifiers, propagate first */

        DEBUG_TRACE (DEBUG::Accelerators, "\tpropagate, then activate\n");

	if (!gtk_window_propagate_key_event (win, ev)) {
                DEBUG_TRACE (DEBUG::Accelerators, "\tpropagation didn't handle, so activate\n");
		if (allow_activating) {
			return gtk_window_activate_key (win, ev);
		}

	} else {
                DEBUG_TRACE (DEBUG::Accelerators, "\thandled by propagate\n");
		return true;
	}

        DEBUG_TRACE (DEBUG::Accelerators, "\tnot handled\n");
	return true;
}

Glib::RefPtr<Gdk::Pixbuf>
get_xpm (std::string name)
{
	if (!xpm_map[name]) {

		SearchPath spath(ARDOUR::ardour_search_path());
		spath += ARDOUR::system_data_search_path();

		spath.add_subdirectory_to_paths("pixmaps");

		sys::path data_file_path;

		if(!find_file_in_search_path (spath, name, data_file_path)) {
			fatal << string_compose (_("cannot find XPM file for %1"), name) << endmsg;
		}

		try {
			xpm_map[name] =  Gdk::Pixbuf::create_from_file (data_file_path.to_string());
		} catch(const Glib::Error& e)	{
			warning << "Caught Glib::Error: " << e.what() << endmsg;
		}
	}

	return xpm_map[name];
}

std::string
get_icon_path (const char* cname)
{
	string name = cname;
	name += X_(".png");

	SearchPath spath(ARDOUR::ardour_search_path());
	spath += ARDOUR::system_data_search_path();

	spath.add_subdirectory_to_paths("icons");

	sys::path data_file_path;

	if (!find_file_in_search_path (spath, name, data_file_path)) {
		fatal << string_compose (_("cannot find icon image for %1"), name) << endmsg;
	}

	return data_file_path.to_string();
}

Glib::RefPtr<Gdk::Pixbuf>
get_icon (const char* cname)
{
	Glib::RefPtr<Gdk::Pixbuf> img;
	try {
		img = Gdk::Pixbuf::create_from_file (get_icon_path (cname));
	} catch (const Gdk::PixbufError &e) {
		cerr << "Caught PixbufError: " << e.what() << endl;
	} catch (...) {
		g_message("Caught ... ");
	}

	return img;
}

string
longest (vector<string>& strings)
{
	if (strings.empty()) {
		return string ("");
	}

	vector<string>::iterator longest = strings.begin();
	string::size_type longest_length = (*longest).length();

	vector<string>::iterator i = longest;
	++i;

	while (i != strings.end()) {

		string::size_type len = (*i).length();

		if (len > longest_length) {
			longest = i;
			longest_length = len;
		}

		++i;
	}

	return *longest;
}

bool
key_is_legal_for_numeric_entry (guint keyval)
{
	switch (keyval) {
	case GDK_minus:
	case GDK_plus:
	case GDK_period:
	case GDK_comma:
	case GDK_0:
	case GDK_1:
	case GDK_2:
	case GDK_3:
	case GDK_4:
	case GDK_5:
	case GDK_6:
	case GDK_7:
	case GDK_8:
	case GDK_9:
	case GDK_KP_Add:
	case GDK_KP_Subtract:
	case GDK_KP_Decimal:
	case GDK_KP_0:
	case GDK_KP_1:
	case GDK_KP_2:
	case GDK_KP_3:
	case GDK_KP_4:
	case GDK_KP_5:
	case GDK_KP_6:
	case GDK_KP_7:
	case GDK_KP_8:
	case GDK_KP_9:
	case GDK_Return:
	case GDK_BackSpace:
	case GDK_Delete:
	case GDK_KP_Enter:
	case GDK_Home:
	case GDK_End:
	case GDK_Left:
	case GDK_Right:
		return true;

	default:
		break;
	}

	return false;
}
void
set_pango_fontsize ()
{
	long val = ARDOUR::Config->get_font_scale();

	/* FT2 rendering - used by GnomeCanvas, sigh */

	pango_ft2_font_map_set_resolution ((PangoFT2FontMap*) pango_ft2_font_map_for_display(), val/1024, val/1024);

	/* Cairo rendering, in case there is any */

	pango_cairo_font_map_set_resolution ((PangoCairoFontMap*) pango_cairo_font_map_get_default(), val/1024);
}

void
reset_dpi ()
{
	long val = ARDOUR::Config->get_font_scale();
	set_pango_fontsize ();
	/* Xft rendering */

	gtk_settings_set_long_property (gtk_settings_get_default(),
					"gtk-xft-dpi", val, "ardour");
	DPIReset();//Emit Signal
}

void
resize_window_to_proportion_of_monitor (Gtk::Window* window, int max_width, int max_height)
{
	Glib::RefPtr<Gdk::Screen> screen = window->get_screen ();
	Gdk::Rectangle monitor_rect;
	screen->get_monitor_geometry (0, monitor_rect);

	int const w = std::min (int (monitor_rect.get_width() * 0.8), max_width);
	int const h = std::min (int (monitor_rect.get_height() * 0.8), max_height);

	window->resize (w, h);
}


/** Replace _ with __ in a string; for use with menu item text to make underscores displayed correctly */
string
escape_underscores (string const & s)
{
	string o;
	string::size_type const N = s.length ();

	for (string::size_type i = 0; i < N; ++i) {
		if (s[i] == '_') {
			o += "__";
		} else {
			o += s[i];
		}
	}

	return o;
}


