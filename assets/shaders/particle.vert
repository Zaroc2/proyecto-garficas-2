#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;
layout(location = 2) in float aSize;

out vec4 vColor;

uniform mat4 projection;
uniform mat4 view;

void main() {
    vColor = aColor;
    gl_PointSize = aSize;
    gl_Position = projection * view * vec4(aPos, 1.0);
}