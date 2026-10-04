# ✅ Checklist de tareas — Proyecto #2 CGI

Leyenda:
- 👤 **A** = Compañero A (Escena, UI, Lógica)
- 👤 **B** = Compañero B (Renderizado, I/O)
- 🔥 = Tarea crítica / alto riesgo
- 🟢 = Tarea sencilla / bajo riesgo
- 🤝 = Requiere ambos

---

## 🚀 FASE 0 — Definición conjunta de interfaces (30 min)

- [x] 🤝 Definir `struct Transform` (position, rotation, scale + `getModelMatrix()`)
- [x] 🤝 Definir `struct Object` (id, name, mesh, transform, color, alpha, banderas de visualización)
- [x] 🤝 Definir `class Scene` (objects, camera, backgroundColor, flags, métodos)
- [x] 🤝 Definir `struct Vertex` (`glm::vec3 position; glm::vec3 normal;`) en `src/render/Vertex.h`
- [x] 🤝 Definir interfaz de `Renderer` (`renderScene`, `renderForPicking`, `readPixel`)
- [x] 🤝 Definir interfaz de `ModelLoader` (`loadOBJ` con `LoadedModel`)
- [x] 🤝 Definir interfaz de `SceneSerializer` (`saveScene`, `loadScene`)
- [ ] 🤝 Definir `enum class SelectionMode { LOCAL, GLOBAL }`
- [x] 🤝 Configurar repositorio Git y `.gitignore` (build/, .vscode/, etc.)

---

## 🧱 COMPAÑERO A — Escena, UI y Lógica

### A1. Clases base de escena

- [x] 👤 A Crear `src/scene/Transform.h` con struct y `getModelMatrix()` (orden TRS)
- [x] 👤 A Crear `src/scene/Object.h` con struct `Object` y banderas (`wireframe`, `showNormals`, `showVertices`, `showBBox`, `selected`)
- [x] 👤 A Crear `src/scene/Scene.h` y `Scene.cpp`
- [x] 👤 A Implementar `Scene::addObject` (asignar ID, mover unique_ptr, devolver raw)
- [x] 👤 A Implementar `Scene::removeObject(uint32_t id)`
- [x] 👤 A Implementar `Scene::clear()` (limpiar objetos y resetear `nextId`)
- [x] 👤 A Implementar contador `nextId` empezando en **1** (0 reservado para "vacío")

### A2. Cámara

- [x] 👤 A Crear `src/scene/Camera.h` y `Camera.cpp`
- [x] 👤 A Implementar `getViewMatrix()` con `glm::lookAt(position, position + front, up)`
- [x] 👤 A Implementar `getProjectionMatrix(aspect)` con `glm::perspective`
- [x] 👤 A Implementar `updateFromMouse(dx, dy)` (yaw, pitch, clamp pitch a ±89°)
- [x] 👤 A Implementar `moveForward(dt)` y `moveRight(dt)` (WASD)
- [ ] 👤 A Integrar callback de ratón con `glfwSetCursorPosCallback`
- [ ] 👤 A Integrar lectura de teclado WASD en el bucle principal
- [ ] 👤 A Implementar toggle de captura del cursor (click en la ventana)

### A3. Carga de `.mtl`

- [ ] 👤 A Crear `src/io/MaterialLoader.h` y `.cpp`
- [ ] 👤 A Implementar `parseMTL(path)` que extraiga `Kd` (color difuso)
- [ ] 👤 A Devolver color por defecto `{0.7, 0.7, 0.7}` si no hay `Kd`
- [ ] 👤 A Coordinar con B para que `loadOBJ` devuelva `diffuseColor` en `LoadedModel`

### A4. Selección — lógica

- [ ] 👤 A Implementar `Scene::findById(uint32_t id)` (chequear `id == 0` primero)
- [ ] 👤 A Implementar `Scene::selectAtPixel(pixelId, mode)`
- [ ] 👤 A Definir semántica de limpieza de selección (GLOBAL limpia todo, LOCAL toggle)
- [ ] 👤 A Añadir método `Scene::getSelected()` que devuelva el/los seleccionados
- [ ] 👤 A Documentar TODO para sub-mallas en modo LOCAL

### A5. Primitivas parametrizables

- [ ] 👤 A Crear `src/io/Primitives.h` y `.cpp` (o dentro de ModelLoader)
- [ ] 👤 A Implementar `generateCube(LoadedModel& out, float size)`
- [ ] 👤 A Implementar `generateSphere(LoadedModel& out, float radius, int segments)`
- [ ] 👤 A Implementar `generatePyramid(LoadedModel& out, float base, float height)`
- [ ] 👤 A Verificar que las normales apunten hacia afuera en las tres
- [ ] 👤 A Integrar botones "Crear Cubo/Esfera/Pirámide" en la UI
- [ ] 🟢 👤 A *Opcional*: `generateCylinder(LoadedModel& out, float radius, float height, int segments)`

