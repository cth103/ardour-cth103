#ifndef __CANVAS_ROOT_GROUP_H__
#define __CANVAS_ROOT_GROUP_H__

#include "group.h"

namespace ArdourCanvas {

class RootGroup : public Group
{
public:
	RootGroup (Canvas *);

	void compute_bounding_box () const;
	void child_changed ();
};

}

#endif
