/*
    Copyright (C) 2004-2011 Paul Davis

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <inttypes.h>

#include <cmath>
#include <cerrno>
#include <fstream>
#include <cstdlib>
#include <string>
#include <cstdio>
#include <locale.h>
#include <unistd.h>
#include <float.h>
#include <iomanip>

#include <glibmm.h>

#include "pbd/cartesian.h"
#include "pbd/convert.h"
#include "pbd/error.h"
#include "pbd/failed_constructor.h"
#include "pbd/xml++.h"
#include "pbd/enumwriter.h"

#include "evoral/Curve.hpp"

#include "ardour/audio_buffer.h"
#include "ardour/audio_buffer.h"
#include "ardour/buffer_set.h"
#include "ardour/pan_controllable.h"
#include "ardour/pannable.h"
#include "ardour/runtime_functions.h"
#include "ardour/session.h"
#include "ardour/utils.h"

#include "panner_2in2out.h"

#include "i18n.h"

#include "pbd/mathfix.h"

using namespace std;
using namespace ARDOUR;
using namespace PBD;

static PanPluginDescriptor _descriptor = {
        "Equal Power Stereo",
        2, 2,
        Panner2in2out::factory
};

extern "C" { PanPluginDescriptor* panner_descriptor () { return &_descriptor; } }

Panner2in2out::Panner2in2out (boost::shared_ptr<Pannable> p)
	: Panner (p)
{
        _pannable->pan_azimuth_control->set_value (0.5);
        _pannable->pan_width_control->set_value (1.0);

        /* LEFT SIGNAL, panned hard left */
        left[0] = 1.0;
        right[0] = 0.0;
        desired_left[0] = left_interp[0] = left[0];
        desired_right[0] = right_interp[0] = right[0];

        /* RIGHT SIGNAL, panned hard right */
        left[1] = 0;
        right[1] = 1.0;
        desired_left[1] = left_interp[1] = left[1];
        desired_right[1] = right_interp[1] = right[1];

        _pannable->pan_azimuth_control->Changed.connect_same_thread (*this, boost::bind (&Panner2in2out::update, this));
        _pannable->pan_width_control->Changed.connect_same_thread (*this, boost::bind (&Panner2in2out::update, this));
}

Panner2in2out::~Panner2in2out ()
{
}

double 
Panner2in2out::position () const
{
        return _pannable->pan_azimuth_control->get_value();
}

double 
Panner2in2out::width () const
{
        return _pannable->pan_width_control->get_value();
}

void
Panner2in2out::set_position (double p)
{
        if (clamp_position (p)) {
                _pannable->pan_azimuth_control->set_value (p);
        }
}

void
Panner2in2out::set_width (double p)
{
        if (clamp_width (p)) {
                _pannable->pan_width_control->set_value (p);
        }
}

void
Panner2in2out::update ()
{
        /* it would be very nice to split this out into a virtual function
           that can be accessed from BaseStereoPanner and used in do_distribute_automated().
           
           but the place where its used in do_distribute_automated() is a tight inner loop,
           and making "nframes" virtual function calls to compute values is an absurd
           overhead.
        */
        
        /* x == 0 => hard left = 180.0 degrees
           x == 1 => hard right = 0.0 degrees
        */
        
        float pos[2];
        const double width = _pannable->pan_width_control->get_value();
        const double direction_as_lr_fract = _pannable->pan_azimuth_control->get_value();

        cerr << "new pan values width=" << width << " LR = " << direction_as_lr_fract << endl;

        if (width < 0.0) {
                pos[0] = direction_as_lr_fract + (width/2.0); // left signal lr_fract
                pos[1] = direction_as_lr_fract - (width/2.0); // right signal lr_fract
        } else {
                pos[1] = direction_as_lr_fract + (width/2.0); // right signal lr_fract
                pos[0] = direction_as_lr_fract - (width/2.0); // left signal lr_fract
        }
        
        /* compute target gain coefficients for both input signals */
        
        float const pan_law_attenuation = -3.0f;
        float const scale = 2.0f - 4.0f * powf (10.0f,pan_law_attenuation/20.0f);
        float panR;
        float panL;
        
        /* left signal */
        
        panR = pos[0];
        panL = 1 - panR;
        desired_left[0] = panL * (scale * panL + 1.0f - scale);
        desired_right[0] = panR * (scale * panR + 1.0f - scale);
        
        /* right signal */
        
        panR = pos[1];
        panL = 1 - panR;
        desired_left[1] = panL * (scale * panL + 1.0f - scale);
        desired_right[1] = panR * (scale * panR + 1.0f - scale);
}

