#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

//lighting
uniform	vec3 lightDir;
uniform	vec3 lightColor;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

//point light uniforms
uniform mat4 view;
uniform vec3 pointLightPosition;
uniform vec3 pointLightColor;

uniform float pointLightConstant;  	
uniform float pointLightLinear;    
uniform float pointLightQuadratic; 


vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

vec3 calculatePointLight() {

	vec3 cameraPosEye = vec3(0.0f);
	vec3 lightPosEye = (view * vec4(pointLightPosition, 1.0)).xyz;
    vec3 lightDirN = normalize(lightPosEye - fPosEye.xyz);					
	vec3 normalEye = normalize(fNormal);
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);	

    // Diffuse term
    float diff = max(dot(normalEye, lightDirN), 0.0);

	// Specular term
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);


	//compute distance to light
	float dist = length(lightPosEye - fPosEye.xyz);
	//compute attenuation
	float att = 1.0f / (pointLightConstant + pointLightLinear * dist + pointLightQuadratic * (dist * dist));

   
    // Combine results
    vec3 ambient = ambientStrength * pointLightColor;
    vec3 diffuse = diff * pointLightColor;
    vec3 specular = specularStrength * specCoeff * pointLightColor;

    return att * (ambient + diffuse + specular);
}

void computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(fNormal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDir);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
		
	//compute ambient light
	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
}

float computeShadow() {
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	normalizedCoords = normalizedCoords * 0.5 + 0.5;
	if (normalizedCoords.z > 1.0f) return 0.0f;
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
	float currentDepth = normalizedCoords.z;
	float bias = 0.005f;
	float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
	return shadow;
}

void main() 
{
	computeLightComponents();
	
	vec3 baseColor = vec3(0.9f, 0.35f, 0.0f);//orange
	
	ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;

	float shadow = computeShadow();

	vec3 color = min((ambient + (1.0f - shadow) * diffuse) + (1.0f - shadow) * specular, 1.0f);

	vec3 pointColor = calculatePointLight();

	color = min(color + pointColor, vec3(1.0f));
    
    fColor = vec4(color, 1.0f);


}
