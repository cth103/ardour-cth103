#include "ardour/types.h"
#include "ardour/audioregion.h"
#include "canvas/wave_view.h"
#include "canvas/utils.h"

using namespace std;
using namespace ARDOUR;
using namespace ArdourCanvas;

WaveView::WaveView (Group* parent, boost::shared_ptr<ARDOUR::AudioRegion> region)
	: Item (parent)
	, _region (region)
	, _frames_per_pixel (0)
	, _height (64)
{

}

void
WaveView::set_frames_per_pixel (double frames_per_pixel)
{
	_frames_per_pixel = frames_per_pixel;
}

void
WaveView::render (Rect const & area, Cairo::RefPtr<Cairo::Context> context) const
{
	assert (_frames_per_pixel != 0);

	set_source_rgba (context, 0xff0000ff);

	framepos_t const start = floor (area.x0 * _frames_per_pixel);
	framepos_t const end   = ceil  (area.x1 * _frames_per_pixel);

	uint32_t const npeaks = ceil ((end - start) / _frames_per_pixel);

	PeakData* buf = new PeakData[npeaks];
	_region->read_peaks (buf, npeaks, start + _region->position(), end - start, 0, _frames_per_pixel);

	context->move_to (area.x0, area.y0);
	for (uint32_t i = 0; i < npeaks; ++i) {
		context->line_to (area.x0 + i, (buf[i].max + 1) * _height / 2);
	}

	context->stroke ();
}

void
WaveView::compute_bounding_box () const
{
	/* XXX */
	_bounding_box = Rect (0, 0, 256, _height);
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

