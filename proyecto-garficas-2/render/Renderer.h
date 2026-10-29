#pragma once

#include <glad/gl.h>
#include <scene/Scene.h>
#include <algorithm>
#include <scene/ParticleSystem.h>

struct PickingResult {
    uint32_t objectId;    // 0 si no hay nada
    uint32_t triangleId;  // solo válido si objectId != 0
};

class Renderer {
public:
    void init();
    void renderScene(Scene* scene, float aspectRatio);
    ~Renderer();
    void initPicking(int width, int height);
    void renderForPicking(Scene* scene);
    PickingResult readPixel(int x, int y);
    void drawParticles(Scene* scene, float aspectRatio);

private:
    GLuint shaderProgram = 0;
    GLuint ModelID;
    GLuint ViewID;
    GLuint ProjectionID;
    GLuint ObjectColorID;
    GLuint LightDirID;
    GLuint LightColorID;
    GLuint AmbientID;
    //Para las normales y la bounding box
    GLuint debugProgram = 0;
    GLuint debugVao = 0, debugVbo = 0;
    GLuint debugMvpID = 0, debugColorID = 0;
    //Texturas
    GLuint TextureID;     // sampler2D
    GLuint UseTextureID;  // bool para si se esta usando o no

    //Para selección
    GLuint pickingProgram = 0;
    GLuint pickingModelID = 0;
    GLuint pickingViewID = 0;
    GLuint pickingProjectionID = 0;
    GLuint pickingObjectIdID = 0;   //el uniform "objectId"
    GLuint pickingFbo = 0;          //el framebuffer object
    GLuint pickingColorTex = 0;     //la texture donde se guardan los IDs.
    GLuint pickingDepthRbo = 0;
    GLuint pickingBaseTriangleID = 0; //Offset para los triángulso bases a la hora de hacer picking en submeshes
    //tamaño del FBO
    int pickingWidth = 0;
    int pickingHeight = 0;

    //Para las luces
    GLuint numLightsID;
    GLuint viewPosID;       // Posición de la cámara para el brillo especular
    GLuint shininessID;     // Brillo del material
    GLuint shadingModeID;   // 0=Flat, 1=Gouraud(Lambert), 2=Blinn-Phong

    // Arrays para soportar hasta 8 luces (MAX_LIGHTS)
    GLuint lightTypeID[8];
    GLuint lightPosID[8];
    GLuint lightDirID[8];
    GLuint lightColorID[8];
    GLuint lightIntensityID[8];
    GLuint lightConstantID[8];
    GLuint lightLinearID[8];
    GLuint lightQuadraticID[8];
    GLuint lightCutOffID[8];
    GLuint lightOuterCutOffID[8];

    //Skybox
    GLuint skyboxVAO;
    GLuint skyboxVBO;
    GLuint skyboxProgram = 0;
    GLuint skyboxProjectionID;
    GLuint skyboxViewID;
    GLuint cubemapTextureID;

    //Particles
    GLuint particleProgram, particleVAO, particleVBO;
    GLuint particleProjID, particleViewID;

    void drawDebugLines(const std::vector<glm::vec3>& pts, const glm::mat4& mvp, const glm::vec3& color);
};

GLuint LoadTexture(const char* path);
GLuint LoadCubemap(const std::vector<std::string>& faces);