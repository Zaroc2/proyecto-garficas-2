# 🎯 Plan por fases — Sistema de Colisiones con Rebotes (Ámbitos 2 y 3)

Leyenda (mismo criterio que el `README.md`):
- 👤 **A** = Compañero A (Escena, UI, Lógica)
- 👤 **B** = Compañero B (Renderizado, I/O)
- 🤝 = Requiere ambos
- 🔥 = Alto riesgo / 💛 = Medio / 🟢 = Bajo

---

## 📌 Punto de partida (lo que ya existe, "B1")

| Pieza | Dónde | Estado |
|---|---|---|
| AABB mundial por objeto | `scene/Collision.cpp → GetWorldAABB()` | ✅ 8 esquinas del bbox del mesh × model matrix |
| Esfera-AABB (solo bool) | `scene/Collision.cpp → SphereVsAabb()` | ✅ clamp del centro al AABB |
| Cámara se desliza | `scene/Camera.cpp → tryMove()` | ✅ resolución por ejes (sin rebote) |
| Flag `solid` | `scene/Object.h` + UI + serializador | ✅ |
| Bucle con `dt` | `core/Application.cpp → calculateDeltaTime()` | ✅ |
| Shader de debug | `assets/shaders/debug.{vert,frag}` | ✅ líneas/puntos de color plano |
| Picking por ID | `render/Renderer` (FBO R32UI) + `readPixel` | ✅ |

**Qué falta para "rebotes"**: velocidad persistente, normal de contacto, reflexión con restitución, corrección de posición, y cuerpos dinámicos (gravedad).

---

## 🏗️ Arquitectura objetivo

Módulo nuevo **`scene/Physics.{h,cpp}`** (mismo espíritu que `Collision`: solo CPU, sin OpenGL, no toca Renderer). La física es un *paso* del bucle, entre input y render:

```
bucle principal (main + Application::run)
 ├─ calculateLoopVariables()      // dt, mouse delta
 ├─ processInput()               // cámara/teclado (ya existe)
 ├─ Physics::step(scene, dt)     // ★ NUEVO: integrar → detectar → resolver → eventos
 ├─ interface()                  // ImGui (ya existe)
 └─ Renderer::renderScene()      // dibujar (compañero B, ya existe)
```

Dentro de `Physics::step` el orden SIEMPRE es:

```
1. Integrar     (aplicar gravedad y mover cada cuerpo dinámico)
2. Broadphase   (descartar pares lejanos)          ← Ámbito 3
3. Narrowphase  (calcular contactos exactos)      ← esfera-AABB / esfera-esfera / esfera-OBB
4. Resolver     (impulsos + corrección de posición)
5. Dormir       (sleeping)                         ← Ámbito 3
6. Eventos      (cola de colisiones para la app)   ← Ámbito 3
```

**Por qué este orden**: resolver mutaciones (velocidad/posición) *después* de detectar todos los contactos evita que una colisión resuelta "esconda" otra del mismo frame. Nunca se resuelve durante la detección.

---

# 🟦 ÁMBITO 2 — Objetos dinámicos con gravedad

> **Definition of Done del Ámbito 2**: suelto un objeto desde altura → cae, rebota contra suelo/estáticos/otros objetos, pierde energía en cada rebote y queda en reposo. Todo configurable desde la UI y sobrevive a guardar/cargar.

---

## FASE 2.1 ✅ — 💛 El cuerpo dinámico: velocidad, gravedad y suelo

🎯 **Objetivo**: que un objeto marcado como "dinámico" caiga con gravedad y rebote en el suelo `y = 0` hasta quedarse quieto. Sin choques contra otros objetos todavía.

### 📚 Teoría

**1. Leyes de Newton aplicadas a un punto material.**
Un objeto dinámico se modela como *partícula con radio* (esfera envolvente): tiene masa `m`, posición `p`, velocidad `v`. La única fuerza es el peso:

```
F = m·g   →   a = F/m = g        (la masa NO afecta la caída: Galileo)
g = (0, -9.81, 0) m/s²           (en pantalla: escala de g configurable, ~×1)
```

**2. Integración numérica: Euler semi-implícito (simplectico).**
El movimiento continuo se aproxima en pasos discretos `dt`. Hay dos Eulers:

```
Explícito (MALO):        p += v·dt;  v += a·dt   // usa la v VIEJA para mover
Semi-implícito (BUENO): v += a·dt;  p += v·dt   // primero actualiza v, mueve con la NUEVA
```

El semi-implícito actualiza la velocidad *antes* de usarla. Con un rebote (`v` cambia de signo) esto evita que el objeto "gane energía" cada paso: en un oscilador, el explícito **explota** (cada rebote sube más alto) y el semi-implícito **amortigua** (cada rebote es un pelín más bajo). Para física de juegos es el estándar. Es una línea de código, pero es LA decisión de integrador del proyecto.

**3. Coeficiente de restitución `e` (rebote).**
`e ∈ [0, 1]` es la fracción de velocidad normal que sobrevive al rebote:

```
e = 1.0  →  rebote perfecto (nunca se detiene, "flubber")
e ≈ 0.7  →  pelota de goma
e ≈ 0.4  →  pelota de fútbol
e = 0.0  →  plasticina (choca y no rebota)
```

**4. Colisión con el plano del suelo.**
Plano `y = 0` con normal `n = (0, 1, 0)`. Una esfera de radio `r` colisiona cuando su centro baja de `r`:

```
if (p.y < r)  →  contacto:
    corrección de posición:  p.y = r                    // sacar el cuerpo del suelo
    velocidad normal:        vn = dot(v, n) = v.y
    si vn < 0 (va hacia abajo):  v.y = -e · v.y         // rebote: invertir y amortiguar
    si |v.y| < umbral_reposo:     v.y = 0               // ver teoría 5
```

La componente horizontal de `v` (tangencial) **no se toca**: la pelota que cae en movimiento horizontal sigue deslizándose.

**5. Umbral de reposo (por qué es obligatorio).**
Con `e < 1` los rebotes son infinitos pero cada vez más pequeños: `1, e, e², e³, ...`. Un cálculo: con `e = 0.7` y `g = 9.81`, a partir de ~1 cm de altura los rebotes son de < 1 ms: la física "vibra" para siempre en el suelo. Solución estándar: si la velocidad normal al chocar es menor que un umbral (p. ej. `|vn| < 2·g·dt`, o un fijo como `0.35 m/s`), se pone `vn = 0`. La bolita queda apoyada sin temblar.

### 🔨 Implementación

- [x] 🟢 👤 A `scene/Object.h`: añadir campos
  ```cpp
  bool isDynamic = false;          // dynamic vs static
  glm::vec3 velocity = {0,0,0};    // m/s
  float restitution = 0.6f;       // e
  float mass = 1.0f;               // kg (masa "real", se usa en Fase 2.3)
  float boundRadius = 0.5f;        // esfera envolvente (calculada al crear/escalar)
  ```
- [x] 🟢 👤 A Helper `computeBoundRadius(Object*)`. **Decisión de implementación (devía del plan original):** esfera **INSCRITA** → `r = 0.5f · min componente de ((bboxMax − bboxMin) · |scale|)`. Razón: con la circunscrita (`0.5 · length`) los cubos flotan ~0.37 sobre el suelo y el demo de 2.1 se ve roto; con la inscrita apoyan plano (y las esferas son exactas). Contrapartida documentada en riesgos de 2.2. Se refresca en cada `Physics::step` (cubre cambios de escala desde la UI).
- [x] 🟢 👤 A Crear `scene/Physics.h/.cpp` (añadido `bool groundEnabled` a `PhysicsParams`):
  ```cpp
  struct PhysicsParams { float gravity = 9.81f; float restThreshold = 0.35f; };
  void Physics::step(Scene* scene, float dt, const PhysicsParams& p);
  ```
  Por ahora `step` solo hace: integrar semi-implícito + suelo `y = 0`.
- [x] 🟢 🤝 Llamar `Physics::step(...)` en `Application::run` entre `processInput` e `interface`. **Nota de implementación:** `calculateDeltaTime()` multiplicaba el dt ×10 (hack histórico de velocidad de cámara) → se añadió `rawDeltaTime` (dt real, clamp 50 ms) usado solo por la física.
- [x] 🟢 👤 A UI (compañero A): checkbox **"Dinámico"** (fuerza `solid`), slider **"Rebote (e)"**, lectura en vivo de velocidad y radio envolvente, y botón **"Soltar"** (+2 en Y) en el panel del objeto.

