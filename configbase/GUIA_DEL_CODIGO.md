# Cómo revisar esta versión

Abrir `Voleibol.sln` en Visual Studio y después **Código principal → Voleibol.cpp**. El escenario, las animaciones y los controles están en ese CPP. Los encabezados de apoyo se pueden consultar desde la misma solución.

Para localizar un cambio durante la presentación, usar **Ctrl+Mayús+F** y buscar una etiqueta con corchetes:

| Buscar | Dónde está |
|---|---|
| `[P01]` — mallas y shaders | `Graficos.h`: `Mesh::upload` y carga de shaders. |
| `[P02]` — proyecciones | `Voleibol.cpp`: viewport; `Camera.h`: perspectiva; `Graficos.h`: ortográfica. |
| `[P03]` — transformaciones | `Voleibol.cpp`: `transform` y funciones que colocan las figuras. |
| `[P04]` — jerarquía | `Voleibol.cpp`: `DibujarJugador`, brazos y piernas unidos a sus matrices padre. |
| `[P05]` — modelos y cámara | `Voleibol.cpp`: carga de OBJ; `Model.h`: Assimp; `Camera.h`: vista. |
| `[P06]` — texturizado | `Graficos.h`: imágenes; `Model.h`: materiales del OBJ; `scene.frag`: UV. |
| `[P07]` — iluminación | `Shader/scene.frag`: luz ambiente, difusa y brillo; `scene.vert`: normales. |
| `[P08]` — reflectores | `Shader/scene.frag`: posiciones, cono y atenuación. |
| `[P09]` — animación | `Voleibol.cpp`: `Animation`; el saque y el trofeo usan el tiempo actualizado aquí. |
| `[KEYFRAMES]`, `[ANIMACION]`, `[ESCALA]` | `Voleibol.cpp`: trayectoria y tiempos del saque, pausa y tamaño del trofeo. |
| `[CANCHA]`, `[RED]`, `[GRADAS]`, `[JUGADORES]`, `[BALON]` | `Voleibol.cpp`: posiciones, tamaños y cantidades. |
| `[COLORES]`, `[CONTROLES]`, `[LETREROS_Y_MARCADOR]` | `Voleibol.cpp`: colores, teclas y colocación de letreros y marcador. |
| `[TEXTURAS]`, `[CARGA_IMAGEN]` | `Graficos.h`: rutas de imágenes y opciones de carga. |
| `[ILUMINACION]`, `[POSICIONES_LUCES]` | `Shader/scene.frag`: intensidades y posiciones de las luces. |
| `[PERSPECTIVA]`, `[VELOCIDAD]`, `[VISTA_INICIAL]` | `Camera.h`: apertura, velocidad y posición inicial. |
| `[MODELOS]`, `[CARGA_OBJ]` | `Voleibol.cpp` y `Model.h`: rutas y carga de modelos. |
| `[SOMBRAS]`, `[PROYECCION_SOMBRA]` | `Voleibol.cpp` y `Graficos.h`: dibujo y proyección al piso. |
| `[INTERFAZ]`, `[PANELES]`, `[FUENTE]` | `Voleibol.cpp` y `Graficos.h`: instrucciones, ubicación y letra de los paneles. |

Las notas junto a cada bloque indican qué valor se puede ajustar y cuándo hay que cambiarlo en más de un lugar. Después de editar C++ o un encabezado, compilar de nuevo; después de editar un shader, cerrar y volver a ejecutar para cargarlo.

## Organización del CPP

| Sección | Qué contiene |
|---|---|
| 1. Variables del programa | Cámara, arreglo de teclas, luces, puntos, tiempos y los ocho cuadros clave del saque. |
| 2. main | Crea la ventana OpenGL, registra los callbacks y carga los dos modelos. En cada ciclo recibe eventos, mueve la cámara, actualiza la animación y dibuja. |
| 3. Animación | `Animation(dt)` actualiza los tiempos; `PoseSaque()` interpola las poses; `EscalaTrofeo()` cambia el tamaño. `Reiniciar()` restaura el estado. |
| 4. Escenario | Funciones `DibujarCancha`, `DibujarBancas`, `DibujarGradas`, `DibujarParedYMarcador`, `DibujarReflectores` y `DibujarJugador`. |
| 5. Dibujar un cuadro | `DibujarPantalla` reúne el escenario, las sombras y los paneles. |
| 6. Controles | `KeyCallback`, `MouseCallback`, `ScrollCallback` y `DoMovement`, siguiendo el esquema de las prácticas. |

