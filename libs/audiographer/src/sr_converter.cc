#include "audiographer/sr_converter.h"
#include "audiographer/exception.h"

#include <cmath>
#include <cstring>
#include <boost/format.hpp>

#define ENABLE_DEBUG 0

#if ENABLE_DEBUG
	#include <iostream>
	#define DEBUG(str) std::cout << str << std::endl;
#else
	#define DEBUG(str)
#endif

namespace AudioGrapher
{
using boost::format;
using boost::str;

SampleRateConverter::SampleRateConverter (uint32_t channels)
  : active (false)
  , channels (channels)
  , max_frames_in(0)
  , leftover_data (0)
  , leftover_frames (0)
  , max_leftover_frames (0)
  , data_out (0)
  , data_out_size (0)
  , src_state (0)
{
}

void
SampleRateConverter::init (nframes_t in_rate, nframes_t out_rate, int quality)
{
	reset();
	
	if (in_rate == out_rate) {
		src_data.src_ratio = 1;
		return;
	}

	active = true;
	int err;
	if ((src_state = src_new (quality, channels, &err)) == 0) {
		throw Exception (*this, str (format ("Cannot initialize sample rate converter: %1%") % src_strerror (err)));
	}

	src_data.src_ratio = (double) out_rate / (double) in_rate;
}

SampleRateConverter::~SampleRateConverter ()
{
	reset();
}

nframes_t
SampleRateConverter::allocate_buffers (nframes_t max_frames)
{
	if (!active) { return max_frames; }
	
	nframes_t max_frames_out = (nframes_t) ceil (max_frames * src_data.src_ratio);
	if (data_out_size < max_frames_out) {

		delete[] data_out;
		data_out = new float[max_frames_out];
		src_data.data_out = data_out;

		max_leftover_frames = 4 * max_frames;
		leftover_data = (float *) realloc (leftover_data, max_leftover_frames * sizeof (float));
		if (!leftover_data) {
			throw Exception (*this, "A memory allocation error occured");
		}

		max_frames_in = max_frames;
		data_out_size = max_frames_out;
	}
	
	return max_frames_out;
}

void
SampleRateConverter::process (ProcessContext<float> const & c)
{
	if (!active) {
		output (c);
		return;
	}

	nframes_t frames = c.frames();
	float * in = const_cast<float *> (c.data()); // TODO check if this is safe!

	if (frames > max_frames_in) {
		throw Exception (*this, str (format (
			"process() called with too many frames, %1% instead of %2%")
			% frames % max_frames_in));
	}
	
	if (frames % channels != 0) {
		throw Exception (*this, boost::str (boost::format (
			"Number of frames given to process() was not a multiple of channels: %1% frames with %2% channels")
			% frames % channels));
	}

	int err;
	bool first_time = true;

	do {
		src_data.output_frames = data_out_size / channels;
		src_data.data_out = data_out;

		if (leftover_frames > 0) {

			/* input data will be in leftover_data rather than data_in */

			src_data.data_in = leftover_data;

			if (first_time) {

				/* first time, append new data from data_in into the leftover_data buffer */

				memcpy (&leftover_data [leftover_frames * channels], in, frames * sizeof(float));
				src_data.input_frames = frames + leftover_frames;
			} else {

				/* otherwise, just use whatever is still left in leftover_data; the contents
					were adjusted using memmove() right after the last SRC call (see
					below)
				*/

				src_data.input_frames = leftover_frames;
			}

		} else {
			src_data.data_in = in;
			src_data.input_frames = frames / channels;
		}

		first_time = false;

		DEBUG ("data_in: " << src_data.data_in);
		DEBUG ("input_frames: " << src_data.input_frames);
		DEBUG ("data_out: " << src_data.data_out);
		DEBUG ("output_frames: " << src_data.output_frames);
		
		if ((err = src_process (src_state, &src_data)) != 0) {
			throw Exception (*this, str (format ("An error occured during sample rate conversion: %1%") % src_strerror (err)));
		}

		leftover_frames = src_data.input_frames - src_data.input_frames_used;

		if (leftover_frames > 0) {
			if (leftover_frames > max_leftover_frames) {
				throw Exception(*this, "leftover frames overflowed");
			}
			memmove (leftover_data, (char *) &src_data.data_in[src_data.input_frames_used * channels],
			         leftover_frames * channels * sizeof(float));
		}

		ProcessContext<float> c_out (c, data_out, src_data.output_frames_gen * channels);
		if (!src_data.end_of_input || leftover_frames) {
			c_out.remove_flag (ProcessContext<float>::EndOfInput);
		}
		output (c_out);

		DEBUG ("src_data.output_frames_gen: " << src_data.output_frames_gen << ", leftover_frames: " << leftover_frames);

		if (src_data.output_frames_gen == 0 && leftover_frames) { throw Exception (*this, boost::str (boost::format (
			"No output frames genereated with %1% leftover frames")
			% leftover_frames)); }
		
	} while (leftover_frames > frames);
	
	// src_data.end_of_input has to be checked to prevent infinite recursion
	if (!src_data.end_of_input && c.has_flag(ProcessContext<float>::EndOfInput)) {
		set_end_of_input (c);
	}
}

void SampleRateConverter::set_end_of_input (ProcessContext<float> const & c)
{
	src_data.end_of_input = true;
	
	float f;
	ProcessContext<float> const dummy (c, &f, 0, channels);
	
	/* No idea why this has to be done twice for all data to be written,
	 * but that just seems to be the way it is...
	 */
	process (dummy);
	process (dummy);
}


void SampleRateConverter::reset ()
{
	active = false;
	max_frames_in = 0;
	src_data.end_of_input = false;
	
	if (src_state) {
		src_delete (src_state);
	}
	
	leftover_frames = 0;
	max_leftover_frames = 0;
	if (leftover_data) {
		free (leftover_data);
	}
	
	data_out_size = 0;
	delete [] data_out;
	data_out = 0;
}

} // namespace
