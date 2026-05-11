#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

//#include "image_import.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"


#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "sky_renderer2D.h"


struct GL_ID {
    GLuint id;
};

void CheckGLError()
{
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        const char* error;

        switch(err)
        {
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
            default:
                error = "Unknown Error";
                break;
        }

        printf("GL Error: %s\n", error);
    }
}

const char *default_vertex_shader_source = "#version 330 core\n"
    "layout (location = 0) in vec3 a_pos;\n"
    "layout (location = 1) in vec2 a_texture_coord;\n"
    "out vec2 tex_coord;\n"
    "out vec4 color_multiplier;\n"
    "uniform mat4 transform;\n"
    "uniform mat4 view;\n"
    "uniform vec4 i_color_multiplier;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = transform * vec4(a_pos.x, a_pos.y , a_pos.z, 1.0);\n"
    "   tex_coord = a_texture_coord;\n"
    "   color_multiplier = i_color_multiplier;\n"
    "}\0";

const char *default_fragment_shader_source = "#version 330 core\n"
    "in vec2 tex_coord;\n"
    "in vec4 color_multiplier;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D texture1;\n"
    "void main()\n"
    "{\n"
    "   FragColor = texture(texture1, tex_coord) * color_multiplier;\n"
    "}\0";


// SECTION: Shaders.
// NOTE: Returns the shader ID
GL_ID* ShaderInit(const char* vertex_path, const char* fragment_path) {
    const char* v_shader_code;
    const char* f_shader_code;

    char* fs_shader_code = nullptr;
    char* vs_shader_code = nullptr;

    if (vertex_path == nullptr || fragment_path == nullptr) {
        v_shader_code = default_vertex_shader_source;
        f_shader_code = default_fragment_shader_source;
    } else {
        FILE *vs_file = fopen(vertex_path, "rb");
        if (vs_file == NULL) {
            printf("VERTEX SHADER FILE ERROR\n");
        }
        FILE *fs_file = fopen(fragment_path, "rb");
        if (fs_file == NULL) {
            printf("FRAGMENT SHADER FILE ERROR\n");
        }


        fseek(vs_file, 0, SEEK_END);
        long vs_file_size = ftell(vs_file);
        fseek(vs_file, 0, SEEK_SET);
        fseek(fs_file, 0, SEEK_END);
        long fs_file_size = ftell(fs_file);
        fseek(fs_file, 0, SEEK_SET);


        char* vs_shader_code = (char *)calloc(vs_file_size + 1, 1);
        char* fs_shader_code = (char *)calloc(fs_file_size + 1, 1);
    

        size_t vs_bytes_read = fread(vs_shader_code, 1, vs_file_size, vs_file);
        if (vs_bytes_read != (size_t)vs_file_size) {
            // Handle reading error
            printf("VERTEX SHADER READING ERROR\n");
        }
        size_t fs_bytes_read = fread(fs_shader_code, 1, fs_file_size, fs_file);
        if (fs_bytes_read != (size_t)fs_file_size) {
            // Handle reading error
            printf("FRAGMENT SHADER READING ERROR\n");
        }  
        

        fclose(vs_file);
        fclose(fs_file);


        vs_shader_code[vs_file_size] = '\0';
        fs_shader_code[fs_file_size] = '\0';

        v_shader_code = vs_shader_code;
        f_shader_code = fs_shader_code;
        
    }
    
    unsigned int vertex, fragment;
    int success;
    char infoLog[512];
    
    // vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &v_shader_code, NULL);
    glCompileShader(vertex);
    // print compile errors if any
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    };

    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &f_shader_code, NULL);
    glCompileShader(fragment);
    // print compile errors if any
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    };

    GLuint ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    // print linking errors if any
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    GL_ID* shader = (GL_ID*)malloc(sizeof(GL_ID));
    shader->id = ID;

    glUseProgram(ID);
    
    if (vs_shader_code != nullptr) free(vs_shader_code);
    if (fs_shader_code != nullptr) free(fs_shader_code);
    
    return shader;
} 

void FreeShaders(GL_ID* shader_id) {
    glDeleteProgram(shader_id->id);
    free(shader_id);
    shader_id = nullptr;
}

void ShaderUse(GL_ID* shader) {
    glUseProgram(shader->id);
}

