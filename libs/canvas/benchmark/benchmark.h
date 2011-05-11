#include "pbd/xml++.h"
#include "canvas/types.h"

extern double double_random ();
extern Canvas::Rect rect_random (double);

namespace Canvas {
	class ImageCanvas;
}

class Benchmark
{
public:
	Benchmark (std::string const &);
	virtual ~Benchmark () {}

	void set_iterations (int);
	double run ();
	
	virtual void do_run (Canvas::ImageCanvas &) = 0;
	virtual void finish (Canvas::ImageCanvas &) {}

private:
	Canvas::ImageCanvas* _canvas;
	int _iterations;
};
