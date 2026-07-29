# ReNeil

ReNeil is a Mario Golf: Toadstool Tour save file editor written in C and C++. It lets players customise Neil and Ella's stats and play as them without the need of the GBA link, and this is currently the only feature the graphical interface supports.

## IMPORTANT!

**This tool is highly experimental and currently works with the PAL version only.
You are advised to back up your save files before using this application to avoid data loss.**

## Building
1. Install [SDL3](https://github.com/libsdl-org/SDL/releases/tag/release-3.4.12) onto your system. This is a requirement by the application.
2. Clone this repository recursively: `git clone --recursive-submodules https://github.com/bedrockreal/reneil`
3. Run `make`

The executable is located at `build/reneil`.

## Usage

You can use this application in either CLI or GUI mode:
- CLI mode: `reneil d|e <infile> <outfile> [init_checksum]`
- GUI mode: `reneil` (without arguments)

Only the CLI mode is capable of converting between encoded and decoded save files:

    reneil d <in_gci_file> <out_decoded_gci_file> [init_checksum]   # for decoding
    reneil e <in_decoded_gci_file> <out_gci_file> [init_checksum]   # for encoding

Note that the decoded GCI file still contains XOR keys (at the start of each block) and block signatures (at the end of each block) for re-encoding purposes.

Here, `init_checksum` is a hard-coded 32-bit integer passed by the game when it decodes/encodes the save file's first block. For PAL, this number is fixed at `0x12345678`. Only CLI mode supports a custom `init_checksum`.

The GUI mode lets one edit the GCI file's data without needing to decode and re-encode it manually.
