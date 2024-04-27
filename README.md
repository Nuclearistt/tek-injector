# TEK Injector
[![Discord](https://img.shields.io/discord/937821572285206659?style=flat-square&label=Discord&logo=discord&logoColor=white&color=7289DA)](https://discord.gg/JBUgcwvpfc)

## Overview

This program launches ARK: Survival Evolved or ARK: Survival Ascended and modifies behavior of certain Steam API and EOS SDK methods while not changing any game's files.

What it changes for both games:
+ Steam API is initialized with app ID 480 (Spacewar) if it cannot be initialized with original one, which allows running the game even if it's not owned on current Steam account
+ App and DLC ownership check methods will always return true
+ ISteamUtils::GetAppID will return the original app ID of the game even if it's actually 480
+ Current Steam account will be returned as app owner even if the game is used via Family Sharing

What it changes for ARK: Survival Evolved:
+ Server search filter is modified to not include BattlEye-protected servers (since BattlEye is not enabled on client due to its intolerance for game process modifications) and servers that don't have [TEK Wrapper](https://github.com/Nuclearistt/TEKWrapper) (due to servers without it still checking authentication token and DLC ownership)
+ Subscribed mods list instead of actually checking subscribtions searches mods folders in Mods directory located in root folder of the game (`..\..\..\Mods` relative to ShooterGame.exe), which allows recognizing and using mods even if game is not owned on current Steam account and cannot have subscriptions, as long as files are available
+ TODO: Add automatic mod downloads for servers via TEK Steam Client

What it changes for ARK: Survival Ascended:
+ When game is not owned on current Steam account, EOS_Connect authentication is done via an Epic Games account (you'll get a prompt in browser when launching game with it for the first time) so you can still use online subsystem and join servers

Besides, TEK Injector has a special treatment for `-high` command-line argument which will make it run the game with high process priority class.

## How to use

Standalone: download tek-injector.exe from [releases](https://github.com/Nuclearistt/tek-injector/releases), place it in your game's `ShooterGame\Binaries\Win64` directory and run it. All command-line arguments that are passed to TEK Injector will be forwarded to the game.

Embedded (for app developers): you may use TEK Injector right inside your process if you properly load its PE image (with deflating sections and filling import table), tek-injector.exe exports 2 functions that can be obtained via GetProcAddress:
```c
void launch_asa(const wchar_t *exePath, int argc, const wchar_t **argv);
void launch_ase(const wchar_t *exePath, int argc, const wchar_t **argv);
```
where `exePath` is path to the game executable (ArkAscended.exe and ShooterGame.exe respectively), and `argc` and `argv` provide the command-line arguments for game process. Keep in mind that you don't need to set first argument to executable path, TEK Injector does it on its own and appends arguments from `argv` after it, also these functions may block for up to 10 seconds to ensure that game process doesn't return an error after launching.

## How it works

TEK Injector creates game process in suspended state, then allocates memory in it and copies itself into it, then modifies main thread's context so it executes custom entry point from TEK Injector's image, which modifies import address tables of game executable to use its own function wrappers where necessary and then passes executions to game's real entry point which is not even aware that something happened before it. Running TEK Injector inside game process after simply copying it is possible due to making game-side code rely only on what is bundled inside the image and kernel32.dll (which resides at the same virtual address across all processes) without any other external dependencies.
