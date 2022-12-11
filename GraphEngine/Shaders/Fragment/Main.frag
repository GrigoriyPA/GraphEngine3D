#version 430 core

const int NR_LIGHTS = 3;
const int NR_CAMERAS = 2;


struct Light {
    bool shadow;
    int type;
    float constant, linear, quadratic, cut_in, cut_out;
    vec3 position, direction, ambient, diffuse, specular;
    mat4 light_space;
};

struct Material {
    bool shadow, use_vertex_color;
    float shininess, alpha;
    vec3 ambient, diffuse, specular, emission;
};


in vec2 tex_coord;
in vec3 frag_pos;
in vec3 norm;
in vec3 vert_color;
in float object_model_id;

out vec4 color;

uniform bool use_diffuse_map;
uniform bool use_specular_map;
uniform bool use_emission_map;
uniform int object_id;
uniform int camera_id;
uniform int number_lights;
uniform float gamma;
uniform sampler2D diffuse_map;
uniform sampler2D specular_map;
uniform sampler2D emission_map;
uniform sampler2DArray shadow_maps;
uniform vec2 check_point;
uniform vec3 view_pos;
uniform Material object_material;
uniform Light lights[NR_LIGHTS];


layout(std430, binding=0) buffer central_object {
    int central_object_id[NR_CAMERAS];
    int central_object_model_id[NR_CAMERAS];
    float depth[NR_CAMERAS];
};


float calc_shadow(Light light, vec3 light_dir, vec3 normal, int id) {
    if (!light.shadow)
        return 0.0;

    float bias = 0.01;
    
    vec4 frag_pos_light_space = light.light_space * vec4(frag_pos, 1.0);
    frag_pos_light_space = frag_pos_light_space / frag_pos_light_space.w;
    vec3 proj_coords = vec3(frag_pos_light_space) * 0.5 + 0.5;
    float current_depth = proj_coords.z;

    if(proj_coords.z > 1.0)
        return 0.0;

    float shadow = 0.0;
    vec2 texel_size = 1.0 / textureSize(shadow_maps, 0).xy;
    for (int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcf_depth = texture(shadow_maps, vec3(proj_coords.xy + vec2(x, y) * texel_size, id)).r;
            shadow += current_depth - bias > pcf_depth ? 1.0 : 0.0;
        }
    }

    return shadow / 9.0;
}


vec3 calc_dir_light(Light light, vec3 normal, vec3 view_dir, Material material, int id) {
    vec3 light_dir = normalize(-light.direction);
    float diff = max(dot(normal, light_dir), 0.0);

    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway_dir), 0.0), material.shininess);

    float shadow = material.shadow ? calc_shadow(light, light_dir, normal, id) : 0.0;
    vec3 ambient = light.ambient * material.ambient;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;

    if (dot(light_dir, normal) < 0.0)
        return ambient;

    return ambient + (1.0 - shadow) * (diffuse + specular);
}


vec3 calc_point_light(Light light, vec3 normal, vec3 view_dir, Material material, int id) {
    vec3 light_dir = normalize(light.position - frag_pos);

    float diff = max(dot(normal, light_dir), 0.0);

    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway_dir), 0.0), material.shininess);

    float distance = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    vec3 ambient = light.ambient * material.ambient * attenuation;
    vec3 diffuse = light.diffuse * diff * material.diffuse * attenuation;
    vec3 specular = light.specular * spec * material.specular * attenuation;

    if (dot(light_dir, normal) < 0.0)
        return ambient;

    return ambient + diffuse + specular;
}


vec3 calc_spot_light(Light light, vec3 normal, vec3 view_dir, Material material, int id) {
    vec3 light_dir = normalize(light.position - frag_pos);
    float diff = max(dot(normal, light_dir), 0.0);

    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway_dir), 0.0), material.shininess);

    float distance = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    float theta = dot(light_dir, normalize(-light.direction));
    float intensity = clamp((theta - light.cut_out) / (light.cut_in - light.cut_out), 0.0, 1.0);    
    
    float shadow = material.shadow ? calc_shadow(light, light_dir, normal, id) : 0.0;
    vec3 ambient = light.ambient * material.ambient * attenuation;
    vec3 diffuse = light.diffuse * diff * material.diffuse * attenuation * intensity;
    vec3 specular = light.specular * spec * material.specular * attenuation * intensity;

    if (dot(light_dir, normal) < 0.0)
        return ambient;

    return ambient + (1.0 - shadow) * (diffuse + specular);
}


void main() {
    if (abs(gl_FragCoord.x - check_point.x) <= 1 && abs(gl_FragCoord.y - check_point.y) <= 1 && gl_FragCoord.z < depth[camera_id]) {
        central_object_id[camera_id] = object_id;
        central_object_model_id[camera_id] = int(object_model_id);
        depth[camera_id] = gl_FragCoord.z;
    }

    Material material = object_material;
    if (object_material.use_vertex_color) {
		material.ambient = vert_color;
        material.diffuse = vert_color;
    }
	if (use_diffuse_map) {
        vec4 diffuse_color = texture(diffuse_map, tex_coord);
		material.ambient = vec3(diffuse_color);
		material.diffuse = vec3(diffuse_color);
        material.alpha = diffuse_color.w;
	}
	if (use_specular_map)
		material.specular = vec3(texture(specular_map, tex_coord));
    if (use_emission_map)
        material.emission = vec3(texture(emission_map, tex_coord));

    if (material.alpha < 0.1)
        discard;

    vec3 normal = normalize(norm);
    vec3 view_dir = normalize(view_pos - frag_pos);

    vec3 result_color = vec3(0.0);
    for(int i = 0; i < NR_LIGHTS; i++) {
        if (i == number_lights)
            break;

        if (lights[i].type == 0)
  	        result_color += calc_dir_light(lights[i], normal, view_dir, material, i);
        else if (lights[i].type == 1)
            result_color += calc_point_light(lights[i], normal, view_dir, material, i);
        else
            result_color += calc_spot_light(lights[i], normal, view_dir, material, i);
    }

    color = vec4(pow(result_color + material.emission, vec3(1.0 / gamma)), material.alpha);
}
