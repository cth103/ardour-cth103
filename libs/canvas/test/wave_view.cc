#include "pbd/textreceiver.h"
#include "midi++/manager.h"
#include "ardour/session.h"
#include "ardour/audioengine.h"
#include "ardour/source_factory.h"
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
WaveViewTest::basics ()
{
	ImageCanvas canvas (Duple (256, 256));

	init (false, true);
	SessionEvent::create_per_thread_pool ("test", 512);

	text_receiver.listen_to (error);
	text_receiver.listen_to (info);
	text_receiver.listen_to (fatal);
	text_receiver.listen_to (warning);

	AudioEngine engine ("test", "");
	MIDI::Manager::create (engine.jack ());
	CPPUNIT_ASSERT (engine.start () == 0);
	
	Session session (engine, "tmp_session", "tmp_session");
	engine.set_session (&session);

	char buf[256];
	getcwd (buf, sizeof (buf));
	string const path = string_compose ("%1/../../libs/canvas/test/sine.wav", buf);

	boost::shared_ptr<Source> source = SourceFactory::createReadable (
		DataType::AUDIO, session, path, 0, (Source::Flag) 0, false, false
		);

	PBD::PropertyList properties;
	boost::shared_ptr<Region> region = RegionFactory::create (source, properties, false);
	boost::shared_ptr<AudioRegion> audio_region = boost::dynamic_pointer_cast<AudioRegion> (region);

	WaveView wave_view (canvas.root(), audio_region);

	canvas.render_to_image (Rect (0, 0, 256, 256));
	canvas.write_to_png ("waveview.png");
}
