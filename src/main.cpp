#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl3.h"
#include "imgui/backends/imgui_impl_sdlrenderer3.h"

#include <mutex>
#include <string>
#include <iostream>
#include <optional>

#include "sdl_utils.hpp"
extern "C" {
#include "gci.h"
}

GCISaveFile encodedGCIFile, decodedGCIFile;

int main(int, char**) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("SDL3 + ImGui File Open/Close",
                                          900, 600,
                                          SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    // Setup platform/renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

	bool running = true;
    bool openDialogPending = false;

    FileDialogState dialogState;
    std::optional<std::string> openedPath;
    std::string fileContent;
    std::string status = "No file opened.";

    // Optional filters
    SDL_DialogFileFilter filters[] = {
        { "GameCube save file (.gci)", "gci" },
        { "All files", "*" }
    };

    while (running) {
		SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        // Consume dialog callback result (if any), safely
        {
            std::lock_guard<std::mutex> lock(dialogState.mtx);
            if (dialogState.hasResult) {
                dialogState.hasResult = false;
                openDialogPending = false;

                if (dialogState.canceled) {
                    status = "Open file canceled.";
                } else if (!dialogState.selectedPath.empty()) {
                    const std::string p = dialogState.selectedPath;
					// TODO
                    // std::string content = ReadTextFile(p);

                    // if (!content.empty() || std::ifstream(p).good()) {
                    //     openedPath = p;
                    //     fileContent = std::move(content);
                    //     status = "Opened: " + p;
                    // } else {
                    //     status = "Failed to open: " + p;
                    // }
                } else {
                    status = "No file selected.";
                }
            }
        }

        // Start ImGui frame
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New", "Ctrl+N")) { /* Handle action */ }
				if (ImGui::MenuItem("Open", "Ctrl+O")) {
					if (!openDialogPending) {
						openDialogPending = true;
						status = "Opening native file dialog...";

						// Asynchronous native open dialog
						// Parameters:
						// callback, userdata, parent window, filters, num filters,
						// default location, allow_many
						SDL_ShowOpenFileDialog(
							OpenFileDialogCallback,
							&dialogState,
							window,
							filters,
							static_cast<int>(SDL_arraysize(filters)),
							nullptr,
							false
						);
					} else {
						status = "Dialog already open/pending.";
					}
				}
				if (ImGui::MenuItem("Save", "Ctrl+S")) {}
			if (ImGui::MenuItem("Save As..")) {}
				ImGui::Separator();
				if (ImGui::MenuItem("Exit", "Ctrl+Q")) { /* Handle action */ }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "Ctrl+Z")) { /* Handle action */ }
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}


        // Render
        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
