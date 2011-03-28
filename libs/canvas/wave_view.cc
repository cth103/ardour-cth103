#include "ardour/types.h"
#include "ardour/audioregion.h"
#include "canvas/wave_view.h"
#include "canvas/utils.h"

using namespace std;
using namespace ARDOUR;
using namespace ArdourCanvas;

WaveView::WaveView (Group* parent, boost::shared_ptr<ARDOUR::AudioRegion> region)
	: Item (parent)
	, Outline (parent)
	, Fill (parent)
	, _region (region)
	, _channel (0)
	, _frames_per_pixel (0)
	, _height (64)
{

}

void
WaveView::set_frames_per_pixel (double frames_per_pixel)
{
	begin_change ();
	
	_frames_per_pixel = frames_per_pixel;

	_bounding_box_dirty = true;
	end_change ();
}

Coord
WaveView::max_position (float max) const
{
	return _height / 2 + (max + 1) * _height / 4;
}

Coord
WaveView::min_position (float min) const
{
	return _height / 2 + (min - 1) * _height / 4;
}

void
WaveView::render (Rect const & area, Cairo::RefPtr<Cairo::Context> context) const
{
	assert (_frames_per_pixel != 0);

	framepos_t const start = floor (area.x0 * _frames_per_pixel);
	framepos_t const end   = ceil  (area.x1 * _frames_per_pixel);

	uint32_t const npeaks = ceil ((end - start) / _frames_per_pixel);

	PeakData* buf = new PeakData[npeaks];
	_region->read_peaks (buf, npeaks, start + _region->position(), end - start, _channel, _frames_per_pixel);

	setup_outline_context (context);
	context->move_to (area.x0, area.y0);
	for (uint32_t i = 0; i < npeaks; ++i) {
		context->line_to (area.x0 + i, max_position (buf[i].max));
	}
	context->stroke ();

	context->move_to (area.x0, area.y0);
	for (uint32_t i = 0; i < npeaks; ++i) {
		context->line_to (area.x0 + i, min_position (buf[i].min));
	}
	context->stroke ();

	set_source_rgba (context, _fill_color);
	for (uint32_t i = 0; i < npeaks; ++i) {
		context->move_to (area.x0 + i, max_position (buf[i].max) - 1);
		context->line_to (area.x0 + i, min_position (buf[i].min) + 1);
		context->stroke ();
	}

	delete[] buf;
}

void
WaveView::compute_bounding_box () const
{
	_bounding_box = Rect (0, 0, _region->length() / _frames_per_pixel, _height);
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

void
WaveView::set_height (Distance height)
{
	begin_change ();

	_height = height;

	_bounding_box_dirty = true;
	end_change ();
}

void
WaveView::set_channel (int channel)
{
	begin_change ();
	
	_channel = channel;

	_bounding_box_dirty = true;
	end_change ();
}

#ifdef CANVAS_COMPATIBILITY
void
gnome_canvas_waveview_cache_destroy (GnomeCanvasWaveViewCache* c)
{
	
}
#endif

