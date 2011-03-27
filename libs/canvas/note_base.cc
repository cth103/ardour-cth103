#include "canvas/note_base.h"

using namespace ArdourCanvas;

const uint32_t NoteBase::midi_channel_colors[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

PBD::Signal1<void, NoteBase*> NoteBase::NoteBaseDeleted;
