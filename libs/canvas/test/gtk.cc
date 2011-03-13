#include <iostream>
#include <gtkmm.h>
#include "canvas/canvas.h"
#include "canvas/rectangle.h"

using namespace std;
using namespace ArdourCanvas;

class Area : public Gtk::DrawingArea
{
public:
	Area () {
		_canvas = new GtkCanvas;

		int const N = 10000;
		double Ns = sqrt (N);
		int max_x = 1024;
		int max_y = 1024;

		srand (42);
		for (int x = 0; x < Ns; ++x) {
			for (int y = 0; y < Ns; ++y) {
				Rectangle* r = new Rectangle (_canvas->root ());
				r->set (Rect (x * max_x / Ns, y * max_y / Ns, (x + 1) * max_x / Ns, (y + 1) * max_y / Ns));
			}
		}
	}

protected:
	virtual bool on_expose_event (GdkEventExpose* ev) {
		Cairo::RefPtr<Cairo::Context> cr = get_window()->create_cairo_context ();
		_canvas->render (Rect (ev->area.x, ev->area.y, ev->area.x + ev->area.width, ev->area.y + ev->area.height), cr);
		return true;
	}

private:
	GtkCanvas* _canvas;
};

int main (int argc, char* argv[])
{
	Gtk::Main kit (argc, argv);

	Gtk::Window window;
	window.set_title ("Hello world");
	window.set_size_request (512, 512);
	Area area;
	area.set_size_request (2048, 2048);
	Gtk::ScrolledWindow scroller;
	scroller.add (area);
	window.add (scroller);
	area.show ();
	window.show_all ();
	
	Gtk::Main::run (window);
	return 0;
}
