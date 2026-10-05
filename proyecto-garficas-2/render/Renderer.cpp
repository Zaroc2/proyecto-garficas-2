#include <render/Renderer.h>
#include <render/Shader.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdio>

void Renderer::init() {
    //Se cargan los sahders
    shaderProgram = LoadShaders("../../../../assets/shaders/base.vert",
        "../../../../assets/shaders/base.frag");
    if (shaderProgram == 0) {
        printf("Error: no se pudieron compilar los shaders\n");
        return;
    }

    //Se inicializan las variables uniform
    ModelID = glGetUniformLocation(shaderProgram, "model");
    ViewID = glGetUniformLocation(shaderProgram, "view");
    ProjectionID = glGetUniformLocation(shaderProgram, "projection");
    ObjectColorID = glGetUniformLocation(shaderProgram, "objectColor"); // vec4
    LightDirID = glGetUniformLocation(shaderProgram, "lightDir");
    LightColorID = glGetUniformLocation(shaderProgram, "lightColor");
    AmbientID = glGetUniformLocation(shaderProgram, "ambientLight");

    //Cargamos el vao y el vbo para el dibujo de normales y bounding boxes
    debugProgram = LoadShaders("../../../../assets/shaders/debug.vert",
        "../../../../assets/shaders/debug.frag");
    debugMvpID = glGetUniformLocation(debugProgram, "MVP");
    debugColorID = glGetUniformLocation(debugProgram, "lineColor");

    glGenVertexArrays(1, &debugVao);
    glGenBuffers(1, &debugVbo);
    glBindVertexArray(debugVao);
    glBindBuffer(GL_ARRAY_BUFFER, debugVbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
    glBindVertexArray(0);

}

void Renderer::initPicking(int width, int height) {
    pickingWidth = width;
    pickingHeight = height;

    pickingProgram = LoadShaders("../../../../assets/shaders/picking.vert",
        "../../../../assets/shaders/picking.frag");
    if (pickingProgram == 0) {
        printf("Error: no se pudo compilar el shader de picking\n");
        return;
    }
    //inicializamos las variables uniform
    pickingModelID = glGetUniformLocation(pickingProgram, "model");
    pickingViewID = glGetUniformLocation(pickingProgram, "view");
    pickingProjectionID = glGetUniformLocation(pickingProgram, "projection");
    pickingObjectIdID = glGetUniformLocation(pickingProgram, "objectId");

    // inicializamos el buffer
    glGenFramebuffers(1, &pickingFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, pickingFbo);

    // bindeamos la textura para poder hacer el color picking
    glGenTextures(1, &pickingColorTex);
    glBindTexture(GL_TEXTURE_2D, pickingColorTex);
    //Configuramos la textura actual para que sea un GL_RGBA32UI (4 uint32), de datos enteros, unsinged, y por ahora dejamos vacíala memoria
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32UI, width, height, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, nullptr);

    // Con esto le decimos a opengl que no interpole ni use mipmaps, para poder tener exactamente los pixeles que escribimos
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //Conectamos la textura al Framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pickingColorTex, 0);

    // un renderbuffer es como una texture pero pensada para ser usada como buffer de render
    // Aqui lo configuramos como depth buffer del FBO, con 24 bits por pixel.
    glGenRenderbuffers(1, &pickingDepthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, pickingDepthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pickingDepthRbo);

    // verificación del buffer
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("Error: FBO de picking incompleto\n");
    }

    // Desbindeamos el frambuffer para que no se dibuje, las texturas y el renderbuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

Renderer::~Renderer() {
    if (shaderProgram != 0)
        glDeleteProgram(shaderProgram);
    if (pickingProgram != 0)
        glDeleteProgram(pickingProgram);
    if (pickingFbo != 0)
        glDeleteFramebuffers(1, &pickingFbo);
    if (pickingColorTex != 0)
        glDeleteTextures(1, &pickingColorTex);
    if (pickingDepthRbo != 0)
        glDeleteRenderbuffers(1, &pickingDepthRbo);
    if (debugProgram != 0)
        glDeleteProgram(debugProgram);
    if (debugVao != 0)
        glDeleteVertexArrays(1, &debugVao);
    if (debugVbo != 0)
        glDeleteBuffers(1, &debugVbo);
}

