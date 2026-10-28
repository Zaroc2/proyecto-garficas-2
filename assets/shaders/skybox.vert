#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main() {
    TexCoords = aPos; // Pasamos la posición local como coordenada de textura
    // Es exactamenteigual al base.vert, pero en éste caso eliminamos la traslación para qeu el cubo siempre se quede en el mismo lugar y no se mueva respecto a la cámara
    mat4 viewNoTranslation = mat4(mat3(view)); 
    gl_Position = projection * viewNoTranslation * vec4(aPos, 1.0);
    // Reordenamos para que z = w, para que cuando se divida z/w = 1 = z y que siempre se pinte en el fondo
    gl_Position = gl_Position.xyww; 
}