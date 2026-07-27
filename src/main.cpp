#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl3.h"
#include "imgui/backends/imgui_impl_sdlrenderer3.h"

#include <fstream>
#include <string>
#include <iostream>
#include <optional>

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
    std::optional<std::string> openedPath;
    std::string fileContent;
    std::string status = "No file opened.";

    // Fallback manual path input (in case you don't use SDL dialog API)
    static char pathBuffer[1024] = "";

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        // Start ImGui frame
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("File Controller");

        ImGui::TextWrapped("Simple Open/Close file demo.");

        // -------- Option A: manual path input + Open --------
        ImGui::InputText("Path", pathBuffer, sizeof(pathBuffer));
        if (ImGui::Button("Open File")) {
            std::string p = pathBuffer;
            if (!p.empty()) {
                std::string content = ReadTextFile(p);
                if (!content.empty() || std::ifstream(p).good()) {
                    openedPath = p;
                    fileContent = std::move(content);
                    status = "Opened: " + p;
                } else {
                    status = "Failed to open file: " + p;
                }
            } else {
                status = "Please enter a path first.";
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Close File")) {
            openedPath.reset();
            fileContent.clear();
            status = "File closed.";
        }

        ImGui::Separator();
        ImGui::Text("Status: %s", status.c_str());

        if (openedPath.has_value()) {
            ImGui::Text("Current file: %s", openedPath->c_str());
            ImGui::Separator();
            ImGui::Text("Content preview:");
            ImGui::BeginChild("preview", ImVec2(0, 300), true);
            ImGui::TextUnformatted(fileContent.c_str());
            ImGui::EndChild();
        } else {
            ImGui::TextDisabled("No file currently open.");
        }

        ImGui::End();

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