void Renderer::renderScene(Scene* scene, float aspectRatio) {
    // Limpiar pantalla con el color de fondo de la escena
    glClearColor(scene->backgroundColor.r,
        scene->backgroundColor.g,
        scene->backgroundColor.b,
        1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Estados configurables
    if (scene->depthTestEnabled)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    if (scene->backFaceCullingEnabled)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);

    // Matrices de cámara
    glm::mat4 Projection = scene->camera->getProjectionMatrix(aspectRatio);
    glm::mat4 View = scene->camera->getViewMatrix();

    // Dibujar cada objeto
    glUseProgram(shaderProgram);
    for (int i = 0; i < scene->objects.size(); ++i) {
        Object* obj = scene->objects[i].get();
        if (obj->mesh == nullptr) continue;

        glm::mat4 Model = obj->transform.getModelMatrix();

        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &Model[0][0]);
        glUniformMatrix4fv(ViewID, 1, GL_FALSE, &View[0][0]);
        glUniformMatrix4fv(ProjectionID, 1, GL_FALSE, &Projection[0][0]);

        // objectColor ahora es vec4 → incluye alpha
        glUniform4f(ObjectColorID, obj->diffuseColor.r,
            obj->diffuseColor.g,
            obj->diffuseColor.b,
            obj->alpha);

        glUniform3f(LightDirID, -0.5f, -1.0f, -0.3f); // dirección (se normaliza en el shader)
        glUniform3f(LightColorID, 1.0f, 1.0f, 1.0f); // o el color que quieran
        glUniform3f(AmbientID, 0.3f, 0.3f, 0.3f);

        if (obj->wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        if (obj->showVertices) {
            glPointSize(6.0f);
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);   // o usar glDrawArrays(GL_POINTS)
        }

        if (obj->showBBox) {
            glm::vec3 mn = obj->mesh->getBBoxMin();
            glm::vec3 mx = obj->mesh->getBBoxMax();
            glm::vec3 c[8] = {
                {mn.x,mn.y,mn.z},{mx.x,mn.y,mn.z},{mx.x,mx.y,mn.z},{mn.x,mx.y,mn.z},
                {mn.x,mn.y,mx.z},{mx.x,mn.y,mx.z},{mx.x,mx.y,mx.z},{mn.x,mx.y,mx.z}
            };
            int e[12][2] = { {0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},
                            {0,4},{1,5},{2,6},{3,7} };
            std::vector<glm::vec3> lines;
            for (auto& ed : e) { lines.push_back(c[ed[0]]); lines.push_back(c[ed[1]]); }
            drawDebugLines(lines, (Projection * View * Model), glm::vec3(0, 1, 0));   // verde
        }

        if (obj->showNormals) {
            std::vector<glm::vec3> lines;
            float len = 0.15f;
            for (const auto& v : obj->mesh->getCPUVertices()) {
                lines.push_back(v.position);
                lines.push_back(v.position + v.normal * len);   // en LOCAL, no mundo
            }
            drawDebugLines(lines, (Projection * View * Model), glm::vec3(1, 1, 0));     // amarillo
        }

        obj->mesh->draw();

        if (obj->selectedTriangleId >= 0 || obj->selectedSubMesh >= 0) {
            //Dibujamos los objetos seleccionados
            glUseProgram(debugProgram);

            glm::mat4 MVP = Projection * View * Model;
            glUniformMatrix4fv(debugMvpID, 1, GL_FALSE, &MVP[0][0]);
            glUniform3f(debugColorID, 1.0f, 0.5f, 0.0f);   // pintamos de naranja

            glDisable(GL_DEPTH_TEST); // desactivamos depthtest para que se dibujo encima si o si
            //bindeamos su vao y dibujamos
            glBindVertexArray(obj->mesh->getVAO());

            if (obj->selectedTriangleId >= 0) {
                // Un solo triángulo
                glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(obj->selectedTriangleId * 3 * sizeof(uint32_t)));
            }
            else if (obj->selectedSubMesh >= 0) {
                // todo el submesh
                const SubMesh& sub = obj->mesh->getSubMeshes()[obj->selectedSubMesh];
                glDrawElements(GL_TRIANGLES, sub.indexCount, GL_UNSIGNED_INT, (void*)(sub.indexOffset * sizeof(uint32_t)));
            }

            //desbindeamos el vao, activamos depth test y el shaderProgram
            glBindVertexArray(0);
            glEnable(GL_DEPTH_TEST);
            glUseProgram(shaderProgram);
        }

        if (obj->wireframe || obj->showVertices) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
}

