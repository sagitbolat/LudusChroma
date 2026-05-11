#pragma once
#include "sky_structs.h"

struct Scene {

    void (*StartScene)(GameState*, KeyboardState*, double); // NOTE: The double is the dt
    void (*UpdateScene)(GameState*, KeyboardState*, double);
    void (*CloseScene)(GameState*, KeyboardState*, double);
    void (*FreeScene)(void);
};

struct SceneManager {
    Scene* scenes;
    int curr_scene; // NOTE: Stored as an id


    // NOTE: Allocates scene memeory
    void InitManager(int num_scenes) {
        scenes = (Scene*)malloc(sizeof(Scene) * num_scenes);
        curr_scene = 0;
    }



    // NOTE: Sets the function pointers for each scene
    // NOTE: The passed Awake function only runs during this Init 
    // (This should be done during the game's Awake function call).
    void AwakeScene(
        int scene_id,
        void (*MainAwake)(),
        void (*Start)(GameState*, KeyboardState*, double), 
        void (*Update)(GameState*, KeyboardState*, double), 
        void (*Close)(GameState*, KeyboardState*, double),
        void (*Free)() 
    ) {
        MainAwake();
        scenes[scene_id].StartScene = Start;
        scenes[scene_id].UpdateScene = Update;
        scenes[scene_id].CloseScene = Close;
        scenes[scene_id].FreeScene = Free;
    }

    // NOTE: Swaps from current scene to a new scene
    void SwitchScene(int scene_id, GameState* gs, KeyboardState* ks, double dt) {
        scenes[curr_scene].CloseScene(gs, ks, dt);
        curr_scene = scene_id;
        scenes[curr_scene].StartScene(gs, ks, dt);
    }

    // NOTE: Runs the Update method of the scene
    void SceneUpdate(GameState* gs, KeyboardState* ks, double dt) {
        scenes[curr_scene].UpdateScene(gs, ks, dt);
    }

    // NOTE: Should be called in UserFree(). Deallocs scene memory and runs the current scene's free method
    void FreeScene(int scene_id) {
        scenes[scene_id].FreeScene();
    }


    // NOTE: Should be called in UserFree after the FreeScenes are called
    void FreeManager() {
        free(scenes);
    }
};