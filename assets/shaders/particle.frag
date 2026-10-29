#version 330 core
in vec4 vColor;
out vec4 FragColor;

void main() {
    // Hacer las partículas redondas en lugar de cuadradas
    vec2 coord = gl_PointCoord - vec2(0.5);
    if (length(coord) > 0.5) discard;
    
    FragColor = vColor;
}