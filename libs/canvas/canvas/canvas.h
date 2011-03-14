#include <gdkmm/window.h>
#include <gtkmm/drawingarea.h>
#include <cairomm/surface.h>
#include <cairomm/context.h>
#include "canvas/group.h"

namespace ArdourCanvas
{

class Rect;
class Group;	

class Canvas
{
public:

	Canvas ();
	virtual ~Canvas () {}
	
	void render (Rect const &, Cairo::RefPtr<Cairo::Context> const &) const;

	Group* root () {
		return &_root;
	}
		
private:
	
	Group _root;
};


class ImageCanvas : public Canvas
{
public:
	ImageCanvas ();
	void render_to_image (Rect const &) const;
	void write_to_png (std::string const &);

private:
	Cairo::RefPtr<Cairo::Surface> _surface;
	Cairo::RefPtr<Cairo::Context> _context;
};

class GtkCanvas : public Canvas
{
public:
	GtkCanvas ();
};

class GtkCanvasDrawingArea : public Gtk::DrawingArea, public GtkCanvas
{
public:
	GtkCanvasDrawingArea ();

protected:
	bool on_expose_event (GdkEventExpose *);

private:
	GtkCanvas _canvas;
};

}