### A6. Serialización JSON

- [ ] 👤 A Añadir `std::string sourcePath` a `Object` (ruta del `.obj` original)
- [ ] 👤 A Crear `src/io/SceneSerializer.h` y `.cpp`
- [ ] 👤 A Implementar `saveScene(scene, path)` → JSON con cámara, fondo, objetos
- [ ] 👤 A Implementar `loadScene(scene, path)` → reconstruir escena (llamando a `loadOBJ`)
- [ ] 👤 A Probar round-trip: guardar → cerrar → abrir → cargar → verificar
- [ ] 👤 A Integrar botones "Guardar/Cargar escena" en la UI

### A7. UI con ImGui

- [ ] 👤 A Añadir ImGui al CMake (FetchContent o submodulo)
- [ ] 👤 A Crear `src/ui/UI.h` y `UI.cpp`
- [ ] 👤 A Inicializar ImGui (contexto + backend GLFW + backend OpenGL3)
- [ ] 👤 A Implementar panel "Objeto seleccionado" con:
  - [ ] 👤 A `DragFloat3` para posición
  - [ ] 👤 A `DragFloat3` para rotación
  - [ ] 👤 A `DragFloat3` para escala
  - [ ] 👤 A `ColorEdit3` para color difuso
  - [ ] 👤 A Botón "Eliminar objeto"
- [ ] 👤 A Implementar panel "Visualización" con checkboxes:
  - [ ] 👤 A Wireframe
  - [ ] 👤 A Normales
  - [ ] 👤 A Vértices
  - [ ] 👤 A Bounding Box
- [ ] 👤 A Implementar panel "Escena":
  - [ ] 👤 A Checkbox "Depth Test"
  - [ ] 👤 A Checkbox "Back-Face Culling"
  - [ ] 👤 A `ColorEdit3` para color de fondo
  - [ ] 👤 A Botón "Limpiar escena"
  - [ ] 👤 A Texto con FPS actuales
- [ ] 👤 A Implementar panel "Crear objeto": Cubo, Esfera, Pirámide
- [ ] 👤 A Implementar panel "Archivo": Guardar, Cargar
- [ ] 👤 A Añadir selector de modo de selección (LOCAL / GLOBAL)

### A8. Opcionales de A

- [ ] 🟢 👤 A *Opcional*: `DragFloat` para alpha del color difuso (transparencias)
- [ ] 🟢 👤 A *Opcional*: Modo de selección por triángulo en la UI
- [ ] 🟢 👤 A *Opcional*: Marcar visualmente el triángulo seleccionado

---

## 🎨 COMPAÑERO B — Renderizado y Entrada/Salida

### B1. Integración de TinyObjLoader

- [ ] 👤 B Añadir `tiny_obj_loader.h` al proyecto (FetchContent o `external/`)
- [ ] 👤 B Configurar include path en CMake
- [ ] 👤 B Crear `src/io/ModelLoader.h` y `.cpp`
- [ ] 👤 B Implementar `loadOBJ(objPath, mtlPath, out)` con `tinyobj::LoadObj`
- [ ] 👤 B Rellenar `LoadedModel` (vertices, indices, diffuseColor, name)
- [ ] 👤 B Manejar errores (archivo no encontrado, formato inválido)
- [ ] 👤 B Probar con un `.obj` de prueba (bunny, teapot, etc.)

### B2. Cálculo de normales promedio

- [ ] 🔥 👤 B Detectar si el `.obj` no tiene normales (`index.normal_index < 0`)
- [ ] 👤 B Acumular la normal de cada triángulo en sus 3 vértices
- [ ] 👤 B Normalizar cada normal acumulada al final
- [ ] 👤 B Verificar que el cálculo funcione en un cubo (normales por cara)
- [ ] 👤 B Normalizar el objeto (escala a ~1 unidad) y sus normales al cargar

### B3. Clase `Shader`

- [ ] 👤 B Crear `src/render/Shader.h` y `.cpp`
- [ ] 👤 B Envolver `LoadShaders` en constructor
- [ ] 👤 B Implementar `use()`, `setMat4`, `setVec3`, `setFloat`, `setUint`
- [ ] 👤 B Implementar `getID()`
- [ ] 👤 B Liberar programa en destructor (`glDeleteProgram`)

### B4. Clase `Mesh`

