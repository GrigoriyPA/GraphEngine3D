#version 460 core


layout (location = 0) in vec3 position;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec2 texture_coord;
layout (location = 3) in mat4 instance_model;

out vec2 tex_coord;
out vec3 frag_pos;
out vec3 norm;
out float object_model_id;

uniform int model_id;
uniform mat4 not_instance_model;
uniform mat4 view;
uniform mat4 projection;


void main() {
    mat4 model = not_instance_model;
    object_model_id = model_id;
    if (model_id == -1) {
        model = instance_model;
        object_model_id = gl_InstanceID;
    }

    gl_Position =  projection * view * model * vec4(position, 1.0);
    tex_coord = vec2(texture_coord.x, 1.0 - texture_coord.y);
    frag_pos = vec3(model * vec4(position, 1.0f));
    norm = transpose(inverse(mat3(model))) * vertex_normal;
}
