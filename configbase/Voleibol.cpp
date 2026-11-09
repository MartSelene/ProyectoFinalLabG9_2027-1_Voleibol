#include <windows.h>
#include "Graficos.h"
#include "Camera.h"
#include "Model.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <array>
#include <filesystem>
#include <iostream>

// 1. VARIABLES DEL PROGRAMA
Camera camera;
bool keys[1024] = {};
// [ESTADO_INICIAL] true/false decide si luces y paneles aparecen al abrir.
bool reflectores=true, mostrarPaneles=false, pedirCaptura=false;
int puntosLocal=0, puntosVisita=0, numeroCaptura=0;
double lastX=0, lastY=0;
bool firstMouse=true;

// Pose: angulos en grados, elevacion y posicion XYZ del balon en metros.
struct Pose {
    float arm=8, knee=0, rise=0;
    glm::vec3 ball={-10.2f,1.45f,0};
};
struct CuadroClave { float tiempo; Pose pose; };
// [KEYFRAMES] Cada fila: {segundos, {brazo, rodilla, elevacion, {balon X,Y,Z}}}.
// Para cambiar la trayectoria editar esos XYZ; para el gesto, brazo y rodilla.
const std::array<CuadroClave,8> cuadros = {{
    {0.00f,{8,0,0,{-10.2f,1.45f,0}}},
    {0.50f,{-75,18,0,{-10.0f,2.65f,0}}},
    {0.90f,{-145,28,.15f,{-9.85f,3.4f,0}}},
    {1.15f,{-95,5,.25f,{-9.7f,3.05f,0}}},
    {1.70f,{25,0,0,{-5.0f,4.6f,0}}},
    {2.30f,{8,0,0,{.6f,4.1f,0}}},
    {3.00f,{8,0,0,{6.2f,1.1f,0}}},
    {3.40f,{8,0,0,{7.4f,.32f,0}}}
}};
float tiempoSaque=0, tiempoFestejo=0;
bool saqueActivo=false, pausaSaque=false, festejoActivo=false, pausaFestejo=false;
// [COLORES] RGB de 0 a 1: teal es verde y orange es naranja. La textura multiplica este color.
const glm::vec3 navy(.055f,.13f,.21f), teal(.07f,.65f,.68f), orange(.96f,.42f,.16f);
const glm::vec3 white(.93f,.96f,.98f), steel(.25f,.30f,.38f), skin(.77f,.52f,.34f);

// Prototipos, como en las practicas.
void KeyCallback(GLFWwindow*,int,int,int,int);
void MouseCallback(GLFWwindow*,double,double);
void ScrollCallback(GLFWwindow*,double,double);
void DoMovement(float dt);
void Animation(float dt);
void Reiniciar();
Pose PoseSaque();
float EscalaTrofeo();
void DibujarPantalla(GLFWwindow*,Graficos&,Model&,Model&);
void DibujarCancha(Graficos&);
void DibujarBancas(Graficos&,Model&);
void DibujarGradas(Graficos&,Model&);
void DibujarParedYMarcador(Graficos&);
void DibujarReflectores(Graficos&);
void DibujarJugador(Graficos&,glm::vec3,float,glm::vec3,const Pose&);
void DibujarJugadoresYBalon(Graficos&,const Pose&);
void DibujarPostes(Graficos&);
void DibujarSombras(Graficos&,const Pose&);
void GuardarCaptura(int,int);

