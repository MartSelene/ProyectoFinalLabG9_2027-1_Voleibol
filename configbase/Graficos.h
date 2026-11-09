#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <array>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

// Una malla contiene la geometría en memoria de la GPU: VAO, VBO y EBO.
class Mesh {
public:
    Mesh() = default;
    ~Mesh();
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    void upload(const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices);
    void draw() const;
    std::size_t triangleCount() const { return count / 3; }
private:
    GLuint vao = 0, vbo = 0, ebo = 0;
    GLsizei count = 0;
};

class Graficos {
public:
    Graficos();
    ~Graficos();
    void begin(const glm::mat4& view, const glm::mat4& projection,
               const glm::vec3& eye, bool reflectors);
    void beginFloorShadows();
    void endFloorShadows();
    void draw(const Mesh& mesh, const glm::mat4& model, const glm::vec3& color,
              GLuint texture = 0, glm::vec2 repeat = glm::vec2(1));
    void box(const glm::vec3& center, const glm::vec3& size, const glm::vec3& color,
             GLuint texture = 0, glm::vec2 repeat = glm::vec2(1));
    void ball(const glm::vec3& center, float radius, const glm::vec3& color, GLuint texture = 0);
    void hud(int width, int height, const std::vector<std::string>& controls,
             int scoreA, int scoreB, const std::string& camera, bool reflectors,
             const std::string& animation);
    GLuint loadTexture(const std::string& path);
    Mesh cube, sphere, quad;
    GLuint wood = 0, ballTexture = 0, bannerGreen = 0, bannerOrange = 0;
    GLuint brick = 0, concrete = 0, metal = 0, fabric = 0, rubber = 0, benchWood = 0, net = 0;
    GLuint scoreDigits[10] = {};
    std::size_t loadedTextureCount() const { return textures.size()-1; }
private:
    GLuint program = 0, white = 0;
    GLint modelLoc, colorLoc, repeatLoc, viewLoc, projectionLoc, eyeLoc, lightsLoc, unlitLoc;
    GLint shadowLoc, shadowProjectionLoc;
    std::vector<GLuint> textures;
    Mesh textMesh;
    GLuint uiFont = 0;
    std::array<float,95> glyphAdvance{};
    void screenText(const std::string& text, float x, float y, float scale, glm::vec3 color);
};


#include <soil2/SOIL2.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace {
std::string readText(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("No se encontro " + path);
    std::ostringstream out;
    out << in.rdbuf();
    return out.str();
}
GLuint compile(GLenum type, const std::string& path) {
    const auto source = readText(path);
    const char* ptr = source.c_str();
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &ptr, nullptr);
    glCompileShader(shader);
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[4096] = {};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        glDeleteShader(shader);
        throw std::runtime_error(path + ": " + log);
    }
    return shader;
}

}

