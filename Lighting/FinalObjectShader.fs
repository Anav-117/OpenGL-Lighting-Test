#version 330 core
#define NR_POINT_LIGHTS 2
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	sampler2D emission;
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

vec3 CalcDirLight(DirLight light, vec3 Normal, vec3 ViewDir);
vec3 CalcPointLight(PointLight light, vec3 Normal, vec3 FragPos, vec3 ViewDir);
vec3 CalcSpotLight(SpotLight light, vec3 Normal, vec3 FragPos, vec3 ViewDir);

float intensity = 0.0f;

void main(){
	vec3 norm = normalize(Normal);
	vec3 ViewDir = normalize(viewPos - FragPos);

	//Directional Lighting
	vec3 result = vec3(0.0f);//CalcDirLight(dirLight, norm, ViewDir);

	//Point Lights
	for (int i=0; i < NR_POINT_LIGHTS; i++) {
		result += CalcPointLight(pointLight[i], norm, FragPos, ViewDir);
	}

	//SpotLight
	//result += CalcSpotLight(spotLight, norm, FragPos, ViewDir);

	FragColor = vec4(result, 1.0f);//-intensity);
}

vec3 CalcDirLight(DirLight light, vec3 Normal, vec3 ViewDir) {
	vec3 lightDir = normalize(-light.direction);

	float diff = max(dot(Normal, lightDir), 0.0);
	vec3 reflectDir = reflect(-lightDir, Normal);
	float spec = pow(max(dot(ViewDir, reflectDir), 0.0), material.shininess);

	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord));
	vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord));

	return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 Normal, vec3 FragPos, vec3 ViewDir) {
	vec3 lightDir = normalize(light.position - FragPos);
	float distance = length(light.position - FragPos);

	float diff = max(dot(Normal, lightDir), 0.0);
	
	vec3 reflectDir = reflect(-lightDir, Normal);
	float spec = pow(max(dot(reflectDir, ViewDir), 0.0), material.shininess);

	float attenuation = 1.0 / (light.AttConstant + (light.AttLinear * distance) + (light.AttQuadratic * (distance * distance)));

	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord));
	vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}

vec3 CalcSpotLight (SpotLight light, vec3 Normal, vec3 FragPos, vec3 ViewDir) {
	vec3 lightDir = normalize(light.position - FragPos);
	float distance = length(light.position - FragPos);
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    
	if (dot(Normal, lightDir) > 0.0) {
		intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	}
	
	float diff = max(dot(Normal, lightDir), 0.0);
	
	vec3 reflectDir = reflect(-lightDir, Normal);
	float spec = pow(max(dot(reflectDir, ViewDir), 0.0), material.shininess);

	float attenuation = 1.0 / (light.AttConstant + light.AttLinear * distance + light.AttQuadratic * distance * distance);

	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord));
	vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	ambient *= intensity;
	diffuse *= intensity;
	specular *= intensity;

	return (ambient + diffuse + specular);
}