// 2. MAIN: configurar OpenGL, cargar recursos y repetir actualizar/dibujar.
int main() {
    // El EXE esta junto a la solucion; los recursos estan en configbase.
    wchar_t ruta[MAX_PATH]={};
    GetModuleFileNameW(nullptr,ruta,MAX_PATH);
    std::filesystem::current_path(std::filesystem::path(ruta).parent_path()/"configbase");
    if(!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES,4);
    glfwWindowHint(GLFW_STENCIL_BITS,8);
    // [VENTANA] Primeros valores: ancho y alto; el texto es el titulo.
    GLFWwindow* window=glfwCreateWindow(1280,800,"Voleibol",nullptr,nullptr);
    if(!window) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    glewExperimental=GL_TRUE;
    if(glewInit()!=GLEW_OK) { glfwDestroyWindow(window); glfwTerminate(); return 1; }
    while(glGetError()!=GL_NO_ERROR) {} // Consulta inicial de GLEW en perfil core.
    glfwSwapInterval(1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glfwSetKeyCallback(window,KeyCallback);
    glfwSetCursorPosCallback(window,MouseCallback);
    glfwSetScrollCallback(window,ScrollCallback);
    int resultado=0;
    try {
        Graficos graficos;
        // [P05][MODELOS] Cambiar aqui la ruta para sustituir un OBJ; conservar su MTL e imagenes.
        Model trofeo(graficos,"Models/trofeo.obj");
        Model butaca(graficos,"Models/butaca/butaca.obj");
        std::cout << "I: controles. J/K: puntos. Espacio: saque. Esc: salir.\n";
        double anterior=glfwGetTime();
        while(!glfwWindowShouldClose(window)) {
            double ahora=glfwGetTime();
            // dt: segundos del cuadro; se usa para que el movimiento dependa del tiempo.
            float dt=std::min(float(ahora-anterior),.1f);
            anterior=ahora;
            glfwPollEvents();
            DoMovement(dt);
            Animation(dt);
            DibujarPantalla(window,graficos,trofeo,butaca);
            glfwSwapBuffers(window);
        }
    } catch(const std::exception& error) {
        std::cerr << error.what() << '\n';
        resultado=1;
    }
    // Los objetos liberan sus mallas antes de destruir la ventana.
    glfwDestroyWindow(window);
    glfwTerminate();
    return resultado;
}

// 3. ANIMACION: variables que cambian en cada ciclo (idea de P09).
// [P09][ANIMACION] Aqui avanza el tiempo; 3.4 s de saque y 2 s de festejo.
// Si se cambia el ultimo keyframe, ajustar tambien 3.4f aqui y en PoseSaque.
void Animation(float dt) {
    if(saqueActivo && !pausaSaque) {
        tiempoSaque=std::min(tiempoSaque+dt,3.4f);
        if(tiempoSaque>=3.4f) saqueActivo=false;
    }
    if(festejoActivo && !pausaFestejo) {
        tiempoFestejo=std::min(tiempoFestejo+dt,2.f);
        if(tiempoFestejo>=2.f) festejoActivo=false;
    }
}
// [INTERPOLACION] Busca los dos cuadros vecinos y mezcla sus posiciones y angulos.
Pose PoseSaque() {
    if(tiempoSaque<=0) return cuadros.front().pose;
    if(tiempoSaque>=3.4f) return cuadros.back().pose;
    for(std::size_t i=1;i<cuadros.size();++i) {
        if(tiempoSaque<=cuadros[i].tiempo) {
            const auto& a=cuadros[i-1]; const auto& b=cuadros[i];
            float t=(tiempoSaque-a.tiempo)/(b.tiempo-a.tiempo);
            float suave=t*t*(3-2*t);
            return {glm::mix(a.pose.arm,b.pose.arm,suave),glm::mix(a.pose.knee,b.pose.knee,suave),
                    glm::mix(a.pose.rise,b.pose.rise,suave),glm::mix(a.pose.ball,b.pose.ball,t)};
        }
    }
    return cuadros.back().pose;
}
// [ESCALA] 0.65 es el tamano base; se suma hasta 0.35 y luego se resta.
float EscalaTrofeo() {
    float crecimiento=tiempoFestejo<=1.f?tiempoFestejo:2.f-tiempoFestejo;
    return .65f+.35f*std::clamp(crecimiento,0.f,1.f);
}
// [REINICIO] R restaura estos valores; la visibilidad de los paneles se conserva.
void Reiniciar() {
    camera.reset();
    tiempoSaque=tiempoFestejo=0;
    saqueActivo=pausaSaque=festejoActivo=pausaFestejo=false;
    puntosLocal=puntosVisita=0;
    reflectores=true;
}
// [P03][TRANSFORMACIONES] p mueve el objeto y size cambia su tamano en X,Y,Z.
glm::mat4 transform(glm::vec3 p,glm::vec3 size) {
    return glm::scale(glm::translate(glm::mat4(1),p),size);
}


// 4. ESCENARIO: cada funcion construye una parte del gimnasio.
void DibujarNumero(Graficos& r, int number, float x, float y, float z) {
    // Cambiar la imagen mantiene el marcador interactivo sin modelar sus segmentos.
    r.draw(r.quad,transform({x,y,z},{.64f,1.25f,1}),white,r.scoreDigits[number%10]);
}
// [P04][JERARQUIA] p coloca al jugador; yaw lo gira; jersey cambia el color de su uniforme.
void DibujarJugador(Graficos& r, glm::vec3 p, float yaw, glm::vec3 jersey, const Pose& pose) {
    // Jerarquía: raíz -> torso -> hombro -> brazo -> antebrazo.
    glm::mat4 root = glm::rotate(glm::translate(glm::mat4(1),p+glm::vec3(0,pose.rise,0)),glm::radians(yaw),{0,1,0});
    // Cada pieza recibe una matriz padre, un centro local y su tamano.
    auto part = [&](const glm::mat4& parent, glm::vec3 local, glm::vec3 size, glm::vec3 color, GLuint texture = 0) {
        r.draw(r.cube,parent*transform(local,size),color,texture);
    };
    part(root,{0,1.24f,0},{.52f,.64f,.3f},jersey,r.fabric);
    part(root,{0,.86f,0},{.50f,.18f,.3f},navy,r.fabric);
    r.draw(r.sphere,root*transform({0,1.79f,0},{.20f,.23f,.19f}),skin);
    part(root,{0,1.97f,-.01f},{.32f,.08f,.28f},navy,r.fabric);
    for (int side : {-1,1}) {
        glm::mat4 shoulder = glm::translate(root,{float(side)*.32f,1.49f,0});
        shoulder = glm::rotate(shoulder,glm::radians(side == -1 ? pose.arm*.60f : pose.arm),{0,0,1});
        part(shoulder,{0,-.19f,0},{.16f,.38f,.18f},jersey,r.fabric);
        glm::mat4 elbow = glm::rotate(glm::translate(shoulder,{0,-.38f,0}),glm::radians(-15.f),{1,0,0});
        part(elbow,{0,-.18f,0},{.13f,.36f,.14f},skin);
        glm::mat4 hip = glm::rotate(glm::translate(root,{float(side)*.15f,.82f,0}),glm::radians(pose.knee),{0,0,1});
        part(hip,{0,-.20f,0},{.19f,.40f,.21f},navy,r.fabric);
        glm::mat4 knee = glm::rotate(glm::translate(hip,{0,-.40f,0}),glm::radians(-pose.knee),{0,0,1});
        part(knee,{0,-.16f,0},{.14f,.32f,.15f},skin);
        part(knee,{.06f,-.33f,.03f},{.25f,.10f,.25f},white,r.rubber);
    }
}
// [CANCHA] r.box(centro XYZ, tamano XYZ, color RGB, textura, repeticiones UV).
// Ejemplo: {18,.015f,9} define largo, grosor y ancho del piso de juego.
void DibujarCancha(Graficos& r) {

    // 1 unidad de escena representa 1 metro. X: largo; Z: ancho; Y: altura.
    r.box({0,-.20f,0},{39,.35f,27},{.35f,.39f,.43f},r.concrete,{19.5f,13.5f});
    // Stencil marca el piso receptor; las sombras no se dibujan fuera de la duela.
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xff);
    glStencilFunc(GL_ALWAYS,1,0xff);
    glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
    r.box({0,-.01f,0},{24,.10f,15},{.94f,.65f,.36f},r.wood,{8,5});
    r.box({0,.048f,0},{18,.015f,9},{.19f,.62f,.70f},r.wood,{6,3});
    glDisable(GL_STENCIL_TEST);
    // Límites de 5 cm, dentro de la cancha. Línea central y ataque a 3 m.
    for (float x : {-8.975f,8.975f}) r.box({x,.065f,0},{.05f,.025f,9},white);
    for (float z : {-4.475f,4.475f}) r.box({0,.065f,z},{18,.025f,.05f},white);
    for (float x : {-3.f,0.f,3.f}) r.box({x,.065f,0},{.05f,.025f,9},white);
    for (float x : {-9.10f,9.10f}) for (float z : {-4.5f,4.5f})
        r.box({x,.065f,z},{.15f,.025f,.05f},white);
    // Se mide la red desde la superficie de la cancha, no desde y = 0.
    const float courtFloor = .048f + .015f / 2.f;
    // [RED] 2.43f es la altura desde el piso; netBottom resta su metro de alto.
    const float netTop = courtFloor + 2.43f;
    const float netBottom = netTop - 1.f;
    DibujarPostes(r);
    r.box({0,netTop-.035f,0},{.035f,.07f,9.8f},white);
    r.box({0,netBottom+.025f,0},{.025f,.05f,9.8f},white);
    // Dos triangulos en el plano YZ. La imagen se repite cada 10 cm.
    glm::mat4 netModel = glm::translate(glm::mat4(1),{0,netBottom+.5f,0});
    netModel = glm::rotate(netModel,glm::radians(90.f),{0,1,0});
    r.draw(r.quad,glm::scale(netModel,{9.8f,1.f,1.f}),white,r.net,{98,10});
    for (float z : {-4.5f,4.5f}) {
        r.box({0,netBottom+.5f,z},{.035f,1.f,.05f},white);
        for (int i = 0; i < 8; ++i)
            r.box({0,netTop+.05f+float(i)*.1f,z},{.025f,.10f,.025f},i%2?white:orange);
    }
}
// [BANCAS_Y_TROFEO] Posiciones de bancas, calentamiento, trofeo y arbitro.
void DibujarBancas(Graficos& r, Model& trofeo) {
    float trophyScale = EscalaTrofeo();
    // Zona 2: bancas y calentamiento al lado opuesto de las gradas.
    r.box({0,.015f,-9.7f},{25,.12f,4},white,r.concrete,{12.5f,2});
    for (float x : {-6.f,6.f}) {
        r.box({x,.54f,-8.8f},{4.8f,.18f,.65f},white,r.benchWood,{4,1});
        r.box({x,.98f,-9.05f},{4.8f,.65f,.13f},teal,r.benchWood,{4,1});
        for (float dx : {-1.8f,1.8f}) r.box({x+dx,.26f,-8.8f},{.12f,.5f,.5f},steel,r.metal);
        r.box({x,.08f,-10.3f},{3,.035f,2.7f},teal,r.rubber,{3,3});
    }
    r.box({0,.48f,-9},{2,.85f,.80f},white,r.benchWood,{2,1});
    trofeo.draw(r,transform({0,.94f,-9},{trophyScale,trophyScale,trophyScale}),{.95f,.72f,.20f});
    // Arbitro previsto en el plano: misma jerarquia y geometria simple que los jugadores.
    // La plataforma queda fuera de la zona libre (su borde mas cercano es z = -7.6).
    r.box({0,.90f,-8.05f},{1,.12f,.90f},steel,r.metal);
    for (float x : {-.40f,.40f}) for (float z : {-8.4f,-7.7f})
        r.box({x,.48f,z},{.08f,.84f,.08f},steel,r.metal);
    for (float y : {.25f,.50f,.75f}) r.box({0,y,-8.45f},{.80f,.07f,.08f},steel,r.metal);
    DibujarJugador(r,{0,.96f,-8.05f},0,white,Pose{});
}
// [GRADAS] row<4: filas; seatIndex<30: asientos por fila; .9f: separacion.
void DibujarGradas(Graficos& r, Model& butaca) {
    // Zona 3: espectadores. Gradas por niveles y butacas repetidas con bucles.
    for (int row = 0; row < 4; ++row) {
        float z = 8.1f+row*.9f, top = .45f+row*.52f;
        r.box({0,top/2,z},{28,top,.95f},{.72f,.75f,.78f},r.concrete,{14,1});
        for (int seatIndex = 0; seatIndex < 30; ++seatIndex) {
            float x = -13.f+float(seatIndex)*.9f;
            glm::vec3 seatColor = (seatIndex+row)%4 == 0 ? orange : teal;
            // El OBJ y la textura se cargan una vez; aquí solo cambia su posición y color.
            butaca.draw(r,glm::translate(glm::mat4(1),{x,top+.02f,z}),seatColor);
        }
    }
}
// [LETREROS_Y_MARCADOR] Aqui se colocan pared, nombres de equipos y cuatro digitos.
void DibujarParedYMarcador(Graficos& r) {
    int scoreA=puntosLocal, scoreB=puntosVisita;
    // Pared de fondo abierta hacia la cámara para inspeccionar las tres áreas.
    // La imagen JPG contiene los ladrillos; el muro es un solo cubo delgado.
    r.box({0,3.2f,-12.7f},{37,6.4f,.18f},white,r.brick,{37.f/1.28f,6.4f/1.28f});
    for (float x : {-15.f,-10.f,10.f,15.f}) r.box({x,3.3f,-12.5f},{.25f,6.6f,.25f},steel,r.metal,{1,6});
    // Delante de las columnas para que no tapen los nombres de los equipos.
    r.box({-10,3.8f,-12.30f},{6,1.4f,.08f},white,r.bannerGreen);
    r.box({10,3.8f,-12.30f},{6,1.4f,.08f},white,r.bannerOrange);
    // Marcador de imagenes: interacción visible en el mundo y HUD.
    r.box({0,4.65f,-12.5f},{4.8f,2.1f,.25f},{.015f,.035f,.055f});
    DibujarNumero(r,scoreA/10,-1.65f,4.65f,-12.34f);
    DibujarNumero(r,scoreA%10,-.90f,4.65f,-12.34f);
    DibujarNumero(r,scoreB/10,.90f,4.65f,-12.34f);
    DibujarNumero(r,scoreB%10,1.65f,4.65f,-12.34f);
}
// [REFLECTORES] Dibujo de los postes; la luz real esta en scene.frag.
// Si se mueven, ajustar lights en el shader y la luz de sombra en Graficos.h.
void DibujarReflectores(Graficos& r) {
    bool lights=reflectores;
    // Reflectores fuera de la zona libre y de las gradas. Las mismas posiciones están en el shader.
    for (float x : {-15.f,15.f}) for (float z : {-8.f,8.f}) {
        r.box({x,3.5f,z},{.12f,7,.12f},steel,r.metal,{1,7});
        r.box({x,7.05f,z},{1.5f,.25f,.72f},navy,r.metal);
        r.box({x,6.88f,z},{1.32f,.03f,.58f},lights?glm::vec3(1,.92f,.70f):steel);
    }
}
// [JUGADORES] Primero va el sacador; athletes guarda los XYZ de los otros once.
void DibujarJugadoresYBalon(Graficos& r, const Pose& pose) {
    const float floor = .0555f;
    Pose idle;
    DibujarJugador(r,{-10.35f,.04f,0},0,teal,pose);
    const glm::vec3 athletes[] = {{-6,0,-2.5f},{-6,0,2.5f},{-2,0,-2.5f},{-2,0,2.5f},{-3.5f,0,0},
                                 {6,0,0},{6,0,-2.5f},{6,0,2.5f},{2,0,-2.5f},{2,0,2.5f},{3.5f,0,0}};
    for (std::size_t i = 0; i < 11; ++i)
        DibujarJugador(r,athletes[i]+glm::vec3(0,floor,0),i<5?0.f:180.f,i<5?teal:orange,idle);
    // [BALON] .105f es el radio; pose.ball es la posicion animada.
    r.ball(pose.ball,.105f,white,r.ballTexture);
}
// [SOMBRAS] Redibuja las mismas poses proyectadas sobre el piso.
void DibujarSombras(Graficos& r, const Pose& pose) {
    // Reutilizamos exactamente las poses de los jugadores y del balon.
    // Es una sombra plana de un reflector; no calcula sombras entre objetos.
    r.beginFloorShadows();
    DibujarJugadoresYBalon(r,pose);
    DibujarPostes(r);
    r.endFloorShadows();
}
void DibujarPostes(Graficos& r) {
    const float courtFloor = .0555f;
    for (float z : {-5.1f,5.1f}) {
        r.box({0,courtFloor+1.3f,z},{.14f,2.6f,.14f},steel,r.metal,{1,3});
        r.box({0,courtFloor+.48f,z},{.28f,.92f,.28f},orange,r.rubber,{1,2});
    }
}


