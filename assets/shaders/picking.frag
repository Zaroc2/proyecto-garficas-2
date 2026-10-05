#version 330 core

uniform uint objectId;

out uvec4 fragIds;

void main() {
    //Devolvemos el id del píxel actual, y del triángulo en el que está con gl_PrimitiveID a fragId
    fragIds = uvec4(objectId, uint(gl_PrimitiveID), 0u, 0u);
    //El fragment shader escribirá ésto en la textura del fbo
}