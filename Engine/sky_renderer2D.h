#pragma once

#include "sky_structs.h"




// SECTION: Shaders.
// NOTE: Returns the shader ID. Calling with no arguments will use the default shaders.
GL_ID* ShaderInit(const char* vertex_path = nullptr, const char* fragment_path = nullptr);
void FreeShaders(GL_ID* shader);
void ShaderUse(GL_ID* shader);
void ShaderSetBool(GL_ID* shader, const char* name, bool value);
void ShaderSetInt(GL_ID* shader, const char* name, int value);
void ShaderSetFloat(GL_ID* shader, const char* name, float value);
void ShaderSetVector(GL_ID* shader, const char* name, Vector2 v);
void ShaderSetVector(GL_ID* shader, const char* name, Vector4 v);

// SECTION: Rendering
WindowContext* InitWindowContext(int width, int height, const char* window_name, fColor clear_color, bool fullscreen=false);
void FreeWindowContext(WindowContext* window_context);
// NOTE: Returns a handler to the gpu buffers (VBO, VAO, EBO). If called with no arguments, will init to default buffers.
GPUBufferIDs InitGPUBuffers(float* vertices = nullptr, size_t vertices_size = 0, unsigned int* indices = nullptr, size_t indices_size = 0);
GPUBufferIDs InitGPUBuffersInstanced(Vector3* translations, int max_num_instances = 100, float* vertices = nullptr, size_t vertices_size = 0, unsigned int* indices = nullptr, size_t indices_size = 0);
void FreeGPUBuffers(GPUBufferIDs);
void ClearScreen();


// SECTION: Higher Level Loading and Displaying
Sprite LoadSprite(
    const char* texture_path, 
    GL_ID* shader_id, GPUBufferIDs buffers 
);
void FreeSprite(Sprite);
void DrawSprite(Sprite sprite, Transform transform, Camera camera);
void DrawSpriteInstanced(Sprite sprite, GL_ID* shader, GPUBufferIDs gpu_ids, Camera camera, int num_intances);

// SECTION: Moveing around intanced positions:
// NOTE: UpdateInstancePositions uses glBufferSubData, and is simple. However, for large or frequent updates, this is slow
// because it has to copy data from the CPU to the GPU.
void UpdateInstancePositions(GPUBufferIDs instance_gpu_buffers, Vector3* positions, int numInstances);
// NOTE: UpdateIntancePositionsMapped uses buffer mapping. It is faster for frequent updates than glBufferSubData
// but still has overhead, as it maps and unmaps the buffers every time it is called. For large buffer sets
// use the persistent mapping methods instead.
void UpdateInstancePositionsMapped(GPUBufferIDs instance_gpu_buffers, Vector3* newPositions, int numInstances);

// NOTE: The functions below initialize the persistent buffer that can then be updated by the update function.
// The init function returns the mapped_buffer, which then can be edited directly. No need to create a seperate array
// and iterate over it.
Vector3* InitInstancePositionsPersistently(GPUBufferIDs instance_gpu_buffers, Vector3* initialData, int numInstances);


// NOTE: For engine use
unsigned int SkyGetGLID(GL_ID* id);