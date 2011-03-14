#include <list>
#include "canvas/item.h"
#include "canvas/types.h"
#include "canvas/lookup_table.h"

namespace ArdourCanvas
{

class Group : public Item
{
public:
	Group ();
	explicit Group (Group *);

	void add (Item *);
	void remove (Item *);
	std::list<Item*> const & items () const {
		return _items;
	}

	void render (Rect const &, Cairo::RefPtr<Cairo::Context>) const;
	Rect bounding_box () const;

private:
	Group (Group const &);
	
	std::list<Item*> _items;
	Duple _position;

	mutable LookupTable _lut;
};

}
