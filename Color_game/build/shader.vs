#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_texture_coord;
out vec2 tex_coord;
out vec4 color_multiplier;
uniform mat4 transform;
uniform mat4 view;
uniform vec4 i_color_multiplier;
uniform vec2 bot_left_uv = vec2(0.0, 0.0); 
uniform vec2 top_right_uv = vec2(1.0, 1.0);
uniform vec2 uv_offset = vec2(0.0, 0.0); // Offset is from bottom left toward top right. between 0 and 1
void main()
{
    gl_Position = transform * vec4(a_pos.x, a_pos.y, a_pos.z, 1.0);
    float tex_height = top_right_uv.y - bot_left_uv.y;
    float tex_width = top_right_uv.x - bot_left_uv.x;
    vec2 tex_coord_scaled = vec2(a_texture_coord.x * tex_width, a_texture_coord.y * tex_height);
    vec2 tex_coord_scaled_and_offset = tex_coord_scaled + uv_offset;
    tex_coord = tex_coord_scaled_and_offset;
    color_multiplier = i_color_multiplier;
}