#version 330 core
in vec2 tex_coord;
in vec4 color_multiplier;
out vec4 FragColor;
uniform sampler2D texture1;
void main()
{
    FragColor = texture(texture1, tex_coord) * color_multiplier;
}
