#version 330 core
out vec4 FragColor;
in vec3 TexCoords;
uniform samplerCube skybox;
void main() {
    //Simplemente le asignamos la textura usando el samplerCube
    FragColor = texture(skybox, TexCoords);
}