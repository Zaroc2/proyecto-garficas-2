#pragma once

#include <glad/gl.h>
#include <scene/Scene.h>

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

    //Para selección
    GLuint pickingProgram = 0;
    GLuint pickingModelID = 0;
    GLuint pickingViewID = 0;
    GLuint pickingProjectionID = 0;
    GLuint pickingObjectIdID = 0;   //el uniform "objectId"
    GLuint pickingFbo = 0;          //el framebuffer object
    GLuint pickingColorTex = 0;     //la texture donde se guardan los IDs.
    GLuint pickingDepthRbo = 0;
    //tamaño del FBO
    int pickingWidth = 0;
    int pickingHeight = 0;

    void drawDebugLines(const std::vector<glm::vec3>& pts, const glm::mat4& mvp, const glm::vec3& color);
};