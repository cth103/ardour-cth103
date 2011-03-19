#include <sys/time.h>
#include "canvas/group.h"
#include "canvas/canvas.h"
#include "canvas/root_group.h"
#include "canvas/rectangle.h"
#include "benchmark.h"

using namespace std;
using namespace ArdourCanvas;

static void
test (int items_per_cell)
{
	Group::default_items_per_cell = items_per_cell;
	
	int const n_rectangles = 1e5;
	int const n_tests = 100;
	double const rough_size = 1000;
	srand (1);

	ImageCanvas canvas (Duple (rough_size * 0.25, rough_size * 0.25));

	list<Item*> rectangles;

	for (int i = 0; i < n_rectangles; ++i) {
		rectangles.push_back (new Rectangle (canvas.root(), rect_random (rough_size)));
	}

	for (int i = 0; i < n_tests; ++i) {
		canvas.render_to_image (rect_random (rough_size));
	}
}

int main ()
{
	int tests[] = { 16, 32, 64, 128, 256, 1e3, 1e4, 1e5 };

	for (unsigned int i = 0; i < sizeof (tests) / sizeof (int); ++i) {
		timeval start;
		timeval stop;
		
		gettimeofday (&start, 0);
		test (tests[i]);
		gettimeofday (&stop, 0);

		int sec = stop.tv_sec - start.tv_sec;
		int usec = stop.tv_usec - start.tv_usec;
		if (usec < 0) {
			--sec;
			usec += 1e6;
		}

		double seconds = sec + ((double) usec / 1e6);

		cout << "Test " << tests[i] << ": " << seconds << "\n";
	}
}
	
