#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 uvRepeat;

void main()
{
    // MATRICES: pasa del objeto al mundo, la camara y la proyeccion.
    gl_Position = projection * view *  model * vec4(position, 1.0f);
    FragPos = vec3(model * vec4(position, 1.0f));
    // NORMALES: mantiene la direccion correcta aunque cambie la escala.
    Normal = mat3(transpose(inverse(model))) * normal;
    // UV: controla cuantas veces se repite la textura sobre el objeto.
    TexCoords = texCoords * uvRepeat.xy;
}