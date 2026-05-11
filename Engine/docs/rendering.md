# Rendering

## Table Of Contents
- [2D Rendering](#2d-rendering)
    - [Initialization](#initialization)
    - [Loading an image file](#loading-an-image-file)
    - [Rendering an image](#rendering-an-image)
    - [Parameters for rendering](#parameters-for-rendering)

## 2D Rendering

Rendering 2D sprites is simple in SkyEngine. Some steps must be made during the initialization or deinitialization of the scene/game (Init/Awake and UserFree functions), while others can (and should) be done during the main gameloop (Update function). The following is the sample pipeline:

1. **(Init)** Set the window state.
2. **(Awake)** Init the vertex and fragment shaders and the GPU buffer objects.
3. **(Awake)** Load any sprite images you need.
4. **(Update, Optional)** set any shader uniforms that need to be set.
5. **(Update or Start/Awake)** Initialize a transform for the quad that will contain the sprite.
6. **(Update)** Draw the sprite.
7. **(UserFree)** Free the allocated sprites, shaders, and GPU buffers.

