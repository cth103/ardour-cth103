#include <gdkmm/general.h>
#include "pbd/xml++.h"
#include "canvas/pixbuf.h"

using namespace std;
using namespace Canvas;

Pixbuf::Pixbuf (Group* g)
	: Item (g)
{
	
}

void
Pixbuf::render (Rect const & area, Cairo::RefPtr<Cairo::Context> context) const
{
	Gdk::Cairo::set_source_pixbuf (context, _pixbuf, 0, 0);
	context->paint ();
}
	
void
Pixbuf::compute_bbox () const
{
	if (_pixbuf) {
		_bbox = boost::optional<Rect> (Rect (0, 0, _pixbuf->get_width(), _pixbuf->get_height()));
	} else {
		_bbox = boost::optional<Rect> ();
	}

	_bbox_dirty = false;
}

void
Pixbuf::set (Glib::RefPtr<Gdk::Pixbuf> pixbuf)
{
	begin_change ();
	
	_pixbuf = pixbuf;
	_bbox_dirty = true;

	end_change ();
}

XMLNode *
Pixbuf::get_state () const
{
	/* XXX */
	return new XMLNode ("Pixbuf");
}

void
Pixbuf::set_state (XMLNode const * node)
{
	/* XXX */
}
