#include "pbd/textreceiver.h"
#include "midi++/manager.h"
#include "ardour/session.h"
#include "ardour/audioengine.h"
#include "ardour/source_factory.h"
#include "ardour/audiosource.h"
#include "ardour/audiofilesource.h"
#include "ardour/region_factory.h"
#include "ardour/audioregion.h"
#include "canvas/wave_view.h"
#include "canvas/canvas.h"
#include "wave_view.h"

using namespace std;
using namespace PBD;
using namespace ARDOUR;
using namespace ArdourCanvas;

CPPUNIT_TEST_SUITE_REGISTRATION (WaveViewTest);

TextReceiver text_receiver ("test");

void
WaveViewTest::setUp ()
{
	_canvas = new ImageCanvas (Duple (256, 256));

	init (false, true);
	SessionEvent::create_per_thread_pool ("test", 512);

	text_receiver.listen_to (error);
	text_receiver.listen_to (info);
	text_receiver.listen_to (fatal);
	text_receiver.listen_to (warning);

	AudioFileSource::set_build_peakfiles (true);
	AudioFileSource::set_build_missing_peakfiles (true);

	AudioEngine engine ("test", "");
	MIDI::Manager::create (engine.jack ());
	CPPUNIT_ASSERT (engine.start () == 0);
	
	Session session (engine, "tmp_session", "tmp_session");
	engine.set_session (&session);

	char buf[256];
	getcwd (buf, sizeof (buf));
	string const path = string_compose ("%1/../../libs/canvas/test/sine.wav", buf);

	boost::shared_ptr<Source> source = SourceFactory::createReadable (
		DataType::AUDIO, session, path, 0, (Source::Flag) 0, false, true
		);

	boost::shared_ptr<AudioFileSource> audio_file_source = boost::dynamic_pointer_cast<AudioFileSource> (source);

	audio_file_source->setup_peakfile ();

	PBD::PropertyList properties;
	properties.add (Properties::position, 128);
	properties.add (Properties::length, audio_file_source->readable_length ());
	_region = RegionFactory::create (source, properties, false);
	boost::shared_ptr<AudioRegion> audio_region = boost::dynamic_pointer_cast<AudioRegion> (_region);

	_wave_view = new WaveView (_canvas->root(), audio_region);
	_wave_view->set_frames_per_pixel ((double) (44100 / 1000) / 64);
}

void
WaveViewTest::all ()
{
	basics ();
	cache ();
}

void
WaveViewTest::basics ()
{
	_canvas->render_to_image (Rect (0, 0, 256, 256));
	_canvas->write_to_png ("waveview.png");

	/* XXX: doesn't check the result! */
}

void
WaveViewTest::cache ()
{
	/* Whole of the render area needs caching from scratch */
	
	_wave_view->invalidate_cache ();
	
	Rect whole (0, 0, 256, 256);
	frameoffset_t start;
	frameoffset_t end;
	list<WaveView::CacheEntry*> render_list = _wave_view->make_render_list (whole, start, end);

	CPPUNIT_ASSERT (_wave_view->_cache.size() == 1);
	CPPUNIT_ASSERT (_wave_view->_cache.front()->start() == 0);
	CPPUNIT_ASSERT (_wave_view->_cache.front()->end() == 256 * _wave_view->_frames_per_pixel);

	_wave_view->invalidate_cache ();
	
	/* Render a bit in the middle */

	Rect part1 (128, 0, 196, 256);
	render_list = _wave_view->make_render_list (part1, start, end);

	CPPUNIT_ASSERT (_wave_view->_cache.size() == 1);
	CPPUNIT_ASSERT (_wave_view->_cache.front()->start() == 128 * _wave_view->_frames_per_pixel);
	CPPUNIT_ASSERT (_wave_view->_cache.front()->end() == ceil (196 * _wave_view->_frames_per_pixel));

	/* Now render the whole thing and check that the cache sorts itself out */

	render_list = _wave_view->make_render_list (whole, start, end);
	
	CPPUNIT_ASSERT (_wave_view->_cache.size() == 3);

	list<WaveView::CacheEntry*>::iterator i = _wave_view->_cache.begin ();
	
	CPPUNIT_ASSERT ((*i)->start() == 0);
	CPPUNIT_ASSERT ((*i)->end() == ceil (128 * _wave_view->_frames_per_pixel));
	++i;

	CPPUNIT_ASSERT ((*i)->start() == ceil (128 * _wave_view->_frames_per_pixel));
	CPPUNIT_ASSERT ((*i)->end() == ceil (196 * _wave_view->_frames_per_pixel));
	++i;

	CPPUNIT_ASSERT ((*i)->start() == ceil (196 * _wave_view->_frames_per_pixel));
	CPPUNIT_ASSERT ((*i)->end() == ceil (256 * _wave_view->_frames_per_pixel));
	++i;
}
