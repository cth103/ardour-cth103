#include <list>
#include "canvas/item.h"
#include "canvas/types.h"

namespace ArdourCanvas
{

class Group : public Item
{
public:
	Group (Group *);

	void add (Item *);

	void render (Rect const &, Cairo::RefPtr<Cairo::Context>) const;
	Rect bounding_box () const;

private:
	std::list<Item*> _items;
	Duple _position;
};

}
