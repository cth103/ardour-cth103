#include "canvas/canvas.h"
#include "canvas/rectangle.h"

using namespace ArdourCanvas;

int main ()
{
	ImageCanvas* c = dynamic_cast<ImageCanvas*> (Canvas::create_image ());
	Rectangle* r = new Rectangle (c->root ());
	r->set (Rect (0, 0, 256, 256));
	c->render (Rect (0, 0, 1024, 1024));
	c->write_to_png ("foo.png");
}
