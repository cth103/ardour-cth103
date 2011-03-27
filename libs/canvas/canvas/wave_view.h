#include <boost/shared_ptr.hpp>
#include "ardour/types.h"
#include "canvas/item.h"

namespace ARDOUR {
	class AudioRegion;
}

#ifdef CANVAS_COMPATIBILITY	
class GnomeCanvasWaveViewCache {

};

extern void gnome_canvas_waveview_cache_destroy (GnomeCanvasWaveViewCache *);
#endif
	
namespace ArdourCanvas {

class WaveView : public Item
{
public:
	WaveView (Group *, boost::shared_ptr<ARDOUR::AudioRegion>);

	void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const;
	void compute_bounding_box () const;

	XMLNode* get_state () const;
	void set_state (XMLNode const *);

	void set_frames_per_pixel (double);
	void set_wave_color (uint32_t);

#ifdef CANVAS_COMPATIBILITY	
	static GnomeCanvasWaveViewCache* create_cache () {
		return 0;
	}
	
	void*& property_data_src () {
		return _foo_void;
	}
	void*& property_gain_src () {
		return _foo_void;
	}
	void*& property_cache () {
		return _foo_void;
	}
	bool& property_cache_updater () {
		return _foo_bool;
	}
	int& property_channel () {
		return _foo_int;
	}
	void*& property_length_function () {
		return _foo_void;
	}
	void*& property_sourcefile_length_function () {
		return _foo_void;
	}
	void*& property_peak_function () {
		return _foo_void;
	}
	void*& property_gain_function () {
		return _foo_void;
	}
	double& property_x () {
		return _foo_double;
	}
	bool& property_rectified () {
		return _foo_bool;
	}
	bool& property_logscaled () {
		return _foo_bool;
	}
	ARDOUR::framepos_t& property_region_start () {
		return _foo_framepos;
	}
	double& property_y () {
		return _foo_double;
	}
	double& property_height () {
		return _foo_double;
	}
	double& property_amplitude_above_axis () {
		return _foo_double;
	}
	uint32_t& property_clip_color () {
		return _foo_uint;
	}
	uint32_t& property_zero_color () {
		return _foo_uint;
	}
	uint32_t& property_fill_color () {
		return _foo_uint;
	}

private:
	void* _foo_void;
	bool _foo_bool;
	int _foo_int;
	uint32_t _foo_uint;
	double _foo_double;
	ARDOUR::framepos_t _foo_framepos;
#endif

	boost::shared_ptr<ARDOUR::AudioRegion> _region;
	double _frames_per_pixel;
	Coord _height;
	uint32_t _wave_color;
};

}
