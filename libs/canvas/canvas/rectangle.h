#include "canvas/item.h"
#include "canvas/types.h"

namespace ArdourCanvas
{

class Rectangle : public Item
{
public:
	Rectangle (Group *);

	void render (Rect const &, Cairo::RefPtr<Cairo::Context>) const;
	Rect bounding_box () const;

	void set (Rect const &);
	
private:
	Rect _rect;
};

}