### ✅ Criterio de aceptación
- Cubo dinámico soltado desde `y = 5` rebota en el suelo con altura decreciente y queda quieto (sin vibrar) en < 10 s.
- Con `e = 1` rebota siempre a la misma altura; con `e = 0` cae y se queda clavado.
- Un objeto estático nunca se mueve solo.

### ⚠️ Riesgos
- ✔ Aplicado: `rawDeltaTime` con clamp de 50 ms en `calculateDeltaTime()` (la solución de verdad va en Fase 3.1).
- 🔧 **Post-mortem — bug "-nan en X/Y" al deseleccionar objetos dinámicos:** causa raíz = **build incremental con `.obj` stale**: `Scene.cpp`, `Collision.cpp`, `Camera.cpp` y `SceneSerializer.cpp` seguían compilados contra el `Object.h` previo a la fase. `Scene::selectObj()` (binario viejo) escribía `selectedTriangleId/-SubMesh = -1` (0xFFFFFFFF) en los offsets donde el layout nuevo vive `velocity.x/y` → `-nan(ind)` solo en X e Y (Z a salvo) → la posición se NaN-ea en el paso siguiente. Detectado con el watchdog de `Physics::step` (huella: `vel=(-nan,-nan,0)`). Fix: rebuild completo (`cmake --build out/build/x64-debug --clean-first`). **Regla:** si un build falla a medias (p. ej. `cl.exe` sin entorno VS), el deplog de Ninja queda parcial → tras cambiar HEADERS, reconstruir limpio. Blindajes añadidos de paso: `dt`/radio NaN-safe en `Physics::step`, comparador del `std::sort` del renderer NaN-safe, bbox definido para meshes sin vértices.

---

## FASE 2.2 ✅ — 💛 Rebote contra estáticos: esfera vs AABB con contacto real

🎯 **Objetivo**: la esfera de un objeto dinámico rebota contra los `solid && !isDynamic` de la escena, con normal de contacto correcta y sin hundirse. Es el corazón del requerimiento.

### 📚 Teoría

**1. Punto más cercano de un AABB (closest point).**
Dado el AABB `[min, max]` y el centro de la esfera `c`, el punto del AABB más cercano a `c` se obtiene *saturando* cada eje:

```
q = clamp(c, min, max)      // por componente
d = c − q                   // vector del AABB hacia la esfera
dist = |d|
colisión  ⟺  dist < r
```

- Si `dist > 0`: la esfera está fuera y `d` apunta del AABB a la esfera.
- Si `dist == 0`: el **centro está dentro del AABB** (caso degenerado, ver teoría 4).

Esto ya lo hace tu `SphereVsAabb` actual; lo que falta es **devolver** `d` en vez de solo un bool.

**2. Normal de contacto.**
La normal es el eje perpendicular a la superficie en el punto de impacto — es LO que define la dirección del rebote:

```
n = d / |d|        // (cuando d ≠ 0)
```

Ejemplo: pelota golpeando una pared vertical por la derecha → `q` está en la cara, `d` apunta en `-x`... (ojo: `d = c − q` apunta DESDE el AABB HACIA la esfera; para reflejar la velocidad de la esfera basta con `n` así.)

**3. Reflexión de la velocidad (la fórmula del rebote).**
Se descompone `v` en parte normal y tangencial, se invierte y amortigua SOLO la normal:

```
vn = dot(v, n)            // componente normal (escalar)
v' = v − (1 + e) · vn · n
```

Derivación (para entenderla, no para memorizarla):

```
v  = v_n + v_t            donde v_n = (v·n)·n  (hacia la superficie), v_t = v − v_n
v' = v_t − e · v_n        // la tangencial se conserva, la normal se invierte y pierde energía
    = v − v_n − e·v_n = v − (1+e)·v_n          ✔ misma fórmula
```

**Casos de control**: con `e=1`, `v' = v − 2·vn·n` es el reflejo especular perfecto. Pelota a 45° contra el suelo sale a 45°. Pelota que cae vertical (`v_t = 0`) sale vertical.

**Condición indispensable**: solo reflejar si `vn < 0` (se acercan). Si `vn > 0` ya se están separando (p. ej. empujado por otra resolución) y reflejar de nuevo lo "pegaría" a la superficie.

**4. Centro DENTRO del AABB (fallback del min-eje).**
Si el `dt` fue largo o el objeto se solapó por diseño, `d = 0` y no hay dirección. Estándar: calcular cuánto "penetra" el centro por cada una de las 6 caras y empujar por la de **menor penetración** ( Minimum Translation):

```
pen_x− = c.x − min.x     pen_x+ = max.x − c.x     // ídem y, z
elegir el menor de los 6 → n = eje correspondente con signo hacia el centro
corrección = pen_elegido + r                     // sacar el centro a la cara y luego r más allá
```

**5. Corrección de posición (push-out).**
El integrador mete la esfera una fracción `depth = r − dist` dentro del AABB. Si no corriges, la esfera queda "dentro" y el siguiente frame detecta otra vez el contacto con la normal movida → temblor. Solución instantánea y estable: reponer el cuerpo justo en contacto:

```
p += n · (r − dist)        // cuando d ≠ 0
p += n · (pen + r)         // cuando estaba dentro (teoría 4)
```

Orden: **primero corregir posición, después reflejar velocidad** (así la reflexión usa la geometría final).

**6. Clasificación de cuerpos (tabla de verdad).**

| Objeto | `solid` | `isDynamic` | Comportamiento |
|---|---|---|---|
| Estático | ✔ | ✘ | Muro: nadie lo mueve, otros rebotan contra él |
| Dinámico | ✔ | ✔ | Participa en todo (cae, rebota, es empujado) |
| Decorativo | ✘ | ✘ | Ignorado por la física (visible, atravesable) |
| (sin sentido) | ✘ | ✔ | No permitir en la UI (o tratarlo como decorativo) |

### 🔨 Implementación

- [x] 💛 👤 A `scene/Collision.h`: nuevo tipo y función (mantener las viejas para la cámara):
  ```cpp
  struct Contact {
      glm::vec3 normal;    // del estático HACIA la esfera
      float     depth;     // penetración (r − dist), >= 0
      glm::vec3 point;     // punto de contacto (q), para eventos/debug (Ámbito 3)
  };
  bool SphereVsAabbContact(const glm::vec3& c, float r,
                           const glm::vec3& min, const glm::vec3& max,
                           Contact& out);
  ```
- [x] 💛 👤 A Implementar las 6 caras del caso "centro dentro" (minimum translation axis). **Limitación verificada:** si el dinámico descansa en la costura entre dos colisionadores (estático+ suelo), el push-out de ambos se equilibra y el objeto queda *incrustado pero estable* (velocidad a 0, sin NaN ni jitter). Evitar spawn dentro de estáticos en demos.
- [x] 💛 👤 A `Physics::step`: tras integrar, para cada dinámico iterar estáticos (`solid && !isDynamic && mesh`), `GetWorldAABB` + `SphereVsAabbContact` → push-out + reflexión con `restitution` + umbral de reposo de la Fase 2.1 (bajo umbral → deslizar en vez de rebotar). **Orden:** estáticos primero (geometría real), suelo al final (red de seguridad).
- [x] 🟢 👤 A Validar en la UI: al marcar `isDynamic`, forzar `solid = true` (ya venía de la Fase 2.1).
- [x] 🟢 🤝 *Extra para tests (añadido, no estaba en el plan):* el "Vel:" del panel pasa a `DragFloat3` **editable** y nuevo botón **"Lanzar"** (impulso horizontal de 6 m/s desde la cámara hacia el objeto). Sin esto no se pueden verificar los criterios de rebote desde la UI. Matemática validada con repro aislado (espejo, bisectriz, reposo, tunneling, cero NaN).

### ✅ Criterio de aceptación
- Pelota lanzada horizontal contra una pared: rebota simétrico y conserva la componente paralela (se ve "ping-pong" entre dos paredes).
- Pelota contra la esquina/arista de un cubo: rebota en la bisectriz (la normal de `clamp` sale de la arista, no de una cara — comprobarlo es la prueba de que el closest-point está bien).
- Un cubo apoyado SOBRE un estático (pelota sobre un cubo) no lo atraviesa ni tiembla.
- Estático rotado: por ahora el AABB es conservador → rebota "antes de tocar" (esperado, se corrige en Fase 3.4).

