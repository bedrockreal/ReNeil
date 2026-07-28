#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "gci_pair.hpp"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl3.h"
#include "imgui/backends/imgui_impl_sdlrenderer3.h"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>
#include <iostream>
#include <optional>

#include "sdl_utils.hpp"
extern "C" {
#include "boutiste.h"
#include "gba_data.h"
#include "gci.h"
}

#define GBA_OFFSET_PAL	0x9c78
#define GBA_DATA_SIZE	(GBA_SAVE_PAIR_SIZE) * (MAX_GBA_SAVE_PAIRS)

static void displayGBACharacterData(GBACharacterData *character, uint8_t characterID, uint8_t *cssPrimaryCharacter) {
	std::string defaultCharactername = (characterID == GBA_CHARACTER_NEIL ? "Neil" : "Ella");
	ImGui::Text("%s's data", defaultCharactername.c_str());
	ImGui::InputTextWithHint("Name",
			defaultCharactername.c_str(),
			character->name,
			10);

	// lefty checkbox
	bool isLeftyBool = character->isLefty;
	ImGui::Checkbox(("make lefty##" + defaultCharactername).c_str(), &isLeftyBool);
	character->isLefty = isLeftyBool;

	ImGui::SameLine();

	// radio button to make this character primary
	if (ImGui::RadioButton("Make primary", *cssPrimaryCharacter == characterID)) {
		*cssPrimaryCharacter = characterID;
	}

	// shot attributes
	ImGui::SeparatorText("Shot Attributes");

	if (ImGui::BeginTable("##ShotAttribThreeColumnLayout", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerH)) {
		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0); // Move to the first column
		ImGui::Text("Shot Type:");
		if (ImGui::RadioButton("Fade", character->shot.type == 0)) {
				character->shot.type = 0;
		}
		if (ImGui::RadioButton("Draw", character->shot.type == 1)) {
				character->shot.type = 1;
		}

		ImGui::TableSetColumnIndex(1); // Move to the second column
		// drive distance
		uint16_t driveDist = get_be16(character->driveDistance);
		ImGui::InputScalar("Drive", ImGuiDataType_U16, &driveDist);
		set_be16(&character->driveDistance, driveDist);

		// shot height
		ImGui::InputScalar("Height", ImGuiDataType_S8, &character->shot.height);

		// curve
		ImGui::InputScalar("Curve", ImGuiDataType_U8, &character->shot.curve);

		// impaact, control, spin
		ImGui::TableSetColumnIndex(2); // Move to the second column
		ImGui::InputScalar("Impact", ImGuiDataType_S8, &character->shot.impact);
		ImGui::InputScalar("Control", ImGuiDataType_S8, &character->shot.control);
		ImGui::InputScalar("Spin", ImGuiDataType_S8, &character->shot.spin);

		ImGui::EndTable();
	}




}

static void displayTaunts(GBATaunt *taunts, int numTaunts, std::string labelPrefix) {
	for (int i = 0; i < numTaunts; ++i) {
		ImGui::InputTextMultiline(
				getControllerString(&taunts[i]),
				taunts[i].str,
				0x40,
				ImVec2(0, ImGui::GetTextLineHeight() * 2.5)
				);
	}
}

static void displayClubs(uint16_be *clubsData) {
	uint16_t clubMask = get_be16(*clubsData);
	bool isClubUnlocked[GBA_NUM_CUSTOM_CLUBS];
	for (int i = 0; i < GBA_NUM_CUSTOM_CLUBS; ++i) {
		isClubUnlocked[i] = (clubMask & (1 << i));
	}

	for (int i = 0; i < GBA_NUM_CUSTOM_CLUBS; ++i) {
		ImGui::Checkbox(getCustomClubName(i), &isClubUnlocked[i]);
	}

	clubMask = 0;
	for (int i = 0; i < GBA_NUM_CUSTOM_CLUBS; ++i) {
		if (isClubUnlocked[i]) {
			clubMask += (1 << i);
		}
	}

	set_be16(clubsData, clubMask);
}