Mesh::~Mesh() {
    if (ebo) glDeleteBuffers(1, &ebo);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
}
// [P01][MALLAS] Vertices, indices, VAO, VBO y atributos de las figuras.
void Mesh::upload(const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices) {
    if (!vao) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
    }
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, uv)));
    for (unsigned i = 0; i < 3; ++i) glEnableVertexAttribArray(i);
    glBindVertexArray(0);
    count = static_cast<GLsizei>(indices.size());
}
void Mesh::draw() const {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

// [P01][SHADERS] Se leen y enlazan los programas de vertices e iluminacion.
Graficos::Graficos() {
    GLuint vs = compile(GL_VERTEX_SHADER, "Shader/scene.vert");
    GLuint fs = compile(GL_FRAGMENT_SHADER, "Shader/scene.frag");
    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    GLint ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) throw std::runtime_error("No se pudo enlazar el shader de escena");
    modelLoc = glGetUniformLocation(program, "model");
    colorLoc = glGetUniformLocation(program, "color");
    repeatLoc = glGetUniformLocation(program, "uvRepeat");
    viewLoc = glGetUniformLocation(program, "view");
    projectionLoc = glGetUniformLocation(program, "projection");
    eyeLoc = glGetUniformLocation(program, "eye");
    lightsLoc = glGetUniformLocation(program, "reflectors");
    unlitLoc = glGetUniformLocation(program, "unlit");
    shadowLoc = glGetUniformLocation(program, "shadowPass");
    shadowProjectionLoc = glGetUniformLocation(program, "shadowProjection");

    std::vector<Vertex> v;
    std::vector<unsigned> idx;
    const glm::vec3 corners[8] = {{-.5f,-.5f,-.5f},{.5f,-.5f,-.5f},{.5f,.5f,-.5f},{-.5f,.5f,-.5f},
                                  {-.5f,-.5f,.5f},{.5f,-.5f,.5f},{.5f,.5f,.5f},{-.5f,.5f,.5f}};
    const int faces[6][4] = {{4,5,6,7},{1,0,3,2},{0,4,7,3},{5,1,2,6},{3,7,6,2},{0,1,5,4}};
    const glm::vec3 normals[6] = {{0,0,1},{0,0,-1},{-1,0,0},{1,0,0},{0,1,0},{0,-1,0}};
    const glm::vec2 uv[4] = {{0,0},{1,0},{1,1},{0,1}};
    for (unsigned f = 0; f < 6; ++f) {
        for (unsigned j = 0; j < 4; ++j) v.push_back({corners[faces[f][j]], normals[f], uv[j]});
        unsigned base = f * 4;
        idx.insert(idx.end(), {base,base+1,base+2,base,base+2,base+3});
    }
    cube.upload(v, idx);
    v.clear(); idx.clear();
    constexpr unsigned rings = 20, sectors = 32;
    for (unsigned r = 0; r <= rings; ++r) {
        float theta = glm::pi<float>() * r / rings;
        for (unsigned s = 0; s <= sectors; ++s) {
            float phi = 2 * glm::pi<float>() * s / sectors;
            glm::vec3 n(std::sin(theta)*std::cos(phi), std::cos(theta), std::sin(theta)*std::sin(phi));
            v.push_back({n, n, {float(s)/sectors, float(r)/rings}});
        }
    }
    for (unsigned r = 0; r < rings; ++r) for (unsigned s = 0; s < sectors; ++s) {
        unsigned a = r * (sectors+1) + s, b = a+sectors+1;
        idx.insert(idx.end(), {a,b,a+1,a+1,b,b+1});
    }
    sphere.upload(v, idx);
    // Superficie plana de dos triangulos: sirve para la red y los numeros.
    quad.upload({{{-.5f,-.5f,0},{0,0,1},{0,0}},{{.5f,-.5f,0},{0,0,1},{1,0}},
                 {{.5f,.5f,0},{0,0,1},{1,1}},{{-.5f,.5f,0},{0,0,1},{0,1}}},
                {0,1,2,0,2,3});
    glGenTextures(1, &white);
    glBindTexture(GL_TEXTURE_2D, white);
    unsigned char pixel[] = {255,255,255};
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,1,1,0,GL_RGB,GL_UNSIGNED_BYTE,pixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    textures.push_back(white);
    // [P06][TEXTURAS] Cambiar aqui la ruta para cargar otra imagen de Textures.
    wood = loadTexture("Textures/duela.jpg");
    ballTexture = loadTexture("Textures/balon.png");
    bannerGreen = loadTexture("Textures/equipo_verde.png");
    bannerOrange = loadTexture("Textures/equipo_naranja.png");
    brick = loadTexture("Textures/pared_ladrillo.jpg");
    concrete = loadTexture("Textures/concreto.jpg");
    metal = loadTexture("Textures/metal.jpg");
    fabric = loadTexture("Textures/tela.jpg");
    rubber = loadTexture("Textures/goma.jpg");
    benchWood = loadTexture("Textures/madera_banca.jpg");
    net = loadTexture("Textures/red.png");
    for (int i=0;i<10;++i)
        scoreDigits[i] = loadTexture("Textures/marcador_"+std::to_string(i)+".png");
    // [FUENTE] Imagen de Arial y anchos de letras usados por la interfaz.
    uiFont = loadTexture("Textures/Fuente/arial_ui.png");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    std::ifstream metrics("Textures/Fuente/arial_ui.txt");
    for (float& advance : glyphAdvance)
        if (!(metrics >> advance) || advance <= 0)
            throw std::runtime_error("Faltan los anchos de la fuente Arial");
    std::cout << "TEXTURAS: 21 imagenes de escena y una textura de Arial cargadas con SOIL2\n";
}
Graficos::~Graficos() {
    if (!textures.empty()) glDeleteTextures(static_cast<GLsizei>(textures.size()), textures.data());
    if (program) glDeleteProgram(program);
}
// [P06][CARGA_IMAGEN] SOIL2 lee el archivo; GL_REPEAT repite y GL_LINEAR suaviza.
GLuint Graficos::loadTexture(const std::string& path) {
    // RGBA conserva los huecos transparentes de la red, igual que las imagenes de las practicas.
    GLuint id = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_RGBA, SOIL_CREATE_NEW_ID,
                                    SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
    if (!id) throw std::runtime_error("Error de textura " + path + ": " + SOIL_last_result());
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GLint width=0,height=0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_WIDTH,&width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_HEIGHT,&height);
    std::cout << "TEXTURA: " << path << " / " << width << 'x' << height << '\n';
    textures.push_back(id);
    return id;
}
void Graficos::begin(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& eye, bool reflectors) {
    glUseProgram(program);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(eyeLoc, 1, glm::value_ptr(eye));
    glUniform1i(lightsLoc, reflectors);
    glUniform1i(unlitLoc, false);
    glUniform1i(shadowLoc, false);
}
// [PROYECCION_SOMBRA] light es la posicion del reflector usado para la sombra plana.
void Graficos::beginFloorShadows() {
    // Plano y = 0.079: queda apenas sobre las lineas para evitar parpadeos.
    // S = (plano . luz) I - luz * plano^T proyecta una malla desde el reflector.
    const glm::vec4 plane(0,1,0,-.079f), light(-15,7,-8,1);
    glm::mat4 shadow = glm::mat4(glm::dot(plane,light)) - glm::outerProduct(light,plane);
    glUniformMatrix4fv(shadowProjectionLoc,1,GL_FALSE,glm::value_ptr(shadow));
    glUniform1i(shadowLoc,true);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xff);
    glStencilFunc(GL_EQUAL,1,0xff);
    // Cada pixel del piso se oscurece una sola vez aunque se superpongan piezas.
    glStencilOp(GL_KEEP,GL_KEEP,GL_INCR);
}
void Graficos::endFloorShadows() {
    glUniform1i(shadowLoc,false);
    glDisable(GL_STENCIL_TEST);
    glStencilMask(0xff);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
// [MATERIAL] Combina matriz, color, textura y repeticiones UV para dibujar una malla.
void Graficos::draw(const Mesh& mesh, const glm::mat4& model, const glm::vec3& color, GLuint texture, glm::vec2 repeat) {
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));
    glUniform2fv(repeatLoc, 1, glm::value_ptr(repeat));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture ? texture : white);
    mesh.draw();
}
void Graficos::box(const glm::vec3& center, const glm::vec3& size, const glm::vec3& color, GLuint texture, glm::vec2 repeat) {
    draw(cube, glm::scale(glm::translate(glm::mat4(1), center), size), color, texture, repeat);
}
void Graficos::ball(const glm::vec3& center, float radius, const glm::vec3& color, GLuint texture) {
    draw(sphere, glm::scale(glm::translate(glm::mat4(1), center), glm::vec3(radius)), color, texture);
}
void Graficos::screenText(const std::string& text, float x, float y, float scale, glm::vec3 color) {
    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;
    // Un rectangulo por letra toma su imagen de una celda de 64x64 de Arial.
    const float factor = scale * .25f, size = 64.f * factor;
    for (unsigned char ch : text) {
        unsigned index = (ch >= 32 && ch <= 126 ? ch : '?') - 32;
        if (ch != ' ') {
            float u0 = float(index % 16) / 16.f, u1 = u0 + 1.f/16.f;
            float v1 = 1.f - float(index / 16) / 6.f, v0 = v1 - 1.f/6.f;
            unsigned base = static_cast<unsigned>(vertices.size());
            vertices.push_back({{x,y,0},{0,0,1},{u0,v1}});
            vertices.push_back({{x+size,y,0},{0,0,1},{u1,v1}});
            vertices.push_back({{x+size,y+size,0},{0,0,1},{u1,v0}});
            vertices.push_back({{x,y+size,0},{0,0,1},{u0,v0}});
            indices.insert(indices.end(), {base,base+1,base+2,base,base+2,base+3});
        }
        x += glyphAdvance[index] * factor;
    }
    textMesh.upload(vertices, indices);
    draw(textMesh, glm::mat4(1), color, uiFont);
}
// [P02][PANELES] Posiciones en pixeles, colores y textos; scale controla el tamano de letra.
void Graficos::hud(int width, int height, const std::vector<std::string>& controls, int scoreA, int scoreB,
                   const std::string& camera, bool reflectors, const std::string& animation) {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    // [P02][ORTOGRAFICA] Proyeccion 2D utilizada por el texto y los paneles.
    glm::mat4 identity(1), ortho = glm::ortho(0.f,float(width),float(height),0.f,-1.f,1.f);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(identity));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(ortho));
    glUniform1i(unlitLoc, true);
    const float scale = width < 1050 ? 1.5f : 2.f;
    box({float(width)/2,38,0},{float(width),76,1},{.025f,.055f,.10f});
    screenText("VOLEIBOL",24,18,scale,{.92f,.96f,1});
    screenText("LOCAL "+std::to_string(scoreA)+" : "+std::to_string(scoreB)+" VISITA",float(width)-300,20,scale,{1,.78f,.23f});
    const float panelHeight = 38.f + 22.f * static_cast<float>(controls.size());
    float y = float(height)-panelHeight;
    if (!controls.empty()) {
        box({208,float(height)-panelHeight/2-18,0},{392,panelHeight,1},{.025f,.055f,.10f});
        screenText("CONTROLES",25,y,scale,{.22f,.8f,.86f});
    }
    y += 28;
    for (const auto& line : controls) { screenText(line,25,y,scale*.75f,{.85f,.91f,.97f}); y += 22; }
    if (!controls.empty()) {
        // Los dos paneles se muestran y ocultan juntos con la tecla I.
        const auto separator = animation.find(" / FESTEJO: ");
        const bool hasTrophy = separator != std::string::npos;
        const float extra = hasTrophy ? 25.f : 0.f;
        box({float(width)-219,float(height)-54-extra/2,0},{414,84+extra,1},{.025f,.055f,.10f});
        screenText("CAMARA: "+camera,float(width)-411,float(height)-85-extra,scale*.8f,{.85f,.91f,.97f});
        screenText(std::string("REFLECTORES: ")+(reflectors?"ENCENDIDOS":"APAGADOS"),float(width)-411,float(height)-60-extra,scale*.8f,{1,.78f,.23f});
        screenText("SAQUE: "+animation.substr(0,separator),float(width)-411,float(height)-35-extra,scale*.8f,{.22f,.8f,.86f});
        if (hasTrophy)
            screenText("FESTEJO: "+animation.substr(separator+12),float(width)-411,float(height)-35,scale*.8f,{.22f,.8f,.86f});
    }
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}