void ShaderSetBool(GL_ID* shader, const char* name, bool value) {         
    glUniform1i(glGetUniformLocation(shader->id, name), (int)value); 
}
void ShaderSetInt(GL_ID* shader, const char* name, int value) { 
    glUniform1i(glGetUniformLocation(shader->id, name), value); 
}
void ShaderSetFloat(GL_ID* shader, const char* name, float value) { 
    glUniform1f(glGetUniformLocation(shader->id, name), value); 
}
void ShaderSetVector(GL_ID* shader, const char* name, Vector2 v) {
    GLfloat value[2] = {v.x, v.y};
    glUniform2fv(glGetUniformLocation(shader->id, name), 1, value);
} 
void ShaderSetVector(GL_ID* shader, const char* name, Vector4 v) {
    GLfloat value[4] = {v.x, v.y, v.z, v.w};
    glUniform4fv(glGetUniformLocation(shader->id, name), 1, value);
} 
void ShaderSetTransform(GL_ID* shader, const char* name, glm::mat4 trans) {
    glUniformMatrix4fv(glGetUniformLocation(shader->id, name), 1, GL_FALSE, glm::value_ptr(trans));
}



// SECTION: Rendering

WindowContext* InitWindowContext(int width, int height, const char* window_name, fColor clear_color, bool fullscreen) {
    WindowContext* window_context = (WindowContext*)malloc(sizeof(WindowContext*)); 


    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize SDL: %s", SDL_GetError());
    }
    
    
    // NOTE: Requests OpenGL 3.3
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    
    // NOTE: Turn off vsync
    SDL_GL_SetSwapInterval(0);
    
    uint32_t sdl_window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
    if (fullscreen) {
        sdl_window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        width = 0;
        height = 0;
    }
    // Create an SDL window
    SDL_Window* window = SDL_CreateWindow(window_name,
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          width, height,
                                          sdl_window_flags);
    if (window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create SDL window: %s", SDL_GetError());
    }

    // Create an OpenGL context
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (glContext == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create OpenGL context: %s", SDL_GetError());
    }

    // Initialize GLEW
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize GLEW: %s", glewGetErrorString(glewError));
    }

    // Print some OpenGL information
    SDL_Log("OpenGL Version: %s", glGetString(GL_VERSION));
    SDL_Log("GLEW Version: %s", glewGetString(GLEW_VERSION));

    // Set the clear color
    glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Set OpenGL rendering to have depth testing:
    glEnable(GL_DEPTH_TEST);


    window_context->window = window;
    window_context->gl_context = glContext;
    return window_context;
}

void FreeWindowContext(WindowContext* wdws) {

    SDL_GL_DeleteContext((wdws->gl_context));
    SDL_DestroyWindow(wdws->window);
    free(wdws);
    wdws = nullptr;
    SDL_Quit();
}

// NOTE: Inits all the buffers and return the VAO
// NOTE: Seperate this into individual initializers for VBO VAO and EBO.
GPUBufferIDs InitGPUBuffers(
    float* vertices, size_t vertices_size, 
    unsigned int* indices, size_t indices_size
) {
    float vertices_arr[] = {
        // positions          // texture coords
        0.5f,  0.5f, 0.0f,    1.0f, 1.0f,   // top right
        0.5f, -0.5f, 0.0f,    1.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,   // bottom left
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f    // top left 
    };
    
    unsigned int indices_arr[] = { 
        0, 1, 2,   // first triangle
        0, 2, 3    // second triangle
    };


    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    if (vertices == nullptr || vertices_size == 0)
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_arr), vertices_arr, GL_STATIC_DRAW);
    else 
        glBufferData(GL_ARRAY_BUFFER, vertices_size, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    if (indices == nullptr || indices_size == 0)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_arr), (GLuint*)indices_arr, GL_STATIC_DRAW);
    else 
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size, (GLuint*)indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0); 
    // Load the texture coordinates to the VAO
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    GL_ID* vbo_id = (GL_ID*)malloc(sizeof(GL_ID)); 
    vbo_id->id = VBO;
    GL_ID* vao_id = (GL_ID*)malloc(sizeof(GL_ID)); 
    vao_id->id = VAO;
    GL_ID* ebo_id = (GL_ID*)malloc(sizeof(GL_ID)); 
    ebo_id->id = EBO;
    
    GPUBufferIDs buffer_info = {0};
    buffer_info.vbo = vbo_id;
    buffer_info.vao = vao_id;
    buffer_info.ebo = ebo_id;
    buffer_info.instance_vbo = nullptr;

    return buffer_info;
}

GPUBufferIDs InitGPUBuffersInstanced(
    Vector3* positions, // Position translation array for the instances
    int max_num_instances,
    float* vertices, size_t vertices_size, 
    unsigned int* indices, size_t indices_size
) {

    float vertices_arr[] = {
        // positions          // texture coords
        0.5f,  0.5f, 0.0f,    1.0f, 1.0f,   // top right
        0.5f, -0.5f, 0.0f,    1.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,   // bottom left
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f    // top left 
    };

    GLuint VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_arr), vertices_arr, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    GLuint instanceVBO;
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, max_num_instances * sizeof(Vector3), &(positions)[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

    // Set up instance data
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);  // Tell OpenGL this is an instanced vertex attribute.

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    GL_ID* vbo_id = (GL_ID*)malloc(sizeof(GL_ID)); 
    vbo_id->id = VBO;
    GL_ID* vao_id = (GL_ID*)malloc(sizeof(GL_ID)); 
    vao_id->id = VAO;
    GL_ID* ebo_id = nullptr;
    GL_ID* instance_vbo_id = (GL_ID*)malloc(sizeof(GL_ID)); 
    instance_vbo_id->id = instanceVBO;

    GPUBufferIDs buffer_info = {0};
    buffer_info.vbo = vbo_id;
    buffer_info.vao = vao_id;
    buffer_info.ebo = ebo_id;
    buffer_info.instance_vbo = instance_vbo_id;


    return buffer_info;
}

