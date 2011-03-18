#include "canvas/root_group.h"
#include "canvas/canvas.h"

using namespace std;
using namespace ArdourCanvas;

RootGroup::RootGroup (Canvas* canvas)
	: Group (canvas)
{

}

void
RootGroup::compute_bounding_box () const
{
	cout << "--------------- RG compute bbox.\n";
	if (_bounding_box) {
		cout << "(was " << _bounding_box.get() << ")\n";
	}
	Group::compute_bounding_box ();
	if (_bounding_box) {
		cout << "(now " << _bounding_box.get() << ")\n";
	}
	if (_bounding_box) {
		_canvas->request_size (Duple (_bounding_box.get().width (), _bounding_box.get().height ()));
	}
}
