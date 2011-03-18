#ifndef __CANVAS_ITEM_H__
#define __CANVAS_ITEM_H__

#include <stdint.h>
#include <cairomm/context.h>
#ifdef CANVAS_COMPATIBILITY
#include <gdkmm/cursor.h>
#endif
#include "pbd/signals.h"
#include "canvas/types.h"

namespace ArdourCanvas
{

class Canvas;
class Group;
class Rect;	

/** The parent class for anything that goes on the canvas.
 *
 *  Items have a position, which is expressed in the coordinates of the parent.
 *  They also have a bounding box, which describes the area in which they have
 *  drawable content, which is expressed in their own coordinates (whose origin
 *  is at the item position).
 *
 *  Any item that is being displayed on a canvas has a pointer to that canvas,
 *  and all except the `root group' have a pointer to their parent group.
 */
	
class Item
{
public:
	Item (Canvas *);
	Item (Group *);
	Item (Group *, Duple);
	virtual ~Item ();

	/** Render this item to a Cairo context.
	 *  @param area Area to draw in this item's coordinates.
	 */
	virtual void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const = 0;

	/** Update _bounding_box and _bounding_box_dirty */
	virtual void compute_bounding_box () const = 0;

	void unparent ();
	void reparent (Group *);

	/** @return Parent group, or 0 if this is the root group */
	Group* parent () const {
		return _parent;
	}

	void set_position (Duple);

	/** @return Position of this item in the parent's coordinates */
	Duple position () const {
		return _position;
	}

	boost::optional<Rect> bounding_box () const;
	
	Duple item_to_parent (Duple const &) const;
	Rect item_to_parent (Rect const &) const;
	Duple parent_to_item (Duple const &) const;
	Rect parent_to_item (Rect const &) const;

	void raise_to_top ();
	void raise (int);
	void lower_to_bottom ();

	void hide ();
	void show ();

	/** @return true if this item is visible (ie it will be rendered),
	 *  otherwise false
	 */
	bool visible () const {
		return _visible;
	}

	/** @return Our canvas, or 0 if we are not attached to one */
	Canvas* canvas () const {
		return _canvas;
	}

	virtual char const * name () const {
		return "unknown";
	}

	void set_watch () {
		_watch = true;
	}

	bool watch () const {
		return _watch;
	}

	/* XXX: maybe this should be a PBD::Signal */
	sigc::signal<bool, GdkEvent*> Event;

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

	void begin_change ();
	void end_change ();

	Canvas* _canvas;
	/** parent group; may be 0 if we are the root group or if we have been unparent()ed */
	Group* _parent;
	/** position of this item in parent coordinates */
	Duple _position;
	/** true if this item is visible (ie to be drawn), otherwise false */
	bool _visible;
	/** our bounding box before any change that is currently in progress */
	boost::optional<Rect> _pre_change_bounding_box;

	/** our bounding box; may be out of date if _bounding_box_dirty is true */
	mutable boost::optional<Rect> _bounding_box;
	/** true if _bounding_box might be out of date, false if its definitely not */
	mutable bool _bounding_box_dirty;

	bool _watch;

#ifdef CANVAS_COMPATIBILITY
	std::map<std::string, void *> _data;
#endif

private:
	void init ();
};

}

#endif
