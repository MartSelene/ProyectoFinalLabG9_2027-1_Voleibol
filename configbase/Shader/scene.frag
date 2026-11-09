#version 330 core
in vec3 positionWorld;
in vec3 normalWorld;
in vec2 uv;
out vec4 fragmentColor;
uniform sampler2D materialTexture;
uniform vec3 color, eye;
uniform vec2 uvRepeat;
uniform bool reflectors, unlit, shadowPass;
void main() {
    if (shadowPass) { fragmentColor = vec4(0.03,0.04,0.05,0.28); return; }
    // [P06][TEXTURA_UV] uvRepeat controla cuantas veces se repite la imagen.
    vec4 image = texture(materialTexture, uv * uvRepeat);
    // La interfaz conserva el alfa suave de las letras y lo mezcla con el fondo.
    if (unlit) { fragmentColor = vec4(image.rgb * color, image.a); return; }
    // Recorte alfa: en los huecos de la red no se escribe color ni profundidad.
    // [TRANSPARENCIA] Descarta los huecos de la red.
    if (image.a < 0.1) discard;
    vec3 albedo = image.rgb * color;
    vec3 n = normalize(normalWorld);
    vec3 v = normalize(eye - positionWorld);
    // [LUZ_DIRECCIONAL] Cambiar este vector cambia la direccion de la luz general.
    vec3 day = normalize(vec3(-0.3, 1.0, 0.4));
    // Lambert difuso + Blinn-Phong especular; la matriz normal respeta escalas.
    // [P07][ILUMINACION] 0.24: ambiente; 0.40: intensidad difusa de la luz general.
    float light = 0.24 + max(dot(n, day), 0.0) * 0.40;
    float specular = 0.0;
    if (reflectors) {
        // [P08][POSICIONES_LUCES] Mismos XYZ que los postes de DibujarReflectores.
        vec3 lights[4] = vec3[4](vec3(-15,7,-8),vec3(15,7,-8),vec3(-15,7,8),vec3(15,7,8));
        for (int i=0; i<4; i++) {
            vec3 toLight = lights[i] - positionWorld;
            vec3 l = normalize(toLight);
            vec3 direction = normalize(vec3(-lights[i].x, -7.0, -lights[i].z));
            // [P08][CONO_Y_DISTANCIA] cone define la apertura; 0.006 regula la atenuacion.
            float cone = smoothstep(0.68, 0.87, dot(-l, direction));
            float attenuation = 1.0 / (1.0 + 0.006 * dot(toLight,toLight));
            // 0.60: intensidad de reflectores; 32: concentracion del brillo; 0.16: intensidad especular.
            light += max(dot(n,l),0.0) * cone * attenuation * 0.60;
            specular += pow(max(dot(n,normalize(l+v)),0.0),32.0)*cone*attenuation*0.16;
        }
    }
    vec3 result = albedo * light + vec3(specular);
    // Niebla tenue permite leer la profundidad del recinto.
    // [NIEBLA] Empieza a 32 y llega al maximo a 65 unidades de distancia.
    float fog = smoothstep(32.0, 65.0, length(eye-positionWorld));
    fragmentColor = vec4(mix(result, vec3(0.10,0.14,0.20), fog),1.0);
}