## Encabezados de apoyo

- `Camera.h`: calcula la vista y la perspectiva; permite primera persona y orbital.
- `Model.h`: carga los OBJ, sus materiales y texturas mediante Assimp.
- `Graficos.h`: reúne el dibujo de mallas, la carga de imágenes con SOIL2, shaders, sombras planas y texto. Es un apoyo del proyecto; no es un archivo proporcionado por el profesor.

Las implementaciones de apoyo están en los encabezados y el proyecto compila una sola unidad: `Voleibol.cpp`. Esto sirve para mantener esta práctica pequeña; sus definiciones no se deben incluir en varios CPP a la vez.

## Relación con el laboratorio

Se consultaron P01-P09 de [JhonnyFeik/CGIH-FI](https://github.com/JhonnyFeik/CGIH-FI) en lectura el 08/11/2026. La referencia de P09 es [Animación Básica](https://github.com/JhonnyFeik/CGIH-FI/blob/cd4975cc27f5046ee0320311d8ef44776b156c7f/configbase/Animaci%C3%B3n_B%C3%A1sica.cpp).

| Práctica | Aplicación en voleibol |
|---|---|
| P01. Introducción a OpenGL | Mallas de triángulos, VAO, VBO, índices y shaders en `Graficos.h`. |
| P02. Proyecciones | `glViewport` en `DibujarPantalla` y perspectiva en `Camera.h`; la interfaz usa ortográfica. |
| P03. Modelado geométrico | Cubos y esferas colocados con matrices en las funciones de dibujo. |
| P04. Modelado jerárquico | En `DibujarJugador`, raíz → hombro → brazo → antebrazo y cadera → pierna → rodilla. |
| P05. Modelos y cámara | Los OBJ de trofeo y butaca se cargan en `main`; los callbacks controlan la cámara. |
| P06. Texturizado | Imágenes JPG descargadas, UV, repetición y filtrado en `Graficos.h`, `Model.h` y los shaders. Las fuentes están en `Textures/Fuentes.json`. |
| P07. Iluminación 1 | Ambiental, difusa de Lambert, especular de Blinn-Phong y normales en `Shader/scene.frag`. |
| P08. Iluminación 2 | Cuatro reflectores con cono y atenuación; L los alterna. La red usa recorte alfa. |
| P09. Animación básica | Callback → variable de estado → actualización por ciclo → transformación al dibujar. En esta versión se localiza en `KeyCallback`, `Animation`, `PoseSaque` y `DibujarJugador`. |

En P09, N activa `AnimBall`, `Animation()` incrementa `rotBall` y `glm::rotate` aplica el ángulo al balón. En voleibol, Espacio activa el saque, `Animation(dt)` avanza su tiempo y el dibujo usa las poses. Los ocho cuadros clave y su interpolación son la solución del proyecto para el saque; la práctica muestra una rotación continua.

## Qué se conservó y qué se simplificó

Se conservaron las tres zonas, medidas, doce jugadores, árbitro, trofeo, butacas, texturas descargadas, cámaras, saque, escala, luces, sombras, marcador, Arial y letreros de equipos.

El acceso principal ahora es una solución `.sln` preparada. Ya no hay que generar la solución con CMake ni ejecutar scripts. Las bibliotecas x64 están compiladas en `External Libraries/lib/x64`, como archivos de apoyo. No se distribuyen los generadores de recursos ni las herramientas de auditoría en esta carpeta.

Las rutas de recursos son relativas: `Textures`, `Models` y `Shader`, dentro de `configbase`. F12 guarda una captura y no sustituye archivos de capturas anteriores. Las verificaciones automáticas de esta reorganización se guardan por separado; no acreditan revisión del equipo, video ni envío al aula.
