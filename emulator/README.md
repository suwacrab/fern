# Building

Only Windows is supported (due to main.cpp using win32 for opening files).
Requires lua 5.4, SDL2, and Clang installed.

```
lua54 build.lua <arguments>
```

Arguments:
- `compile`: Compiles program to `bin\fern.exe`
- `clean`:  Cleans all object files.
- `rebuild`: Cleans object files, then compiles.
- `build_release`: Compiles the program into the `release` folder.
- `build_zip`: Packages the `release\fern` folder into one suitable for distribution. 

With that in mind, if you want to build the emulator, `lua54 build.lua clean build_release build_zip` will build a working version at `release\fern`.

# Usage

`fern <source rom> <options>`

- `-vs`: enable vsync (not recommended atm!)

- `-g`: enable debugger

- `-v`: verbose error/warn logging

- `--help`: show help

Additionally, using `fern` with no options brings up a ROM open prompt.

## Controls

- Arrow keys: D-pad

- A button: `S`

- B button: `A`

- Select: `V`

- Start: `B`

- Enable debugger: `G`

you can exit the debugger by entering `r` in the command window.

