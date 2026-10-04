# CMakeList.txt: proyecto de CMake para proyecto-garficas-2, incluya el origen y defina
# la lógica específica del proyecto aquí.
#

set(PROJECT_SOURCES
    proyecto-garficas-2
    "proyecto-garficas-2.cpp"
    "core/Window.cpp"
    # core/Application.cpp
    "render/Renderer.cpp"
    "render/Shader.cpp"
    "render/Mesh.cpp"
    "render/MeshManager.cpp"
    # render/Texture.cpp
    "scene/Scene.cpp"
    "scene/Object.cpp"
    "scene/Camera.cpp"
    # scene/Material.cpp
    "io/ModelLoader.cpp"
    # "io/MaterialLoader.cpp"
    "io/SceneSerializer.cpp"
    # ui/UI.cpp
)

set(PROJECT_HEADERS
    "proyecto-garficas-2.h"
    "core/Window.h"
    # core/Application.h
    "render/Renderer.h"
    "render/Vertex.h"
    "render/Shader.h"
    "render/Mesh.h"
    "render/MeshManager.h"
    # render/Texture.h
    "scene/Scene.h"
    "scene/Object.h"
    "scene/Camera.h"
    # scene/Material.h
    "io/ModelLoader.h"
    # "io/MaterialLoader.h"
    "io/SceneSerializer.h"
    # ui/UI.h
)

add_executable (proyecto-garficas-2 ${PROJECT_SOURCES} ${PROJECT_HEADERS} "render/MeshManager.h" "render/MeshManager.cpp")

# add_executable (proyecto-garficas-2 "proyecto-garficas-2.cpp" "proyecto-garficas-2.h" "core/window.cpp" "core/window.h" "scene/Scene.h" "scene/Scene.cpp" "scene/Object.h" "scene/Object.cpp" "scene/Camera.h" "scene/Camera.cpp")

# Agregue un origen al ejecutable de este proyecto.
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET proyecto-garficas-2 PROPERTY CXX_STANDARD 20)
endif()

# TODO: Agregue pruebas y destinos de instalación si es necesario.


target_include_directories(proyecto-garficas-2 PRIVATE ./)

target_link_libraries(proyecto-garficas-2 PRIVATE 
    glfw 
    glm
    glad
    imgui
    OpenGL::GL
)