- [ ] 👤 B Crear `src/render/Mesh.h` y `.cpp`
- [ ] 👤 B Constructor que recibe `vector<Vertex>` y `vector<uint32_t>`
- [ ] 👤 B Crear VAO, VBO, EBO con `glGen*`
- [ ] 👤 B Bindear VAO y subir VBO con `glBufferData`
- [ ] 👤 B Configurar atributo 0 (posición) con `glVertexAttribPointer`
- [ ] 👤 B Configurar atributo 1 (normal) con `glVertexAttribPointer`
- [ ] 👤 B Bindear EBO con `glBufferData`
- [ ] 👤 B Desbindear VAO al final
- [ ] 👤 B Implementar `draw()` con `glBindVertexArray` + `glDrawElements`
- [ ] 👤 B Implementar destructor que libere VAO, VBO, EBO

### B5. Shaders del proyecto

- [ ] 👤 B Escribir `assets/shaders/basic.vert` (pos, normal, MVP, modelMatrix)
- [ ] 👤 B Escribir `assets/shaders/basic.frag` (Lambert: `max(dot(N, -L), 0)`)
- [ ] 👤 B **No modificar** la matemática del shader base si se les da
- [ ] 👤 B Escribir `assets/shaders/picking.vert` (solo MVP, sin color)
- [ ] 👤 B Escribir `assets/shaders/picking.frag` (`out uint fragID; uniform uint objectID;`)
- [ ] 👤 B Escribir shader para visualización de vértices (puntos)
- [ ] 👤 B Escribir shader para visualización de normales (líneas)

### B6. FBO de color picking 🔥

- [ ] 🔥 👤 B Crear FBO en `Renderer::init()`
- [ ] 🔥 👤 B Crear textura de color con formato `GL_R32UI` (⚠️ **no** `GL_RGBA8`)
- [ ] 🔥 👤 B Crear renderbuffer de profundidad `GL_DEPTH_COMPONENT24`
- [ ] 🔥 👤 B Adjuntar ambos al FBO
- [ ] 🔥 👤 B Verificar `glCheckFramebufferStatus` == `GL_FRAMEBUFFER_COMPLETE`
- [ ] 🔥 👤 B Implementar `renderForPicking(scene)` (bindear FBO, limpiar con ID=0, dibujar)
- [ ] 🔥 👤 B Implementar `readPixel(x, y)` con `glReadPixels` (invertir Y)
- [ ] 🔥 👤 B Verificar que ID leído coincide con `Object::id` exacto (sin offsets)
- [ ] 🔥 👤 B Redimensionar FBO si cambia el tamaño de la ventana

### B7. Renderer principal

- [ ] 👤 B Crear `src/render/Renderer.h` y `.cpp`
- [ ] 👤 B Implementar `init()` (crear FBO, shaders, estado inicial)
- [ ] 👤 B Implementar `renderScene(scene)`:
  - [ ] 👤 B Limpiar color y depth con `backgroundColor`
  - [ ] 👤 B Activar/desactivar `GL_DEPTH_TEST` según flag
  - [ ] 👤 B Activar/desactivar `GL_CULL_FACE` según flag
  - [ ] 👤 B Activar blending para transparencias (`GL_SRC_ALPHA`, `GL_ONE_MINUS_SRC_ALPHA`)
  - [ ] 👤 B Iterar objetos y calcular MVP por cada uno
  - [ ] 👤 B Enviar uniforms (MVP, modelMatrix, objectColor, alpha, lightDir)
  - [ ] 👤 B Llamar `mesh->draw()`
- [ ] 👤 B Renderizar el objeto seleccionado de forma destacada (outline o color distinto)

### B8. Visualizaciones

- [ ] 👤 B Implementar wireframe con `glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)`
- [ ] 👤 B Implementar visualización de vértices con `glPointSize` + `glDrawArrays(GL_POINTS)`
- [ ] 👤 B Implementar visualización de normales:
  - [ ] 👤 B Generar buffer de líneas al cargar el mesh (vértice → vértice + normal × longitud)
  - [ ] 👤 B Crear VAO separado para las líneas
  - [ ] 👤 B Dibujar con `GL_LINES`
- [ ] 👤 B Calcular Bounding Box al cargar el mesh (min/max de cada eje)
- [ ] 👤 B Implementar visualización de Bounding Box (12 líneas, VAO separado)
- [ ] 👤 B Exponer cada visualización como flag independiente consultable desde `Object`

### B9. Depth test y back-face culling

- [ ] 🟢 👤 B Configurar estado inicial: `glEnable(GL_DEPTH_TEST)`, `glEnable(GL_CULL_FACE)`
- [ ] 🟢 👤 B Configurar `glDepthFunc(GL_LESS)`, `glCullFace(GL_BACK)`, `glFrontFace(GL_CCW)`
- [ ] 🟢 👤 B Implementar toggle desde `renderScene` según flags de la escena