// 5. DIBUJAR UN CUADRO: perspectiva, escenario, sombras e interfaz.
void DibujarPantalla(GLFWwindow* window,Graficos& r,Model& trofeo,Model& butaca) {
    int width=0,height=0;
    glfwGetFramebufferSize(window,&width,&height);
    if(width<=0 || height<=0) return;
    // [P02][VIEWPORT] Area de la ventana donde se dibuja el escenario.
    glViewport(0,0,width,height);
    glClearColor(.10f,.14f,.20f,1);
    glStencilMask(0xff);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    // [CAMARAS] view y projection vienen de Camera.h; despues se dibujan las tres zonas.
    r.begin(camera.view(),camera.projection(float(width)/height),camera.position(),reflectores);
    DibujarCancha(r);
    DibujarBancas(r,trofeo);
    DibujarGradas(r,butaca);
    DibujarParedYMarcador(r);
    DibujarReflectores(r);
    Pose pose=PoseSaque();
    DibujarJugadoresYBalon(r,pose);
    if(reflectores) DibujarSombras(r,pose);
    std::vector<std::string> controles;
    // [INTERFAZ] Cambiar estos textos si se cambia una tecla. La letra se dibuja en Graficos.h.
    if(mostrarPaneles) controles={"C / 1 / 2 : CAMBIAR CAMARA","W A S D : NAVEGAR / ORBITAR",
        "E / Q : SUBIR / BAJAR","ARRASTRAR MOUSE : MIRAR","RUEDA : ACERCAR / ALEJAR",
        "ESPACIO : SAQUE / F : FESTEJO","P : PAUSAR / REANUDAR","L : REFLECTORES",
        "J : PUNTO LOCAL / K : PUNTO VISITA","R : REINICIAR / I : MOSTRAR U OCULTAR","F12 : CAPTURA / ESC : SALIR"};
    std::string saque=pausaSaque?"PAUSADO":saqueActivo?"EN CURSO":tiempoSaque>0?"FINALIZADO":"LISTO";
    std::string festejo=pausaFestejo?"PAUSADO":festejoActivo?"EN CURSO":tiempoFestejo>0?"FINALIZADO":"LISTO";
    r.hud(width,height,controles,puntosLocal,puntosVisita,camera.name(),reflectores,saque+" / FESTEJO: "+festejo);
    if(pedirCaptura) { GuardarCaptura(width,height); pedirCaptura=false; }
}