bool
Panner2in2out::clamp_position (double& p)
{
        double w = _pannable->pan_width_control->get_value();
        return clamp_stereo_pan (p, w);
}

bool
Panner2in2out::clamp_width (double& w)
{
        double p = _pannable->pan_azimuth_control->get_value();
        return clamp_stereo_pan (p, w);
}

bool
Panner2in2out::clamp_stereo_pan (double& direction_as_lr_fract, double& width)
{
        double r_pos = direction_as_lr_fract + (width/2.0);
        double l_pos = direction_as_lr_fract - (width/2.0);
        bool can_move_left = true;
        bool can_move_right = true;

        cerr << "Clamp pos = " << direction_as_lr_fract << " w = " << width << endl;

        if (width > 1.0 || width < 1.0) {
                return false;
        }

        if (direction_as_lr_fract > 1.0 || direction_as_lr_fract < 0.0) {
                return false;
        }

        if (width < 0.0) {
                swap (r_pos, l_pos);
        }

        /* if the new left position is less than or equal to zero (hard left) and the left panner
           is already there, we're not moving the left signal. 
        */
        
        if (l_pos <= 0.0 && desired_left[0] <= 0.0) {
                can_move_left = false;
        }

        /* if the new right position is less than or equal to 1.0 (hard right) and the right panner
           is already there, we're not moving the right signal. 
        */
        
        if (r_pos >= 1.0 && desired_right[1] >= 1.0) {
                can_move_right = false;
        }

        return can_move_left && can_move_right;
}

void
Panner2in2out::distribute_one (AudioBuffer& srcbuf, BufferSet& obufs, gain_t gain_coeff, pframes_t nframes, uint32_t which)
{
	assert (obufs.count().n_audio() == 2);

	pan_t delta;
	Sample* dst;
	pan_t pan;

	Sample* const src = srcbuf.data();
        
	/* LEFT OUTPUT */

	dst = obufs.get_audio(0).data();

	if (fabsf ((delta = (left[which] - desired_left[which]))) > 0.002) { // about 1 degree of arc

		/* we've moving the pan by an appreciable amount, so we must
		   interpolate over 64 frames or nframes, whichever is smaller */

		pframes_t const limit = min ((pframes_t) 64, nframes);
		pframes_t n;

		delta = -(delta / (float) (limit));

		for (n = 0; n < limit; n++) {
			left_interp[which] = left_interp[which] + delta;
			left[which] = left_interp[which] + 0.9 * (left[which] - left_interp[which]);
			dst[n] += src[n] * left[which] * gain_coeff;
		}

		/* then pan the rest of the buffer; no need for interpolation for this bit */

		pan = left[which] * gain_coeff;

		mix_buffers_with_gain (dst+n,src+n,nframes-n,pan);

	} else {

		left[which] = desired_left[which];
		left_interp[which] = left[which];

		if ((pan = (left[which] * gain_coeff)) != 1.0f) {

			if (pan != 0.0f) {

				/* pan is 1 but also not 0, so we must do it "properly" */

				mix_buffers_with_gain(dst,src,nframes,pan);

				/* mark that we wrote into the buffer */

				// obufs[0] = 0;

			}

		} else {

			/* pan is 1 so we can just copy the input samples straight in */

			mix_buffers_no_gain(dst,src,nframes);
                        
			/* XXX it would be nice to mark that we wrote into the buffer */
		}
	}

	/* RIGHT OUTPUT */

	dst = obufs.get_audio(1).data();

	if (fabsf ((delta = (right[which] - desired_right[which]))) > 0.002) { // about 1 degree of arc

		/* we're moving the pan by an appreciable amount, so we must
		   interpolate over 64 frames or nframes, whichever is smaller */

		pframes_t const limit = min ((pframes_t) 64, nframes);
		pframes_t n;

		delta = -(delta / (float) (limit));

		for (n = 0; n < limit; n++) {
			right_interp[which] = right_interp[which] + delta;
			right[which] = right_interp[which] + 0.9 * (right[which] - right_interp[which]);
			dst[n] += src[n] * right[which] * gain_coeff;
		}

		/* then pan the rest of the buffer, no need for interpolation for this bit */

		pan = right[which] * gain_coeff;

		mix_buffers_with_gain(dst+n,src+n,nframes-n,pan);

		/* XXX it would be nice to mark the buffer as written to */

	} else {

		right[which] = desired_right[which];
		right_interp[which] = right[which];

		if ((pan = (right[which] * gain_coeff)) != 1.0f) {

			if (pan != 0.0f) {

				/* pan is not 1 but also not 0, so we must do it "properly" */
				
				mix_buffers_with_gain(dst,src,nframes,pan);

				/* XXX it would be nice to mark the buffer as written to */
			}

		} else {

			/* pan is 1 so we can just copy the input samples straight in */
			
			mix_buffers_no_gain(dst,src,nframes);

			/* XXX it would be nice to mark the buffer as written to */
		}
	}
}

