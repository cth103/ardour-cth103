#include <list>
#include "canvas/item.h"
#include "canvas/types.h"
#include "canvas/lookup_table.h"

namespace ArdourCanvas {

class Group : public Item
{
public:
	Group ();
	explicit Group (Group *);
	~Group ();

	void add (Item *);
	void remove (Item *);
	std::list<Item*> const & items () const {
		return _items;
	}

	void raise_to_top (Item *);
	void raise (Item *, int);
	void lower_to_bottom (Item *);

	void render (Rect const &, Cairo::RefPtr<Cairo::Context>) const;
	boost::optional<Rect> bounding_box () const;

private:
	Group (Group const &);
	void ensure_lut () const;
	void invalidate_lut () const;

	/* our items, from lowest to highest in the stack */
	std::list<Item*> _items;
	Duple _position;

	mutable LookupTable* _lut;
};

}
