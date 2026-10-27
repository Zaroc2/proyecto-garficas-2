#include <render/Renderer.h>
#include <render/Shader.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdio>
//Para texturas
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h> 
#include <glad/gl.h>

static glm::vec3 cameraPos;

static bool compareDistance(const std::unique_ptr<Object>& a, const std::unique_ptr<Object>& b) {
    float distA = glm::distance(a->transform.position, cameraPos);
    float distB = glm::distance(b->transform.position, cameraPos);
    return distA > distB; // Más lejos primero
}
//Variable y función estática y global para poder ordenarlos por orden respecto a la posición de la cámara con sort


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
    TextureID = glGetUniformLocation(shaderProgram, "objTexture");
    UseTextureID = glGetUniformLocation(shaderProgram, "useTexture");

    //Luces
    numLightsID = glGetUniformLocation(shaderProgram, "numLights");
    viewPosID = glGetUniformLocation(shaderProgram, "viewPos");
    shininessID = glGetUniformLocation(shaderProgram, "shininess");
    shadingModeID = glGetUniformLocation(shaderProgram, "currentShadingMode");

    for (int i = 0; i < 8; i++) {
        // un for para "lights[0].position", "lights[1].position", etc.
        std::string base = "lights[" + std::to_string(i) + "].";

        lightTypeID[i] = glGetUniformLocation(shaderProgram, (base + "type").c_str());
        lightPosID[i] = glGetUniformLocation(shaderProgram, (base + "position").c_str());
        lightDirID[i] = glGetUniformLocation(shaderProgram, (base + "direction").c_str());
        lightColorID[i] = glGetUniformLocation(shaderProgram, (base + "color").c_str());
        lightIntensityID[i] = glGetUniformLocation(shaderProgram, (base + "intensity").c_str());
        lightConstantID[i] = glGetUniformLocation(shaderProgram, (base + "constant").c_str());
        lightLinearID[i] = glGetUniformLocation(shaderProgram, (base + "linear").c_str());
        lightQuadraticID[i] = glGetUniformLocation(shaderProgram, (base + "quadratic").c_str());
        lightCutOffID[i] = glGetUniformLocation(shaderProgram, (base + "cutOff").c_str());
        lightOuterCutOffID[i] = glGetUniformLocation(shaderProgram, (base + "outerCutOff").c_str());
    }

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
    pickingBaseTriangleID = glGetUniformLocation(pickingProgram, "baseTriangle");

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

    if (scene->backFaceCullingEnabled) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);       // Cullear las caras traseras
    }
    else {
        glDisable(GL_CULL_FACE);
    }

    if (scene->blendingEnabled) {
        //Activamos blend, glBlendFunc define color_final = alpha_nuevo * color_nuevo + (1 - alpha_nuevo) * color_detras
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else {
        glDisable(GL_BLEND);
    }

    // Matrices de cámara
    glm::mat4 Projection = scene->camera->getProjectionMatrix(aspectRatio);
    glm::mat4 View = scene->camera->getViewMatrix();

    // Dibujar cada objeto
    glUseProgram(shaderProgram);

    //Tenemos que ordenar los objetos transparentes antes de dibujarlos:
    cameraPos = scene->camera->position;
    std::sort(scene->objects.begin(), scene->objects.end(), compareDistance);

    //Enviamos los datos de luces y cámara al shader
    glUniform3fv(viewPosID, 1, glm::value_ptr(scene->camera->position));
    glUniform1i(numLightsID, (int)scene->lights.size());
    glUniform1i(shadingModeID, scene->shadingMode); // Necesitas añadir esto a tu clase Scene

    for (int i = 0; i < scene->lights.size() && i < 8; i++) {
        const Light& l = scene->lights[i];

        glUniform1i(lightTypeID[i], static_cast<int>(l.type));
        glUniform3fv(lightPosID[i], 1, glm::value_ptr(l.position));
        glUniform3fv(lightDirID[i], 1, glm::value_ptr(l.direction));
        glUniform3fv(lightColorID[i], 1, glm::value_ptr(l.color));

        glUniform1f(lightIntensityID[i], l.intensity);
        glUniform1f(lightConstantID[i], l.constant);
        glUniform1f(lightLinearID[i], l.linear);
        glUniform1f(lightQuadraticID[i], l.quadratic);
        glUniform1f(lightCutOffID[i], l.cutOff);
        glUniform1f(lightOuterCutOffID[i], l.outerCutOff);
    }

    for (int pass = 0; pass < 2; ++pass) {//Hacemos dos pasadas, una para los objetos opacos una para los transparentes
        if (pass == 1) 
            glDepthMask(GL_FALSE);

        for (int i = 0; i < scene->objects.size(); ++i) {
            Object* obj = scene->objects[i].get();
            if (obj->mesh == nullptr) continue;

            bool isTransparent = obj->material.alpha < 1.0f;
            if (pass == 0 && isTransparent) continue;
            if (pass == 1 && !isTransparent) continue;

            glm::mat4 Model = obj->transform.getModelMatrix();

            glUniformMatrix4fv(ModelID, 1, GL_FALSE, &Model[0][0]);
            glUniformMatrix4fv(ViewID, 1, GL_FALSE, &View[0][0]);
            glUniformMatrix4fv(ProjectionID, 1, GL_FALSE, &Projection[0][0]);

            // objectColor ahora es vec4 → incluye alpha
            glUniform4f(ObjectColorID, obj->material.diffuseColor.r,
                obj->material.diffuseColor.g,
                obj->material.diffuseColor.b,
                obj->material.alpha);
            glUniform1f(shininessID, obj->material.shininess);


            if (obj->material.useTexture && obj->material.textureID != 0) {
                glActiveTexture(GL_TEXTURE0); // Activamos la unidad de textura 0
                glBindTexture(GL_TEXTURE_2D, obj->material.textureID); // Bindear la textura de este objeto a la unidad de textura activa
                glUniform1i(TextureID, 0); // objTexture está en la unidad 0
                glUniform1i(UseTextureID, 1); // Activar el flag en el shader
            }
            else {
                glUniform1i(UseTextureID, 0); // Desactivar el flag si no tiene textura
            }

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

            //obj->mesh->draw(); Ya no dibujamos al objeto porque dibujamos por submeshes

            const std::vector<SubMesh>& subMeshes = obj->subMeshes;

            if (subMeshes.empty()) {
                // Si no tiene sub-meshes, se dibuja como antes
                obj->mesh->draw();
            }
            else {
                for (int i = 0; i < subMeshes.size(); ++i) {
                    // matriz final = transform del objeto * transform del submesh
                    glm::mat4 subLocalMatrix = subMeshes[i].transform.getModelMatrix();
                    glm::mat4 finalModel = Model * subLocalMatrix;

                    // Le pasamos la nueva matriz al shader principal
                    glUniformMatrix4fv(ModelID, 1, GL_FALSE, &finalModel[0][0]);

                    glBindVertexArray(obj->mesh->getVAO());
                    glDrawElements(GL_TRIANGLES, subMeshes[i].indexCount, GL_UNSIGNED_INT, (void*)(subMeshes[i].indexOffset * sizeof(uint32_t)));
                    glBindVertexArray(0);
                }
            }

            if (obj->selectedTriangleId >= 0 || obj->selectedSubMesh >= 0) {
                //Dibujamos los submeshes o triángulos
                glUseProgram(debugProgram);


                glm::mat4 selectedModel = Model;
                int subIdx = obj->selectedSubMesh;

                // buscamos a qué sub-mesh pertenece el triángulo
                if (subIdx < 0 && obj->selectedTriangleId >= 0) {
                    uint32_t triStart = obj->selectedTriangleId * 3;
                    for (size_t k = 0; k < obj->subMeshes.size(); ++k) {
                        if (triStart >= obj->subMeshes[k].indexOffset &&
                            triStart < obj->subMeshes[k].indexOffset + obj->subMeshes[k].indexCount) {
                            subIdx = (int)k;
                            break;
                        }
                    }
                }

                // Multiplicamos por la matriz del sub-mesh
                if (subIdx >= 0 && subIdx < obj->subMeshes.size()) {
                    selectedModel = Model * obj->subMeshes[subIdx].transform.getModelMatrix();
                }

                glm::mat4 MVP = Projection * View * selectedModel;

                glUniformMatrix4fv(debugMvpID, 1, GL_FALSE, &MVP[0][0]);
                glUniform3f(debugColorID, 1.0f, 0.5f, 0.0f);   // pintamos de naranja

                GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);
                glDisable(GL_DEPTH_TEST); // desactivamos depthtest para que se dibujo encima si o si
                //bindeamos su vao y dibujamos
                glBindVertexArray(obj->mesh->getVAO());

                if (obj->selectedTriangleId >= 0) {
                    // Un solo triángulo
                    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(obj->selectedTriangleId * 3 * sizeof(uint32_t)));
                }
                else if (obj->selectedSubMesh >= 0) {
                    // todo el submesh
                    const SubMesh& sub = obj->subMeshes[obj->selectedSubMesh];
                    glDrawElements(GL_TRIANGLES, sub.indexCount, GL_UNSIGNED_INT, (void*)(sub.indexOffset * sizeof(uint32_t)));
                }

                //desbindeamos el vao, activamos depth test y el shaderProgram
                glBindVertexArray(0);
                if (depthWasEnabled) 
                    glEnable(GL_DEPTH_TEST);
                glUseProgram(shaderProgram);
            }

            if (obj->wireframe || obj->showVertices) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
        }
    }
    glDepthMask(GL_TRUE);

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

    //Verificamos el tamaño de la pantalla por si hubo resize
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    int width = vp[2];
    int height = vp[3];

    // Si el FBO no tiene ese tamaño, redimensionarlo
    if (width != pickingWidth || height != pickingHeight) {
        pickingWidth = width;
        pickingHeight = height;

        glBindTexture(GL_TEXTURE_2D, pickingColorTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32UI, width, height, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, nullptr);

        glBindRenderbuffer(GL_RENDERBUFFER, pickingDepthRbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    }

    glViewport(0, 0, pickingWidth, pickingHeight);

    glDisable(GL_BLEND);

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

        // eniamos el id del objeto como uint por opengl para que lo reciba el frag shader
        glUniform1ui(pickingObjectIdID, (GLuint)obj->id);


        //enviamos la matriz model como uniform
        glm::mat4 Model = obj->transform.getModelMatrix();

        const std::vector<SubMesh>& subMeshes = obj->subMeshes;


        if (subMeshes.size() < 1) { //Si tieneun solo subMesh o no tiene, es el objeto completo, si no por submeshes para poder escogerlos en su posición
			glUniform1i(pickingBaseTriangleID, 0); //Enviamos el offset en 0 si no hay submeshes
            glUniformMatrix4fv(pickingModelID, 1, GL_FALSE, &Model[0][0]);
            obj->mesh->draw();
        }
        else {
            for (int j = 0; j < subMeshes.size(); ++j) {

                // Calculamos cuántos triángulos hay antes de este sub-mesh
                int baseTri = subMeshes[j].indexOffset / 3;
                glUniform1i(pickingBaseTriangleID, baseTri);


                glm::mat4 finalModel = Model * subMeshes[j].transform.getModelMatrix();
                glUniformMatrix4fv(pickingModelID, 1, GL_FALSE, &finalModel[0][0]);

                glBindVertexArray(obj->mesh->getVAO());
                glDrawElements(GL_TRIANGLES, subMeshes[j].indexCount, GL_UNSIGNED_INT, (void*)(subMeshes[j].indexOffset * sizeof(uint32_t)));
                glBindVertexArray(0);
            }
        }
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

GLuint LoadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrComponents;
    // Forzamos 4 canales (RGBA) para evitar problemas con PNGs transparentes
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 4); 
    
    if (data) {
        GLenum format = GL_RGBA; // Como forzamos 4, siempre es RGBA
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Parámetros para que la textura se repita si las UVs son > 1.0
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        printf("Textura cargada: %s (%dx%d)\n", path, width, height);
    } else {
        printf("Error al cargar textura: %s\n", path);
        stbi_image_free(data);
    }
    
    return textureID;
}
