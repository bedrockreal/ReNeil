#include <SDL3/SDL.h>

#include "imgui.h"
#include "sdl_utils.hpp"

// SDL callback for open-file dialog (single select)
void SDLCALL OpenFileDialogCallback(void* userdata, const char* const* filelist, int filter) {
    (void)filter;
    auto* state = static_cast<FileDialogState*>(userdata);
    std::lock_guard<std::mutex> lock(state->mtx);

    state->openHasResult = true;
    state->openCanceled = false;
    state->openSelectedPath.clear();
    state->error.clear();

    if (!filelist) {
        state->openCanceled = true; // typically user canceled
        return;
    }

    // filelist is null-terminated; first element is selected file for single-select
    if (filelist[0]) {
        state->openSelectedPath = filelist[0];
    } else {
        state->openCanceled = true;
    }
}

void SDLCALL SaveFileDialogCallback(void* userdata, const char* const* filelist, int filter) {
    (void)filter;
    auto* state = static_cast<FileDialogState*>(userdata);

    std::lock_guard<std::mutex> lock(state->mtx);
    state->saveHasResult = true;
    state->saveCanceled = false;
    state->saveSelectedPath.clear();

    if (!filelist) {
        state->saveCanceled = true; // canceled
        return;
    }

    if (filelist[0]) {
        state->saveSelectedPath = filelist[0];
    } else {
        state->saveCanceled = true;
    }
}

void ImGuiSetDocking() {
	// per-frame docking setup
	ImGuiWindowFlags dockspaceFlags = ImGuiDockNodeFlags_None;

	ImGuiWindowFlags hostWindowFlags =
		ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNavFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	const ImGuiViewport* vp = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(vp->WorkPos);
	ImGui::SetNextWindowSize(vp->WorkSize);
	ImGui::SetNextWindowViewport(vp->ID);

	ImGui::Begin("DockSpaceHost", nullptr, hostWindowFlags);
	ImGui::PopStyleVar(3);

	ImGuiID dockspaceID = ImGui::GetID("MainDockSpace");
	ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);
	ImGui::End();
}