static void displayGBASavePair(GBASavePair *pair) {
	if (ImGui::TreeNodeEx("Character Data", ImGuiTreeNodeFlags_DefaultOpen)) {
		// two column layout to display Neil and Ella's data
		if (ImGui::BeginTable("##CharacterTwoColumnLayout", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerH)) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); // Move to the first column
			displayGBACharacterData(&pair->neil,
					GBA_CHARACTER_NEIL,
					&pair->cssPrimaryCharacter);

			ImGui::TableSetColumnIndex(1); // Move to the second column
			displayGBACharacterData(&pair->ella,
					GBA_CHARACTER_ELLA,
					&pair->cssPrimaryCharacter);
			ImGui::EndTable();
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Taunt Data (for primary character)", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (ImGui::BeginTable("##TauntTwoColumnLayout", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerH)) {
			ImGui::TableNextRow();

			// taunts (0-3)
			ImGui::TableSetColumnIndex(0); // Move to the first column
			displayTaunts(pair->taunts, 4, "Taunt");

			// cheers (4-7)
			ImGui::TableSetColumnIndex(1); // Move to the second column
			displayTaunts(&pair->taunts[4], 4, "Cheer");
			ImGui::EndTable();
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Custom Club Data", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (ImGui::BeginTable("##ClubThreeColumnLayout", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerH)) {
			ImGui::TableNextRow();

			// woods
			ImGui::TableSetColumnIndex(0); // Move to the first column
			ImGui::Text("Woods");
			displayClubs(&pair->customWoodsBitmask);

			// irons
			ImGui::TableSetColumnIndex(1); // Move to the second column
			ImGui::Text("Irons");
			displayClubs(&pair->customIronsBitmask);
			
			// wedges
			ImGui::TableSetColumnIndex(2); // Move to the third column
			ImGui::Text("Wedges");
			displayClubs(&pair->customWedgesBitmask);
			ImGui::EndTable();
		}

		ImGui::TreePop();
	}
}

static void handleInvalidArgs(char *programName) {
	fprintf(stderr, "Usage:\t%s\n\t\t%s d|e <infile> <outfile> [init_checksum]\n", programName, programName);
	exit(1);
}

