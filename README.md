# TEK Injector
[![Discord](https://img.shields.io/discord/937821572285206659?style=flat-square&label=Discord&logo=discord&logoColor=white&color=7289DA)](https://discord.gg/JBUgcwvpfc)

tek-injector is a C/C++ library and program built on top of it for starting a game process and injecting [tek-game-runtime](https://github.com/teknology-hub/tek-game-runtime) into it.

## Using

### Executable (for regular users)
Get `tek-injector.exe` from [releases](https://github.com/teknology-hub/tek-injector/releases), get [tek-game-runtime](https://github.com/teknology-hub/tek-game-runtime/releases), and create a valid settings file for it (see tek-game-runtime repo for details).

In the simplest use case, you can just place `libtek-game-runtime.dll` and `tek-gr-settings.json` (settings file *must* have this name) next to game's executable, then run `tek-injector.exe` and select game's executable in the dialog.

For more advanced uses, the following command-line options are available:
|Option|Description|
|-|-|
|`--ti-exe-path "C:\path\to\game.exe"`|Path to the game executable so you don't have to select it manually|
|`--ti-current-dir "C:\path\to\current\dir"`|Path to current directory for the game process. If not specified, game executable's parent directory is used|
|`--ti-dll-path "C:\path\to\libtek-game-runtime.dll"`|Path to the tek-game-runtime DLL to inject. If not specified, [Windows' standard DLL search order](https://learn.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-search-order#standard-search-order-for-unpackaged-apps) relative to game process is used|
|`--ti-settings-path "C:\path\to\tek-gr-settings.json"`|Path to the settings file that tek-game-runtime should load. If not specified, it'll look for it in game's current directory|
|`--ti-high-priority`|Run game process with high priority (via `HIGH_PRIORITY_CLASS` flag)|
|`--ti-run-as-admin`|Run game process with admin privileges if tek-injector.exe itself is elevated. By default, it would still run the game without admin privileges, to avoid related issues|
All other command-line options not listed here are forwarded to the game process as-is.

### Library (for developers)

The library comes both in static `libtek-injector.a` and dynamic (`libtek-injector.dll`/`libtek-injector.dll.a`) falvors. [tek-injector.h](https://github.com/teknology-hub/tek-injector/blob/main/include/tek-injector.h) declares a single function, `tek_inj_run_game`, that you can use with a filled `tek_inj_game_args` structure to run the game the way you need.
