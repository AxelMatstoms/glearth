#version 330 core

#define PI 3.1415926535897932

out vec4 FragColor;

in vec3 texCoord;

uniform sampler2D skybox;

vec2 sphere_uv(vec3 direction)
{
    float u = 0.5 + atan(direction.z, direction.x) / (2 * PI);
    float v = 0.5 - asin(direction.y) / PI;

    return vec2(u, v);
}

void main()
{
    FragColor = vec4(vec3(0.25), 1.0) * texture(skybox, sphere_uv(normalize(texCoord)));
}
