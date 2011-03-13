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
		Rectangle* r = new Rectangle (_canvas->root ());
		r->set (Rect (0, 0, 256, 256));
	}

protected:
	virtual bool on_expose_event (GdkEventExpose* ev) {
		Cairo::RefPtr<Cairo::Context> cr = get_window()->create_cairo_context ();
		_canvas->render (Rect (0, 0, 1024, 1024), cr);
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
	Area area;
	window.add (area);
	area.show ();
	window.show_all ();
	
	Gtk::Main::run (window);
	return 0;
}
