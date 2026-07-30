#include <SDL3/SDL.h>
#include <SDL3/SDL_keycode.h>
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

static void displayGBACharacterData(GBACharacterData *character, uint8_t characterID, uint8_t *cssPrimaryCharacter, uint16_be *characterExp) {
	assert(characterID == GBA_CHARACTER_NEIL || characterID == GBA_CHARACTER_ELLA);
	char defaultCharacterName[5];
	strcpy(defaultCharacterName, characterID == GBA_CHARACTER_NEIL ? "Neil" : "Ella");

	ImGui::Text("%s's data", defaultCharacterName);

	char buf[128];
	sprintf(buf, "Name##%s", defaultCharacterName);
	ImGui::InputTextWithHint(buf,
			defaultCharacterName,
			character->name,
			10);

	// skin (unused)
	/*
	float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
	uint8_t skinCtr = character->icon;
	sprintf(buf, "##skinLeft%s", defaultCharacterName);
	if (ImGui::ArrowButton(buf, ImGuiDir_Left)) {
		skinCtr -= 2;
	}
	ImGui::SameLine(0.0f, spacing);
	ImGui::Text("%u", character->icon);
	ImGui::SameLine(0.0f, spacing);
	sprintf(buf, "##skinRight%s", defaultCharacterName);
    if (ImGui::ArrowButton(buf, ImGuiDir_Right)) {
		skinCtr += 2;
	}
	ImGui::SameLine(0.0f, spacing);
	ImGui::Text("skin");
	skinCtr %= (MAX_GBA_SAVE_PAIRS * 2);
	character->icon = skinCtr;
	*/

	// experience
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x / 3.f);
	uint16_t characterExpHost = get_be16(*characterExp);
	sprintf(buf, "Experience##%s", defaultCharacterName);
	ImGui::InputScalar(buf, ImGuiDataType_U16, &characterExpHost);
	set_be16(characterExp, characterExpHost);

	// lefty checkbox
	ImGui::SameLine();
	sprintf(buf, "make lefty##%s", defaultCharacterName);
	bool isLeftyBool = character->isLefty;
	ImGui::Checkbox(buf, &isLeftyBool);
	character->isLefty = isLeftyBool;

	ImGui::SameLine();

	// radio button to make this character primary
	sprintf(buf, "Make primary##%s", defaultCharacterName);
	if (ImGui::RadioButton(buf, *cssPrimaryCharacter == characterID)) {
		*cssPrimaryCharacter = characterID;
	}

	ImGui::PopItemWidth();

	// shot attributes
	// sprintf(buf, "Shot Attributes##%s", defaultCharacterName);
	ImGui::SeparatorText("Shot Attributes");

	if (ImGui::BeginTable("##ShotAttribThreeColumnLayout", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerH)) {
		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0); // Move to the first column
		ImGui::Text("Shot Type:");
		if (ImGui::RadioButton("Draw", character->shot.type == SHOTTYPE_DRAW)) {
				character->shot.type = SHOTTYPE_DRAW;
		}
		if (ImGui::RadioButton("Fade", character->shot.type == SHOTTYPE_FADE)) {
				character->shot.type = SHOTTYPE_FADE;
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

static void displayTaunts(GBATaunt *taunts, int numTaunts, const char *type) {
	ImGui::Text("%s", type);
	char buf[GBA_TAUNT_SIZE];
	for (int i = 0; i < numTaunts; ++i) {
		// change \x01 to \n for display, then revert back to \x01
		strReplace(buf, taunts[i].str, GBA_TAUNT_SIZE, '\x01', '\n');
		ImGui::InputTextMultiline(
				getControllerString(&taunts[i]),
				buf,
				0x40,
				ImVec2(0, 2 * ImGui::GetTextLineHeightWithSpacing())
				);
		strReplace(taunts[i].str, buf, GBA_TAUNT_SIZE, '\n', '\x01');
	}
}

static void displayClubs(uint16_be *clubData, const char *clubType) {
	ImGui::Text("%s", clubType);

	uint16_t clubMask = get_be16(*clubData);
	uint16_t iMask = 1;
	char buf[64];
	for (int i = 0; i < GBA_NUM_CUSTOM_CLUBS; ++i) {
		bool isCurrentClubUnlocked = (clubMask & iMask);
		sprintf(buf, "%s##%s", getCustomClubName(i), clubType);
		ImGui::Checkbox(buf, &isCurrentClubUnlocked);
		if (isCurrentClubUnlocked) {
			clubMask |= iMask;
		} else {
			clubMask &= ~iMask;
		}
		iMask <<= 1;
	}

	// add 'toggle all' button
	sprintf(buf, "Toggle all##%s", clubType);
	if (ImGui::Button(buf)) {
		if (clubMask == (uint16_t)(-1)) {
			clubMask = 0;
		} else {
			clubMask = (uint16_t)(-1);
		}
	}

	set_be16(clubData, clubMask);
}

static void displayGBASavePair(GBASavePair *pair) {
	if (ImGui::TreeNodeEx("Character Data", ImGuiTreeNodeFlags_DefaultOpen)) {
		// two column layout to display Neil and Ella's data
		if (ImGui::BeginTable("##CharacterTwoColumnLayout", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerH)) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); // Move to the first column
			displayGBACharacterData(&pair->neil,
					GBA_CHARACTER_NEIL,
					&pair->cssPrimaryCharacter,
					&pair->experience[GBA_CHARACTER_NEIL]);

			ImGui::TableSetColumnIndex(1); // Move to the second column
			displayGBACharacterData(&pair->ella,
					GBA_CHARACTER_ELLA,
					&pair->cssPrimaryCharacter,
					&pair->experience[GBA_CHARACTER_ELLA]);
			ImGui::EndTable();
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Taunt Data (for primary character)", ImGuiTreeNodeFlags_DefaultOpen)) {
		// add 'set default' button

		if (ImGui::Button("Set default##taunts")) {
			GBASetDefaultTaunts(pair);
		}

		if (ImGui::BeginTable("##TauntTwoColumnLayout", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerH)) {
			ImGui::TableNextRow();

			// taunts (0-3)
			ImGui::TableSetColumnIndex(0); // Move to the first column
			displayTaunts(pair->taunts, 4, "Taunts");

			// cheers (4-7)
			ImGui::TableSetColumnIndex(1); // Move to the second column
			displayTaunts(&pair->taunts[4], 4, "Cheers");
			ImGui::EndTable();
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Custom Club Data", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (ImGui::BeginTable("##ClubThreeColumnLayout", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerH)) {
			ImGui::TableNextRow();

			// woods
			ImGui::TableSetColumnIndex(0); // Move to the first column
			displayClubs(&pair->customWoodsBitmask, "Woods");

			// irons
			ImGui::TableSetColumnIndex(1); // Move to the second column
			displayClubs(&pair->customIronsBitmask, "Irons");
			
			// wedges
			ImGui::TableSetColumnIndex(2); // Move to the third column
			displayClubs(&pair->customWedgesBitmask, "Wedges");
			ImGui::EndTable();
		}

		ImGui::TreePop();
	}

	// add delete button
	// ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(255, 0, 0, 0));
	if (ImGui::Button("Delete This Pair")) {
		GBADeletePair(pair);
	}
	// ImGui::PopStyleColor();
}

