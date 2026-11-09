#pragma once
#include <glm/glm.hpp>
#include <string>

class Camera {
public:
    bool orbital = true;
    glm::vec3 position() const;
    glm::mat4 view() const;
    glm::mat4 projection(float aspect) const;
    void move(float forward, float right, float vertical, float dt);
    void turn(float dx, float dy);
    void zoom(float amount);
    void reset();
    void focusTrophy();
    std::string name() const { return orbital ? "ORBITAL / TV" : "PRIMERA PERSONA"; }
private:
    // [VISTA_INICIAL] eye: posicion de primera persona; target: centro de la orbita.
    glm::vec3 eye = {12.f,1.7f,7.f};
    glm::vec3 target = {0,1,0};
    float yaw = -145.f, pitch = -5.f;
    // Angulo horizontal, inclinacion y distancia inicial de la camara orbital.
    float orbitYaw = 43.f, orbitPitch = 35.f, distance = 27.f;
    glm::vec3 front() const;
};


#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

glm::vec3 Camera::front() const {
    float y = glm::radians(yaw), p = glm::radians(pitch);
    return glm::normalize(glm::vec3(std::cos(y)*std::cos(p), std::sin(p), std::sin(y)*std::cos(p)));
}
glm::vec3 Camera::position() const {
    if (!orbital) return eye;
    float y = glm::radians(orbitYaw), p = glm::radians(orbitPitch);
    return target + distance * glm::vec3(std::cos(y)*std::cos(p), std::sin(p), std::sin(y)*std::cos(p));
}
// [P05][CAMARA_SINTETICA] Vista desde primera persona o camara orbital.
glm::mat4 Camera::view() const {
    auto e = position();
    return glm::lookAt(e, orbital ? target : e+front(), glm::vec3(0,1,0));
}
// [P02][PERSPECTIVA] 48 grados de apertura; .1 y 120 son los limites de vision.
glm::mat4 Camera::projection(float aspect) const {
    return glm::perspective(glm::radians(48.f), aspect, .1f, 120.f);
}
// [VELOCIDAD] 7.f mueve primera persona; 45.f gira y 10.f acerca la orbita.
void Camera::move(float forward, float right, float vertical, float dt) {
    if (orbital) {
        orbitYaw += right*45.f*dt;
        distance = std::clamp(distance-forward*10.f*dt, 8.f, 50.f);
        orbitPitch = std::clamp(orbitPitch+vertical*30.f*dt, 8.f, 78.f);
    } else {
        // Navegación sobre el suelo. E/Q permite elevarse para inspeccionar modelos.
        glm::vec3 f = glm::normalize(glm::vec3(front().x,0,front().z));
        glm::vec3 r = glm::normalize(glm::cross(f, glm::vec3(0,1,0)));
        eye += (f*forward + r*right + glm::vec3(0,vertical,0))*7.f*dt;
        eye.x = std::clamp(eye.x, -19.f, 19.f);
        eye.z = std::clamp(eye.z, -13.f, 13.f);
        eye.y = std::clamp(eye.y, 1.1f, 12.f);
    }
}
// [SENSIBILIDAD_MOUSE] Factores .25/.20 orbital y .15 primera persona.
void Camera::turn(float dx, float dy) {
    if (orbital) {
        orbitYaw -= dx*.25f;
        orbitPitch = std::clamp(orbitPitch+dy*.20f, 8.f, 78.f);
    } else {
        yaw += dx*.15f;
        pitch = std::clamp(pitch-dy*.15f, -85.f, 85.f);
    }
}
void Camera::zoom(float amount) { distance = std::clamp(distance-amount*1.5f, 8.f,50.f); }
void Camera::reset() { *this = Camera(); }
// [ENFOQUE_TROFEO] F usa este objetivo, distancia y angulos para mostrar el festejo.
void Camera::focusTrophy() {
    orbital = true;
    target = {0,1.8f,-9};
    distance = 8.f;
    // Vista lateral del trofeo para que el arbitro no lo tape durante el festejo.
    orbitYaw = 20.f;
    orbitPitch = 25.f;
}
