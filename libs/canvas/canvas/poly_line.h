namespace ArdourCanvas {

#include "canvas/item.h"

class PolyLine : public Item
{
public:
	PolyLine (Group *);

	void set (Points const &);
	Points const & get () const;

private:
	Points _points;
};
	
}

	
