#include <iostream>
#include <gtkmm.h>
#include "canvas/canvas.h"
#include "canvas/rectangle.h"

using namespace std;
using namespace ArdourCanvas;

int main (int argc, char* argv[])
{
	Gtk::Main kit (argc, argv);

	Gtk::Window window;
	window.set_title ("Hello world");
	window.set_size_request (512, 512);
	GtkCanvasDrawingArea canvas;
	canvas.set_size_request (2048, 2048);

	Rectangle a (canvas.root(), Rect (64, 64, 128, 128));
	a.set_outline_color (0xff0000aa);
	Rectangle b (canvas.root(), Rect (64, 64, 128, 128));
	b.set_position (Duple (256, 256));
	b.set_outline_width (4);
	b.set_outline_what (0x2 | 0x8);
	b.set_outline_color (0x00ff00ff);

	Gtk::ScrolledWindow scroller;
	scroller.add (canvas);
	window.add (scroller);
	canvas.show ();
	window.show_all ();
	
	Gtk::Main::run (window);
	return 0;
}
