#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_texture_coord;
layout (location = 2) in vec3 a_offset;
out vec2 screen_uv;
void main()
{
    // Bypass camera/transform — map unit quad directly to full NDC
    gl_Position = vec4(a_pos.x * 2.0, a_pos.y * 2.0, 0.0, 1.0);
    screen_uv = a_texture_coord;
}
