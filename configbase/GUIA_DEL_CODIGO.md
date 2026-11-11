# Guia del codigo de voleibol

Abrir Voleibol.sln en Visual Studio, seleccionar Debug / x86 y pulsar Ctrl+F5. El proyecto usa MSVC v143. El directorio de trabajo es la carpeta de la solucion; los recursos se buscan en configbase. Para ejecutar el EXE preparado, conservarlo junto a configbase y sus dos DLL.

## Estructura como las practicas

- Voleibol.cpp: variables, main, vertices del cubo/plano, texturas, luces, escenario, animaciones y controles.
- Shader.h: lectura, compilacion y uso de shaders; base del apoyo de clase.
- Mesh.h: Vertex, Texture, VAO/VBO/EBO y Draw; base del apoyo de clase.
- Model.h: Assimp, processNode, processMesh y TextureFromFile; base del apoyo de clase. Se adaptaron los colores y el material sin textura.
- Camera.h: ProcessKeyboard, ProcessMouseMovement, GetViewMatrix; se adapto tambien la camara orbital solicitada.
- Shader/lighting.vs: model, view, projection y matriz normal.
- Shader/lighting.frag: CalcDirLight y CalcSpotLight, con Phong como en iluminacion.

## Buscar durante la presentacion

| Tema | Funcion o archivo |
|---|---|
| Ventana y ciclo de dibujo | main, linea 87 de Voleibol.cpp |
| Vertices y buffers P01 | PrepararPrimitivas, linea 405 de Voleibol.cpp |
| Imagenes y filtros P06 | CargarTextura, linea 453 de Voleibol.cpp |
| Rutas de las imagenes | CargarTexturas, linea 473 de Voleibol.cpp |
| Propiedades de las luces P07/P08 | ConfigurarLuces, linea 501 de Voleibol.cpp |
| Saque por keyframes P10 | PoseSaque, linea 151 de Voleibol.cpp |
| Actualizacion y pausa P09 | Animation, linea 133 de Voleibol.cpp |
| Escala del trofeo | EscalaTrofeo, linea 168 de Voleibol.cpp |
| Matrices padre-hijo P04 | DibujarJugador, linea 200 de Voleibol.cpp |
| Cancha y medidas P03 | DibujarCancha, linea 226 de Voleibol.cpp |
| Bancas, trofeo y arbitro | DibujarBancas, linea 276 de Voleibol.cpp |
| Gradas y butacas OBJ | DibujarGradas, linea 310 de Voleibol.cpp |
| Sombras simuladas | SombraEn, linea 567 de Voleibol.cpp |
| Paneles Arial e I | DibujarInterfaz, linea 597 de Voleibol.cpp |
| Teclas I/J/K y animaciones | KeyCallback, linea 653 de Voleibol.cpp |
| Movimiento WASD y E/Q | DoMovement, linea 700 de Voleibol.cpp |

Las lineas cambian al editar; buscar tambien el nombre con Ctrl+F.

## Animaciones y controles

Espacio inicia el saque de ocho poses durante 3.4 segundos. Animation actualiza tiempoSaque; PoseSaque interpola brazo, rodilla, elevacion y posicion del balon con operaciones explicitas. F enfoca el trofeo y cambia su escala de 0.65 a 1.0 y de regreso en dos segundos. P pausa y R reinicia.

C/1/2 cambia camaras. WASD y E/Q mueven o giran; arrastrar el mouse orienta; la rueda cambia la distancia orbital. L alterna luces y las sombras simuladas. I muestra u oculta ambos paneles inferiores. J/K suman puntos. Esc cierra.

La captura F12 se retiro de esta version para mantener el codigo limitado a la base de clase. Las capturas de prueba se realizan con una herramienta externa, que no se incluye en el proyecto.

## Sombras e interfaz sencillas

Las sombras son imagenes semitransparentes dibujadas sobre el piso con traslacion, rotacion y escala. No calculan una proyeccion real desde la luz ni sombras entre objetos. Los paneles Arial son imagenes; el programa elige la imagen de cada estado y los digitos del marcador. No tiene un sistema de generacion de letras por caracter.

Se mantienen las siete imagenes de materiales descargadas de Internet. Su procedencia y licencia siguen en Textures/Fuentes.json. Las imagenes de interfaz y sombra son apoyos graficos del programa, no nuevos materiales fotograficos.

## Base de laboratorio

Referencias P01-P10 consultadas en lectura: JhonnyFeik/CGIH-FI, commit 2e31a29f8028ff8754658a31207974f8a95d14bf y las copias historicas conservadas. No se modificaron las practicas. Los apoyos de clase se adaptaron al escenario: no son copias sin cambios. Las dos camaras, las cuatro luces, el marcador y las sombras simuladas se construyen combinando sus matrices, imagenes, callbacks y funciones.
