#version 330 core
#define NR_POINT_LIGHTS 4
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	float shininess;
};

struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
	vec3 position;

	float AttConstant;
	float AttLinear;
	float AttQuadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct SpotLight {
	vec3 position;
	vec3 direction;

	float cutOff; 
	float outerCutOff;
	float AttConstant;
	float AttLinear;
	float AttQuadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLight[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform vec3 viewPos;
float shininess;

vec3 CalcDirLight(DirLight light, vec3 Normal, vec3 ViewDir);
vec3 CalcPointLight(PointLight light, vec3 Normal, vec3 FragPos, vec3 ViewDir);
vec3 CalcSpotLight(SpotLight light, vec3 Normal, vec3 FragPos, vec3 ViewDir);

void main(){
	vec3 norm = normalize(Normal);
	vec3 ViewDir = normalize(viewPos - FragPos);

	shininess = 32 * length(vec3(texture(material.texture_specular1, TexCoord)));

	//Directional Lighting
	vec3 result = CalcDirLight(dirLight, norm, ViewDir);

	//Point Lights
	for (int i=0; i < NR_POINT_LIGHTS; i++) {
		result += CalcPointLight(pointLight[i], norm, FragPos, ViewDir);
	}

	//SpotLight
	//result += CalcSpotLight(spotLight, norm, FragPos, ViewDir);

	FragColor = vec4(result, 1.0f);
}

vec3 CalcDirLight(DirLight light, vec3 Normal, vec3 ViewDir) {
	vec3 lightDir = normalize(-light.direction);
	vec3 halfwayDir = normalize(lightDir + ViewDir);

	float diff = max(dot(Normal, lightDir), 0.0);

	float spec = pow(max(dot(Normal, halfwayDir), 0.0), shininess);

	if (diff == 0.0f){
		spec = 0.0f;
	}

	vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoord));
	vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoord));

	return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 Normal, vec3 FragPos, vec3 ViewDir) {
	vec3 lightDir = normalize(light.position - FragPos);
	vec3 halfwayDir = normalize(lightDir + ViewDir);
	float distance = length(light.position - FragPos);

	float diff = max(dot(Normal, lightDir), 0.0);
	
	float spec = pow(max(dot(Normal, halfwayDir), 0.0), shininess);

	if (diff == 0.0f) {
		spec = 0.0f;
	}

	float attenuation = 1.0 / (light.AttConstant + (light.AttLinear * distance) + (light.AttQuadratic * (distance * distance)));

	vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoord));
	vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoord));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}

vec3 CalcSpotLight (SpotLight light, vec3 Normal, vec3 FragPos, vec3 ViewDir) {
	vec3 lightDir = normalize(light.position - FragPos);
	vec3 halfwayDir = normalize(lightDir + ViewDir);
	float distance = length(light.position - FragPos);
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
		
	float diff = max(dot(Normal, lightDir), 0.0);

	float spec = pow(max(dot(Normal, halfwayDir), 0.0), shininess);

	float attenuation = 1.0 / (light.AttConstant + light.AttLinear * distance + light.AttQuadratic * distance * distance);

	vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoord));
	vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoord));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	ambient *= intensity;
	diffuse *= intensity;
	specular *= intensity;

	return (ambient + diffuse + specular);
}