void
Panner2in2out::distribute_one_automated (AudioBuffer& srcbuf, BufferSet& obufs,
                                         framepos_t start, framepos_t end, pframes_t nframes,
                                         pan_t** buffers, uint32_t which)
{
	assert (obufs.count().n_audio() == 2);

	Sample* dst;
	pan_t* pbuf;
	Sample* const src = srcbuf.data();
        pan_t* const position = buffers[0];
        pan_t* const width = buffers[1];

	/* fetch positional data */

	if (!_pannable->pan_azimuth_control->list()->curve().rt_safe_get_vector (start, end, position, nframes)) {
		/* fallback */
                distribute_one (srcbuf, obufs, 1.0, nframes, which);
		return;
	}

	if (!_pannable->pan_width_control->list()->curve().rt_safe_get_vector (start, end, width, nframes)) {
		/* fallback */
                distribute_one (srcbuf, obufs, 1.0, nframes, which);
		return;
	}

	/* apply pan law to convert positional data into pan coefficients for
	   each buffer (output)
	*/

	const float pan_law_attenuation = -3.0f;
	const float scale = 2.0f - 4.0f * powf (10.0f,pan_law_attenuation/20.0f);

	for (pframes_t n = 0; n < nframes; ++n) {

                float panR;

                if (which == 0) { 
                        // panning left signal
                        panR = position[n] - (width[n]/2.0f); // center - width/2
                } else {
                        // panning right signal
                        panR = position[n] + (width[n]/2.0f); // center - width/2
                }

                const float panL = 1 - panR;

                /* note that are overwriting buffers, but its OK
                   because we're finished with their old contents
                   (position/width automation data) and are
                   replacing it with panning/gain coefficients 
                   that we need to actually process the data.
                */
                
                buffers[0][n] = panL * (scale * panL + 1.0f - scale);
                buffers[1][n] = panR * (scale * panR + 1.0f - scale);
        }

	/* LEFT OUTPUT */

	dst = obufs.get_audio(0).data();
	pbuf = buffers[0];

	for (pframes_t n = 0; n < nframes; ++n) {
		dst[n] += src[n] * pbuf[n];
	}

	/* XXX it would be nice to mark the buffer as written to */

	/* RIGHT OUTPUT */

	dst = obufs.get_audio(1).data();
	pbuf = buffers[1];

	for (pframes_t n = 0; n < nframes; ++n) {
		dst[n] += src[n] * pbuf[n];
	}

	/* XXX it would be nice to mark the buffer as written to */
}

Panner*
Panner2in2out::factory (boost::shared_ptr<Pannable> p, Speakers& /* ignored */)
{
	return new Panner2in2out (p);
}

XMLNode&
Panner2in2out::get_state (void)
{
	return state (true);
}

XMLNode&
Panner2in2out::state (bool /*full_state*/)
{
	XMLNode& root (Panner::get_state ());
	root.add_property (X_("type"), _descriptor.name);
	return root;
}

int
Panner2in2out::set_state (const XMLNode& node, int version)
{
	LocaleGuard lg (X_("POSIX"));
	Panner::set_state (node, version);
	return 0;
}
