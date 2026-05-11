# Getting Started with the SkyEngine

## Table of contents

1. [Installation](#1-installation)
    - [System Requirements](#11-system-requirements)
    - [Installation Steps](#12-installation-steps)
2. [Your First Project](#2-your-first-project)
    - [Creating your projec](#21-creating-your-project)
    - [Building your project](#22-building-your-project)
3. [Basic Script Layout](#3-basic-usage)
    - [Init Function](#31-init)
    - [Awake Function](#32-awake)
    - [Start Function](#33-start)
    - [Update Function](#34-update)
    - [UserFree Function](#35-userfree)
4. [Additional Resources](#4-additional-resources)

## 1. Installation

In this section, you will learn how to install the SkyEngine on your local system.

### 1.1 System Requirements

The engine will run on most operating systems, but has a set of dependencies:

- Your system needs to be compatible with **OpenGL 3.3** or later. 
- You need to have **SDL2** installed.
- You need to have **OpenAL** installed.

As of version 1.0 (Aug 19th, 2023), only Windows and Linux have working build systems. MacOSX users can write their own build system by recycling the **build.sh** file if they wish.

### 1.2 Installation Steps
These steps are the same for all operating systems.

1. Navigate to [the SkyEngine github repository](https://github.com/SkyGameEngine/engine).
2. Either `clone` the repository or download the source code as a zip.
3. The project has a number of files and directories. All users require the `/Engine` directory, as that is the source code of the engine, as well as the `create_project.py` file. For Windows users, you also need the `build.bat` file, and Linux users will need the `build.sh` file. It is wise to also keep the `debug.bat` or `debug.sh` files, as these will allow you to run your projects with `gdb` enabled.
4. You can delete all files not mentioned in the previous step.

## 2. Your First Project

This section describes how project creation and project building processes look in the SkyEngine.

### 2.1 Creating your project

After the installation is complete, run 
```
python create_project.py [project_name]
``` 
in the same directory where your build scripts are located. This should generate a new directory with your chosen `[project_name]`.

The layout of your directories should now be:

- Root directory
    - Engine directory
    - [project_name] directory
    - Build and Debug scripts

Inside your project directory, you will find a `main.cpp` file. You may change the name of the file if you wish.

### 2.2 Building your project

To build your project, simply run 
```
build.bat [project_name] main -f
``` 
from the root directory (same directory as the build script). Replace `build.bat` with `build.sh` for Linux. If you changed the name of `main.cpp` in the previous step, change the `main` in the command to that new name. Ex: if your project directory is called `my_project`, and you renamed `main.cpp` to `my_file.cpp`, you would run 
```
build.bat my_project my_file -f
```

Building your project generates a new `build` directory inside your project directory. An executable should appear inside of it. If it does not, there is likely a linking error. Go into the `build.bat` or `build.sh` script and make sure the include paths match your SDL and OpenAL include paths. 

When you run your game, you should see a splash screen that says "Made with the SkyEngine" (Note: You can remove the splashscreen by simply adding `#define NO_SPLASH_SCREEN` at the top of your main file). If you do not see it, or the game does not start, you are likely missing .dll files. Make sure you have an "SDL2.dll" and a "glew32.dll" in the build directory. 

#### 2.2.1 Incremental Builds

The command `build.bat [project_name] main -f` builds the whole project from scratch, including the engine itself. As such, you might notice it takes multiple seconds to build the empty project. Thankfully, you only need to do this once. 

After you build the project from scratch, you will notice that the build directory also has `.o` files, such as "OpenGL_renderer.o" and so on. These object files are the compiled engine files, and once you have compiled them, you can ommit the `-f` flag in the compilation command, and use this intead:

```
build.bat [project_name] main
```

This will significantly speed up the build time, as the renderer, UI system, and sound system no longer need to be recompiled. Any additional sets of the engine still need to be compiled every time, but they take up a negligible amount of time, and all empty projects will still take less than a second to compile with all the extra sets of the engine.


## 3. Basic Script Layout

The SkyEngine encapsulates the program entry point within engine code. This makes initialization of the engine and the underlying APIs very easy for the end user. In place of the main function, SkyEngine offers five callback functions: `Init`, `Awake`, `Start`, `Update`, and `UserFree`.

### 3.1 Init

The `Init` function runs **before the initialization** of most of the underlying APIs. As such, it is used for setting certain parameters that have to do with window and rendering initialization, such as:
- Window name
- Window size
- Windowed/Fullscreen
- Background clear color
- Screen-size in world coordinates

These parameters are passed as pointer values, and need to be dereferenced and set to the appropriate value. If you created a starter project as described in [the project creation section](#21-creating-your-project) of this article, you will have the `Init` function filled out with sample values. Simply change them to your desired values.

### 3.2 Awake

The `Awake` function runs **after the initialization** of the underlying APIs, but before the start of the game loop. This is the place that you should do your pre-allocation. 

If you have a relatively small game, it is recommended to pre-allocate your image sprites, audio sources and clips, and any entity state you may require. 

For larger projects, if you are using the Scene Manager, you should initialize it here, and call all individual scene Awake methods here. For a detailed look at how to use the Scene Manger, visit the [SceneManager Documentation Page](scene_managment.md)

### 3.3 Start

The `Start` function runs during the **first iteration** of the game loop. 

This is used to set any default state or run any logic that you might only want to execute once.

#### KeyboardState
The `Start` function is passed a `KeyboardState` parameter that holds two member variables: `state` and `prev_state`.

These hold a bitfield of keystates for the keyboard and mouse. Every bit that corresponds to a key that is pressed down will be set to `true`, and otherwise will be set to `false`. For the start method, the `prev_state` will contain only `false` values.

For more information on input, read the [Input Documentation page](input.md).

### 3.4 Update

The `Update` function runs during **every iteration except the first** of the game loop.

This is where the majority of your game logic should reside.

The `Update` method is passed a `KeyboardState` just like the start method. The `prev_state` member is set to the state of the keyboard during the previous frame, while the `state` member is set to the state of the keyboard during the current frame.

In addition to the `KeyboardState` parameter, the `Update` function also provides a `float` parameter that measures the delta time of the current frame in milliseconds. The measurement is taken from the start of the previous frame to the start of the current frame. It is as precise as the CPU of the machine allows, and as such can go to sub-millisecond precision, hence why it is stored as a float.

### 3.5 UserFree

The `UserFree` function runs **after the update loop, and before the API resources are deallocated**.

This function is meant for the user to call any free functions that they require. If you allocate image sprites in the `Awake` or `Start` function, it is good practice to free them here. The same applies to any other resources the user has allocated.

Note that because this function essentially runs right before the game process terminates, unfreed resources should automatically be freed by most modern operating systems, however it is not guaranteed, and it good practice to free your resources anyway.


## 4. Additional Resources:

There are two other major ways that you can learn to use the SkyEngine effectively.

1. The SkyEngine comes with a number of example projects that can be built with the default build system. You are welcome to explore these, and modify them in any way you like.
2. The [SkyEngine Documentation](index.md) provides a complete overlook of the engine features.

---
[Back to homepage](index.md)
