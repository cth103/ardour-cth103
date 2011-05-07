#include "canvas/root_group.h"
#include "canvas/canvas.h"

using namespace std;
using namespace ArdourCanvas;

RootGroup::RootGroup (Canvas* canvas)
	: Group (canvas)
{
#ifdef CANVAS_DEBUG
	name = "ROOT";
#endif	
}

void
RootGroup::compute_bbox () const
{
	Group::compute_bbox ();

	if (_bbox) {
		_canvas->request_size (Duple (_bbox.get().width (), _bbox.get().height ()));
	}
}
