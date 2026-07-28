# ReNeil

ReNeil is a Mario Golf: Toadstool Tour save file editor written in C and C++.
It lets players customise Neil and Ella's stats and play as them
without the need of the GBA link.

## IMPORTANT!

**This tool is highly experimental and currently works with the PAL version only.
You are adviced to back up you save files before using this application to avoid data loss.**

## Building

1. Install [SDL3](https://github.com/libsdl-org/SDL/releases/tag/release-3.4.12) onto your system. This is a requirement by the application.
2. Build the `lib/libimgui.a` static library, if the included one does not work.
3. Run `make`

The executable is located at `build/reneil`.
