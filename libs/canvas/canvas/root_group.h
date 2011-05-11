#ifndef __CANVAS_ROOT_GROUP_H__
#define __CANVAS_ROOT_GROUP_H__

#include "group.h"

namespace Canvas {

class RootGroup : public Group
{
private:
	friend class Canvas;
	
	RootGroup (Canvas *);

	void compute_bbox () const;
	void child_changed ();
};

}

#endif
