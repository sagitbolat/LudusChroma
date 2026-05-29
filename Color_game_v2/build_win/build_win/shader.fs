#version 330 core
in vec2 tex_coord;
in vec4 color_multiplier;
in vec2 v_screen_uv;
in vec2 v_quad_uv;
out vec4 FragColor;
uniform sampler2D texture1;
uniform float iris_mask_enabled;
uniform float iris_mask_radius;
uniform float iris_mask_aspect;
uniform float iris_mask_invert;
uniform float portal_clip_enabled;
uniform float portal_clip_axis;    // 0 = clip X, 1 = clip Y
uniform float portal_clip_uv_min;  // discard if quad uv coord < this
uniform float portal_clip_uv_max;  // discard if quad uv coord > this
void main()
{
    FragColor = texture(texture1, tex_coord) * color_multiplier;
    if (FragColor.a < 0.01) discard;
    if (iris_mask_enabled > 0.5) {
        vec2 c = v_screen_uv - vec2(0.5);
        c.x *= iris_mask_aspect;
        float max_dist = sqrt(0.25 * iris_mask_aspect * iris_mask_aspect + 0.25);
        float dist = length(c) / max_dist;
        if (iris_mask_invert < 0.5 ? dist < iris_mask_radius : dist > iris_mask_radius) discard;
    }
    if (portal_clip_enabled > 0.5) {
        float coord = portal_clip_axis < 0.5 ? v_quad_uv.x : v_quad_uv.y;
        if (coord < portal_clip_uv_min || coord > portal_clip_uv_max) discard;
    }
}
