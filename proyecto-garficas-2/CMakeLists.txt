# CMakeList.txt: proyecto de CMake para proyecto-garficas-2, incluya el origen y defina
# la lógica específica del proyecto aquí.
#

# Agregue un origen al ejecutable de este proyecto.
add_executable (proyecto-garficas-2 "proyecto-garficas-2.cpp" "proyecto-garficas-2.h" "core/window.cpp" "core/window.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET proyecto-garficas-2 PROPERTY CXX_STANDARD 20)
endif()

# TODO: Agregue pruebas y destinos de instalación si es necesario.


target_include_directories(proyecto-garficas-2 PRIVATE include)

target_link_libraries(proyecto-garficas-2 PRIVATE 
    glfw  
    glm
    glad
    imgui
    OpenGL::GL
)