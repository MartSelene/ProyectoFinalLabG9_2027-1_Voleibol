#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SOIL2/SOIL2.h"
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
// VARIABLES: ventana, camaras, teclado, luces y marcador.
const GLuint WIDTH = 1280, HEIGHT = 800;
GLfloat deltaTime = 0.0f, lastFrame = 0.0f; // Segundos entre cuadros; controla la velocidad de movimiento.
Camera camera(glm::vec3(12,1.7f,7), glm::vec3(0,1,0), -145.0f, -5.0f);
Shader lightingShader;
vector<Model> modelos;
bool keys[1024] = {};
bool firstMouse = true, reflectores = true, mostrarPaneles = false;
GLfloat lastX = 0, lastY = 0;
int puntosLocal = 0, puntosVisita = 0;
GLuint VAO_Cubo, VBO_Cubo, VAO_Plano, VBO_Plano;
GLuint madera, balon, pared, concreto, metal, tela, goma, maderaBanca, red;
GLuint letreroVerde, letreroNaranja, texturaBlanca, sombra;
GLuint digitos[10], digitosArial[10], estados[8];
GLuint menuControles, titulo, marcador, menuEstado;
int texturasCargadas = 0;
// KEYFRAMES: tiempos y poses del saque; aqui se ajusta la secuencia.
struct Pose { GLfloat arm, knee, rise;
    glm::vec3 ball;
};
struct FRAME { GLfloat tiempo;
    Pose pose;
};
const int MAX_FRAMES = 8;
FRAME cuadros[MAX_FRAMES] = {
    {0.00f,{8,0,0,glm::vec3(-10.2f,1.45f,0)}},
    {0.50f,{-75,18,0,glm::vec3(-10.0f,2.65f,0)}},
    {0.90f,{-145,28,.15f,glm::vec3(-9.85f,3.4f,0)}},
    {1.15f,{-95,5,.25f,glm::vec3(-9.7f,3.05f,0)}},
    {1.70f,{25,0,0,glm::vec3(-5,4.6f,0)}},
    {2.30f,{8,0,0,glm::vec3(.6f,4.1f,0)}},
    {3.00f,{8,0,0,glm::vec3(6.2f,1.1f,0)}},
    {3.40f,{8,0,0,glm::vec3(7.4f,.32f,0)}}
};
GLfloat tiempoSaque = 0, tiempoFestejo = 0;
bool saqueActivo = false, pausaSaque = false;
bool festejoActivo = false, pausaFestejo = false;
glm::vec3 navy(.055f,.13f,.21f), teal(.07f,.65f,.68f), orange(.96f,.42f,.16f);
glm::vec3 white(.93f,.96f,.98f), steel(.25f,.30f,.38f), skin(.77f,.52f,.34f);
glm::vec3 pointLightPositions[4] = {glm::vec3(-15,7,-8),glm::vec3(15,7,-8),glm::vec3(-15,7,8),glm::vec3(15,7,8)};
void KeyCallback(GLFWwindow*,int,int,int,int);
void MouseCallback(GLFWwindow*,double,double);
void ScrollCallback(GLFWwindow*,double,double);
void DoMovement();
void Animation();
void Reiniciar();
Pose PoseInicial();
Pose PoseSaque();
float EscalaTrofeo();
void PrepararPrimitivas();
void CargarTexturas();
GLuint CargarTextura(string ruta);
void ConfigurarLuces();
void DibujarPantalla(GLFWwindow*);
void DibujarCancha();
void DibujarBancas();
void DibujarGradas();
void DibujarParedYMarcador();
void DibujarReflectores();
void DibujarJugador(glm::vec3,float,glm::vec3,const Pose&);
void DibujarJugadoresYBalon(const Pose&);
void DibujarPostes();
void DibujarSombras(const Pose&);
void DibujarInterfaz(GLfloat,GLfloat);
glm::mat4 transform(glm::vec3,glm::vec3);
void DibujarMalla(GLuint,int,const glm::mat4&,glm::vec3,GLuint,glm::vec2);
void DibujarCubo(glm::vec3,glm::vec3,glm::vec3,GLuint=0,glm::vec2=glm::vec2(1));
void DibujarPlano(const glm::mat4&,glm::vec3,GLuint,glm::vec2=glm::vec2(1));
void DibujarModelo(int,const glm::mat4&,glm::vec3,GLuint=0);
void DibujarBalon(glm::vec3,float,glm::vec3,GLuint);
void DibujarParte(const glm::mat4&,glm::vec3,glm::vec3,glm::vec3,GLuint=0);
// INICIO: ventana, eventos y ciclo de actualizacion y dibujo.
int main() {
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(WIDTH,HEIGHT,"Voleibol",NULL,NULL);
    if (window == NULL) { glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) return 1;
    glfwSwapInterval(1); // Sincroniza los cuadros con la pantalla.
    glEnable(GL_DEPTH_TEST);
    glfwSetKeyCallback(window,KeyCallback);
    glfwSetCursorPosCallback(window,MouseCallback);
    glfwSetScrollCallback(window,ScrollCallback); // Registra la rueda para acercar o alejar.
    lightingShader = Shader("configbase/Shader/lighting.vs","configbase/Shader/lighting.frag");
    lightingShader.Use();
    PrepararPrimitivas();
    CargarTexturas();
    // MODELOS OBJ: trofeo, butaca y esfera; se cargan una vez al iniciar.
    modelos.push_back(Model((GLchar*)"configbase/Models/trofeo.obj"));
    modelos.push_back(Model((GLchar*)"configbase/Models/butaca/butaca.obj"));
    modelos.push_back(Model((GLchar*)"configbase/Models/balon.obj"));
    cout << "Modelos: " << modelos[0].GetTriangleCount() << ", " << modelos[1].GetTriangleCount() << ", " << modelos[2].GetTriangleCount() << " triangulos" << endl;
    lastFrame = (GLfloat)glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        GLfloat currentFrame = (GLfloat)glfwGetTime();
        deltaTime = currentFrame-lastFrame;
        lastFrame = currentFrame;
        if (deltaTime > .1f) deltaTime = .1f;
        glfwPollEvents();
        DoMovement();
        Animation();
        DibujarPantalla(window);
        glfwSwapBuffers(window);
    }
    glDeleteVertexArrays(1,&VAO_Cubo);
    glDeleteVertexArrays(1,&VAO_Plano);
    glDeleteBuffers(1,&VBO_Cubo);
    glDeleteBuffers(1,&VBO_Plano);
    glfwTerminate();
    return 0;
}
// ANIMACION: avanza los tiempos del saque y festejo, respetando la pausa.
void Animation() {
    if (saqueActivo && !pausaSaque) {
        tiempoSaque += deltaTime;
        if (tiempoSaque >= cuadros[MAX_FRAMES-1].tiempo) {
            tiempoSaque = cuadros[MAX_FRAMES-1].tiempo;
            saqueActivo = false;
        }
    }
    if (festejoActivo && !pausaFestejo) {
        tiempoFestejo += deltaTime;
        if (tiempoFestejo >= 2) { tiempoFestejo = 2;
            festejoActivo = false;
        }
    }
}
Pose PoseInicial() { return cuadros[0].pose;
}
// SAQUE: interpola brazo, rodilla, salto y balon entre dos poses.
Pose PoseSaque() {
    if (tiempoSaque <= 0) return cuadros[0].pose;
    if (tiempoSaque >= cuadros[MAX_FRAMES-1].tiempo) return cuadros[MAX_FRAMES-1].pose;
    Pose resultado = cuadros[0].pose;
    for (int i=1; i<MAX_FRAMES; i++) {
        if (tiempoSaque <= cuadros[i].tiempo) {
            GLfloat t = (tiempoSaque-cuadros[i-1].tiempo)/(cuadros[i].tiempo-cuadros[i-1].tiempo);
            resultado.arm = cuadros[i-1].pose.arm + (cuadros[i].pose.arm-cuadros[i-1].pose.arm)*t;
            resultado.knee = cuadros[i-1].pose.knee + (cuadros[i].pose.knee-cuadros[i-1].pose.knee)*t;
            resultado.rise = cuadros[i-1].pose.rise + (cuadros[i].pose.rise-cuadros[i-1].pose.rise)*t;
            resultado.ball = cuadros[i-1].pose.ball + (cuadros[i].pose.ball-cuadros[i-1].pose.ball)*t;
            return resultado;
        }
    }
    return resultado;
}
// [ESCALA] El trofeo crece durante un segundo y vuelve a su tamano.
float EscalaTrofeo() {
    GLfloat avance = tiempoFestejo;
    if (tiempoFestejo > 1) avance = 2-tiempoFestejo;
    if (avance < 0) avance = 0;
    if (avance > 1) avance = 1;
    return .65f+.35f*avance;
}
// REINICIO: restaura camara, animaciones, puntos y reflectores.
void Reiniciar() {
    camera = Camera(glm::vec3(12,1.7f,7),glm::vec3(0,1,0),-145.0f,-5.0f);
    tiempoSaque = 0;
    tiempoFestejo = 0;
    saqueActivo = false;
    pausaSaque = false;
    festejoActivo = false;
    pausaFestejo = false;
    puntosLocal = 0;
    puntosVisita = 0;
    reflectores = true;
}
// TRANSFORMACIONES: combina posicion y tamano en la matriz del objeto.
glm::mat4 transform(glm::vec3 p,glm::vec3 size) {
    glm::mat4 model(1);
    model = glm::translate(model,p);
    model = glm::scale(model,size);
    return model;
}
void DibujarNumero(int number, float x, float y, float z) {
    // Cambiar la imagen mantiene el marcador interactivo sin modelar sus segmentos.
    DibujarPlano(transform({x,y,z},{.64f,1.25f,1}),white,digitos[number%10]);
}
// JERARQUIA: p coloca al jugador; yaw lo gira; jersey cambia su uniforme.
void DibujarJugador(glm::vec3 p, float yaw, glm::vec3 jersey, const Pose& pose) {
    // Jerarquía: raíz -> torso -> hombro -> brazo -> antebrazo.
    glm::mat4 root = glm::rotate(glm::translate(glm::mat4(1),p+glm::vec3(0,pose.rise,0)),glm::radians(yaw),{0,1,0});
    DibujarParte(root,{0,1.24f,0},{.52f,.64f,.3f},jersey,tela);
    DibujarParte(root,{0,.86f,0},{.50f,.18f,.3f},navy,tela);
    DibujarModelo(2,root*transform({0,1.79f,0},{.20f,.23f,.19f}),skin);
    DibujarParte(root,{0,1.97f,-.01f},{.32f,.08f,.28f},navy,tela);
    int posiciones0[] = {-1,1};
    for (int indice0=0; indice0<2; indice0++) {
        int side = posiciones0[indice0];
        glm::mat4 shoulder = glm::translate(root,{float(side)*.32f,1.49f,0});
        GLfloat anguloBrazo = pose.arm;
        if (side == -1) anguloBrazo = pose.arm*.60f;
        shoulder = glm::rotate(shoulder,glm::radians(anguloBrazo),{0,0,1});
        DibujarParte(shoulder,{0,-.19f,0},{.16f,.38f,.18f},jersey,tela);
        glm::mat4 elbow = glm::rotate(glm::translate(shoulder,{0,-.38f,0}),glm::radians(-15.f),{1,0,0});
        DibujarParte(elbow,{0,-.18f,0},{.13f,.36f,.14f},skin);
        glm::mat4 hip = glm::rotate(glm::translate(root,{float(side)*.15f,.82f,0}),glm::radians(pose.knee),{0,0,1});
        DibujarParte(hip,{0,-.20f,0},{.19f,.40f,.21f},navy,tela);
        glm::mat4 knee = glm::rotate(glm::translate(hip,{0,-.40f,0}),glm::radians(-pose.knee),{0,0,1});
        DibujarParte(knee,{0,-.16f,0},{.14f,.32f,.15f},skin);
        DibujarParte(knee,{.06f,-.33f,.03f},{.25f,.10f,.25f},white,goma);
    }
}
// [CANCHA] DibujarCubo(centro XYZ, tamano XYZ, color RGB, textura, repeticiones UV).
// Ejemplo: {18,.015f,9} define largo, grosor y ancho del piso de juego.
void DibujarCancha() {
    // 1 unidad de escena representa 1 metro. X: largo; Z: ancho; Y: altura.
    DibujarCubo({0,-.20f,0},{39,.35f,27},{.35f,.39f,.43f},concreto,{19.5f,13.5f});
    DibujarCubo({0,-.01f,0},{24,.10f,15},{.94f,.65f,.36f},madera,{8,5});
    DibujarCubo({0,.048f,0},{18,.015f,9},{.19f,.62f,.70f},madera,{6,3});
    // Límites de 5 cm, dentro de la cancha. Línea central y ataque a 3 m.
    float posiciones1[] = {-8.975f,8.975f};
    for (int indice1=0; indice1<2; indice1++) {
        float x = posiciones1[indice1];
        DibujarCubo({x,.065f,0},{.05f,.025f,9},white);
    }
    float posiciones2[] = {-4.475f,4.475f};
    for (int indice2=0; indice2<2; indice2++) {
        float z = posiciones2[indice2];
        DibujarCubo({0,.065f,z},{18,.025f,.05f},white);
    }
    float posiciones3[] = {-3.f,0.f,3.f};
    for (int indice3=0; indice3<3; indice3++) {
        float x = posiciones3[indice3];
        DibujarCubo({x,.065f,0},{.05f,.025f,9},white);
    }
    GLfloat marcasX[] = {-9.10f,9.10f};
    GLfloat marcasZ[] = {-4.5f,4.5f};
    for (int i=0;i<2;i++) {
        for (int j=0;j<2;j++) DibujarCubo({marcasX[i],.065f,marcasZ[j]},{.15f,.025f,.05f},white);
    }
    // Se mide la red desde la superficie de la cancha, no desde y = 0.
    const float courtFloor = .048f + .015f / 2.f;
    // [RED] 2.43f es la altura desde el piso; netBottom resta su metro de alto.
    const float netTop = courtFloor + 2.43f;
    const float netBottom = netTop - 1.f;
    DibujarPostes();
    DibujarCubo({0,netTop-.035f,0},{.035f,.07f,9.8f},white);
    DibujarCubo({0,netBottom+.025f,0},{.025f,.05f,9.8f},white);
    // Dos triangulos en el plano YZ. La imagen se repite cada 10 cm.
    glm::mat4 netModel = glm::translate(glm::mat4(1),{0,netBottom+.5f,0});
    netModel = glm::rotate(netModel,glm::radians(90.f),{0,1,0});
    DibujarPlano(glm::scale(netModel,{9.8f,1.f,1.f}),white,red,{98,10});
    float posiciones6[] = {-4.5f,4.5f};
    for (int indice6=0; indice6<2; indice6++) {
        float z = posiciones6[indice6];
        DibujarCubo({0,netBottom+.5f,z},{.035f,1.f,.05f},white);
        for (int i = 0; i < 8; ++i) {
            glm::vec3 color = orange;
            if (i%2 == 1) color = white;
            DibujarCubo({0,netTop+.05f+float(i)*.1f,z},{.025f,.10f,.025f},color);
        }
    }
}
// [BANCAS_Y_TROFEO] Posiciones de bancas, calentamiento, trofeo y arbitro.
void DibujarBancas() {
    float trophyScale = EscalaTrofeo();
    // Zona 2: bancas y calentamiento al lado opuesto de las gradas.
    DibujarCubo({0,.015f,-9.7f},{25,.12f,4},white,concreto,{12.5f,2});
    float posiciones7[] = {-6.f,6.f};
    for (int indice7=0; indice7<2; indice7++) {
        float x = posiciones7[indice7];
        DibujarCubo({x,.54f,-8.8f},{4.8f,.18f,.65f},white,maderaBanca,{4,1});
        DibujarCubo({x,.98f,-9.05f},{4.8f,.65f,.13f},teal,maderaBanca,{4,1});
        float posiciones8[] = {-1.8f,1.8f};
        for (int indice8=0; indice8<2; indice8++) {
            float dx = posiciones8[indice8];
            DibujarCubo({x+dx,.26f,-8.8f},{.12f,.5f,.5f},steel,metal);
        }
        DibujarCubo({x,.08f,-10.3f},{3,.035f,2.7f},teal,goma,{3,3});
    }
    DibujarCubo({0,.48f,-9},{2,.85f,.80f},white,maderaBanca,{2,1});
    DibujarModelo(0,transform({0,.94f,-9},{trophyScale,trophyScale,trophyScale}),{.95f,.72f,.20f});
    // ARBITRO: reutiliza las partes del jugador sobre una plataforma.
    // La plataforma queda fuera de la zona libre (su borde mas cercano es z = -7.6).
    DibujarCubo({0,.90f,-8.05f},{1,.12f,.90f},steel,metal);
    GLfloat patasX[] = {-.40f,.40f};
    GLfloat patasZ[] = {-8.4f,-7.7f};
    for (int i=0;i<2;i++) {
        for (int j=0;j<2;j++) DibujarCubo({patasX[i],.48f,patasZ[j]},{.08f,.84f,.08f},steel,metal);
    }
    float posiciones11[] = {.25f,.50f,.75f};
    for (int indice11=0; indice11<3; indice11++) {
        float y = posiciones11[indice11];
        DibujarCubo({0,y,-8.45f},{.80f,.07f,.08f},steel,metal);
    }
    DibujarJugador({0,.96f,-8.05f},0,white,PoseInicial());
}
// [GRADAS] row<4: filas; seatIndex<30: asientos por fila; .9f: separacion.
void DibujarGradas() {
    // Zona 3: espectadores. Gradas por niveles y butacas repetidas con bucles.
    for (int row = 0; row < 4; ++row) {
        float z = 8.1f+row*.9f, top = .45f+row*.52f;
        DibujarCubo({0,top/2,z},{28,top,.95f},{.72f,.75f,.78f},concreto,{14,1});
        for (int seatIndex = 0; seatIndex < 30; ++seatIndex) {
            float x = -13.f+float(seatIndex)*.9f;
            glm::vec3 seatColor = teal;
            if ((seatIndex+row)%4 == 0) seatColor = orange;
            // El OBJ y la textura se cargan una vez; aquí solo cambia su posición y color.
            DibujarModelo(1,glm::translate(glm::mat4(1),{x,top+.02f,z}),seatColor);
        }
    }
}
// [LETREROS_Y_MARCADOR] Aqui se colocan pared, nombres de equipos y cuatro digitos.
void DibujarParedYMarcador() {
    int scoreA=puntosLocal, scoreB=puntosVisita;
    // Pared de fondo abierta hacia la cámara para inspeccionar las tres áreas.
    // La imagen JPG contiene los ladrillos; el muro es un solo cubo delgado.
    DibujarCubo({0,3.2f,-12.7f},{37,6.4f,.18f},white,pared,{37.f/1.28f,6.4f/1.28f});
    float posiciones12[] = {-15.f,-10.f,10.f,15.f};
    for (int indice12=0; indice12<4; indice12++) {
        float x = posiciones12[indice12];
        DibujarCubo({x,3.3f,-12.5f},{.25f,6.6f,.25f},steel,metal,{1,6});
    }
    // Delante de las columnas para que no tapen los nombres de los equipos.
    DibujarCubo({-10,3.8f,-12.30f},{6,1.4f,.08f},white,letreroVerde);
    DibujarCubo({10,3.8f,-12.30f},{6,1.4f,.08f},white,letreroNaranja);
    // Marcador de imagenes: interacción visible en el mundo y HUD.
    DibujarCubo({0,4.65f,-12.5f},{4.8f,2.1f,.25f},{.015f,.035f,.055f});
    DibujarNumero(scoreA/10,-1.65f,4.65f,-12.34f);
    DibujarNumero(scoreA%10,-.90f,4.65f,-12.34f);
    DibujarNumero(scoreB/10,.90f,4.65f,-12.34f);
    DibujarNumero(scoreB%10,1.65f,4.65f,-12.34f);
}
// [REFLECTORES] Dibujo de los postes; la luz real esta en lighting.frag.
// Si se mueven, ajustar tambien pointLightPositions en las variables.
void DibujarReflectores() {
    bool lights=reflectores;
    // Reflectores fuera de la zona libre y de las gradas. Las mismas posiciones están en el shader.
    GLfloat postesX[] = {-15,15};
    GLfloat postesZ[] = {-8,8};
    glm::vec3 color = steel;
    if (reflectores) color = glm::vec3(1,.92f,.70f);
    for (int i=0;i<2;i++) {
        for (int j=0;j<2;j++) {
            DibujarCubo({postesX[i],3.5f,postesZ[j]},{.12f,7,.12f},steel,metal,{1,7});
            DibujarCubo({postesX[i],7.05f,postesZ[j]},{1.5f,.25f,.72f},navy,metal);
            DibujarCubo({postesX[i],6.88f,postesZ[j]},{1.32f,.03f,.58f},color);
        }
    }
}
// [JUGADORES] Primero va el sacador; athletes guarda los XYZ de los otros once.
void DibujarJugadoresYBalon(const Pose& pose) {
    const float floor = .0555f;
    Pose idle = PoseInicial();
    DibujarJugador({-10.35f,.04f,0},0,teal,pose);
    const glm::vec3 athletes[] = {{-6,0,-2.5f},{-6,0,2.5f},{-2,0,-2.5f},{-2,0,2.5f},{-3.5f,0,0},
        {6,0,0},{6,0,-2.5f},{6,0,2.5f},{2,0,-2.5f},{2,0,2.5f},{3.5f,0,0}};
    for (int i = 0; i < 11; ++i) {
        GLfloat giro = 0;
        glm::vec3 color = teal;
        if (i >= 5) { giro=180;
            color=orange;
        }
        DibujarJugador(athletes[i]+glm::vec3(0,floor,0),giro,color,idle);
    }
    // [BALON] .105f es el radio; pose.ball es la posicion animada.
    DibujarBalon(pose.ball,.105f,white,balon);
}
void DibujarPostes() {
    const float courtFloor = .0555f;
    float posiciones15[] = {-5.1f,5.1f};
    for (int indice15=0; indice15<2; indice15++) {
        float z = posiciones15[indice15];
        DibujarCubo({0,courtFloor+1.3f,z},{.14f,2.6f,.14f},steel,metal,{1,3});
        DibujarCubo({0,courtFloor+.48f,z},{.28f,.92f,.28f},orange,goma,{1,2});
    }
}
// BUFFERS: posicion, normal y coordenadas UV; 8 valores por vertice.
void PrepararVAO(GLuint& vao,GLuint& vbo,GLfloat vertices[],GLsizeiptr bytes) {
    glGenVertexArrays(1,&vao);
    glGenBuffers(1,&vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,bytes,vertices,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(GLfloat),(GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(GLfloat),(GLvoid*)(3*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,8*sizeof(GLfloat),(GLvoid*)(6*sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}
// GEOMETRIA: vertices del cubo y del plano usados para construir el escenario.
void PrepararPrimitivas() {
    GLfloat verticesCubo[] = {
        -0.500f,-0.500f,0.500f,0.000f,0.000f,1.000f,0.000f,1.000f,
        0.500f,-0.500f,0.500f,0.000f,0.000f,1.000f,1.000f,1.000f,
        0.500f,0.500f,0.500f,0.000f,0.000f,1.000f,1.000f,0.000f,
        -0.500f,-0.500f,0.500f,0.000f,0.000f,1.000f,0.000f,1.000f,
        0.500f,0.500f,0.500f,0.000f,0.000f,1.000f,1.000f,0.000f,
        -0.500f,0.500f,0.500f,0.000f,0.000f,1.000f,0.000f,0.000f,
        0.500f,-0.500f,-0.500f,0.000f,0.000f,-1.000f,0.000f,1.000f,
        -0.500f,-0.500f,-0.500f,0.000f,0.000f,-1.000f,1.000f,1.000f,
        -0.500f,0.500f,-0.500f,0.000f,0.000f,-1.000f,1.000f,0.000f,
        0.500f,-0.500f,-0.500f,0.000f,0.000f,-1.000f,0.000f,1.000f,
        -0.500f,0.500f,-0.500f,0.000f,0.000f,-1.000f,1.000f,0.000f,
        0.500f,0.500f,-0.500f,0.000f,0.000f,-1.000f,0.000f,0.000f,
        -0.500f,-0.500f,-0.500f,-1.000f,0.000f,0.000f,0.000f,1.000f,
        -0.500f,-0.500f,0.500f,-1.000f,0.000f,0.000f,1.000f,1.000f,
        -0.500f,0.500f,0.500f,-1.000f,0.000f,0.000f,1.000f,0.000f,
        -0.500f,-0.500f,-0.500f,-1.000f,0.000f,0.000f,0.000f,1.000f,
        -0.500f,0.500f,0.500f,-1.000f,0.000f,0.000f,1.000f,0.000f,
        -0.500f,0.500f,-0.500f,-1.000f,0.000f,0.000f,0.000f,0.000f,
        0.500f,-0.500f,0.500f,1.000f,0.000f,0.000f,0.000f,1.000f,
        0.500f,-0.500f,-0.500f,1.000f,0.000f,0.000f,1.000f,1.000f,
        0.500f,0.500f,-0.500f,1.000f,0.000f,0.000f,1.000f,0.000f,
        0.500f,-0.500f,0.500f,1.000f,0.000f,0.000f,0.000f,1.000f,
        0.500f,0.500f,-0.500f,1.000f,0.000f,0.000f,1.000f,0.000f,
        0.500f,0.500f,0.500f,1.000f,0.000f,0.000f,0.000f,0.000f,
        -0.500f,0.500f,-0.500f,0.000f,1.000f,0.000f,0.000f,1.000f,
        -0.500f,0.500f,0.500f,0.000f,1.000f,0.000f,1.000f,1.000f,
        0.500f,0.500f,0.500f,0.000f,1.000f,0.000f,1.000f,0.000f,
        -0.500f,0.500f,-0.500f,0.000f,1.000f,0.000f,0.000f,1.000f,
        0.500f,0.500f,0.500f,0.000f,1.000f,0.000f,1.000f,0.000f,
        0.500f,0.500f,-0.500f,0.000f,1.000f,0.000f,0.000f,0.000f,
        -0.500f,-0.500f,-0.500f,0.000f,-1.000f,0.000f,0.000f,1.000f,
        0.500f,-0.500f,-0.500f,0.000f,-1.000f,0.000f,1.000f,1.000f,
        0.500f,-0.500f,0.500f,0.000f,-1.000f,0.000f,1.000f,0.000f,
        -0.500f,-0.500f,-0.500f,0.000f,-1.000f,0.000f,0.000f,1.000f,
        0.500f,-0.500f,0.500f,0.000f,-1.000f,0.000f,1.000f,0.000f,
        -0.500f,-0.500f,0.500f,0.000f,-1.000f,0.000f,0.000f,0.000f
    };
    GLfloat verticesPlano[] = {
        -.5f,-.5f,0, 0,0,1, 0,1,  .5f,-.5f,0, 0,0,1, 1,1,
        .5f,.5f,0, 0,0,1, 1,0, -.5f,-.5f,0, 0,0,1, 0,1,
        .5f,.5f,0, 0,0,1, 1,0, -.5f,.5f,0, 0,0,1, 0,0
    };
    PrepararVAO(VAO_Cubo,VBO_Cubo,verticesCubo,sizeof(verticesCubo));
    PrepararVAO(VAO_Plano,VBO_Plano,verticesPlano,sizeof(verticesPlano));
}
// CARGA DE TEXTURA: lee la imagen, la envia a OpenGL y configura filtros.
GLuint CargarTextura(string ruta) {
    GLuint id;
    glGenTextures(1,&id);
    int width,height;
    unsigned char* image = SOIL_load_image(ruta.c_str(),&width,&height,0,SOIL_LOAD_RGBA);
    if (image == NULL) { cout << "No se pudo cargar " << ruta << endl;
        return 0;
    }
    glBindTexture(GL_TEXTURE_2D,id);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,image);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    SOIL_free_image_data(image);
    texturasCargadas++;
    return id;
}
// RUTAS: cambiar aqui las imagenes de materiales, letreros y menus.
void CargarTexturas() {
    texturaBlanca = CargarTextura("configbase/Textures/blanco.png");
    madera = CargarTextura("configbase/Textures/duela.jpg");
    balon = CargarTextura("configbase/Textures/balon.png");
    pared = CargarTextura("configbase/Textures/pared_ladrillo.jpg");
    concreto = CargarTextura("configbase/Textures/concreto.jpg");
    metal = CargarTextura("configbase/Textures/metal.jpg");
    tela = CargarTextura("configbase/Textures/tela.jpg");
    goma = CargarTextura("configbase/Textures/goma.jpg");
    maderaBanca = CargarTextura("configbase/Textures/madera_banca.jpg");
    red = CargarTextura("configbase/Textures/red.png");
    sombra = CargarTextura("configbase/Textures/sombra.png");
    letreroVerde = CargarTextura("configbase/Textures/equipo_verde.png");
    letreroNaranja = CargarTextura("configbase/Textures/equipo_naranja.png");
    for (int i=0;i<10;i++) {
        stringstream numero;
        numero << i;
        digitos[i] = CargarTextura("configbase/Textures/marcador_"+numero.str()+".png");
        digitosArial[i] = CargarTextura("configbase/Textures/Interfaz/digito_"+numero.str()+".png");
    }
    menuControles = CargarTextura("configbase/Textures/Interfaz/controles.png");
    titulo = CargarTextura("configbase/Textures/Interfaz/titulo.png");
    marcador = CargarTextura("configbase/Textures/Interfaz/marcador.png");
    menuEstado = CargarTextura("configbase/Textures/Interfaz/estado.png");
    string nombres[] = {"orbital","primera","encendidos","apagados","listo","curso","pausado","finalizado"};
    for (int i=0;i<8;i++) estados[i] = CargarTextura("configbase/Textures/Interfaz/"+nombres[i]+".png");
}
// ILUMINACION: luz direccional y cuatro reflectores; aqui se ajusta su intensidad.
void ConfigurarLuces() {
    lightingShader.Use();
    GLuint program = lightingShader.Program;
    glUniform3f(glGetUniformLocation(program,"viewPos"),camera.GetPosition().x,camera.GetPosition().y,camera.GetPosition().z);
    glUniform3f(glGetUniformLocation(program,"dirLight.direction"),.3f,-1,-.4f);
    glUniform3f(glGetUniformLocation(program,"dirLight.ambient"),.24f,.24f,.24f);
    glUniform3f(glGetUniformLocation(program,"dirLight.diffuse"),.4f,.4f,.4f);
    glUniform3f(glGetUniformLocation(program,"dirLight.specular"),0,0,0);
    GLfloat intensidad = 0;
    if (reflectores) intensidad = .60f;
    for (int i=0;i<4;i++) {
        stringstream indice;
        indice << i;
        string nombre = "spotLights["+indice.str()+"]";
        glUniform3f(glGetUniformLocation(program,(nombre+".position").c_str()),pointLightPositions[i].x,7,pointLightPositions[i].z);
        glUniform3f(glGetUniformLocation(program,(nombre+".direction").c_str()),-pointLightPositions[i].x,-7,-pointLightPositions[i].z);
        glUniform3f(glGetUniformLocation(program,(nombre+".ambient").c_str()),0,0,0);
        glUniform3f(glGetUniformLocation(program,(nombre+".diffuse").c_str()),intensidad,intensidad,intensidad);
        glUniform3f(glGetUniformLocation(program,(nombre+".specular").c_str()),intensidad*.26f,intensidad*.26f,intensidad*.26f);
        glUniform1f(glGetUniformLocation(program,(nombre+".constant").c_str()),1);
        glUniform1f(glGetUniformLocation(program,(nombre+".linear").c_str()),0);
        glUniform1f(glGetUniformLocation(program,(nombre+".quadratic").c_str()),.006f);
        glUniform1f(glGetUniformLocation(program,(nombre+".cutOff").c_str()),.87f);
        glUniform1f(glGetUniformLocation(program,(nombre+".outerCutOff").c_str()),.68f);
    }
    glUniform1i(glGetUniformLocation(program,"material.diffuse"),0);
    glUniform1i(glGetUniformLocation(program,"material.specular"),1);
    glUniform1f(glGetUniformLocation(program,"material.shininess"),32);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D,texturaBlanca);
    glActiveTexture(GL_TEXTURE0);
}
// DIBUJO: envia matriz, color, textura y repeticion UV antes de dibujar.
void DibujarMalla(GLuint vao,int cantidad,const glm::mat4& model,glm::vec3 color,GLuint textura,glm::vec2 repeat) {
    glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program,"model"),1,GL_FALSE,glm::value_ptr(model));
    glUniform3f(glGetUniformLocation(lightingShader.Program,"objectColor"),color.x,color.y,color.z);
    glUniform3f(glGetUniformLocation(lightingShader.Program,"uvRepeat"),repeat.x,repeat.y,0);
    if (textura == 0) textura = texturaBlanca;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,textura);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES,0,cantidad);
    glBindVertexArray(0);
}
void DibujarCubo(glm::vec3 p,glm::vec3 size,glm::vec3 color,GLuint textura,glm::vec2 repeat) {
    DibujarMalla(VAO_Cubo,36,transform(p,size),color,textura,repeat);
}
void DibujarPlano(const glm::mat4& model,glm::vec3 color,GLuint textura,glm::vec2 repeat) {
    DibujarMalla(VAO_Plano,6,model,color,textura,repeat);
}
// OBJ: numero selecciona 0=trofeo, 1=butaca, 2=esfera.
void DibujarModelo(int numero,const glm::mat4& model,glm::vec3 color,GLuint textura) {
    glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program,"model"),1,GL_FALSE,glm::value_ptr(model));
    glUniform3f(glGetUniformLocation(lightingShader.Program,"uvRepeat"),1,1,0);
    glActiveTexture(GL_TEXTURE0);
    if (textura == 0) textura = texturaBlanca;
    modelos[numero].Draw(lightingShader,color,textura);
}
void DibujarBalon(glm::vec3 p,float radio,glm::vec3 color,GLuint textura) {
    DibujarModelo(2,transform(p,glm::vec3(radio)),color,textura);
}
// PADRE-HIJO: la parte hereda el movimiento y giro de su matriz padre.
void DibujarParte(const glm::mat4& padre,glm::vec3 p,glm::vec3 size,glm::vec3 color,GLuint textura) {
    DibujarMalla(VAO_Cubo,36,padre*transform(p,size),color,textura,glm::vec2(1));
}
// SOMBRA SIMULADA: coloca, gira y escala una imagen transparente sobre el piso.
void SombraEn(glm::vec3 p,GLfloat size) {
    glm::mat4 model = glm::translate(glm::mat4(1),glm::vec3(p.x+.25f,.081f,p.z+.25f));
    model = glm::rotate(model,glm::radians(-90.0f),glm::vec3(1,0,0));
    model = glm::scale(model,glm::vec3(size,size,1));
    DibujarPlano(model,glm::vec3(1),sombra);
}
// TRANSPARENCIA: mezcla las sombras de jugadores y balon con el piso.
void DibujarSombras(const Pose& pose) {
    glUniform1i(glGetUniformLocation(lightingShader.Program,"sinLuz"),1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    SombraEn(glm::vec3(-10.35f,0,0),1.5f);
    glm::vec3 posiciones[] = {glm::vec3(-6,0,-2.5f),glm::vec3(-6,0,2.5f),glm::vec3(-2,0,-2.5f),glm::vec3(-2,0,2.5f),glm::vec3(-3.5f,0,0),glm::vec3(6,0,0),glm::vec3(6,0,-2.5f),glm::vec3(6,0,2.5f),glm::vec3(2,0,-2.5f),glm::vec3(2,0,2.5f),glm::vec3(3.5f,0,0)};
    for (int i=0;i<11;i++) SombraEn(posiciones[i],1.5f);
    SombraEn(pose.ball,.45f+pose.ball.y*.04f);
    glDisable(GL_BLEND);
    glUniform1i(glGetUniformLocation(lightingShader.Program,"sinLuz"),0);
}
// IMAGEN 2D: posicion y tamano del panel en pixeles.
void ImagenPantalla(GLuint imagen,GLfloat x,GLfloat y,GLfloat width,GLfloat height) {
    DibujarPlano(transform(glm::vec3(x+width/2,y+height/2,0),glm::vec3(width,-height,1)),glm::vec3(1),imagen);
}
// ESTADO: elige la imagen de listo, en curso, pausado o finalizado.
int EstadoAnimacion(bool activo,bool pausa,GLfloat tiempo) {
    if (pausa) return 6;
    if (activo) return 5;
    if (tiempo > 0) return 7;
    return 4;
}
// Los paneles son imagenes Arial; I los muestra u oculta.
void DibujarInterfaz(GLfloat width,GLfloat height) {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    // Proyeccion ortografica: coloca los menus en 2D sin perspectiva.
    glm::mat4 view(1),projection=glm::ortho(0.0f,(GLfloat)width,(GLfloat)height,0.0f,-1.0f,1.0f);
    glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program,"view"),1,GL_FALSE,glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program,"projection"),1,GL_FALSE,glm::value_ptr(projection));
    glUniform1i(glGetUniformLocation(lightingShader.Program,"sinLuz"),1);
    DibujarCubo(glm::vec3(width/2.0f,38,0),glm::vec3(width,76,1),glm::vec3(.025f,.055f,.1f));
    ImagenPantalla(titulo,12,10,210,52);
    ImagenPantalla(marcador,width-330,10,310,52);
    ImagenPantalla(digitosArial[puntosLocal/10],width-233,20,22,36);
    ImagenPantalla(digitosArial[puntosLocal%10],width-211,20,22,36);
    ImagenPantalla(digitosArial[puntosVisita/10],width-155,20,22,36);
    ImagenPantalla(digitosArial[puntosVisita%10],width-133,20,22,36);
    if (mostrarPaneles) {
        ImagenPantalla(menuControles,12,height-322,420,310);
        ImagenPantalla(menuEstado,width-426,height-124,414,112);
        int camara=0,luces=2;
        if (!camera.orbital) camara=1;
        if (!reflectores) luces=3;
        ImagenPantalla(estados[camara],width-267,height-116,225,25);
        ImagenPantalla(estados[luces],width-267,height-91,225,25);
        ImagenPantalla(estados[EstadoAnimacion(saqueActivo,pausaSaque,tiempoSaque)],width-267,height-66,225,25);
        ImagenPantalla(estados[EstadoAnimacion(festejoActivo,pausaFestejo,tiempoFestejo)],width-267,height-41,225,25);
    }
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}
// ESCENA: configura camara y perspectiva; dibuja objetos y despues la interfaz.
void DibujarPantalla(GLFWwindow* window) {
    int width,height;
    glfwGetFramebufferSize(window,&width,&height);
    if (width<=0 || height<=0) return;
    glViewport(0,0,width,height);
    glClearColor(.1f,.14f,.2f,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    lightingShader.Use();
    ConfigurarLuces();
    glm::mat4 view=camera.GetViewMatrix();
    glm::mat4 projection=glm::perspective(glm::radians(48.0f),(GLfloat)width/height,.1f,120.0f);
    glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program,"view"),1,GL_FALSE,glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program,"projection"),1,GL_FALSE,glm::value_ptr(projection));
    glUniform1i(glGetUniformLocation(lightingShader.Program,"sinLuz"),0);
    Pose pose=PoseSaque();
    DibujarCancha();
    if (reflectores) DibujarSombras(pose);
    DibujarBancas();
    DibujarGradas();
    DibujarParedYMarcador();
    DibujarReflectores();
    DibujarJugadoresYBalon(pose);
    DibujarInterfaz((GLfloat)width,(GLfloat)height);
}
// TECLADO: registra pulsaciones y activa camaras, animaciones, luces y marcador.
void KeyCallback(GLFWwindow* window,int key,int scancode,int action,int mode) {
    if (key>=0 && key<1024) {
        if (action==GLFW_PRESS) keys[key]=true;
        if (action==GLFW_RELEASE) keys[key]=false;
    }
    if (action!=GLFW_PRESS) return;
    switch (key) {
        case GLFW_KEY_ESCAPE:glfwSetWindowShouldClose(window,GL_TRUE);
        break;
        case GLFW_KEY_C:camera.orbital=!camera.orbital;
        break;
        case GLFW_KEY_1:camera.orbital=false;
        break;
        case GLFW_KEY_2:camera.orbital=true;
        break;
        // Espacio: inicia el saque. F: enfoca el trofeo y activa su cambio de escala.
        case GLFW_KEY_SPACE:tiempoSaque=0;
        saqueActivo=true;
        pausaSaque=false;
        break;
        case GLFW_KEY_F:camera.orbital=true;
        camera.target=glm::vec3(0,1.8f,-9);
        camera.distance=8;
        camera.orbitYaw=20;
        camera.orbitPitch=25;
        tiempoFestejo=0;
        festejoActivo=true;
        pausaFestejo=false;
        break;
        // P pausa; L cambia luces; I muestra paneles; J/K suman puntos; R reinicia.
        case GLFW_KEY_P:
        if (saqueActivo) pausaSaque=!pausaSaque;
        if (festejoActivo) pausaFestejo=!pausaFestejo;
        break;
        case GLFW_KEY_L:reflectores=!reflectores;
        break;
        case GLFW_KEY_I:mostrarPaneles=!mostrarPaneles;
        break;
        case GLFW_KEY_J:puntosLocal=(puntosLocal+1)%100;
        break;
        case GLFW_KEY_K:puntosVisita=(puntosVisita+1)%100;
        break;
        case GLFW_KEY_R:Reiniciar();
        break;
    }
}
// MOVIMIENTO CONTINUO: WASD/flechas desplazan o giran; E/Q cambian altura.
void DoMovement() {
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP]) camera.ProcessKeyboard(FORWARD,deltaTime);
    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN]) camera.ProcessKeyboard(BACKWARD,deltaTime);
    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT]) camera.ProcessKeyboard(LEFT,deltaTime);
    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) camera.ProcessKeyboard(RIGHT,deltaTime);
    if (keys[GLFW_KEY_E]) camera.SetHeight(UP,deltaTime);
    if (keys[GLFW_KEY_Q]) camera.SetHeight(DOWN,deltaTime);
}
// MOUSE: calcula cuanto se movio el cursor para orientar la camara.
void MouseCallback(GLFWwindow* window,double xpos,double ypos) {
    if (firstMouse) {lastX=(GLfloat)xpos;
        lastY=(GLfloat)ypos;
        firstMouse=false;
    }
    GLfloat xOffset=(GLfloat)xpos-lastX;
    GLfloat yOffset=lastY-(GLfloat)ypos;
    lastX=(GLfloat)xpos;
    lastY=(GLfloat)ypos;
    if (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS || glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT)==GLFW_PRESS) // Mira solo al arrastrar el mouse.
    camera.ProcessMouseMovement(xOffset,yOffset);
}
// RUEDA: cambia la distancia de la camara orbital.
void ScrollCallback(GLFWwindow* window,double xOffset,double yOffset) { camera.ProcessMouseScroll((GLfloat)yOffset);
}