### ⚠️ Riesgos
- 💛 Objects escalados no uniformemente: el AABB mundial de `GetWorldAABB` es correcto (8 esquinas), pero la **esfera envolvente** con escala no uniforme sobre-estima: aceptable en Ámbito 2, documentado.
- 💛 Cuadrado contra pared (esfera envolvente de un cubo): con la esfera **inscrita** (convención por defecto desde 2.1) rebota un poco **después** de tocar visualmente. Para las demos contra paredes, usar esferas (esfera exacta: cero error).

---

## FASE 2.3 ✅ — 💛 Objeto vs objeto: impulsos esfera-esfera

🎯 **Objetivo**: dos dinámicos chocan e intercambian momento según sus masas; un dinámico golpea a otro en reposo y lo despierta.

### 📚 Teoría

**1. Detección esfera-esfera.**

```
d  = pB − pA                 // A hacia B
dist = |d|
n  = d / dist                // normal de contacto A → B (si dist > 0)
pen = (rA + rB) − dist       // penetración
colisión ⟺ pen > 0
```

Si `dist == 0` (centros coincidentes, rarísimo): elegir `n = (0,1,0)` y `pen = rA + rB` (desempate arbitrario pero determinista).

**2. Masa inversa (la convención que unifica todo el motor).**
En lugar de "estático = masa infinita", se guarda **masa inversa** `invM = 1/m`, y un cuerpo **estático tiene `invM = 0`**:

```
dinámico:  invM = 1/m
estático:  invM = 0            // cualquier impulso × 0 = no se mueve
```

Con esto, la MISMA fórmula de impulso sirve para bola-vs-bola, bola-vs-muro, y en Fase 3.2 para el apilado. Regla de oro de los motores por impulsos (Box2D, Bullet): **toda la física se escribe contra `invM`, nunca contra `m`**.

**3. Impulso de colisión (la fórmula central).**
Queremos que tras el choque la velocidad relativa normal se invierta amortiguada: `v_rel'·n = −e · (v_rel·n)`. De conservación del momento + esa condición sale el impulso escalar `j`:

```
v_rel = vB − vA                       // velocidad de B respecto a A
vn    = dot(v_rel, n)                 // negativo ⟹ se acercan
si vn >= 0  →  no resolver (se separan)

den = invMA + invMB
j   = −(1 + e) · vn / den             // j > 0, en N·s (impulso a lo largo de n)

vA -= (j · invMA) · n                 // A recibe el impulso en −n
vB += (j · invMB) · n                 // B lo recibe en +n
```

**Verificaciones de sanidad (úsalas como tests)**:
- Masas iguales, `e = 1`: las velocidades se intercambian (bola de billar frontal).
- `invMB = 0` (B es un muro), masas iguales, `e = 1`: `vA' = −vA`, `vB` intacto → **es exactamente la reflexión de la Fase 2.2**. Esfera-AABB estático = caso particular de impulso con `invM = 0`. Buenísmo para la defensa: "una sola fórmula explica todos los rebotes del sistema".
- `e = 0`: se quedan pegadas moviéndose juntas en choque frontal.

**4. Corrección de posición proporcional (Baumgarte ligero).**
Con dos dinámicos hay que repartir la corrección: quien tiene menos masa (más `invM`) se corrige más. Además NO se corrige el 100 % de la penetración (solo ~40–80 %): corregir todo genera "pop" y temblor cuando hay varias colisiones simultáneas (torre). El resto se absorbe en los frames siguientes:

```
slop = 0.01                // tolerancia de penetración que NO se corrige (1 cm), evita jitter
pct  = 0.6                 // fracción corregida por paso
corr = max(pen − slop, 0) · pct / (invMA + invMB)

pA -= n · corr · invMA
pB += n · corr · invMB
```

Ojo: si `(invMA + invMB) == 0` (dos estáticos solapados), saltar la corrección — nunca resolver estático-vs-estático.

**5. Pares O(n²): por qué es suficiente AHORA.**
Con `n` dinámicos hay `n(n−1)/2` pares (15 con 6 objetos, 4950 con 100). Para las demos del proyecto (< 60 objetos) el narrowphase esfera-esfera es baratísimo (unas decenas de ops por par). El broadphase de la Fase 3.3 existe para cuando esto deje de ser verdad. No prematuro-optimizar: medir FPS con 50 objetos y *entonces* decidir.

**6. Ecuación de la energía (para la defensa).**
En cada rebote la energía cinética `K = ½·m·|v|²` pierde exactamente el factor `e²` en la componente normal. Con `e = 0.7` se pierde ~51 % por choque. Contar esto en el video suena a física de verdad y son 2 líneas.

### 🔨 Implementación

