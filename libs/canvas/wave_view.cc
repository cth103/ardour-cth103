#include "canvas/wave_view.h"

using namespace ArdourCanvas;

WaveView::WaveView (Group* parent, boost::shared_ptr<ARDOUR::AudioRegion> region)
	: Item (parent)
	, _region (region)
{

}

void
WaveView::render (Rect const & area, Cairo::RefPtr<Cairo::Context> context) const
{

}

void
WaveView::compute_bounding_box () const
{
	/* XXX */
	_bounding_box_dirty = false;
}
	
XMLNode *
WaveView::get_state () const
{
	/* XXX */
	return new XMLNode ("WaveView");
}

void
WaveView::set_state (XMLNode const * node)
{
	/* XXX */
}

	   
#ifdef CANVAS_COMPATIBILITY
void
gnome_canvas_waveview_cache_destroy (GnomeCanvasWaveViewCache* c)
{
	
}
#endif

