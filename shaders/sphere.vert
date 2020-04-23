#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 worldPos;
out vec3 normal;
out vec3 texCoord;

uniform mat4 model;
uniform mat3 normalMtrx;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    worldPos = vec3(model * vec4(aPos, 1.0));
    normal = normalMtrx * aNormal;
    texCoord = aNormal;
}
