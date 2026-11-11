#version 330 core

#define NUMBER_OF_POINT_LIGHTS 1

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight
{
    vec3 direction;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight
{
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight
{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 color;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NUMBER_OF_POINT_LIGHTS];
uniform SpotLight spotLights[4];
uniform bool sinLuz;
uniform vec3 objectColor;
uniform Material material;
uniform int transparency;

// ILUMINACION: funciones para luz direccional, puntual y reflector.
vec3 CalcDirLight( DirLight light, vec3 normal, vec3 viewDir );
vec3 CalcPointLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir );
vec3 CalcSpotLight( SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir );

void main( )
{
    if (sinLuz) { // Interfaz y sombras: usa el color de la imagen sin iluminarla.
        color = texture(material.diffuse,TexCoords) * vec4(objectColor,1);
        if (color.a < 0.01) discard;
        return;
    }
    // Properties
    vec3 norm = normalize( Normal );
    vec3 viewDir = normalize( viewPos - FragPos );
    
    // LUZ DIRECCIONAL: aporta la iluminacion general.
    vec3 result = CalcDirLight( dirLight, norm, viewDir );
    
    // Suma la contribucion de los cuatro reflectores.
    for (int i=0;i<4;i++) result += CalcSpotLight(spotLights[i],norm,FragPos,viewDir);
    
    color = vec4(result * objectColor, texture(material.diffuse, TexCoords).a);
	  if(color.a < 0.1 )
        discard;

}

// DIRECCIONAL: combina ambiente, difuso y brillo especular.
vec3 CalcDirLight( DirLight light, vec3 normal, vec3 viewDir )
{
    vec3 lightDir = normalize( -light.direction );
    
    // DIFUSO: depende del angulo entre normal y luz.
    float diff = max( dot( normal, lightDir ), 0.0 );
    
    // ESPECULAR: brillo segun la vista y la reflexion.
    vec3 reflectDir = reflect( -lightDir, normal );
    float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), material.shininess );
    
    // Combine results
    vec3 ambient = light.ambient * vec3( texture( material.diffuse, TexCoords ) );
    vec3 diffuse = light.diffuse * diff * vec3( texture( material.diffuse, TexCoords ) );
    vec3 specular = light.specular * spec * vec3( texture( material.specular, TexCoords ) );
    
    return ( ambient + diffuse + specular );
}

// PUNTUAL: calcula una fuente con atenuacion; no se usa en la escena actual.
vec3 CalcPointLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir )
{
    vec3 lightDir = normalize( light.position - fragPos );
    
    // DIFUSO: depende del angulo entre normal y luz.
    float diff = max( dot( normal, lightDir ), 0.0 );
    
    // ESPECULAR: brillo segun la vista y la reflexion.
    vec3 reflectDir = reflect( -lightDir, normal );
    float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), material.shininess );
    
    // ATENUACION: reduce la luz con la distancia.
    float distance = length( light.position - fragPos );
    float attenuation = 1.0f / ( light.constant + light.linear * distance + light.quadratic * ( distance * distance ) );
    
    // Combine results
    vec3 ambient = light.ambient * vec3( texture( material.diffuse, TexCoords ) );
    vec3 diffuse = light.diffuse * diff * vec3( texture( material.diffuse, TexCoords ) );
    vec3 specular = light.specular * spec * vec3( texture( material.specular, TexCoords ) );
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    return ( ambient + diffuse + specular );
}

// REFLECTOR: agrega direccion, apertura del cono y atenuacion.
vec3 CalcSpotLight( SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir )
{
    vec3 lightDir = normalize( light.position - fragPos );
    
    // DIFUSO: depende del angulo entre normal y luz.
    float diff = max( dot( normal, lightDir ), 0.0 );
    
    // ESPECULAR: brillo segun la vista y la reflexion.
    vec3 reflectDir = reflect( -lightDir, normal );
    float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), material.shininess );
    
    // ATENUACION: reduce la luz con la distancia.
    float distance = length( light.position - fragPos );
    float attenuation = 1.0f / ( light.constant + light.linear * distance + light.quadratic * ( distance * distance ) );
    
    // CONO: suaviza la intensidad entre los dos angulos de apertura.
    float theta = dot( lightDir, normalize( -light.direction ) );
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp( ( theta - light.outerCutOff ) / epsilon, 0.0, 1.0 );
    
    // Combine results
    vec3 ambient = light.ambient * vec3( texture( material.diffuse, TexCoords ) );
    vec3 diffuse = light.diffuse * diff * vec3( texture( material.diffuse, TexCoords ) );
    vec3 specular = light.specular * spec * vec3( texture( material.specular, TexCoords ) );
    
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    
    return ( ambient + diffuse + specular );
}