#ifndef RENEIL_SDL_UTILS_HPP
#define RENEIL_SDL_UTILS_HPP

#include <SDL3/SDL.h>

#include <mutex>
#include <string>

// ---------- App state shared with SDL file dialog callback ----------
// struct FileDialogState {
// 	std::mutex mtx;
// 	bool hasResult = false;          // callback fired
// 	bool canceled = false;
// 	std::string selectedPath;        // single-file selection
// 	std::string error;
// };

struct FileDialogState {
    std::mutex mtx;

    // Open dialog result
    bool openHasResult = false;
    bool openCanceled = false;
    std::string openSelectedPath;

    // Save dialog result
    bool saveHasResult = false;
    bool saveCanceled = false;
    std::string saveSelectedPath;

    std::string error;
};

void SDLCALL OpenFileDialogCallback(void* userdata, const char* const* filelist, int filter);
void SDLCALL SaveFileDialogCallback(void* userdata, const char* const* filelist, int filter);

void ImGuiSetDocking();

#endif
