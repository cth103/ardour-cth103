#include "canvas/debug.h"

using namespace std;

uint64_t PBD::DEBUG::CanvasItems = PBD::new_debug_bit ("canvasitems");
uint64_t PBD::DEBUG::CanvasItemsDirtied = PBD::new_debug_bit ("canvasitemsdirtied");