//Esta funcion es para dibujar las lineas usando los debug shader para las normales y la bounding box
void Renderer::drawDebugLines(const std::vector<glm::vec3>& pts,
    const glm::mat4& mvp,
    const glm::vec3& color)
{
    if (pts.empty()) return;
    glUseProgram(debugProgram);
    glUniformMatrix4fv(debugMvpID, 1, GL_FALSE, &mvp[0][0]);
    glUniform3f(debugColorID, color.r, color.g, color.b);

    glBindVertexArray(debugVao);
    glBindBuffer(GL_ARRAY_BUFFER, debugVbo);
    glBufferData(GL_ARRAY_BUFFER, pts.size() * sizeof(glm::vec3),
        pts.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINES, 0, (GLsizei)pts.size());
    glBindVertexArray(0);
    glUseProgram(shaderProgram);   // volver al shader principal
}

void Renderer::renderForPicking(Scene* scene) {
    // bindeamos el fbo
    glBindFramebuffer(GL_FRAMEBUFFER, pickingFbo);
    glViewport(0, 0, pickingWidth, pickingHeight);

    //limpiamos la pantalla, un objeto no puede ser 0.0f, es el color de no selección
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // mismos toggles 
    if (scene->depthTestEnabled)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
    if (scene->backFaceCullingEnabled)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);

    // usamos los shaders para seleccion
    glUseProgram(pickingProgram);

    // calculamos igual las matrices de projecion y view
    float aspect = (float)pickingWidth / (float)pickingHeight;
    glm::mat4 Projection = scene->camera->getProjectionMatrix(aspect);
    glm::mat4 View = scene->camera->getViewMatrix();

    //Enviamos las matrices projection y view como uniform
    glUniformMatrix4fv(pickingViewID, 1, GL_FALSE, &View[0][0]);
    glUniformMatrix4fv(pickingProjectionID, 1, GL_FALSE, &Projection[0][0]);

    // dibujamos los objetos
    for (int i = 0; i < scene->objects.size(); ++i) {
        Object* obj = scene->objects[i].get();
        if (obj->mesh == nullptr)
            continue;

        //enviamos la matriz model como uniform
        glm::mat4 Model = obj->transform.getModelMatrix();
        glUniformMatrix4fv(pickingModelID, 1, GL_FALSE, &Model[0][0]);

        // eniamos el id del objeto como uint por opengl para que lo reciba el frag shader
        glUniform1ui(pickingObjectIdID, (GLuint)obj->id);

        obj->mesh->draw();
    }

    //desbindeamos y y volvemos al shader del programa
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(shaderProgram);
}


PickingResult Renderer::readPixel(int x, int y) {
    PickingResult result = { 0u, 0u };

    //bindeamos el buffer y lo configuramos para leer de él el atachment de color 0
    glBindFramebuffer(GL_READ_FRAMEBUFFER, pickingFbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    // OpenGL lee desde la esquina inferior izquierda.
    // Si x,y vienen en coords de ventana (arriba-izquierda), hay que invertir la Y.
    int newY = pickingHeight - 1 - y;

    //obtenemos el pixel en pixel (id,trianguloid,0,0)
    uint32_t pixel[4] = { 0, 0, 0, 0 };
    glReadPixels(x, newY, 1, 1, GL_RGBA_INTEGER, GL_UNSIGNED_INT, pixel);

    result.objectId = pixel[0];
    result.triangleId = pixel[1];

    //desbindeamos nuevamente y retornamos
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    return result;
}