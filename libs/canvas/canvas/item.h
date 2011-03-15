#ifndef __CANVAS_ITEM_H__
#define __CANVAS_ITEM_H__

#include <stdint.h>
#include <cairomm/context.h>
#ifdef CANVAS_COMPATIBILITY
#include <gdkmm/cursor.h>
#endif
#include "canvas/types.h"

namespace ArdourCanvas
{

class Canvas;
class Group;
class Rect;	

class Item
{
public:
	Item (Group *);
	virtual ~Item ();

	void unparent ();
	void reparent (Group *);
	Group* parent () const {
		return _parent;
	}

	/** Set the position of this item in the parent's coordinates */
	void set_position (Duple);

	Duple position () const {
		return _position;
	}

	/** @return Bounding box in this item's coordinates */
	virtual boost::optional<Rect> bounding_box () const = 0;

	/** @param area Area to draw in this item's coordinates */
	virtual void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const = 0;

	Duple item_to_parent (Duple const &) const;
	Rect item_to_parent (Rect const &) const;
	Duple parent_to_item (Duple const &) const;

	void raise_to_top ();
	void raise (int);
	void lower_to_bottom ();

	void hide ();
	void show ();
	bool visible () const {
		return _visible;
	}

#ifdef CANVAS_COMPATIBILITY

	void set_data (char const *, void *);
	void* get_data (char const *);
	void w2i (double &, double &);
	void i2w (double &, double &);
	void move (double, double);
	void grab (int, Gdk::Cursor, uint32_t &);
	void grab (int, uint32_t);
	void ungrab (int);
	void grab_focus ();
	void get_bounds (double &, double &, double &, double &) const;

#endif	
	
	
protected:
	Group* _parent;
	/** position of this item in parent coordinates */
	Duple _position;
	bool _visible;

#ifdef CANVAS_COMPATIBILITY
	std::map<std::string, void *> _data;
#endif	
};

}

#endif