void FreeGPUBuffers(GPUBufferIDs buffer_info) {
    glDeleteBuffers(1, &(buffer_info.vbo->id)); 
    glDeleteVertexArrays(1, &(buffer_info.vao->id)); 
    glDeleteBuffers(1, &(buffer_info.ebo->id)); 

    free(buffer_info.vbo);
    free(buffer_info.vao);
    free(buffer_info.ebo);
    free(buffer_info.instance_vbo);

    buffer_info.vbo = nullptr;
    buffer_info.vao = nullptr;
    buffer_info.ebo = nullptr;
    buffer_info.instance_vbo = nullptr;
}


// SECTION: Texture Loading and rendering
GL_ID* LoadTexture(const char* image_path, unsigned int* width_p, unsigned int* height_p) {
    // NOTE: Load the image texture.
#ifdef STB_IMAGE_IMPLEMENTATION
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(image_path, &width, &height, &nrChannels, 0);
    unsigned int gl_format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
#else
    ImageData img = {0};
    LoadBitmap(image_path, &img);
    int width = img.width;
    int height = img.height;
    unsigned char* data = (unsigned char*)img.data;
    unsigned int gl_format = (img.bytes_per_pixel == 4) ? GL_RGBA : GL_RGB;
#endif

    if (width_p != nullptr) *width_p = width;
    if (height_p != nullptr) *height_p = height;
    // NOTE: Generate a texture and bind it to current state/context
    GLuint texture;
    glGenTextures(1, &texture); 
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    GLenum error;
    error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("ERROR: LoadSprite::%s::%s \n", image_path, error);
    }

    // NOTE: Generate the mipmap
    glTexImage2D(GL_TEXTURE_2D, 0, gl_format, width, height, 0, gl_format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("ERROR: LoadSprite::%s::%s \n", image_path, error);
    }
    // NOTE: Enable mirrored repeat for both the S and T axes (the x and y axes)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("ERROR: LoadSprite::%s::%s \n", image_path, error);
    }

    // NOTE: For better scaling we use mipmapping. We set the filter to be the point filter
    // NOTE: The first one is for shrinking, so we use mipmapping for it
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    // NOTE: For magnification, we dont use mipmapping, but we can use a point filter anyway.
    // (just not a mipmap point filter.)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("ERROR: LoadSprite::%s::%s \n", image_path, error);
    }
   
    //int max = 0;
    //glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
    //printf("Max texture size: %d\n", max);
    // NOTE: Free the image afterwards:
#ifdef STB_IMAGE_IMPLEMENTATION
    stbi_image_free(data);
#else
    FreeBitmap(&img);
#endif

    GL_ID* tex_id = (GL_ID*)malloc(sizeof(GL_ID));
    tex_id->id = texture;
    
    return tex_id; 
}

void FreeTexture(GL_ID* texture_id) {
    glDeleteTextures(1, &(texture_id->id));
    free(texture_id);
    texture_id = nullptr;
}

void DrawTexture(GL_ID* shader_id, GL_ID* texture, GL_ID* vao) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ShaderUse(shader_id);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glBindVertexArray(vao->id);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void DrawTextureInstanced(GL_ID* shader_id, GL_ID* texture, GPUBufferIDs gpu_ids, int num_transforms) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ShaderUse(shader_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glBindVertexArray(gpu_ids.vao->id);

    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, num_transforms); 
    //glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, num_transforms);
}

void ClearScreen() {
    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);
    // Clear the depth buffer (for proper transparency rendering)
    glClear(GL_DEPTH_BUFFER_BIT);
}



// SECTION: Higher level texture loading and displaying

