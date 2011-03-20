#include <sys/time.h>
#include "pbd/compose.h"
#include "pbd/xml++.h"
#include "canvas/group.h"
#include "canvas/canvas.h"
#include "canvas/root_group.h"
#include "canvas/rectangle.h"
#include "benchmark.h"

using namespace std;
using namespace ArdourCanvas;

void
test (string path, int items_per_cell)
{
	Group::default_items_per_cell = items_per_cell;
	
	ImageCanvas canvas (new XMLTree (path.c_str()), Duple (4096, 1024));

	timeval start;
	timeval stop;
	
	gettimeofday (&start, 0);
	
	for (int i = 0; i < 1e4; i += 50) {
		canvas.render_to_image (Rect (i, 0, i + 50, 1024));
	}
		
	gettimeofday (&stop, 0);
	
	int sec = stop.tv_sec - start.tv_sec;
	int usec = stop.tv_usec - start.tv_usec;
	if (usec < 0) {
		--sec;
		usec += 1e6;
	}
	
	double seconds = sec + ((double) usec / 1e6);
	
	cout << "render_parts; ipc=" << items_per_cell << ": " << seconds << "\n";
}

int main (int argc, char* argv[])
{
	if (argc < 2) {
		cerr << "Syntax: render_parts <session>\n";
		exit (EXIT_FAILURE);
	}

	string path = string_compose ("../../libs/canvas/benchmark/sessions/%1.xml", argv[1]);

	int tests[] = { 16, 32, 64, 128, 256, 512, 1024, 1e4, 1e5, 1e6 };

	for (unsigned int i = 0; i < sizeof (tests) / sizeof (int); ++i) {
		test (path, tests[i]);
	}

	return 0;
}

	
