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

class RenderParts : public Benchmark
{
public:
	RenderParts (string const & session) : Benchmark (session) {}
	
	void do_run (ImageCanvas& canvas)
	{
		for (int i = 0; i < 1e4; i += 50) {
			canvas.render_to_image (Rect (i, 0, i + 50, 1024));
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

	RenderParts render_parts (argv[1]);
	cout << render_parts.run () << "\n";

	return 0;
}

	
