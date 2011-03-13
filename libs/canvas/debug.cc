#include "canvas/debug.h"

using namespace ArdourCanvas;

Debug* Debug::_instance = 0;

Debug::Debug ()
{

}

Debug *
Debug::instance ()
{
	if (_instance == 0) {
		_instance = new Debug;
	}

	return _instance;
}
