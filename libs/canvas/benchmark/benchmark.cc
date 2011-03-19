#include "canvas/types.h"

using namespace ArdourCanvas;

double
double_random ()
{
	return ((double) rand() / RAND_MAX);
}

ArdourCanvas::Rect
rect_random (double rough_size)
{
	double const x = double_random () * rough_size / 2;
	double const y = double_random () * rough_size / 2;
	double const w = double_random () * rough_size / 2;
	double const h = double_random () * rough_size / 2;
	return Rect (x, y, x + w, y + h);
}
