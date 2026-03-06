# Turok (Xbox 360) - Static Recompilation

Native PC port of Turok (2008, Propaganda Games) via static recompilation. PowerPC to x86-64, no emulator.

Built with [ReXGlue SDK](https://github.com/hedge-dev/ReXGlue) and [XenonRecomp](https://github.com/hedge-dev/XenonRecomp).

## Status

**Foundation** - Binary extracted, codegen config ready, project scaffold complete.

## Build

Requires: CMake 3.25+, Clang 18+, Ninja, ReXGlue SDK built.

```bash
# 1. Extract game files from your Turok disc image
extract-xiso -d game_files/ "path/to/Turok.iso"

# 2. Extract PE from XEX
python tools/extract_pe.py game_files/default.xex pe_image.bin

# 3. Run codegen
rexglue codegen config/turok_rexglue.toml

# 4. Build
cd project
cmake --preset win-amd64
cmake --build out/build/win-amd64 --config Release
```

## Settings

Xenia-style settings panel (Config > Settings) with tabs for Audio, Clock, GPU, GPU/D3D12, GPU/Shader, Input, Kernel, Keybinds, Log, Memory, Runtime, UI/D3D12, UI/Presenter, UI/Window, and Debug.

Settings persist to `turok_settings.toml`.

## Controls

Keyboard + XInput controller (both work simultaneously).

| Action | Key |
|--------|-----|
| Move | WASD / Arrows |
| A / B / X / Y | Z J / X K / C L / V |
| Start / Back | Enter / Esc |
| LB / RB | Q / E |
| LT / RT | 1 / 3 |
| Fullscreen | F11 |

## License

Project scaffold and tools under MIT. Game assets belong to their respective owners.