// 6. CONTROLES: mismo esquema de callbacks y arreglo de teclas que las practicas.
// [INTERACCION][CONTROLES] Para reasignar una tecla cambiar su GLFW_KEY_*; conservar su accion.
void KeyCallback(GLFWwindow* window,int key,int,int action,int) {
    if(key>=0 && key<1024) keys[key]=action!=GLFW_RELEASE;
    if(action!=GLFW_PRESS) return;
    switch(key) {
    case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window,true); break;
    case GLFW_KEY_C: camera.orbital=!camera.orbital; break;
    case GLFW_KEY_1: camera.orbital=false; break;
    case GLFW_KEY_2: camera.orbital=true; break;
    case GLFW_KEY_SPACE: tiempoSaque=0; saqueActivo=true; pausaSaque=false; break;
    case GLFW_KEY_F: camera.focusTrophy(); tiempoFestejo=0; festejoActivo=true; pausaFestejo=false; break;
    case GLFW_KEY_P:
        if(saqueActivo) pausaSaque=!pausaSaque;
        if(festejoActivo) pausaFestejo=!pausaFestejo;
        break;
    case GLFW_KEY_L: reflectores=!reflectores; break;
    case GLFW_KEY_I: mostrarPaneles=!mostrarPaneles; break;
    case GLFW_KEY_J: puntosLocal=(puntosLocal+1)%100; break;
    case GLFW_KEY_K: puntosVisita=(puntosVisita+1)%100; break;
    case GLFW_KEY_R: Reiniciar(); break;
    case GLFW_KEY_F12: pedirCaptura=true; break;
    // Un paso inicial permite responder tambien a pulsaciones cortas.
    case GLFW_KEY_W: case GLFW_KEY_UP: camera.move(1,0,0,.05f); break;
    case GLFW_KEY_S: case GLFW_KEY_DOWN: camera.move(-1,0,0,.05f); break;
    case GLFW_KEY_D: case GLFW_KEY_RIGHT: camera.move(0,1,0,.05f); break;
    case GLFW_KEY_A: case GLFW_KEY_LEFT: camera.move(0,-1,0,.05f); break;
    case GLFW_KEY_E: camera.move(0,0,1,.05f); break;
    case GLFW_KEY_Q: camera.move(0,0,-1,.05f); break;
    }
}
// [MOVIMIENTO] Combina las teclas mantenidas; la velocidad se ajusta en Camera.h.
void DoMovement(float dt) {
    camera.move(float(keys[GLFW_KEY_W]||keys[GLFW_KEY_UP])-float(keys[GLFW_KEY_S]||keys[GLFW_KEY_DOWN]),
                float(keys[GLFW_KEY_D]||keys[GLFW_KEY_RIGHT])-float(keys[GLFW_KEY_A]||keys[GLFW_KEY_LEFT]),
                float(keys[GLFW_KEY_E])-float(keys[GLFW_KEY_Q]),dt);
}
// [MOUSE] Arrastrar orienta; ScrollCallback usa la rueda para acercar o alejar.
void MouseCallback(GLFWwindow* window,double x,double y) {
    if(firstMouse) { lastX=x; lastY=y; firstMouse=false; }
    if(glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS ||
       glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT)==GLFW_PRESS)
        camera.turn(float(x-lastX),float(y-lastY));
    lastX=x; lastY=y;
}
void ScrollCallback(GLFWwindow*,double,double y) { camera.zoom(float(y)); }
// [CAPTURAS] F12 guarda lo que se dibujo en configbase/Capturas.
void GuardarCaptura(int width,int height) {
    std::filesystem::create_directories("Capturas");
    std::string nombre;
    do { nombre="Capturas/captura_"+std::to_string(++numeroCaptura)+".bmp"; }
    while(std::filesystem::exists(nombre));
    std::vector<unsigned char> pixels(static_cast<std::size_t>(width)*height*3), flipped(pixels.size());
    glPixelStorei(GL_PACK_ALIGNMENT,1);
    glReadBuffer(GL_BACK);
    glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,pixels.data());
    for(int y=0;y<height;++y)
        std::copy_n(pixels.data()+static_cast<std::size_t>(height-1-y)*width*3,width*3,
                    flipped.data()+static_cast<std::size_t>(y)*width*3);
    if(!SOIL_save_image(nombre.c_str(),SOIL_SAVE_TYPE_BMP,width,height,3,flipped.data()))
        throw std::runtime_error("No se pudo guardar la captura");
    std::cout << "Captura: " << nombre << '\n';
}
