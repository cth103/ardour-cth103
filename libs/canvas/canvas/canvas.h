#include <gdkmm/window.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/viewport.h>
#include <cairomm/surface.h>
#include <cairomm/context.h>
#include "pbd/signals.h"
#include "canvas/root_group.h"

class XMLTree;

namespace ArdourCanvas
{

class Rect;
class Group;	

class Canvas
{
public:

	Canvas ();
	Canvas (XMLTree const *);
	virtual ~Canvas () {}

	virtual void request_redraw (Rect const &) = 0;
	virtual void request_size (Duple) = 0;
	virtual void grab (Item *) = 0;
	virtual void ungrab () = 0;

	void render (Rect const &, Cairo::RefPtr<Cairo::Context> const &) const;

	Group* root () {
		return &_root;
	}

	void item_changed (Item *, boost::optional<Rect>);
	void item_moved (Item *, boost::optional<Rect>);

	XMLTree* get_state ();

protected:
	RootGroup _root;
	
private:
	void queue_draw_item_area (Item *, Rect);
};


class ImageCanvas : public Canvas
{
public:
	ImageCanvas (Duple size = Duple (1024, 1024));
	ImageCanvas (XMLTree const *, Duple size = Duple (1024, 1024));

	void request_redraw (Rect const &) {
		/* XXX */
	}

	void request_size (Duple) {
		/* XXX */
	}
	
	void grab (Item *) {}
	void ungrab () {}

	void render_to_image (Rect const &) const;
	void write_to_png (std::string const &);

private:
	Cairo::RefPtr<Cairo::Surface> _surface;
	Cairo::RefPtr<Cairo::Context> _context;
};

class GtkCanvas : public Canvas, public Gtk::EventBox
{
public:
	GtkCanvas ();
	GtkCanvas (XMLTree const *);

	void request_redraw (Rect const &);
	void request_size (Duple);
	void grab (Item *);
	void ungrab ();
	
	PBD::Signal1<bool, GdkEventButton *> ButtonPress;
	
protected:
	bool on_expose_event (GdkEventExpose *);
	bool on_button_press_event (GdkEventButton *);
	bool on_button_release_event (GdkEventButton* event);
	bool on_motion_notify_event (GdkEventMotion *);
	
	bool button_handler (GdkEventButton *);
	bool motion_notify_handler (GdkEventMotion *);
	bool deliver_event (Duple, GdkEvent *);

private:
	Item const * _current_item;
	Item const * _grabbed_item;
};

class GtkCanvasViewport : public Gtk::Viewport
{
public:
	GtkCanvasViewport (Gtk::Adjustment &, Gtk::Adjustment &);

	GtkCanvas* canvas () {
		return &_canvas;
	}

protected:
	void on_size_request (Gtk::Requisition *);

private:
	GtkCanvas _canvas;
};

}
