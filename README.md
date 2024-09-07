# Feather-Engine
Feather Engine is a 2D game engine written in C++ and OpenGL.

## Goal
The goal here is simple, build a custom 2D game engine capable of making games, practice skills and learn new stuff along the process.

It is important to note that this engine is not, and will not be for quite a while, production-ready. It is a learning tool and as such is not yet ready and optimized for use in production-quality products.

## Getting Started
To build the engine you will need Visual Studio 2022 installed.

<ins>**1. Download the repository:**</ins>

Start by cloning the repository with `git clone https://github.com/Tomahawk01/Feather-Engine` in your chosen directory.

<ins>**2. Configure the dependencies:**</ins>

Run the `Win-GenProjects.bat` file. This will generate a Visual Studio solution file for user's usage.

<ins>**3. Run the Engine:**</ins>

If you are using Visual Studio 2022 press `F5` key.

## Technologies used
* [SDL2](https://github.com/libsdl-org/SDL) for windowing and input functionality
* [Glad](https://glad.dav1d.de/) as OpenGL loader
* [glm](https://github.com/g-truc/glm) math library
* [soil](https://github.com/littlstar/soil) for editor embedded textures loading
* [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) for image loading
* [stb_truetype](https://github.com/nothings/stb/blob/master/stb_truetype.h) for characters rasterisation
* [entt](https://github.com/skypjack/entt) for Entity Component System
* [Box2D](https://github.com/erincatto/box2d) for a physics system
* [Lua](https://www.lua.org/download.html) scripting language
* [Premake](https://premake.github.io/) for project building automatization
* [Sol](https://github.com/ThePhD/sol2) to bind Lua functions, user types, etc. 
* [ImGui](https://github.com/ocornut/imgui) for editor GUI
* [tinyfiledialogs](https://sourceforge.net/projects/tinyfiledialogs) for file dialogs

## Games made using Feather Engine
To play any available game created by the developer of this engine check [Releases](https://github.com/Tomahawk01/Feather-Engine/releases) tab.

### Asteroids clone (Feather [v0.1](https://github.com/Tomahawk01/Feather-Engine/releases/tag/v0.1))

### *Description*
As a title says this is a remake of Asteroids game from 1979.
The main goal was to test capabilities of the engine at that point of time.
### *Images*

### *Contorls*
| **Key** | **Function** |
| ------- | ------------ |
| `W S`   | Move forward, backward |
| `A D`   | Rotate left, right |
| `Space` or `LMB` | Shoot |
| `Shift` | Speed up |
| `Enter` | Restart after Game Over |

this is WIP
