#include <gdkmm/window.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/viewport.h>
#include <cairomm/surface.h>
#include <cairomm/context.h>
#include "canvas/root_group.h"

namespace ArdourCanvas
{

class Rect;
class Group;	

class Canvas
{
public:

	Canvas ();
	virtual ~Canvas () {}

	virtual void request_redraw (Rect const &) = 0;
	virtual void request_size (Duple) = 0;
	
	void render (Rect const &, Cairo::RefPtr<Cairo::Context> const &) const;

	Group* root () {
		return &_root;
	}

	void item_changed (Item *, boost::optional<Rect>);
	void item_moved (Item *, boost::optional<Rect>);
		
private:
	void queue_draw_item_area (Item *, Rect);
	
	RootGroup _root;
};


class ImageCanvas : public Canvas
{
public:
	ImageCanvas ();

	void request_redraw (Rect const &) {
		/* XXX */
	}

	void request_size (Duple) {
		/* XXX */
	}
	
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

	void request_redraw (Rect const &);
	void request_size (Duple);
	
protected:
	bool on_expose_event (GdkEventExpose *);
};

class GtkCanvasViewport : public Gtk::Viewport
{
public:
	GtkCanvasViewport (Gtk::Adjustment &, Gtk::Adjustment &);

	GtkCanvasDrawingArea* canvas () {
		return &_canvas;
	}

protected:
	
	void on_size_request (Gtk::Requisition *);

private:
	GtkCanvasDrawingArea _canvas;
};

}
