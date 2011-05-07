#include <gtkmm/label.h>
#include "pbd/xml++.h"
#include "canvas/text.h"
#include "canvas/canvas.h"
#include "canvas/utils.h"

using namespace std;
using namespace ArdourCanvas;

Text::Text (Group* parent)
	: Item (parent)
	, _font_description (0)
	, _color (0x000000ff)
	, _alignment (Pango::ALIGN_LEFT)
{

}

void
Text::set (string const & text)
{
	begin_change ();
	
	_text = text;
	
	_bbox_dirty = true;
	end_change ();
}

void
Text::compute_bbox () const
{
	if (!_canvas || !_canvas->context ()) {
		_bbox = boost::optional<Rect> ();
		_bbox_dirty = false;
		return;
	}
	
	Pango::Rectangle const r = layout (_canvas->context())->get_ink_extents ();
	
	_bbox = Rect (
		r.get_x() / Pango::SCALE,
		r.get_y() / Pango::SCALE,
		(r.get_x() + r.get_width()) / Pango::SCALE,
		(r.get_y() + r.get_height()) / Pango::SCALE
		);
	
	_bbox_dirty = false;
}

Glib::RefPtr<Pango::Layout>
Text::layout (Cairo::RefPtr<Cairo::Context> context) const
{
	Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create (context);
	layout->set_text (_text);
	if (_font_description) {
		layout->set_font_description (*_font_description);
	}
	layout->set_alignment (_alignment);
	return layout;
}

void
Text::render (Rect const & area, Cairo::RefPtr<Cairo::Context> context) const
{
	set_source_rgba (context, _color);
	layout (context)->show_in_cairo_context (context);
}

XMLNode *
Text::get_state () const
{
	XMLNode* node = new XMLNode ("Text");
#ifdef CANVAS_DEBUG
	if (!name.empty ()) {
		node->add_property ("name", name);
	}
#endif
	return node;
}

void
Text::set_state (XMLNode const * node)
{
	/* XXX */
}

void
Text::set_alignment (Pango::Alignment alignment)
{
	begin_change ();
	
	_alignment = alignment;

	_bbox_dirty = true;
	end_change ();
}

void
Text::set_font_description (Pango::FontDescription* font_description)
{
	begin_change ();
	
	_font_description = font_description;

	_bbox_dirty = true;
	end_change ();
}

void
Text::set_color (Color color)
{
	begin_change ();

	_color = color;

	end_change ();
}

		
