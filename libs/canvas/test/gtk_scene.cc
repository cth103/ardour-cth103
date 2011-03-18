#include <iostream>
#include <gtkmm.h>
#include "canvas/canvas.h"
#include "canvas/rectangle.h"

using namespace std;
using namespace ArdourCanvas;

bool
foo (GdkEvent* ev)
{
	cout << "click.\n";
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
	GtkCanvasDrawingArea* canvas = viewport.canvas ();

	overall_vbox.pack_start (viewport, true, true);
	overall_vbox.pack_start (h_scroll, false, false);

	Rectangle a (canvas->root(), Rect (64, 64, 128, 128));
	a.set_outline_color (0xff0000aa);
	Rectangle b (canvas->root(), Rect (64, 64, 128, 128));
	b.set_position (Duple (256, 256));
	b.set_outline_width (4);
	b.set_outline_what (0x2 | 0x8);
	b.set_outline_color (0x0000ffff);

	PBD::ScopedConnection connection;
	b.Event.connect_same_thread (connection, sigc::ptr_fun (foo));

	Rectangle c (canvas->root(), Rect (2048, 2048, 2096, 2096));

	window.add (overall_vbox);
	canvas->show ();
	window.show_all ();
	
	Gtk::Main::run (window);
	return 0;
}
