#include "ardour/session.h"
#include "ardour/tempo.h"
#include "canvas/bbt_lines.h"
#include "canvas/utils.h"

using namespace std;
using namespace ArdourCanvas;

BBTLines::BBTLines (Group* parent, Color bar_color, Color beat_color)
	: Item (parent)
	, Outline (parent)
	, _session (0)
	, _frames_per_pixel (0)
	, _bar_color (bar_color)
	, _beat_color (beat_color)
{
	_bbox.reset (Rect (0, 0, COORD_MAX, COORD_MAX));
	_bbox_dirty = false;
	set_outline_color (0xffffffff);
}

void
BBTLines::render (Rect const & area, Cairo::RefPtr<Cairo::Context> context) const
{
	if (!_session) {
		return;
	}

	setup_outline_context (context);

	/* Ask the tempo map for the points that we need to plot.  We subtract 0.5 from
	   the render area, since we will add 0.5 onto the x coordinates of the lines
	   to keep cairo from blurring them across 2 pixels.
	*/
	ARDOUR::TempoMap::BBTPointList const * points = _session->tempo_map().get_points (
		rint ((area.x0 - 0.5) * _frames_per_pixel),
		rint ((area.x1 - 0.5) * _frames_per_pixel) - 1
		);

	/* Work out the number of beats per pixel; the number of points minus the number of
	   bars in the points vector gives us the number of beats, and then we divide
	   that by the number of pixels that we are plotting this time.
	*/
	double const beats_per_pixel = (points->size() - (points->back().bar - points->front().bar)) / area.width ();

	if (beats_per_pixel > 0.4) {
		/* too dense; plot nothing */
		return;
	}
	
	for (ARDOUR::TempoMap::BBTPointList::const_iterator i = points->begin(); i != points->end(); ++i) {

		if (i->type == ARDOUR::TempoMap::Bar) {
			continue;
		}

		Coord const x = rint (i->frame / _frames_per_pixel) + 0.5;
		
		if (i->beat == 1) {
			set_source_rgba (context, _bar_color);
		} else {
			set_source_rgba (context, _beat_color);
			if (beats_per_pixel > 0.2) {
				/* beats too dense, so just plot bars */
				continue;
			}
		}
		
		context->move_to (x, area.y0);
		context->line_to (x, area.y1);
		context->stroke ();
	}
}

void
BBTLines::compute_bbox () const
{
	/* Our bounding box is always the same */
}

XMLNode*
BBTLines::get_state () const
{
	/* XXX */
	return new XMLNode ("XXX");
}

void
BBTLines::set_state (XMLNode const * node)
{
	/* XXX */
}

void
BBTLines::set_session (ARDOUR::Session* session)
{
	begin_change ();
	_session = session;
	end_change ();
}

void
BBTLines::set_frames_per_pixel (double frames_per_pixel)
{
	begin_change ();
	_frames_per_pixel = frames_per_pixel;
	end_change ();
}

void
BBTLines::tempo_map_changed ()
{
	begin_change ();
	end_change ();
}
