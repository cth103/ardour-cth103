#include <iostream>
#include <gtkmm.h>
#include "canvas/canvas.h"
#include "canvas/rectangle.h"
#include "canvas/line.h"
#include "canvas/pixbuf.h"

using namespace std;
using namespace Canvas;

bool
foo (GdkEvent* ev)
{
	cout << "click.\n";
	return true;
}

Rectangle* a = 0;
GtkCanvas* canvas = 0;

gboolean timer (void *)
{
	static bool foo = false;
	if (foo) {
		a->hide ();
		foo = true;
	} else {
		a->show ();
		foo = false;
	}

	return true;
}

int main (int argc, char* argv[])
{
	Gtk::Main kit (argc, argv);

	Gtk::Window window;
	window.set_title ("Hello world");
	window.set_size_request (512, 512);

	Gtk::VBox overall_vbox;
	Gtk::HScrollbar h_scroll;
	Gtk::VScrollbar v_scroll;
	
	GtkCanvasViewport viewport (*h_scroll.get_adjustment(), *v_scroll.get_adjustment());
	canvas = viewport.canvas ();

	overall_vbox.pack_start (viewport, true, true);
	overall_vbox.pack_start (h_scroll, false, false);

	a = new Rectangle (canvas->root(), Rect (64, 64, 128, 128));
	a->set_outline_color (0xff0000aa);
	Rectangle b (canvas->root(), Rect (64, 64, 128, 128));
	b.set_position (Duple (256, 256));
	b.set_outline_width (4);
	b.set_outline_what (0x2 | 0x8);
	b.set_outline_color (0x0000ffff);
	b.Event.connect (sigc::ptr_fun (foo));

	Rectangle c (canvas->root(), Rect (2048, 2048, 2096, 2096));

	Rectangle d (canvas->root(), Rect (0, 256, COORD_MAX, 284));
	d.name = "d";

	Line e (canvas->root());
	e.set (Duple (256, 0), Duple (256, COORD_MAX));
	e.name = "e";
	e.set_outline_color (0xff0000ff);

	Pixbuf pixbuf (canvas->root());
	pixbuf.set_position (Duple (192, 192));
	Glib::RefPtr<Gdk::Pixbuf> p = Gdk::Pixbuf::create_from_file ("../../libs/canvas/test/test.png");
	pixbuf.set (p);

	window.add (overall_vbox);
	canvas->show ();
	window.show_all ();

	g_timeout_add (33, (GSourceFunc) timer, 0);	
	
	Gtk::Main::run (window);
	return 0;
}

