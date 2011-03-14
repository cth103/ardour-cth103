#include "canvas/item.h"
#include "canvas/types.h"

namespace ArdourCanvas
{

class Rectangle : public Item
{
public:
	Rectangle (Group *);
	Rectangle (Group *, Rect const &);

	void render (Rect const &, Cairo::RefPtr<Cairo::Context>) const;
	boost::optional<Rect> bounding_box () const;

	void set (Rect const &);
	
private:
	Rect _rect;
};

}
