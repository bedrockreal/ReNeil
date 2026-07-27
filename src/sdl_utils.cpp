#include <SDL3/SDL.h>

#include "sdl_utils.hpp"

// SDL callback for open-file dialog (single select)
void SDLCALL OpenFileDialogCallback(void* userdata, const char* const* filelist, int filter) {
    (void)filter;
    auto* state = static_cast<FileDialogState*>(userdata);
    std::lock_guard<std::mutex> lock(state->mtx);

    state->hasResult = true;
    state->canceled = false;
    state->selectedPath.clear();
    state->error.clear();

    if (!filelist) {
        state->canceled = true; // typically user canceled
        return;
    }

    // filelist is null-terminated; first element is selected file for single-select
    if (filelist[0]) {
        state->selectedPath = filelist[0];
    } else {
        state->canceled = true;
    }
}
