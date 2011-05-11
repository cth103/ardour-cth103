#include <gdkmm/pixbuf.h>
#include "canvas/item.h"

namespace Canvas {

class Pixbuf : public Item
{
public:
	Pixbuf (Group *);

	void render (Rect const &, Cairo::RefPtr<Cairo::Context>) const;
	void compute_bbox () const;
	XMLNode* get_state () const;
	void set_state (XMLNode const *);

	void set (Glib::RefPtr<Gdk::Pixbuf>);

private:
	Glib::RefPtr<Gdk::Pixbuf> _pixbuf;
};

}
