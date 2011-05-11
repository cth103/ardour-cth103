#include <sys/time.h>
#include <pangomm/init.h>
#include "pbd/compose.h"
#include "pbd/xml++.h"
#include "canvas/group.h"
#include "canvas/canvas.h"
#include "canvas/root_group.h"
#include "canvas/rectangle.h"
#include "benchmark.h"

using namespace std;
using namespace Canvas;

class RenderFromLog : public Benchmark
{
public:
	RenderFromLog (string const & session) : Benchmark (session) {}

	void do_run (ImageCanvas& canvas)
	{
		canvas.set_log_renders (false);

		list<Rect> const & renders = canvas.renders ();
		
		for (list<Rect>::const_iterator i = renders.begin(); i != renders.end(); ++i) {
			canvas.render_to_image (*i);
		}
	}
};

int main (int argc, char* argv[])
{
	if (argc < 2) {
		cerr << "Syntax: render_parts <session>\n";
		exit (EXIT_FAILURE);
	}

	Pango::init ();

	RenderFromLog render_from_log (argv[1]);
	cout << render_from_log.run () << "\n";

	return 0;
}

	
