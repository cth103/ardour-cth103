#include "pbd/xml++.h"
#include "canvas/types.h"

extern double double_random ();
extern ArdourCanvas::Rect rect_random (double);

namespace ArdourCanvas {
	class ImageCanvas;
}

class Benchmark
{
public:
	Benchmark (std::string const &);
	virtual ~Benchmark () {}
	
	double run ();
	
	virtual void do_run (ArdourCanvas::ImageCanvas &) = 0;

private:
	ArdourCanvas::ImageCanvas* _canvas;
};
