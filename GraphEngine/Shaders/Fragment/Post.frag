#version 430 core


in vec2 tex_coord;

out vec4 color;

uniform bool grayscale;
uniform int offset;
uniform int border_width;
uniform float kernel[9];
uniform vec2 screen_texture_size;
uniform vec3 border_color;
uniform sampler2D screen_texture;
uniform usampler2D stencil_texture;


void main() {
    vec2 texel_size = 1.0 / textureSize(screen_texture, 0).xy;

    uint cur = texture(stencil_texture, tex_coord * screen_texture_size).r;
    float r = border_width * border_width / 4;
    for (int i = -border_width / 2; i <= border_width / 2; i++) {
        vec2 pos_up = tex_coord + texel_size * vec2(i, sqrt(r - i * i));
        vec2 pos_down = tex_coord + texel_size * vec2(i, -sqrt(r - i * i));
        if ((texture(stencil_texture, pos_up * screen_texture_size).r | cur) > cur || (texture(stencil_texture, pos_down * screen_texture_size).r | cur) > cur) {
            color = vec4(border_color, 1);
            return;
        }
    }

    vec2 offsets[9] = vec2[] (
        vec2(-offset, offset),
        vec2(0, offset),
        vec2(offset, offset),
        vec2(-offset, 0),
        vec2(0, 0),
        vec2(offset, 0),
        vec2(-offset, -offset),
        vec2(0, -offset),
        vec2(offset, -offset)
    );

    vec3 frag_color = vec3(0.0);
    for(int i = 0; i < 9; i++)
        frag_color += vec3(texture(screen_texture, (tex_coord + offsets[i] * texel_size) * screen_texture_size)) * kernel[i];

    if (grayscale)
        color = vec4(vec3(0.2126 * frag_color.x + 0.7152 * frag_color.y + 0.0722 * frag_color.z), 1.0);
    else
        color = vec4(frag_color, 1.0);
}
