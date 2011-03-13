#include <cairomm/surface.h>
#include <cairomm/context.h>
#include "canvas/group.h"

namespace ArdourCanvas
{

class Rect;
class Group;	

class Canvas
{
public:

	virtual ~Canvas () {}
	
	static Canvas* create_image ();
	
	void render (Rect const &) const;

	Group* root () {
		return &_root;
	}
		
protected:
	
	Canvas ();
	virtual Cairo::RefPtr<Cairo::Surface> create_surface () = 0;
	Cairo::RefPtr<Cairo::Surface> _surface;

private:
	
	Cairo::RefPtr<Cairo::Context> create_context ();

	Cairo::RefPtr<Cairo::Context> _context;
	Group _root;
};


class ImageCanvas : public Canvas
{
public:
	void write_to_png (std::string const &);

private:
	friend class Canvas;
	
	virtual Cairo::RefPtr<Cairo::Surface> create_surface ();
	ImageCanvas ();
};

};