### B10. Integración de picking

- [ ] 🤝 Conectar callback de click de ratón con `renderer.renderForPicking(scene)`
- [ ] 🤝 Conectar `renderer.readPixel(x, y)` con `scene.selectAtPixel(id, mode)`
- [ ] 🤝 Verificar que click en el vacío devuelva ID=0 y no seleccione nada

### B11. Opcionales de B

- [ ] 🟢 👤 B *Opcional*: Soporte de alpha con ordenamiento atrás→adelante
- [ ] 🟢 👤 B *Opcional*: Buffer de IDs por triángulo para selección por triángulo
- [ ] 🟢 👤 B *Opcional*: Marcar visualmente el triángulo seleccionado (color distinto)

---

## 🔄 HITOS DE INTEGRACIÓN 🤝

- [ ] 🤝 **Hito 1** — `loadOBJ` funcional + `Mesh` + shader básico → A puede crear objetos desde archivo
- [ ] 🤝 **Hito 2** — FBO de picking funcional → A puede seleccionar objetos
- [ ] 🤝 **Hito 3** — UI de ImGui básica → B puede activar wireframe, normales, etc.
- [ ] 🤝 **Hito 4** — Generación de primitivas → B tiene modelos para renderizar
- [ ] 🤝 **Hito 5** — Serialización JSON → ambos pueden probar guardado/carga
- [ ] 🤝 **Hito 6** — Integración final + pruebas intensivas + video de defensa

---

## 🧪 PRUEBAS FINALES

- [ ] 🤝 Cargar un `.obj` con normales incluidas → se ve correctamente iluminado
- [ ] 🤝 Cargar un `.obj` sin normales → se calculan y se ve correctamente
- [ ] 🤝 Cargar un `.mtl` y verificar que el color difuso se aplica
- [ ] 🤝 Click en objeto → se selecciona (modo GLOBAL)
- [ ] 🤝 Click en vacío → se deselecciona todo
- [ ] 🤝 Cambiar posición/rotación/escala del objeto seleccionado → se refleja
- [ ] 🤝 Cambiar color difuso → se refleja
- [ ] 🤝 Eliminar objeto → desaparece de la escena
- [ ] 🤝 Cambiar a LOCAL → comportamiento según diseño
- [ ] 🤝 Wireframe on/off → funciona
- [ ] 🤝 Normales on/off → funciona
- [ ] 🤝 Vértices on/off → funciona
- [ ] 🤝 Bounding Box on/off → funciona
- [ ] 🤝 Depth test on/off → funciona
- [ ] 🤝 Back-face culling on/off → funciona
- [ ] 🤝 WASD + ratón → cámara se mueve correctamente
- [ ] 🤝 Crear cubo/esfera/pirámide → se añaden a la escena y son manipulables
- [ ] 🤝 Guardar escena → cerrar programa → abrir → cargar → todo restaurado
- [ ] 🤝 Limpiar escena → no queda nada
- [ ] 🤝 Cambiar color de fondo → se refleja
- [ ] 🤝 FPS visibles y razonables (>30 con varios objetos)
- [ ] 🤝 Sin fugas de memoria visibles (Valgrind o similar)
- [ ] 🤝 Sin warnings de compilación

---

## 🎁 OPCIONALES (puntos extra)

- [ ] 🟢 Alpha del color difuso → transparencias funcionan
- [ ] 🔥 Selección por triángulo → se puede seleccionar un triángulo individual y se marca visualmente
- [ ] 🟢 Cilindro en la lista de objetos parametrizables

---

## 📦 ENTREGA

- [ ] 🤝 Añadir `assets/` con modelos y shaders de prueba
- [ ] 🤝 Verificar que `mkdir build && cd build && cmake .. && cmake --build .` funciona en limpio
- [ ] 🤝 Grabar video de defensa
- [ ] 🤝 Comprimir como `PROY2_CEDULA1_CEDULA2.zip`
- [ ] 🤝 Enviar a `brysanilva.dev@gmail.com` con el asunto replicando el nombre del zip
- [ ] 🤝 Preparar explicación de decisiones de diseño para la defensa

---

## 📊 Resumen de carga

| Persona | Tareas críticas 🔥 | Tareas totales |
|---|---|---|
| **A** | 1 (opcional de triángulo) | ~40 |
| **B** | 3 (FBO, normales, triángulo opcional) | ~55 |

**Consejo**: B tiene más carga y más riesgo. Si A termina su parte antes, que ayude a B con visualizaciones (wireframe, bbox) o con el FBO de picking. La regla es: **B nunca debe quedarse bloqueado esperando a A**, porque A depende de B para casi todo (Mesh, loadOBJ, FBO), pero B casi nunca depende de A.