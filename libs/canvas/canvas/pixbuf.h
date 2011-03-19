#include "canvas/item.h"

namespace ArdourCanvas {

class Pixbuf : public Item
{
public:
	Pixbuf (Group *);

	void render (Rect const &, Cairo::RefPtr<Cairo::Context>) const;
	void compute_bounding_box () const;

	void set (Glib::RefPtr<Gdk::Pixbuf>);

private:
	Glib::RefPtr<Gdk::Pixbuf> _pixbuf;
};

}
