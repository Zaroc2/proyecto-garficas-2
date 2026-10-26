//Cambien la Version si es necesario
#version 330 core

#define MAX_LIGHTS 8
#define DIRECTIONAL 0
#define POINT 1
#define SPOT 2

//Entradas
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

//Salida
out vec4 FragColor;

//Propiedades del objeto y la luz
struct Light {
    int type;
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
};
uniform int numLights;
uniform Light lights[MAX_LIGHTS];

uniform vec3 viewPos; // Posición de la cámara
uniform vec4 objectColor;  //Vector 4 para soportar el color difuso (kd) y el canal alfa
uniform float shininess;
uniform int currentShadingMode; // 0=Flat, 1=Lambert, 2=Blinn-Phong

//texturea
uniform sampler2D objTexture;
uniform bool useTexture;

void main()
{

    vec3 norm = normalize(Normal); //Precaución por si se deformo 
    if (currentShadingMode == 0) {
        norm = normalize(cross(dFdx(FragPos), dFdy(FragPos)));
    }
    vec3 viewDir = normalize(viewPos - FragPos);
    
    vec3 baseColor = objectColor.rgb;
    if (useTexture) {
        vec4 texColor = texture(objTexture, TexCoord);
        baseColor *= texColor.rgb;
    }

    vec3 result = 0.15 * baseColor; 

    //En un bucle, acumulamos las luces
    for(int i = 0; i < numLights; i++) {
        vec3 lightDir;
        float attenuation = 1.0;
        
        if (lights[i].type == DIRECTIONAL) {
            lightDir = normalize(-lights[i].direction); //Si es direccional, la dirección de la luz es opuesta a la dirección desde donde viene (si el sol esta arriba, la luz va hacia abajo)
        } else {
            //Si es point o spot, calculamos distancia y dirección y aplicamos una atenuación cuadrática (mayor distancia, menos luz).
            lightDir = normalize(lights[i].position - FragPos);
            float distance = length(lights[i].position - FragPos);
            attenuation = 1.0 / (lights[i].constant + lights[i].linear * distance + lights[i].quadratic * (distance * distance));
            
            //En el caso de Spot, calculamos la intensidad en funcion de theta que es el coseno del angulo entre la direccion del pixel y la luz (1 en el centro, 0 fuera) y su difuminacion
            if (lights[i].type == SPOT) {
                float theta = dot(lightDir, normalize(-lights[i].direction));
                float epsilon = lights[i].cutOff - lights[i].outerCutOff;
                float intensity = clamp((theta - lights[i].outerCutOff) / epsilon, 0.0, 1.0);
                attenuation *= intensity;
            }
        }

        // Calculamos cada componente de luz y las sumamos
        
        //Difusa (Lambert)
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * baseColor * lights[i].color * lights[i].intensity;
        
        //Especular (Blinn-Phong)
        vec3 specular = vec3(0.0); // Lo inicializamos en 0
        
        // solo calcumaos blinfon si NO estamos en modo Lambert/Gouraud (1)
        if (currentShadingMode != 1) {
            vec3 halfwayDir = normalize(lightDir + viewDir);
            float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
            specular = spec * lights[i].color * lights[i].intensity;
        }

        result += (diffuse + specular) * attenuation;
    }

    
    //Se asigna el color resultante y se preserva el canal alfa original
    FragColor = vec4(result, objectColor.a);
}