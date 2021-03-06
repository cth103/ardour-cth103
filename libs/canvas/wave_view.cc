#include <gdkmm.h>
#include "gtkmm2ext/utils.h"
#include "pbd/compose.h"
#include "pbd/signals.h"
#include "ardour/types.h"
#include "ardour/audioregion.h"
#include "canvas/wave_view.h"
#include "canvas/utils.h"

using namespace std;
using namespace ARDOUR;
using namespace Canvas;

WaveView::WaveView (Group* parent, boost::shared_ptr<ARDOUR::AudioRegion> region)
	: Item (parent)
	, Outline (parent)
	, Fill (parent)
	, _region (region)
	, _channel (0)
	, _frames_per_pixel (0)
	, _height (64)
	, _wave_color (0xffffffff)
	, _region_start (0)
{
	
}

void
WaveView::set_frames_per_pixel (double frames_per_pixel)
{
	begin_change ();
	
	_frames_per_pixel = frames_per_pixel;

	_bbox_dirty = true;
	end_change ();

	invalidate_whole_cache ();
}

void
WaveView::render (Rect const & area, Cairo::RefPtr<Cairo::Context> context) const
{
	assert (_frames_per_pixel != 0);

	if (!_region) {
		return;
	}

	/* p, start and end are offsets from the start of the source.
	   area is relative to the position of the region.
	 */
	
	int const start = rint (area.x0 + _region_start / _frames_per_pixel);
	int const end   = rint (area.x1 + _region_start / _frames_per_pixel);

	int p = start;
	list<CacheEntry*>::iterator cache = _cache.begin ();

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

		CacheEntry* render = 0;

		if (cache == _cache.end ()) {

			/* Case 1: we have run out of cache entries, so make a new one for
			   the whole required area and put it in the list.
			*/
			
			CacheEntry* c = new CacheEntry (this, p, end);
			_cache.push_back (c);
			render = c;

		} else if ((*cache)->start() > p) {

			/* Case 2: we have a cache entry, but it starts after p, so we
			   need another one for the missing bit.
			*/

			CacheEntry* c = new CacheEntry (this, p, (*cache)->start());
			cache = _cache.insert (cache, c);
			++cache;
			render = c;

		} else {

			/* Case 3: we have a cache entry that will do at least some of what
			   we have left, so render it.
			*/

			render = *cache;
			++cache;

		}

		int const this_end = min (end, render->end ());
		
		Coord const left  =        p - _region_start / _frames_per_pixel;
		Coord const right = this_end - _region_start / _frames_per_pixel;
		
		context->save ();
		
		context->rectangle (left, area.y0, right, area.height());
		context->clip ();
		
		context->translate (left, 0);

		Gdk::Cairo::set_source_pixbuf (context, render->pixbuf (), render->start() - p, 0);
		context->paint ();
		
		context->restore ();

		p = min (end, render->end ());
	}
}

void
WaveView::compute_bbox () const
{
	if (_region) {
		_bbox = Rect (0, 0, _region->length() / _frames_per_pixel, _height);
	} else {
		_bbox = boost::optional<Rect> ();
	}
	
	_bbox_dirty = false;
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

	_bbox_dirty = true;
	end_change ();

	invalidate_pixbuf_cache ();
}

void
WaveView::set_channel (int channel)
{
	begin_change ();
	
	_channel = channel;

	_bbox_dirty = true;
	end_change ();

	invalidate_whole_cache ();
}

void
WaveView::invalidate_whole_cache ()
{
	for (list<CacheEntry*>::iterator i = _cache.begin(); i != _cache.end(); ++i) {
		delete *i;
	}

	_cache.clear ();
}

void
WaveView::invalidate_pixbuf_cache ()
{
	for (list<CacheEntry*>::iterator i = _cache.begin(); i != _cache.end(); ++i) {
		(*i)->clear_pixbuf ();
	}
}

void
WaveView::region_resized ()
{
	_bbox_dirty = true;
}

void
WaveView::set_region_start (frameoffset_t start)
{
	_region_start = start;
	_bbox_dirty = true;
}

/** Construct a new CacheEntry with peak data between two offsets
 *  in the source.
 */
WaveView::CacheEntry::CacheEntry (
	WaveView const * wave_view,
	int start,
	int end
	)
	: _wave_view (wave_view)
	, _start (start)
	, _end (end)
{
	_n_peaks = _end - _start;
	_peaks = new PeakData[_n_peaks];

	_wave_view->_region->read_peaks (
		_peaks,
		_n_peaks,
		_start * _wave_view->_frames_per_pixel,
		(_end - _start) * _wave_view->_frames_per_pixel,
		_wave_view->_channel,
		_wave_view->_frames_per_pixel
		);
}

WaveView::CacheEntry::~CacheEntry ()
{
	delete[] _peaks;
}

Glib::RefPtr<Gdk::Pixbuf>
WaveView::CacheEntry::pixbuf ()
{
	if (!_pixbuf) {
		_pixbuf = Gdk::Pixbuf::create (Gdk::COLORSPACE_RGB, true, 8, _n_peaks, _wave_view->_height);
		Cairo::RefPtr<Cairo::ImageSurface> surface = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, _n_peaks, _wave_view->_height);
		Cairo::RefPtr<Cairo::Context> context = Cairo::Context::create (surface);

		_wave_view->setup_outline_context (context);
		context->move_to (0.5, position (_peaks[0].min));
		for (int i = 1; i < _n_peaks; ++i) {
			context->line_to (i + 0.5, position (_peaks[i].max));
		}
		context->stroke ();
		
		context->move_to (0.5, position (_peaks[0].min));
		for (int i = 1; i < _n_peaks; ++i) {
			context->line_to (i + 0.5, position (_peaks[i].min));
		}
		context->stroke ();

		set_source_rgba (context, _wave_view->_fill_color);
		for (int i = 0; i < _n_peaks; ++i) {
			context->move_to (i + 0.5, position (_peaks[i].max) - 1);
		 	context->line_to (i + 0.5, position (_peaks[i].min) + 1);
			context->stroke ();
		}

		Gtkmm2ext::convert_bgra_to_rgba (surface->get_data(), _pixbuf->get_pixels(), _n_peaks, _wave_view->_height);
	}

	return _pixbuf;
}


Coord
WaveView::CacheEntry::position (float s) const
{
	return (s + 1) * _wave_view->_height / 2;
}

void
WaveView::CacheEntry::clear_pixbuf ()
{
	_pixbuf.reset ();
}


		
