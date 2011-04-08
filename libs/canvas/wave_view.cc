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

	invalidate_cache ();
}

Coord
WaveView::position (float s) const
{
	return (s + 1) * _height / 2;
}

list<WaveView::CacheEntry*>
WaveView::make_render_list (Rect const & area, frameoffset_t& start, frameoffset_t& end) const
{
	assert (_frames_per_pixel != 0);

	list<CacheEntry*> render_list;

	if (!_region) {
		return render_list;
	}

	start = floor (area.x0 * _frames_per_pixel);
	end   = ceil  (area.x1 * _frames_per_pixel);

	list<CacheEntry*>::iterator cache = _cache.begin ();

	framepos_t p = start;

	while (p < end) {

		/* Step through cache entries that end at or before our current position, p */
		while (cache != _cache.end() && (*cache)->end() <= p) {
			++cache;
		}

		/* Now either:
		   1. we have run out of cache entries
		   2. the one we are looking at finishes after p but also starts after p.
		   3. the one we are looking at finishes after p and starts before p.

		   Set up a pointer to the cache entry that we will use on this iteration.
		*/

		if (cache == _cache.end ()) {

			/* Case 1: we have run out of cache entries, so make a new one for
			   the whole required area and put it in the list.
			*/
			
			CacheEntry* c = new CacheEntry (this, p, end);
			_cache.push_back (c);
			render_list.push_back (c);
			p = end;

		} else if ((*cache)->start() > p) {

			/* Case 2: we have a cache entry, but it starts after p, so we
			   need another one for the missing bit.
			*/

			CacheEntry* c = new CacheEntry (this, p, (*cache)->start());
			cache = _cache.insert (cache, c);
			render_list.push_back (c);
			++cache;
			p = (*cache)->start ();

		} else {

			/* Case 3: we have a cache entry that will do at least some of what
			   we have left, so render it.
			*/

			render_list.push_back (*cache);
			p = min (end, (*cache)->end ());
			++cache;

		}
	}

	return render_list;
}

void
WaveView::render (Rect const & area, Cairo::RefPtr<Cairo::Context> context) const
{
	framepos_t start;
	framepos_t end;
	
	list<CacheEntry*> render_list = make_render_list (area, start, end);

	framepos_t p = start;

	for (list<CacheEntry*>::iterator i = render_list.begin(); i != render_list.end(); ++i) {

		Rect r (
			floor (((*i)->start() - _region->start()) / _frames_per_pixel),
			area.y0,
			ceil (((*i)->end() - _region->start()) / _frames_per_pixel),
			area.y1
			);

		p = render_cache_entry (context, r, *i, p, end);
	}
}

frameoffset_t
WaveView::render_cache_entry (Cairo::RefPtr<Cairo::Context> context, Rect const & area, CacheEntry* cache, frameoffset_t start, frameoffset_t end) const
{
	end = min (end, cache->end ());
	
	uint32_t const npeaks = ceil ((end - start) / _frames_per_pixel);

	PeakData* peaks = cache->peaks ();

	setup_outline_context (context);
	context->move_to (area.x0, area.y0);
	for (uint32_t i = 0; i < npeaks; ++i) {
		context->line_to (area.x0 + i, position (peaks[i].max));
	}
	context->stroke ();

	context->move_to (area.x0, area.y0);
	for (uint32_t i = 0; i < npeaks; ++i) {
		context->line_to (area.x0 + i, position (peaks[i].min));
	}
	context->stroke ();

	set_source_rgba (context, _fill_color);
	for (uint32_t i = 0; i < npeaks; ++i) {
		context->move_to (area.x0 + i, position (peaks[i].max) - 1);
		context->line_to (area.x0 + i, position (peaks[i].min) + 1);
		context->stroke ();
	}

	return end;
}

void
WaveView::compute_bounding_box () const
{
	if (_region) {
		_bounding_box = Rect (0, 0, _region->length() / _frames_per_pixel, _height);
	} else {
		_bounding_box = boost::optional<Rect> ();
	}
	
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

	invalidate_cache ();
}

#ifdef CANVAS_COMPATIBILITY
void
gnome_canvas_waveview_cache_destroy (GnomeCanvasWaveViewCache* c)
{
	
}
#endif

/** Construct a new CacheEntry with peak data between two offsets
 *  in the source.
 */
WaveView::CacheEntry::CacheEntry (
	WaveView const * wave_view,
	frameoffset_t start,
	frameoffset_t end
	)
	: _wave_view (wave_view)
	, _start (start)
	, _end (end)
{
	_n_peaks = ceil ((_end - _start) / _wave_view->_frames_per_pixel);
	_peaks = new PeakData[_n_peaks];
	_wave_view->_region->read_peaks (_peaks, _n_peaks, _start, _end - _start, _wave_view->_channel, _wave_view->_frames_per_pixel);
}

WaveView::CacheEntry::~CacheEntry ()
{
	delete[] _peaks;
}

void
WaveView::invalidate_cache ()
{
	for (list<CacheEntry*>::iterator i = _cache.begin(); i != _cache.end(); ++i) {
		delete *i;
	}

	_cache.clear ();
}
