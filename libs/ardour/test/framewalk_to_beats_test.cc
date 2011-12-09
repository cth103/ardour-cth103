#include "framewalk_to_beats_test.h"
#include "ardour/tempo.h"
#include "timecode/bbt_time.h"

CPPUNIT_TEST_SUITE_REGISTRATION (FramewalkToBeatsTest);

using namespace std;
using namespace ARDOUR;
using namespace Timecode;

void
FramewalkToBeatsTest::singleTempoTest ()
{
	int const sampling_rate = 48000;
	int const bpm = 120;

	double const frames_per_beat = (60 / double (bpm)) * double (sampling_rate);
	
	TempoMap map (sampling_rate);
	Tempo tempo (bpm);
	Meter meter (4, 4);

	map.add_meter (meter, BBT_Time (1, 1, 0));
	map.add_tempo (tempo, BBT_Time (1, 1, 0));

	/* Walk 1 beats-worth of frames from beat 3 */
	double r = map.framewalk_to_beats (frames_per_beat * 2, frames_per_beat * 1);
	CPPUNIT_ASSERT_EQUAL (r, 1.0);

	/* Walk 6 beats-worth of frames from beat 4 */
	r = map.framewalk_to_beats (frames_per_beat * 3, frames_per_beat * 6);
	CPPUNIT_ASSERT_EQUAL (r, 6.0);

	/* Walk 1.5 beats-worth of frames from beat 3 */
	r = map.framewalk_to_beats (frames_per_beat * 2, frames_per_beat * 1.5);
	CPPUNIT_ASSERT_EQUAL (r, 1.5);

	/* Walk 1.5 beats-worth of frames from beat 2.5 */
	r = map.framewalk_to_beats (frames_per_beat * 2.5, frames_per_beat * 1.5);
	CPPUNIT_ASSERT_EQUAL (r, 1.5);
}

void
FramewalkToBeatsTest::doubleTempoTest ()
{
	int const sampling_rate = 48000;

	TempoMap map (sampling_rate);
	Meter meter (4, 4);
	map.add_meter (meter, BBT_Time (1, 1, 0));

	/*
	  120bpm at bar 1, 240bpm at bar 4
	  
	  120bpm = 24e3 samples per beat
	  240bpm = 12e3 samples per beat
	*/
	

	/*
	  
	  120bpm                                                240bpm
	  0 beats                                               12 beats
	  0 frames                                              288e3 frames
	  |                 |                 |                 |                 |
	  | 1.1 1.2 1.3 1.4 | 2.1 2.2 2.3.2.4 | 3.1 3.2 3.3 3.4 | 4.1 4.2 4.3 4.4 |

	*/

	Tempo tempoA (120);
	map.add_tempo (tempoA, BBT_Time (1, 1, 0));
	Tempo tempoB (240);
	map.add_tempo (tempoB, BBT_Time (4, 1, 0));

	/* Now some tests */

	/* Walk 1 beat from 1.2 */
	double r = map.framewalk_to_beats (24e3, 24e3);
	CPPUNIT_ASSERT_EQUAL (r, 1.0);

	/* Walk 2 beats from 3.3 to 4.1 (over the tempo change) */
	r = map.framewalk_to_beats (264e3, (24e3 + 12e3));
	CPPUNIT_ASSERT_EQUAL (r, 2.0);

	/* Walk 2.5 beats from 3.3-and-a-half to 4.2 (over the tempo change) */
	r = map.framewalk_to_beats (264e3 - 12e3, (24e3 + 12e3 + 12e3));
	CPPUNIT_ASSERT_EQUAL (r, 2.5);

	/* Walk 3 beats from 3.4-and-a-half to 4.3-and-a-half (over the tempo change) */
	r = map.framewalk_to_beats (264e3 - 12e3, (24e3 + 12e3 + 12e3 + 6e3));
	CPPUNIT_ASSERT_EQUAL (r, 3.0);

	/* Walk 3.5 beats from 3.4-and-a-half to 4.4 (over the tempo change) */
	r = map.framewalk_to_beats (264e3 - 12e3, (24e3 + 12e3 + 12e3 + 12e3));
	CPPUNIT_ASSERT_EQUAL (r, 3.5);
}