static void handleInvalidArgs(char *programName) {
	fprintf(stderr, "Usage:\t%s\n\t\t%s d|e <infile> <outfile> [init_checksum]\n", programName, programName);
	exit(1);
}

static std::string getFileNameFromPath(std::string path) {
#if defined (_WIN32)
	size_t i = path.rfind('\\');
#else
	size_t i = path.rfind('/');
#endif

	if (i == std::string::npos) {
		return path;
	}
	return path.substr(i + 1, path.length() - i - 1);
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

	auto handleSaveFile = [
		&loadedGCIPair,
		&loadedGBAPairs,
		&savedGBAData
	](std::string filename) {
		loadedGCIPair.setDecodedData(&loadedGBAPairs, GBA_OFFSET_PAL, GBA_DATA_SIZE);
		memcpy(savedGBAData, loadedGBAPairs, GBA_DATA_SIZE);
		return loadedGCIPair.saveEncodedFile(filename);
	};

	// the main loop
    while (appState.running) {
		struct {
			bool open = 0;
			bool save = 0;
			bool saveAs = 0;
			bool close = 0;
			bool exit = 0;
		} uiAction;
		SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT) {
                appState.running = false;
            }

			// Key down (no repeat to avoid retrigger spam)
				if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
					// SDL3 modifier state from this event
					const SDL_Keymod mods = static_cast<SDL_Keymod>(event.key.mod);
					const bool ctrlOrCmd =
#if defined(__APPLE__)
						(mods & SDL_KMOD_GUI) != 0;   // Command on macOS
#else
						(mods & SDL_KMOD_CTRL) != 0;  // Ctrl on Windows/Linux
#endif

					// Optional: don't fire app shortcuts while ImGui wants keyboard
					ImGuiIO& io = ImGui::GetIO();
					if (io.WantCaptureKeyboard) {
						continue;
					}

					// Key symbol
					const SDL_Keycode key = event.key.key;

					// Ctrl/Cmd + O => Open
					if (ctrlOrCmd && key == SDLK_O) {
						uiAction.open = 1;
					}
					// Ctrl/Cmd + Shift + S => Save As
					else if (ctrlOrCmd && (mods & SDL_KMOD_SHIFT) && key == SDLK_S) {
						uiAction.saveAs = 1;
					}
					// Ctrl/Cmd + S => Save
					else if (ctrlOrCmd && key == SDLK_S) {
						uiAction.save = 1;
					}
					// Ctrl/Cmd + W => Close file
					else if (ctrlOrCmd && key == SDLK_W) {
						uiAction.close = 1;
					}
					// Ctrl/Cmd + Q => Exit
					else if (ctrlOrCmd && key == SDLK_Q) {
						uiAction.exit = 1;
					}
				}
        }

        // Consume dialog callback result (if any), safely
        {
            std::lock_guard<std::mutex> lock(dialogState.mtx);
			// check open file state
            if (dialogState.openHasResult) {
				// std::cerr << "open start" << std::endl;
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
				// std::cerr << "open end" << std::endl;
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
					uiAction.open = 1;
				}
				if (ImGui::MenuItem("Save", "Ctrl+S")) {
					uiAction.save = 1;
				}
				if (ImGui::MenuItem("Save As..")) {
					uiAction.saveAs = 1;
				}
				if (ImGui::MenuItem("Close", "Ctrl+W")) {
					uiAction.close = 1;
				}

				ImGui::Separator();
				if (ImGui::MenuItem("Exit", "Ctrl+Q")) {
					uiAction.exit = 1;
				}
				ImGui::EndMenu();
			}
			// if (ImGui::BeginMenu("Edit"))
			// {
			// 	if (ImGui::MenuItem("Undo", "Ctrl+Z")) { /* Handle action */ }
			// 	ImGui::EndMenu();
			// }
			ImGui::EndMainMenuBar();
		}

		// handle UI actions (triggered by menu or shortcut)
		if (uiAction.open) {
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
		if (uiAction.save) {
			if (loadedGCIPair.isInitSuccess()) {
				assert(openedPath.has_value());
				handleSaveFile(openedPath.value());
			}
		}
		if (uiAction.saveAs) {
			if (loadedGCIPair.isInitSuccess()) {
				assert(openedPath.has_value());
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
			} else {
				status = "No file opened";
			}
		}
		if (uiAction.close) {
			loadedGCIPair.close();
		}
		if (uiAction.exit) {
			appState.running = 0;
		}

		ImGuiWindowFlags mainWindowFlags = ImGuiWindowFlags_None;
		if (openedPath.has_value() && memcmp(&loadedGBAPairs, &savedGBAData, GBA_DATA_SIZE) != 0) {
			mainWindowFlags |= ImGuiWindowFlags_UnsavedDocument;
		}

		ImGui::Begin("Main Window", NULL, mainWindowFlags);
		if (loadedGCIPair.isInitSuccess()) {
			assert(openedPath.has_value());
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

				char summaryLabel[256];
				sprintf(summaryLabel, "Summary - %s", getFileNameFromPath(*openedPath).c_str());
				if (ImGui::BeginTabItem(summaryLabel)) {
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
