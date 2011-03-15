#include "canvas/unimplemented.h"

using namespace ArdourCanvas;

const uint32_t CanvasNoteEvent::midi_channel_colors[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

PBD::Signal1<void, CanvasNoteEvent*> CanvasNoteEvent::CanvasNoteEventDeleted;

void
gnome_canvas_waveview_cache_destroy (GnomeCanvasWaveViewCache* c)
{
	
}