// NOTE: Converts a transform object into a glm transform matrix. 
// NOTE: Rotations are done in xyz order for now.
glm::mat4 TransformToMatrix(Transform transform) {
    glm::mat4 trans = glm::mat4(1.0);

    glm::vec3 translation_vec = glm::vec3(transform.position.x, transform.position.y, transform.position.z);
    glm::vec3 rotation_vec = glm::vec3(transform.rotation.x, transform.rotation.y, transform.rotation.z);
    glm::vec3 scale_vec = glm::vec3(transform.scale.x, transform.scale.y, transform.scale.z);

    trans = glm::translate(trans, translation_vec);
    trans = glm::rotate(trans, glm::radians(rotation_vec.x), glm::vec3(1.0f, 0.0f, 0.0f));
    trans = glm::rotate(trans, glm::radians(rotation_vec.y), glm::vec3(0.0f, 1.0f, 0.0f));
    trans = glm::rotate(trans, glm::radians(rotation_vec.z), glm::vec3(0.0f, 0.0f, 1.0f));
    trans = glm::scale(trans, scale_vec); 

    return trans;
}

glm::mat4 CameraToMatrix(Camera camera) {

    // SECTION: Projection matrix
    glm::mat4 proj;
    if (camera.projection == ORTHOGRAPHIC_CAMERA) {
        proj = glm::ortho(-camera.width/2, camera.width/2, -camera.height/2, camera.height/2, camera.near_plane, camera.far_plane);
    } else if (camera.projection == PERSPECTIVE_CAMERA) {
        proj = glm::perspective(camera.FOV, camera.width/camera.height, camera.near_plane, camera.far_plane);
    }

    // SECTION: view matrix
    glm::vec3 position_vec = glm::vec3(camera.position.x, camera.position.y, camera.position.z);
    glm::vec3 target_vec = glm::vec3(camera.look_target.x, camera.look_target.y, camera.look_target.z);
    glm::vec3 up_vec = glm::vec3(camera.up_direction.x, camera.up_direction.y, camera.up_direction.z);
    
    glm::mat4 view = glm::lookAt(position_vec, target_vec, up_vec);


    glm::mat4 R = proj * view;
    return R;
}


Sprite LoadSprite(
    const char* texture_path, 
    GL_ID* shader_id,
    GPUBufferIDs buffers 
) {




    Sprite sprite = {0};

    sprite.shader_id = shader_id;
    sprite.buffers = buffers; 
    sprite.texture_id = LoadTexture(texture_path, &(sprite.pixel_width), &(sprite.pixel_height));

    return sprite;
}

void FreeSprite(Sprite sprite) {
    FreeTexture(sprite.texture_id);
}

void DrawSprite(Sprite sprite, Transform transform, Camera camera) {

    ShaderSetTransform(sprite.shader_id, "transform", CameraToMatrix(camera) * TransformToMatrix(transform));
    
    DrawTexture(sprite.shader_id, sprite.texture_id, sprite.buffers.vao);
}

void DrawSpriteInstanced(Sprite sprite, GL_ID* shader, GPUBufferIDs gpu_ids, Camera camera, int num_intances) {

    Transform transform = {0};
    transform.position = {0.0f, 0.0f, 0.0f};
    transform.scale = {1.0f, 1.0f, 1.0f};

    ShaderSetTransform(sprite.shader_id, "transform", CameraToMatrix(camera) * TransformToMatrix(transform));
    
    DrawTextureInstanced(shader, sprite.texture_id, gpu_ids, num_intances);
}

void UpdateInstancePositions(GPUBufferIDs instance_gpu_buffers, Vector3* positions, int numInstances) {
    glBindBuffer(GL_ARRAY_BUFFER, instance_gpu_buffers.instance_vbo->id);
    glBufferSubData(GL_ARRAY_BUFFER, 0, numInstances * sizeof(Vector3), positions);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void UpdateInstancePositionsMapped(GPUBufferIDs instance_gpu_buffers, Vector3* newPositions, int numInstances) {
    glBindBuffer(GL_ARRAY_BUFFER, instance_gpu_buffers.instance_vbo->id);

    // Map the buffer
    Vector3* mappedBuffer = (Vector3*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    if (mappedBuffer != NULL) {
        // Update the data
        for (int i = 0; i < numInstances; i++) {
            mappedBuffer[i] = newPositions[i];
        }

        // Unmap the buffer
        glUnmapBuffer(GL_ARRAY_BUFFER);
    } else {
        // Handle error 
        printf("UpdateInstancePositionMapped Error: buffer mapping failed");
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Vector3* InitInstancePositionsPersistently(GPUBufferIDs instance_gpu_buffers, Vector3* initialData, int numInstances) {

    glBindBuffer(GL_ARRAY_BUFFER, instance_gpu_buffers.instance_vbo->id);
    // Allocate the buffer with persistent mapping flags
    glBufferStorage(GL_ARRAY_BUFFER, numInstances * sizeof(Vector3), initialData,
                    GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

    // Map the buffer persistently
    Vector3* mappedBuffer = (Vector3*)glMapBufferRange(GL_ARRAY_BUFFER, 0, numInstances * sizeof(Vector3),
                                                       GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return mappedBuffer;
}

unsigned int SkyGetGLID(GL_ID* id) {
    return id->id;
}