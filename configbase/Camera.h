#pragma once

// Std. Includes
#include <vector>

// GL Includes
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// DIRECCIONES: opciones que reciben las funciones de movimiento.
enum Camera_Movement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
    UP,
    DOWN
};

// AJUSTES: velocidad, sensibilidad y orientacion inicial.
const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 6.0f;
const GLfloat SENSITIVTY = 0.25f;
const GLfloat ZOOM = 45.0f;

// CAMARA: posicion, orientacion y modos orbital y primera persona.
class Camera
{
public:
    bool orbital = true;
    GLfloat orbitYaw = 43.0f, orbitPitch = 35.0f, distance = 27.0f;
    glm::vec3 target = glm::vec3(0,1,0);
	// Constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW, GLfloat pitch = PITCH) : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVTY), zoom(ZOOM)
	{
		this->position = position;
		this->worldUp = up;
		this->yaw = yaw;
		this->pitch = pitch;
		this->updateCameraVectors();
	}

	// Constructor with scalar values
	Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch) : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVTY), zoom(ZOOM)
	{
		this->position = glm::vec3(posX, posY, posZ);
		this->worldUp = glm::vec3(upX, upY, upZ);
		this->yaw = yaw;
		this->pitch = pitch;
		this->updateCameraVectors();
	}

	// VISTA: apunta al centro en orbital o hacia el frente en primera persona.
	glm::mat4 GetViewMatrix()
	{
		if (orbital) return glm::lookAt(GetPosition(), target, glm::vec3(0,1,0));
        return glm::lookAt(this->position, this->position + this->front, this->up);
	}

	// TECLADO: cambia distancia/giro orbital o desplaza la camara.
	void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
	{
		if (orbital) {
            if (direction == FORWARD) distance -= 10.0f*deltaTime;
            if (direction == BACKWARD) distance += 10.0f*deltaTime;
            if (direction == LEFT) orbitYaw -= 45.0f*deltaTime;
            if (direction == RIGHT) orbitYaw += 45.0f*deltaTime;
            if (direction == UP) orbitPitch += 30.0f*deltaTime;
            if (direction == DOWN) orbitPitch -= 30.0f*deltaTime;
            if (distance < 8) distance = 8;
            if (distance > 50) distance = 50;
            if (orbitPitch < 8) orbitPitch = 8;
            if (orbitPitch > 78) orbitPitch = 78;
            return;
        }
        GLfloat velocity = this->movementSpeed * deltaTime;

		if (direction == FORWARD)
		{
			this->position += glm::normalize(glm::vec3(front.x,0,front.z)) * velocity;
		}

		if (direction == BACKWARD)
		{
			this->position -= glm::normalize(glm::vec3(front.x,0,front.z)) * velocity;
		}

		if (direction == LEFT)
		{
			this->position -= this->right * velocity;
		}

		if (direction == RIGHT)
		{
			this->position += this->right * velocity;
		}
	}

	// ALTURA: E/Q eleva la vista o cambia la inclinacion orbital.
	void SetHeight(Camera_Movement direction, GLfloat deltaTime) {
        if (orbital) { ProcessKeyboard(direction,deltaTime); return; }
        if (direction == UP) position.y += 7*deltaTime;
        if (direction == DOWN) position.y -= 7*deltaTime;
        if (position.y < 1.1f) position.y = 1.1f;
        if (position.y > 12) position.y = 12;
    }
    // GIRO: aplica el desplazamiento horizontal y vertical del mouse.
	void ProcessMouseMovement(GLfloat xOffset, GLfloat yOffset, GLboolean constrainPitch = true)
	{
		if (orbital) {
            orbitYaw += xOffset*0.25f;
            orbitPitch += yOffset*0.20f;
            if (orbitPitch < 8) orbitPitch = 8;
            if (orbitPitch > 78) orbitPitch = 78;
            return;
        }
        xOffset *= this->mouseSensitivity;
		yOffset *= this->mouseSensitivity;

		this->yaw += xOffset;
		this->pitch += yOffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (this->pitch > 89.0f)
			{
				this->pitch = 89.0f;
			}

			if (this->pitch < -89.0f)
			{
				this->pitch = -89.0f;
			}
		}

		// Update Front, Right and Up Vectors using the updated Eular angles
		this->updateCameraVectors();
	}

	// ZOOM ORBITAL: la rueda cambia la distancia, limitada entre 8 y 50.
	void ProcessMouseScroll(GLfloat yOffset) {
        if (orbital) {
            distance -= yOffset*1.5f;
            if (distance < 8) distance = 8;
            if (distance > 50) distance = 50;
        }
    }

	GLfloat GetZoom()
	{
		return this->zoom;
	}

	// POSICION: calcula la orbita con seno y coseno, o devuelve la posicion libre.
	glm::vec3 GetPosition()
	{
		if (orbital) {
            GLfloat y = glm::radians(orbitYaw), p = glm::radians(orbitPitch);
            return target + distance*glm::vec3(cos(y)*cos(p),sin(p),sin(y)*cos(p));
        }
        return this->position;
	}

	glm::vec3 GetFront()
	{
		return this->front;
	}

private:
	// Camera Attributes
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	// Eular Angles
	GLfloat yaw;
	GLfloat pitch;

	// Camera options
	GLfloat movementSpeed;
	GLfloat mouseSensitivity;
	GLfloat zoom;

	// ORIENTACION: calcula frente, derecha y arriba a partir de yaw y pitch.
	void updateCameraVectors()
	{
		// Calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
		front.y = sin(glm::radians(this->pitch));
		front.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
		this->front = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		this->right = glm::normalize(glm::cross(this->front, this->worldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		this->up = glm::normalize(glm::cross(this->right, this->front));
	}
};