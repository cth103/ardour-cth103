#include <sys/time.h>
#include "pbd/xml++.h"
#include "pbd/compose.h"
#include "canvas/canvas.h"
#include "canvas/types.h"

using namespace std;
using namespace ArdourCanvas;

int main (int argc, char* argv[])
{
	if (argc < 2) {
		cerr << "Syntax: session <session-name>\n";
		exit (EXIT_FAILURE);
	}

	string path = string_compose ("../../libs/canvas/benchmark/sessions/%1.xml", argv[1]);
	
	ImageCanvas canvas (new XMLTree (path.c_str()), Duple (4096, 1024));
	canvas.render_to_image (Rect (0, 0, 4096, 1024));
	canvas.write_to_png ("session.png");

	return 0;
}

	