int main(int argc, char** argv) {
	uint32_t initChecksum = 0x12345678;
	if (argc != 1) {
		// CLI Mode
		if (argc < 4 || argc > 5) {
			handleInvalidArgs(argv[0]);
		}

		if (argc == 5) {
			initChecksum = (uint32_t)strtol(argv[4], NULL, 16);
		}

		GCIFileType srcFileType;
		if (strcmp(argv[1], "d") == 0) {
			srcFileType = GCI_FILE_TYPE_ENCODED;
		} else if (strcmp(argv[1], "e") == 0) {
			srcFileType = GCI_FILE_TYPE_DECODED;
		} else {
			handleInvalidArgs(argv[0]);
		}

		GCIFile srcFile, destFile;
		assert(readGCIFile(&srcFile, argv[2]));
		assert(convertGCIFile(&destFile, &srcFile, initChecksum, srcFileType));
		assert(writeGCIFile(&destFile, argv[3]));

		return 0;
	}

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("ReNeil 0.0.1",
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
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable docking
    (void)io;
    ImGui::StyleColorsDark();

    // Setup platform/renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    FileDialogState dialogState;
    std::optional<std::string> openedPath;
    std::string status = "No file opened.";

    // Optional filters
    SDL_DialogFileFilter filters[] = {
        { "GameCube save file (.gci)", "gci" },
        { "All files", "*" }
    };

	// set up application state
	struct {
		bool running = 1;
		bool openDialogPending = 0;
		bool saveDialogPending = 0;
		bool isDecodedGCIActive = 0;
	} appState;

	// save data-related
	GCIPair loadedGCIPair;
	GBASavePair loadedGBAPairs[MAX_GBA_SAVE_PAIRS];
	uint8_t savedGBAData[GBA_DATA_SIZE];

	// lambdas
	auto handleOpenFile = [
		&loadedGCIPair,
		&loadedGBAPairs,
		&initChecksum,
		&openedPath,
		&savedGBAData
	](std::string filename) {
		loadedGCIPair.init(filename, initChecksum);
		bool ret = loadedGCIPair.isInitSuccess();
		if (ret) {
			openedPath = filename;
			loadedGCIPair.getDecodedData(loadedGBAPairs, GBA_OFFSET_PAL, GBA_DATA_SIZE);
			memcpy(savedGBAData, loadedGBAPairs, GBA_DATA_SIZE);
		}
		return ret;
	};

	auto handleSaveFile = [&loadedGCIPair, &loadedGBAPairs](std::string filename) {
		loadedGCIPair.setDecodedData(&loadedGBAPairs, GBA_OFFSET_PAL, GBA_DATA_SIZE);
		return loadedGCIPair.saveEncodedFile(filename);
	};

	// the main loop
    while (appState.running) {
		SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT) {
                appState.running = false;
            }
        }

        // Consume dialog callback result (if any), safely
        {
            std::lock_guard<std::mutex> lock(dialogState.mtx);
			// check open file state
            if (dialogState.openHasResult) {
                dialogState.openHasResult = false;
                appState.openDialogPending = false;

                if (dialogState.openCanceled) {
                    status = "Open file canceled.";
                } else if (!dialogState.openSelectedPath.empty()) {
                    const std::string p = dialogState.openSelectedPath;

					// read and decode GCI
					if (handleOpenFile(p)) {
						status = "Opened: " + p;
					} else {
                        status = "Failed to open: " + p;
                    }
                } else {
                    status = "No file selected.";
                }
            }

			// check save file state
			if (dialogState.saveHasResult) {
				dialogState.saveHasResult = false;
				appState.saveDialogPending = false;

				if (dialogState.saveCanceled) {
					status = "Save canceled.";
				} else if (!dialogState.saveSelectedPath.empty()) {
					const std::string outPath = dialogState.saveSelectedPath;
					if (handleSaveFile(outPath)) {
						openedPath = outPath; // optional: treat saved file as current file
						status = "Saved: " + outPath;
					} else {
						status = "Failed to save: " + outPath;
					}
				} else {
					status = "No save path selected.";
				}
			}
        }

        // Start ImGui frame
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

		ImGuiSetDocking();

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open", "Ctrl+O")) {
					if (!appState.openDialogPending) {
						appState.openDialogPending = true;
						status = "Opening open dialog...";

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
				if (ImGui::MenuItem("Save", "Ctrl+S")) {
					if (openedPath.has_value()) {
						handleSaveFile(openedPath.value());
					}
				}
				if (ImGui::MenuItem("Save As..")) {
					if (!appState.saveDialogPending) {
						appState.saveDialogPending = true;
						status = "Opening save dialog...";

						// default_location can be nullptr or a folder/file hint
						SDL_ShowSaveFileDialog(
							SaveFileDialogCallback,
							&dialogState,
							window,
							filters,
							static_cast<int>(SDL_arraysize(filters)),
							nullptr // default location
						);
					} else {
						status = "Dialog already open/pending.";
					}
				}
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

		ImGuiWindowFlags mainWindowFlags = ImGuiWindowFlags_None;
		if (openedPath.has_value() && memcmp(&loadedGBAPairs, &savedGBAData, GBA_DATA_SIZE) != 0) {
			mainWindowFlags |= ImGuiWindowFlags_UnsavedDocument;
		}

		ImGui::Begin("Main Window", NULL, mainWindowFlags);
		if (loadedGCIPair.isInitSuccess()) {
			if (ImGui::BeginTabBar("GBATabBar", ImGuiTabBarFlags_None)) {
				int activeGBAPairMask = 0;
				int numActiveGBAPairs = 0;
				int firstInactiveGBAPair = -1;
				for (int i = 0; i < MAX_GBA_SAVE_PAIRS; ++i) {
					if (isGBAPairActive(&loadedGBAPairs[i])) {
						activeGBAPairMask |= (1 << i);
						numActiveGBAPairs++;
					} else if (firstInactiveGBAPair == -1) {
						firstInactiveGBAPair = i;
					}
				}

				if (ImGui::BeginTabItem("Summary"))
                {
					ImGui::Text("Number of Active GBA Pairs: %d", numActiveGBAPairs);

					ImGui::BeginDisabled(numActiveGBAPairs == MAX_GBA_SAVE_PAIRS);
					if (ImGui::Button("Add Default Pair")) {
						assert(firstInactiveGBAPair != -1);
						GBAMakeDefaultPair(&loadedGBAPairs[firstInactiveGBAPair]);
					}
					ImGui::EndDisabled();
                    ImGui::EndTabItem();
                }

				for (int i = 0; i < MAX_GBA_SAVE_PAIRS; ++i) {
					if ((activeGBAPairMask & (1 << i)) != 0) {
						std::string curTabName = "Save Pair #" + std::to_string(i + 1);
						if (ImGui::BeginTabItem(curTabName.c_str())) {
							displayGBASavePair(&loadedGBAPairs[i]);
							ImGui::EndTabItem();
						}

						// add delete button
						if (ImGui::Button("Delete this pair")) {
							GBADeletePair(&loadedGBAPairs[i]);
						}
					}
				}

                ImGui::EndTabBar();
			}
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