- [x] 💛 👤 A `Object`: helper `getInvMass()` (invM = 0 ⇔ estático; con guard anti masa inválida: masa ≤ 0 se trata como 1 kg).
- [x] 💛 👤 A `Collision.h/.cpp`: `SphereVsSphereContact(...) → Contact` reutilizando el struct (normal A→B, depth = pen; centros coincidentes → normal (0,1,0) determinista).
- [x] 💛 👤 A `Physics::step` reorganizado a la arquitectura final: **integrar TODO → detectar TODO (lista de pares, sin resolver) → resolver TODO (impulsos + Baumgarte) → suelo**. Clasificación previa de cuerpos (`dyns`/`stats`; decorativos fuera desde el inicio).
- [x] 💛 👤 A Unificación implementada (opción recomendada del plan): una sola fórmula de impulso para todos los pares; el estático aporta `invM = 0` y el resultado es **exactamente** la reflexión de la Fase 2.2 (verificado numéricamente: din-vs-muro da v.x = −e·v, idéntico). La reflexión manual de 2.2 se eliminó. `e` del par = media simple de ambos `restitution`; bajo el umbral de reposo → e = 0 (cancela la normal: deslizar/reposo, mismo truco de 2.1/2.2).
- [x] 💛 🤝 Corrección posicional (Baumgarte ligero): `slop = 0.01`, `pct = 0.6`, repartida por invM.
- [x] 🟢 🤝 Demo posible desde la UI: dos+ esferas dinámicas con `Crear Esfera` + `Dinámico` + `Vel` editable/**Lanzar**.
- [x] 🟢 👤 A *Extra (adelantado de 2.4):* slider **"Masa (kg)"** (log, 0.1–100) en el panel del objeto — los criterios de aceptación de 2.3 requieren ajustar masas.
- [x] 💛 🤝 Validación con repro aislado: intercambio exacto (e=1), pérdida 1−e² exacta (e=0.6), billar pesado/ligero exacto, **cadena croquet** (A→B→C: 0, 0, +6.000), equivalencia muro exacta, apiladas 2000 pasos sin NaN. Cero NaN en todo.

### ✅ Criterio de aceptación
- Dos esferas iguales chocan frontal a la misma velocidad → ambas rebotan a la mitad de la velocidad relativa (masas iguales se reparten el impulso).
- Esfera pequeña y rápida contra cubo grande y pesado: la pelota rebota casi a su velocidad, el cubo apenas se mueve.
- Cadena: alinear 3 esferas en reposo y lanzar una → se transmite el impulso (croquet / newton's cradle simplificado).
- FPS > 60 con 50 esferas cayendo a un tablero.

### ⚠️ Riesgos
- 💛 Múltiples contactos resueltos en secuencia pueden inyectar energía ("pop"): la corrección al 60 % + slop lo mitiga; la solución completa (iteraciones) es la Fase 3.2.

---

## FASE 2.4 ✅ — 🟢 Integración completa: UI, serialización y robustez del bucle

🎯 **Objetivo**: todo lo anterior sobrevive a guardar/cargar, es configurable, y no explota a bajos FPS. Cierra el Ámbito 2.

### 📚 Teoría

**1. Clamp de `dt` (por qué el dt crudo es un enemigo).**
`dt` mide el frame real: con un `std::cout`, una carga de ImGui o un stall de Windows, saltan valores de 100–500 ms. Efectos: (a) un paso gigante atraviesa paredes (tunneling: el desplazamiento `v·dt` supera el radio), (b) con vsync roto, `dt` alternando 5/40 ms hace rebotes asimétricos. Regla práctica:

```
dt = min(dt_real, 0.05f)     // nunca más de 50 ms de física por frame
```

Sigue siendo "por frame", pero acota el desastre. La solución de verdad (timestep fijo) es la Fase 3.1 — este clamp es el cinturón de seguridad del Ámbito 2.

**2. Substeps (mitigar tunneling sin motor completo).**
El desplazamiento máximo por paso es `|v|·dt`. Tunneling si `|v|·dt > r`. En vez de física continua (cara), partir el paso:

```
substeps = clamp(ceil(|v_max|·dt / (0.5·r_min)), 1, 4)   // o fijo: 2–3
for cada substep: step(dt / substeps)
```

Con 2–3 substeps una pelota a 15 m/s con r=0.35 nunca salta una pared de 1 m. Es la mitad de efectividad del timestep fijo por un 10 % del costo.

**3. Qué serializar (contrato de la escena con física).**
Regla de round-trip: la escena guardada debe **reconstruir el estado físico exacto**, no solo el visual. Campos nuevos obligatorios en `SceneSerializer`: `isDynamic`, `velocity`, `restitution`, `mass`. Y los parámetros globales en el bloque de escena: `gravity`. `boundRadius` NO se serializa (se recalcula al cargar, igual que el mesh) — nunca persistir lo derivable: se desincroniza.

**4. UI de física: qué exponer y con qué límites.**

| Control | Tipo | Rango útil | Por qué |
|---|---|---|---|
| Gravedad | slider | 0 – 25 m/s² | 0 = "modo luna/flotar", 9.81 default |
| Restitución por objeto | slider | 0 – 1 | la estrella del show |
| Masa por objeto | slider (log) | 0.1 – 100 kg | log porque 1→2 kg cambia mucho, 50→100 casi nada |
| Dinámico | checkbox | — | con `solid` forzado a true |
| Suelo invisible | checkbox | — | hay escenas (skybox) donde y=0 no debería existir |

### 🔨 Implementación

- [x] 🟢 🤝 `Application`: clamp de `dt` (ya venía de 2.1: `rawDeltaTime` 50 ms) + **2 substeps fijos** (`Physics::step` ×2 con `dt/2`) — mitiga tunneling por ~10 % de costo.
- [x] 🟢 👤 A Panel **"Fisica global"**: `Gravedad (m/s2)` (0–25, `AlwaysClamp`) + `Suelo implicito (y=0)`. (Dinámico/restitución/masa del objeto ya existían de 2.2/2.3.)
- [x] 🟢 👤 A `SceneSerializer`: por objeto `isDynamic`, `velocity`, `restitution`, `mass` (con saneo defensivo al cargar: e→[0,1], masa≤0→1); bloque de escena `"physics": {gravity, groundEnabled}` (escenas viejas → defaults). `boundRadius` NO se persiste (derivable — "nunca persistir lo derivable").
- [x] 🟢 🤝 **Nota de arquitectura:** `PhysicsParams` se movió de `Application` a **`Scene`** (`scene->physics`) — el estado de la escena vive en la escena y se serializa con ella.
- [x] 🟢 🤝 Botón **"Escena de prueba: 10 pelotas"**: tablero estático (cubo 7×0.4×7 elevado) + grilla 5×2 de esferas dinámicas (e=0.75) con **velocidades cruzadas** — chocan en el aire, llueven sobre el tablero, algunas caen al suelo. Setup determinista (sin azar) para regresiones y el video.
- [ ] 🟢 🤝 Checklist de la FASE 2 completa: **pendiente de la prueba de round-trip** (guardar → cargar → estado físico restaurado) y del repaso 2.1→2.3 tras los substeps.

### ✅ Criterio de aceptación
- Guardar con pelotas rebotando → cerrar → cargar → siguen rebotando (estado físico restaurado).
- Provocar un stall (p. ej. mover la ventana): nada atraviesa el suelo.
- La "Escena de prueba" corre a > 60 FPS.
- **Ámbito 2 DONE** → grabar este bloque del video de defensa aquí (no dejarlo para el final).

### ⚠️ Riesgos
- 🟢 ImGui roba input/frames cuando el panel está abierto: los valores de UI deben aplicar en el próximo frame de física, no a mitad de `step` (leer de la escena al inicio del step, nunca escribir mid-step).

---

# 🟪 ÁMBITO 3 — Mini motor de física

> **Definition of Done del Ámbito 3**: la física es estable (misma simulación a 30/60/144 FPS), escala (200+ objetos), es creíble (fricción, apilado sin temblor) y es *vistosa* (eventos, lanzar con el ratón, debug de contactos). Las fases 3.1–3.7 son independientes entre sí **excepto** donde se indica; se pueden hacer a la carta y en paralelo A/B.

---

## FASE 3.1 — 💛 Timestep fijo con acumulador

🎯 **Objetivo**: física determinista e independiente de los FPS, render suave mediante interpolación. Es la fase que "profesionaliza" el motor.

### 📚 Teoría

**1. Por qué el dt variable arruina la física (los 3 pecados).**
- **Estabilidad**: el apilado (Fase 3.2) es sensible a `dt`: correcciones porcentuales + impulsos con dt distinto = una torre aguanta a 60 FPS y explota a 35.
- **Repetibilidad**: con dt variable NO se puede reproducir un bug ni comparar dos ejecuciones (cada frame da un paso distinto). El debugging se vuelve lotería.
- **Justicia de la energía**: el rebote calculado por paso pierde `e²` **por paso**, no por segundo. A 144 FPS hay 2.4× más pasos que a 60: las pelotas "mueren" antes en el monitor rápido.

**2. El patrón acumulador (Fix Your Timestep, Glenn Fiedler — *the* referencia de física de juegos).**
La física SIEMPRE avanza en pasos fijos `h` (p. ej. `1/120 s`); el render va al ritmo del monitor. El frame real aporta tiempo a un acumulador y la física "cobra" pasos completos:

```
h = 1.0f / 120.0f                 // paso físico fijo
acumulador += min(dt_real, 0.25f) // cap anti spiral-of-death
mientras (acumulador >= h && pasos < MAX_PASOS):
    step(h)                       // SIEMPRE h, nunca dt_real
    acumulador -= h
alpha = acumulador / h            // fracción de paso "sobrante", ∈ [0, 1)
```

Ahora la simulación es una función pura del tiempo acumulado: determinista, estable y con energía constante entre monitores.

**3. Espiral de la muerte (spiral of death) y por qué el cap.**
Si `step(h)` cuesta más que `h` de CPU (muchos objetos), el acumulador crece cada frame → más pasos → más coste → acumulador crece más... hasta congelar la app. Defensas: (a) `MAX_PASOS = 5` por frame: si el acumulador sigue sobrando, **se descarta** (la física "pierde" tiempo, aceptable); (b) el `min(dt, 0.25)` descarta stalls de ventana. Ambos juntos hacen el bucle irrompible.

**4. Interpolación de render (eliminar el stutter).**
Con pasos fijos, el render ocurre "en medio" de dos pasos físicos (el sobrante `alpha`): si dibujas el estado tal cual, a 144 Hz verás saltitos porque algunos frames no consumen paso físico. Solución estándar: guardar la posición del paso anterior e interpolar **solo para dibujar**:

```
p_render = lerp(prevPosition, position, alpha)   // solo en el ModelMatrix del render
```

La física NUNCA usa `prevPosition`: es cosmética para la cámara. (La rotación se interpola igual o por ángulo más corto — con este proyecto basta lerpear la posición y ya.)

**5. Elección de `h` (compromiso exactitud/coste).**
`h` más chico = más exacto, más CPU: `1/60` es lo mínimo digno; `1/120` hace al apilado 4× más estable que `1/60` (más oportunidades de corrección por segundo) al costo de 2× steps. Recomendado para el proyecto: **`h = 1/120` con `MAX_PASOS = 5`** — cubre hasta 24 FPS reales antes de descartar tiempo.

### 🔨 Implementación

- [ ] 💛 🤝 `Application`: acumulador, `MAX_PASOS`, `alpha`; el clamp+substeps de 2.4 se sustituye por esto (los substeps fijos de Fase 2.4 pueden eliminarse: el paso ya es chico).
- [ ] 💛 👤 A `Object`: `glm::vec3 prevPosition` (solo si `isDynamic`).
- [ ] 💛 🤝 `Physics::step` guarda `prevPosition = position` al INICIO de cada paso fijo.
- [ ] 💛 👤 B El render (ModelMatrix de objetos dinámicos) usa `lerp(prev, curr, alpha)` — `Object` gana `getRenderPosition(alpha)` para que B no duplique la lógica.
- [ ] 🟢 🤝 Test A/B de "misma trayectoria": soltar la misma pelota a 60 FPS vs con la ventana limitada a 30 → misma secuencia de rebotes (mayor altura del 2º rebote ±1 %).

### ✅ Criterio de aceptación
- Pelota rebotando con vsync OFF en un monitor de 144 Hz: trayectoria suave, sin saltitos (interpolación ON) y sin "morir" antes que a 60 Hz.
- La torre de la fase siguiente se comporta igual a 30 y a 144 FPS.
- Mover/arrastrar la ventana (stall de 2 s): no explota, no atraviesa el suelo.

### ⚠️ Riesgos
- 💛 Interacción con el picking: el ratón lee píxeles del RENDER (posición interpolada) pero la física está en `position` — es correcto, pero el debug de contactos (3.7) debe dibujar sobre la posición FÍSICA, no la interpolada. Documentarlo o el 3.7 "parecerá" desalineado.

---

## FASE 3.2 — 🔥 Fricción, iteraciones y apilado estable

🎯 **Objetivo**: torres de cubos que se derrumban de forma creíble y quedan quietas sin temblar. Es la fase más difícil del plan — y la más impresionante en un video.

### 📚 Teoría

**1. El problema del apilado (por qué TODO lo anterior tiembla).**
Un cubo encima de otro: la gravedad le inyecta `g·h` de velocidad cada paso, el contacto la cancela, la corrección de posición lo empuja... en el orden que sea, algo sobra: el cubo "flota-sube-baja" un poco cada frame (jitter). Sin fricción, además, cualquier toque lateral dispara la torre como hielo. El apilado estable necesita TRES piezas: fricción + iteraciones + dormir. Las tres van juntas.

**2. Fricción de Coulomb (el cono de fricción).**
La fricción es un impulso **tangencial** limitado por el impulso normal: los materiales dan un coeficiente `μ` (madera-madera ≈ 0.4, goma ≈ 0.9, hielo ≈ 0.02):

```
t = v_rel − (v_rel·n)·n        // parte tangencial de la velocidad relativa
si |t| > ε:  t /= |t|          // dirección tangencial unitaria

j_t = −dot(v_rel, t) / den     // impulso que anularía el deslizamiento (den = Σ invM)
j_t = clamp(j_t, −μ·j_n, μ·j_n)  // ★ el cono: |fricción| ≤ μ × impulso normal

vA -= (j_t · invMA) · t
vB += (j_t · invMB) · t
```

Intuición: la fricción "quiere" parar el deslizamiento, pero solo puede empujar hasta `μ·j_n`. Si el impulso normal es 0 (no hay presión), no hay fricción — por eso la fricción se calcula **después** del impulso normal de ese mismo contacto.
Fricción estática vs dinámica: para el proyecto, un solo `μ` basta (los motores reales usan `μ_s > μ_d`; mencionable en la defensa).

**3. Resolución iterativa (impulsos secuenciales de Gauss-Seidel).**
Un cubo de la base de una torre tiene 4 contactos; resolver cada contacto una vez deja a los demás "desactualizados". La solución de todos los motores por impulsos: **recorrer todos los contactos N veces** (8–16 iteraciones típicas en 3D; con esferas 4–8 sobran):

```
for iter in 1..N:                       // N = 8
    for cada contacto c:                // usar SIEMPRE las v actuales
        resolver impulso normal de c    // (recalcula vn con v actuales)
        resolver fricción de c
    if (ningún contacto cambió) break   // convergió antes → free perf
```

Es Gauss-Seidel: cada resolución usa el resultado de la anterior y converge rápido (~4-8 pasadas). Esto NO inyecta energía porque cada impulso individual cumple su condición (solo actúa si `vn < 0`, con clamps). Iterar en un orden consistente (pares de la lista en el mismo orden) para mantener determinismo.

**4. Reposo y "dormir" (sleeping) — el fin del jitter y de la CPU quemada.**
Con todo lo anterior el cubo aún temblaría microscópicamente (energía numérica de redondeo). Estándar de la industria: **dormir** cuerpos que llevan un tiempo sin moverse:

```
si (|v|² < (0.01)²) durante T = 0.5 s consecutivos:  sleeping = true
dormido:  NO integrar (ni gravedad), NO resolver → cero CPU, cero jitter
despertar: cualquier impulso/contacto con cuerpo despierto, o cambio desde la UI
```

Reglas para que no "muera" de forma rara: un cuerpo despierto que golpea a uno dormido lo despierta SIEMPRE (el solver comprueba `other->sleeping`); al arrastrar con el ratón (3.6) se despierta al agarrar; al marcar `isDynamic` en la UI se despierta. La condición debe resetearse con cualquier velocidad no trivial — usar `|v|² < umbral²` (comparar cuadrados, sin sqrt).

**5. Corrección de posición amortiguada por el tiempo (Baumgarte temporal).**
Con `h` fijo, el `pct` de corrección por paso se convierte en velocidad de corrección constante: `pct = 0.6` con `h = 1/120` corrige ~todo en < 20 ms y sin pop. La `slop` (0.01) sigue siendo la regla de oro anti-jitter: los cuerpos en reposo pueden penetrarse visualmente 1 cm sin que el motor los moleste. Un cubo apoyado "descansa" DENTRO de la slop.

**6. La gran mentira de este motor (para la defensa, saberlo diferencia del resto).**
Este es un motor de **partículas con esfera envolvente**: no hay rotación dinámica (el cubo que cae de canto no gira y aterriza "flotando" sobre su esfera). El torque/rotación real requiere el formalismo de cuerpos rígidos (tensor de inercia, impulso angular, puntos de contacto múltiples) — está fuera de alcance y está BIEN: es la elección de diseño estándar (las pelotas de física estilo "Pong 3D" no rotan). Decirlo con orgullo en el video: "sistema de partículas con impulsos, rotación explícitamente excluida del alcance".

### 🔨 Implementación

- [ ] 🔥 👤 A `Object`: `friction μ` (default 0.4), `sleeping = false`, `sleepTimer`.
- [ ] 🔥 👤 A Solver dentro de `Physics`: lista de contactos del frame → bucle de 8 iteraciones (normal + fricción), orden estable.
- [ ] 🔥 👤 A Sleeping: lógica de dormir/despertar (despertar en cualquier resolución si el otro está despierto y se le inyecta impulso).
- [ ] 🔥 🤝 Tuning session A+B: `pct`, `slop`, `μ` default, umbral y tiempo de sueño — apartar 2–3 horas juntas, es empírico.
- [ ] 🟢 👤 B Demo visual del sueño (en 3.5/3.7): color azulado cuando `sleeping` — hace evidente que el sistema funciona.

### ✅ Criterio de aceptación
- Torre de 5 cubos: quieta eternamente (FPS suben al dormirse todo — medir y presumir).
- Empujón lateral a la base: se derrumba de forma creíble, los cubos despiertan al contacto.
- Esferas dejan de derraparse: una pelota en el suelo con `μ` alto se detiene al soltarla tras un rebote.
- Test de "billar con fricción": pelota rodando en el plano se frena suave (no de golpe).

### ⚠️ Riesgos
- 🔥 Es LA fase de ajuste fino. Si el tiempo aprieta: bajar a 4 iteraciones + solo sleeping (sin fricción) ya quita el 80 % del jitter. La fricción es la que más rompe (cone clamp con contactos casi nulos).

---

## FASE 3.3 — 💛 Broadphase: rejilla espacial uniforme

🎯 **Objetivo**: 200–500 objetos rebotando sin que el O(n²) mate los FPS.

### 📚 Teoría

**1. Broadphase vs narrowphase (la arquitectura de dos etapas de todo motor).**
- **Narrowphase**: test exacto par-a-par (esfera-esfera/AABB) — exacto pero costoso: 200 objetos = 19 900 pares, el 99 % lejanos entre sí.
- **Broadphase**: filtro barato que descarta los pares **imposibles** y le pasa al narrowphase solo los "candidatos". Regla: broadphase debe ser ≥ 10× más barato por par.

**2. Rejilla espacial uniforme (uniform grid / spatial hash).**
Divide el mundo en celdas cúbicas de lado `cell`. Un par puede colisionar solo si ocupan celdas iguales o vecinas, luego:

```
cell ≥ diámetro máximo de objeto   // garantiza: overlap ⟹ celdas iguales o vecinas
```

Cada objeto se inserta en las celdas que toca su AABB; los pares se generan **dentro de cada celda** (2 objetos en la misma celda = candidato una vez):

```
celda (3,0,5) con objetos [A, B, C]  →  candidatos (A,B), (A,C), (B,C)
```

**3. Hashing espacial (por qué no un array 3D).**
Un array `[W][H][D]` de celdas gasta memoria infinita (mundo abierto, cámara libre). La solución: **hash del índice 3D a una tabla compacta** (`unordered_map`):

```
key = (int64)ix * 73856093  ^  (int64)iy * 19349663  ^  (int64)iz * 83492791
```

(son los multiplicadores mágicos clásicos de la rejilla de hashing de Teschner et al.; con XOR dispersan bien índices negativos/positivos). Colisiones de hash: distintas celdas con igual key → solo generan un candidato extra (el narrowphase lo descarta); **no puede haber falsos negativos** si la celda se calcula igual para ambos.

**4. Deduplicación de pares (el detalle que muerde).**
Un AABB grande toca 4 celdas → el par (A,B) aparece en 2+ celdas. Sin deduplicar, el narrowphase resuelve el mismo contacto 2 veces = doble impulso = explosión. Solución estándar: clave canónica de par ordenado + `unordered_set`:

```
pairKey = (uint64)min(idA, idB) << 32  |  (uint64)max(idA, idB)
```

**5. Coste real (para venderlo en la defensa).**
Rebuild por frame (clear + insert): O(n). Pares generados: O(n + k) con k = pares en celdas compartidas — con 300 pelotas en un tablero, k ≈ n (vecinos inmediatos), ~300 candidatos vs 44 850 del O(n²). **150× menos narrowphase**. Rebuild completo en vez de incremental: con 300–500 objetos el rebuild es trivial (< 0.1 ms); el incremental (objetos persistentes, update al moverse) solo paga con 10 000+ — fuera de alcance, mencionar como "trabajo futuro" en la defensa.

**6. ¿Cuándo NO hace falta?** Medir primero (criterio 2.3): si el proyecto nunca pasa de 100 objetos, esta fase aporta poco. Hacerla si: la demo de "lluvia de 200 pelotas" baja de 60 FPS.

### 🔨 Implementación

- [ ] 💛 👤 A `Physics`: reconstruir la rejilla cada paso (físico, no de frame): `cell = 1.2 × máx(r)`.
- [ ] 💛 👤 A Generación de candidatos con `unordered_map<int64, vector<obj*>>` + `unordered_set<uint64>` de dedup.
- [ ] 💛 👤 A El narrowphase existente (2.2/2.3) recibe SOLO la lista de candidatos.
- [ ] 🟢 👤 A Test de stress: botón "Lluvia de 200 pelotas" (spawner temporal).

### ✅ Criterio de aceptación
- 200 pelotas cayendo: > 60 FPS con física a `h = 1/120` (y comparar FPS con el broadphase desactivado para el video — antes/después).
- Sin comportamientos raros: ninguna colisión "se pierde" (pelotas nunca se atraviesan en el suelo).

### ⚠️ Riesgos
- 💛 Todas las pelotas cayendo en la MISMA celda (nacen apiladas): k degenera → el primer segundo va a saltos. Mitigar: spawner con dispersión aleatoria de XZ.

---

## FASE 3.4 — 💛 OBB: estáticos rotados de verdad

🎯 **Objetivo**: una rampa rotada desvía la pelota correctamente (hoy el AABB conservador hace rebotes fantasma "antes de tocar").

### 📚 Teoría

**1. El problema del AABB con rotación.**
`GetWorldAABB` proyecta las 8 esquinas a mundo y toma el min/max → un cubo rotado 45° genera un AABB hasta ~41 % mayor. Una pelota rebota a 1 m del cubo real (falso positivo). El OBB (Oriented Bounding Box) es la caja con su **orientación**: la forma exacta de estáticos rotados (para dinámicos, la esfera envolvente sigue siendo el modelo — con OBB solo en estáticos basta).

**2. Esfera vs OBB: transformar al espacio local del OBB.**
El truco universal: llevar el centro de la esfera al espacio LOCAL del OBB, donde el OBB vuelve a ser un AABB axis-aligned, reusar el closest-point de la Fase 2.2, y transformar de vuelta:

```
M = T·R·S                     // el ModelMatrix del estático (Transform::getModelMatrix)
c_local = M⁻¹ · c_world       // centro en espacio del mesh (con rotación "des-hecha")
q_local = clamp(c_local, bboxMin, bboxMax)    // ★ mismo código de 2.2, con el bbox del mesh
q_world = M · q_local          // punto de contacto de vuelta a mundo
d = c_world − q_world
dist = |d|                     // |M·x − M·y| ≠ dist world si hay escala... ⚠ ver teoría 3
```

**3. La trampa de la escala no uniforme (EL detalle técnico de esta fase).**
`M = T·R·S`: si la escala es no uniforme (x≠y≠z), un punto clampado en local y transformado a mundo NO está a distancia euclídea correcta: la rotación preserva distancias, pero la escala no. La forma correcta y barata — medir la distancia en mundo, no en local:

```
q_world = M · q_local                       // el punto del OBB más cercano, en mundo
d = c_world − q_world                       // vector en MUNDO
colisión ⟺ |d| < r                          // ¡la esfera vive en mundo, medir ahí!
```

El clamp en local sigue siendo válido como "punto del OBB más cercano" (aproximación estándar de la industria con escala no uniforme: Bullet/PhysX hacen exactamente esto). Para escala uniforme es EXACTO; con no uniforme, error sub-centimétrico aceptable.

Cálculo de `M⁻¹` barato (sin `glm::inverse` general): con `M = T·R·S`, la inversa es `S⁻¹·Rᵀ·(−T)`:

```
c_rot = transpose(mat3(M)) · (c − T)        // espacio "solo rotado" (R orthonormal ⟹ R⁻¹ = Rᵀ)
                                        // aquí el OBB es axis-aligned pero ESCALADO
```

Alternativa aún más simple para el proyecto: `glm::inverse(M)` una vez por estático por paso (los estáticos no se mueven → cachear la inversa al marcar `solid`). Con < 100 estáticos, el costo es irrelevante y el código, trivial de defender.

**4. Por qué solo estáticos.**
Un OBB dinámico exige resolver OBB-OBB (SAT: Separating Axis Theorem, 15 ejes en 3D) — costoso y complejo. Como los dinámicos usan esfera envolvente, el par dinámico-estático es esfera-OBB (un solo closest-point) y dinámico-dinámico queda esfera-esfera (Fase 2.3). El triángulo de costes del proyecto: **esfera-esfera + esfera-OBB cubre TODO con cero SAT**.

**5. Smooth normals en aristas (la calidad de los rebotes).**
El closest-point en una ARISTA del OBB da una normal en la bisectriz (como en 2.2) → rebotes naturales en esquinas de rampas. Esto es gratis con closest-point y es lo que hace que "la pelota rebote en la esquina de la mesa" se vea creíble.

### 🔨 Implementación

- [ ] 💛 👤 A `Collision.h`: `bool SphereVsObbContact(c, r, const Object* staticObj, Contact&)` (usa `mesh->getBBox*` + `transform.getModelMatrix()`, cacheando la inversa si hay perf issues — no habrá).
- [ ] 💛 👤 A `Physics::step`: los estáticos rotados (ángulo ≠ 0 en cualquier eje o detectado por flag `useObb`) usan el test OBB; los axis-aligned siguen por AABB (más barato) — o simplemente todos por OBB (unifica el código: el AABB es un OBB sin rotación; decisión de diseño recomendada).
- [ ] 🟢 🤝 Escena demo: rampa rotada 30° + pelotas dinámicas cayendo.

### ✅ Criterio de aceptación
- Pelota cae sobre rampa de 30°: sale desviada en la dirección física correcta (comprobar contra "rebota en invisible" del AABB).
- Pasar la esfera rozando la esquina de un cubo rotado: rebote en bisectriz, sin falsos positivos previos.
- FPS sin regresión (el OBB con inversa cacheada por estático ≈ costo AABB).

### ⚠️ Riesgos
- 💛 Estáticos con `rotation` animada desde la UI: la inversa cacheada se invalida → cachear "por frame" no "por sesión", o no cachear (irrelevante a esta escala).

---

## FASE 3.5 — 🟢 Eventos de colisión

🎯 **Objetivo**: la app "se entera" de las colisiones (parpadeo de color proporcional al impacto) — el feedback visual que hace la física *perceptible* en el video.

### 📚 Teoría

**1. Evento ≠ resolución (la regla de no-mutar-durante-el-recorrido).**
El solver recorre contactos inyectando impulsos: si un handler de evento (cambiar color, eliminar objeto, reproducir sonido) mutara la lista de objetos DURANTE ese recorrido, invalidaría iteradores y estados. Estándar: **cola de eventos durante el paso, dispatch después**:

```
Physics::step:                              Application (después del step):
  resolver contacto → push event {A, B,     ←  for ev : events:
  punto, normal, j}                            obj->hitFlash = f(j);
                                               obj->tint = color(j);
```

**2. Qué información lleva un evento útil (y por qué el impulso `j`).**
El impulso `j` ya lo calcula el solver (Fase 2.3): es la medida física del "golpe". Intensidad visual ∝ `j` (o `j/m` del receptor = cambio de velocidad recibido, más intuitivo):

```
struct CollisionEvent {
    uint32_t idA, idB;        // 0 = suelo/estático sin id relevante
    glm::vec3 point, normal;  // para efectos (chispas, decals)
    float impulse;            // j en N·s
};
```

`j/m` del receptor → Δv que recibió: pelota de 0.1 kg golpeada con j=0.5 → Δv=5 m/s → flash intenso aunque la pelota sea chica. Mapear con saturación (`flash = min(1, k·Δv)`).

**3. Umbral y debounce (evitar spam de eventos).**
Dos esferas en contacto sostenido (apiladas) generan un evento POR PASO FÍSICO → a `h = 1/120` son 7200 eventos/min de "estoy tocando". Dos mitigaciones estándar: (a) **umbral de impulso**: solo evento si `j > j_min` (el reposo casi no genera j); (b) **cooldown por par**: no re-eventear el mismo par (A,B) en < 100 ms.

### 🔨 Implementación

- [ ] 🟢 👤 A `Physics`: `std::vector<CollisionEvent> events` del paso + `getEvents()` (o callback `std::function` registrado — vector + poll es más simple y suficiente).
- [ ] 🟢 🤝 `Application`: tras `step()`, consumir eventos → `Object::hitFlash = 0.25 s` + intensidad ∝ `j/m`.
- [ ] 🟢 👤 B `Renderer`: tint de `diffuseColor` hacia blanco/emissivo según `hitFlash` (decae linealmente con `dt` de frame).
- [ ] 🟢 🤝 Umbral `j_min` + cooldown por par.

### ✅ Criterio de aceptación
- Pelota fuerte contra pared: flash intenso; pelota suave: flash tenue (proporcionalidad visible).
- Apilado quieto: cero flashes (el umbral funciona).
- 200 pelotas en lluvia: FPS sin regresión (el vector de eventos es barato).

### ⚠️ Riesgos
- 🟢 Ninguno serio. Riesgo de "muy vistoso": calibrar el tint para el video (no cegar la escena con 200 pelotas cayendo).

---

## FASE 3.6 — 💛 Lanzar objetos con el ratón (drag & throw)

🎯 **Objetivo**: agarrar un objeto con click (picking existente), arrastrarlo con el ratón y lanzarlo al soltar. La interacción estrella del video.

### 📚 Teoría

**1. Planos de arrastre (qué movimiento esperas del drag).**
Dos convenciones: (a) plano **paralelo a la vista** (el objeto se mueve "en la pantalla" como en un editor — el estándar); (b) plano **horizontal** a la altura del objeto (estilo "agarro de mesa", mueve en XZ). Para lanzar, (a) es superior: permite "cargar" hacia atrás y arriba antes de soltar. Se define con un punto (posición del objeto al agarrar) y la normal `= camera.front`:

```
plane: punto P₀ = posición del objeto, normal n = normalize(camera.front)
```

**2. Rayo desde el ratón (reusar la matemática del picking).**
El FBO de picking dice QUÉ objeto, pero para arrastrar hace falta el rayo continuo (a 60 Hz, no solo al click). De pantalla a mundo:

```
ndc = ( 2·x/w − 1 , 1 − 2·y/h , −1 )            // clip space
invVP = inverse(proj · view)                     // se puede cachear por frame
rayDir = normalize( (invVP · vec4(ndc, 1.0)).xyz / w  −  cameraPos )
```

(GLM hace la parte pesada: `glm::unProject` — más simple y defendible que manual.)

**3. Intersección rayo-plano (la fórmula única del drag).**
Sustituyendo la recta `P(t) = camPos + t·rayDir` en el plano `dot(P − P₀, n) = 0`:

```
den = dot(rayDir, n)
si |den| < ε: rayo casi paralelo → no mover este frame
t = dot(P₀ − camPos, n) / den
P = camPos + t · rayDir          // punto del plano bajo el cursor → nueva posición
```

**4. Velocidad de lanzamiento (el corazón del "throw").**
La velocidad de salida se estima con las últimas posiciones de arrastre — pero NO con solo 2 frames (ruidoso y dependiente de FPS). Estándar: **historial circular de ~100–150 ms** con timestamps:

```
v_lanzamiento = (P_última − P_hace_120ms) / (t_última − t_hace_120ms)
```

Suavizado extra opcional (media móvil exponencial: `v = lerp(v, v_medida, 0.4)` por frame). El historial de 120 ms es el compromiso: suficientes frames para promediar, suficientemente fresco para captar el "latigazo".

**5. El drag es cinemático, no dinámico (el truco de estabilidad).**
Mientras se arrastra, el cuerpo NO participa como dinámico normal (la física pelearía contra el ratón y temblaría). Patrones estándar:
- **Kinematic-drag**: mientras está agarrado, se posiciona directamente (sin gravedad/solver), pero SIGUE resolviendo colisiones contra otros cuerpos (empuja a los demás — MUY vistoso);
- Al soltar: `sleeping = false`, `velocity = v_lanzamiento · factor` (factor 1.0–1.5 para que "llegue").
Cuidado con empujar dentro de un estático (teleport del drag): si la posición del plano penetra, clamp con `SphereVsAabbContact` de la Fase 2.2 (reuso directo).

**6. Despertar a los vecinos (encadenar el efecto dominó).**
Al arrastrar, el cuerpo agarrado despierta; los que él empuja reciben impulso → despiertan ellos también (regla de 3.2). Elegante: solo hay que garantizar que el solver despierta a quien recibe impulso.

### 🔨 Implementación

- [ ] 💛 🤝 Click en objeto dinámico (picking FBO existente) → entrar en `DRAG` (nuevo estado de input en `Application`, junto a SELECT/MOVE).
- [ ] 💛 🤝 Mientras `DRAG`: `unProject` + rayo-plano → posición; clamp anti-penetración contra estáticos (2.2); historial circular 120 ms.
- [ ] 💛 🤝 Al soltar: `velocity = v` del historial × factor, `sleeping = false`, evento de "throw" para flash.
- [ ] 🟢 👤 A Los cuerpos que el drag empuja se resuelven con el solver normal (ya funciona si 3.2 está: despertar al recibir impulso).
- [ ] 🟢 🤝 Integrar con `collisionsEnabled` y con el modo MOVE de cámara (click en vacío = selección actual; click en dinámico = grab; ESC suelta).

### ✅ Criterio de aceptación
- Agarrar pelota, cargar atrás y soltar → sale disparada con la dirección del gesto.
- Lanzarla contra una torre → la derriba (dominó con eventos de 3.5).
- Arrastrar un cubo DENTRO de una pila → los empuja, nada atraviesa nada.
- El drag funciona igual con 30 o 200 objetos alrededor (interacción con 3.3 sin regresión).

### ⚠️ Riesgos
- 💛 Conflicto de input: el click hoy selecciona. Definir claramente: modo SELECT normal (click corto = pick/selección) vs grab (si el objeto es dinámico y se mueve el ratón con el botón abajo → drag). Probar que la selección normal no se rompe.
- 💛 Lanzar "contra la cámara" (den ≈ 0): el rayo-plano degenera → guard `|den| < ε` (teoría 3).

---

## FASE 3.7 — 🟢 Debug rendering de colisiones

🎯 **Objetivo**: VER la física: AABBs/OBBs, esferas envolventes, normales de contacto, cuerpos dormidos. La herramienta que además hace el video de defensa autoexplicativo.

### 📚 Teoría

**1. Debug inmediato vs persistente (por qué "immediate mode").**
Debug de física cambia cada frame (contactos aparecen/desaparecen): la forma correcta es **regenerar el buffer de líneas cada frame** y subirlo con `GL_DYNAMIC_DRAW` (OpenGL espera cambios → driver lo pone en memoria de escritura rápida). Nada de VAOs persistentes ni cachear: es data desechable. El costo (unas centenas de vértices) es despreciable; es exactamente lo que hacen los debug drawers de Bullet/PhysX.

**2. Geometría de las primitivas de debug (listas de segmentos).**
Todo se dibuja como `GL_LINES` — cero shaders nuevos, el `debug` shader existente (color plano + MVP) sirve:
- **AABB**: las 12 aristas de la caja (24 vértices). Generador trivial de 12 segmentos desde min/max.
- **Esfera envolvente**: 3 círculos ortogonales (XY, YZ, XZ) de ~24 segmentos cada uno = 3×24 segmentos. (Alambre de esfera barato y clarísimo.)
- **Normal de contacto**: segmento `point → point + n·0.25f` — LA línea que explica cada rebote.
- **Velocidad**: segmento `position → position + v·(0.1 s)` (predicción del movimiento).
- **Sueño**: color de la esfera azulada si `sleeping` (código de color, no geometría nueva).

**3. Separación render/física (la coherencia visual).**
El paso fijo (3.1) interpola la posición para RENDER, pero los contactos viven en la posición FÍSICA: dibujar el debug sobre la posición física (con el toggle de debug, desactivar la interpolación de render para los dinámicos — o dibujar ambos y verás la discrepancia, que es además pedagógico en la defensa). Decisión simple recomendada: cuando el debug está ON, render de dinámicos sin interpolar → TODO cuadra al milímetro.

**4. Toggle global y cap de líneas (higiene).**
Checkbox global "Mostrar colisiones" (off por defecto); con 200 objetos × 3 círculos + contactos, son ~30 000 segmentos — cap con un límite (p. ej. primeros 500 contactos) para no frene el driver de líneas. Medir y ya.

### 🔨 Implementación

- [ ] 🟢 👤 B `render/DebugDraw.h/.cpp` (compañero B — OpenGL):
  ```cpp
  class DebugDraw {                    // reusa el Shader debug existente
      void begin(const glm::mat4& vp);           // bind shader + VAO dinámico
      void line(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color);
      void aabb(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color);
      void sphereWire(const glm::vec3& c, float r, const glm::vec3& color);
      void end();                                // glBufferData(DYNAMIC) + draw
  };
  ```
- [ ] 🟢 🤝 `Physics` expone por paso: AABBs usados (o OBBs), esferas con radio, contactos `{point, normal}` (ya existen desde 2.2/2.3/3.4 — solo exponerlos).
- [ ] 🟢 👤 A UI: checkbox "Mostrar colisiones" + sub-toggles (volúmenes / contactos / velocidades / sueño).
- [ ] 🟢 🤝 Código de color: estáticos = blanco, dinámicos = amarillo, dormidos = azul, contactos = rojo, velocidades = verde.

### ✅ Criterio de aceptación
- Encender debug y rebotar una pelota: la línea roja del contacto aparece EXACTAMENTE donde toca y apunta hacia donde rebota.
- La esfera amarilla es la que "realmente choca" (se ve el margen vs la malla del cubo: autoexplicativo en el video).
- Torre quieta: todo azul (dormido) — demo visual del sleeping.

### ⚠️ Riesgos
- 🟢 Ninguno funcional. Solo cuidar el orden de dibujo (debug después de la escena, sin depth-test o con depth-test según se prefiera ver "a través").

---

# 🗺️ Orden, dependencias y reparto

## Grafo de dependencias

```
ÁMBITO 2 (en orden):
2.1 ──→ 2.2 ──→ 2.3 ──→ 2.4        (2.4 también convive con 3.1: substeps → timestep fijo)

ÁMBITO 3 (tras 2.4):
3.1 (timestep)  ──┬──→ 3.2 (apilado: REQUIERE 3.1 para ser estable de verdad)
                 └──→ 3.6 (drag: interpola/estados — hacer tras 3.1, ideal)
3.2 ──→ (3.5 eventos se benefician del umbral de reposo)
3.3 (broadphase) ── independiente (tras 2.3)
3.4 (OBB) ────────── independiente (tras 2.2)
3.5 (eventos) ────── independiente (tras 2.3)
3.7 (debug) ──────── independiente (tras 2.2), HACERLO PRONTO: ayuda a tunear todo lo demás
```

**Recomendación de calendario** (con defensas en la fecha de entrega):

| Semana | Fases | Nota |
|---|---|---|
| 1 | 2.1 → 2.2 | + 3.7 en paralelo (B) desde ya: el debug acelera TODO el tuning |
| 2 | 2.3 → 2.4 → **video del Ámbito 2** | grabar el bloque 2 cuando funciona, no al final |
| 3 | 3.1 → 3.2 | la pareja dura; tunear juntos |
| 4 | 3.4 + 3.5 + 3.6 (a la carta) + integración final | 3.3 solo si la demo de 200 pelotas lo pide |

## Reparto por persona

| Fase | A (escena/UI/lógica) | B (render/IO) |
|---|---|---|
| 2.1–2.4 | Campos, `Physics`, solver, serialización, paneles | Loop/llamada, validación visual, escenas de prueba |
| 3.1 | Acumulador, `prevPosition`, `getRenderPosition` | Interpolación en render (ModelMatrix) |
| 3.2 | Solver iterativo, fricción, sleeping | Tuning + demo torre |
| 3.3 | Rejilla + dedup | Test de stress + medición FPS |
| 3.4 | `SphereVsObbContact` | Cache de inversas + demo rampa |
| 3.5 | Cola de eventos | Tint en `Renderer` |
| 3.6 | Estados de input, historial, rayo-plano | Integración con picking FBO |
| 3.7 | Exposición de datos físicos | `DebugDraw` completo |

**Regla del README sigue vigente**: B nunca bloqueado por A. En 3.6/3.7 hay solapamiento natural (picking y debug son territorio de B; UI y estados de A).

---

# 🎥 Bloques del video de defensa (por qué cada fase "vende")

1. **2.1** — "Euler semi-implícito vs explícito": pelota con integrador explícito explotando (5 s de demo) vs la nuestra estable. Diferenciador real.
2. **2.2** — rebote en esquina/arista del cubo (normal bisectriz del closest-point): prueba de calidad del narrowphase.
3. **2.3** — billar: 3 esferas alineadas + impulso transmitido. Mencionar `e²` y la pérdida de energía.
4. **2.4** — guardar/cargar con la física en marcha (round-trip).
5. **3.1** — misma pelota a 30 vs 144 FPS, misma altura de rebote (determinismo).
6. **3.2** — torre quieta eternamente + derribo. El clímax.
7. **3.3** — 200 pelotas con FPS visibles en pantalla, antes/después del broadphase.
8. **3.4** — rampa rotada desviando pelotas (contra el "rebota en el aire" del AABB).
9. **3.5** — flashes de impacto proporcionales (¡jugarlo en vivo!).
10. **3.6** — lanzar una pelota contra la torre y derribarla con el ratón. El final del video.

---

# ⚠️ Riesgos globales y plan de contención

| Riesgo | Síntoma | Contención |
|---|---|---|
| 🔥 Jitter del apilado (3.2) | Torres que tiemblan/explotan solas | 4 iteraciones + slop generoso + sleeping agresivo; si no da tiempo: cortar fricción, quedarse con 2.x (cumple el requisito base) |
| 💛 Tunneling residual | pelota atraviesa suelo fino a alta velocidad | `h = 1/120` + cap de velocidad de lanzamiento en 3.6 (p. ej. 20 m/s) |
| 💛 AABB conservador visible | rebotes "en el aire" contra rotados | 3.4 OBB; mientras: en demos usar estáticos axis-aligned |
| 💛 Input conflict (3.6) | el grab rompe la selección | decidir semántica ANTES de codear (teoría 3.6) |
| 🟢 Perf general | FPS < 60 con física | medir por fase (criterios de aceptación lo exigen); broadphase 3.3 solo si los números lo piden |

**Regla de oro de la defensa**: cada fase tiene un demo visual de 10–30 s. Si una fase no tiene su demo, no entró al video — y lo que no entra al video, para el profesor no existe.
