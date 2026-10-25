#version 330 core

uniform uint objectId;
uniform int baseTriangle; //El id del primer triángulo de éste submesh, para poder calcular el id del triángulo actual del objeto

out uvec4 fragIds;

void main() {
    //Devolvemos el id del píxel actual, y del triángulo en el que está con gl_PrimitiveID a fragId
    fragIds = uvec4(objectId, uint(gl_PrimitiveID + baseTriangle), 0u, 0u);
    //El fragment shader escribirá ésto en la textura del fbo
}