#version 330 core
in vec2 screen_uv;
out vec4 FragColor;
uniform vec4  iris_color;
uniform float iris_radius;
uniform float aspect_ratio;
uniform float iris_invert;   // 0 = draw inside circle, 1 = draw outside circle
void main()
{
    vec2 centered = screen_uv - vec2(0.5, 0.5);
    centered.x *= aspect_ratio;
    float max_dist = sqrt(0.25 + 0.25 * aspect_ratio * aspect_ratio);
    float dist = length(centered) / max_dist;
    if (iris_invert < 0.5 ? dist > iris_radius : dist < iris_radius) discard;
    FragColor = vec4(iris_color.rgb, 1.0);
}
