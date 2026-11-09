#version 330 core
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;
uniform mat4 model, view, projection;
uniform mat4 shadowProjection;
uniform bool shadowPass;
out vec3 positionWorld;
out vec3 normalWorld;
out vec2 uv;
void main() {
    // [MATRICES] model coloca el objeto; view mira desde la camara; projection da perspectiva.
    vec4 p = model * vec4(aPosition, 1.0);
    positionWorld = p.xyz;
    // [P07][NORMALES] La normal se corrige al escalar para conservar la iluminacion.
    normalWorld = mat3(transpose(inverse(model))) * aNormal;
    uv = aUV;
    // shadowPass activa la proyeccion al piso para dibujar sombras.
    gl_Position = projection * view * (shadowPass ? shadowProjection * p : p);
}
