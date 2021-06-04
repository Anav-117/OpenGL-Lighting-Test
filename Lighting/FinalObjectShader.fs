#version 330 core
#define NR_POINT_LIGHTS 1
out vec4 FragColor;

in VS_OUT {
	vec3 Normal;
	vec3 FragPos;
	vec2 TexCoord;
	vec4 FragPosLightSpace;
} fs_in;

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

uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLight[NR_POINT_LIGHTS];
uniform sampler2D shadowMap;
uniform samplerCube shadowCube;
uniform vec3 viewPos;
uniform float far_plane;
float shininess;

vec3 CalcDirLight(DirLight light, vec3 Normal, vec3 ViewDir);
vec3 CalcPointLight(PointLight light, vec3 Normal, vec3 FragPos, vec3 ViewDir);
float GenerateDirectionalShadow(vec4 fragPosLightSpace);
float GeneratePointShadows(PointLight light);

float bias = 0.005;
float randomNumber(vec3 pos, float index);

vec3 poissonDisk[16] = vec3[]( 
   vec3( -0.94201624, -0.39906216, 0.81544232), 
   vec3( 0.94558609, -0.76890725, 0.34495938), 
   vec3( -0.094184101, -0.92938870, 0.99706507), 
   vec3( 0.34495938, 0.29387760, -0.38277543), 
   vec3( -0.91588581, 0.45771432, -0.14100790), 
   vec3( -0.81544232, -0.87912464, 0.44323325), 
   vec3( -0.38277543, 0.27676845, 0.14383161), 
   vec3( 0.97484398, 0.75648379, -0.76890725), 
   vec3( 0.44323325, -0.97511554, -0.81544232), 
   vec3( 0.53742981, -0.47373420, 0.29387760), 
   vec3( -0.26496911, -0.41893023, 0.99706507), 
   vec3( 0.79197514, 0.19090188, 0.27676845), 
   vec3( -0.24188840, 0.99706507, 0.094184101), 
   vec3( -0.81409955, 0.91437590,0.75648379), 
   vec3( 0.19984126, 0.78641367, -0.38277543), 
   vec3( 0.14383161, -0.14100790, -0.094184101) 
);
void main(){
	vec3 norm = normalize(fs_in.Normal);
	vec3 ViewDir = normalize(viewPos - fs_in.FragPos);
	float gamma = 2.2;

	shininess = 32 * length(vec3(texture(material.texture_specular1, fs_in.TexCoord)));

	//Directional Lighting
	vec3 result = CalcDirLight(dirLight, norm, ViewDir);
	//vec3 result = vec3(0.0f);

	//Point Lights
	for (int i=0; i < NR_POINT_LIGHTS; i++) {
		result += CalcPointLight(pointLight[i], norm, fs_in.FragPos, ViewDir);
	}

	FragColor = vec4(result, 1.0f);
	//FragColor = vec4(pow(result, vec3(1.0/gamma)), 1.0f); // Gamma Corrected Output
}

vec3 CalcDirLight(DirLight light, vec3 Normal, vec3 ViewDir) {
	vec3 lightDir = normalize(light.direction);
	//vec3 lightDir = normalize(light.direction - fs_in.FragPos);
	vec3 halfwayDir = normalize(lightDir + ViewDir);

	float diff = max(dot(Normal, lightDir), 0.0);

	float spec = pow(max(dot(Normal, halfwayDir), 0.0), shininess);

	if (diff == 0.0f){
		spec = 0.0f;
	}

	vec4 textureColour = texture(material.texture_diffuse1, fs_in.TexCoord);
	if(textureColour.a < 0.5) {
		discard;
	}

	vec3 ambient = light.ambient * vec3(textureColour);
	vec3 diffuse = light.diffuse * diff * vec3(textureColour);
	vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, fs_in.TexCoord));

	bias = max(0.05 * (1.0 - dot(Normal, lightDir)), 0.005);

	float shadow = GenerateDirectionalShadow(fs_in.FragPosLightSpace);

	diffuse *= (1.0 - shadow);
	specular *= (1.0 - shadow);

	return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 Normal, vec3 FragPos, vec3 ViewDir) {
	vec3 lightDir = normalize(light.position - fs_in.FragPos);
	vec3 halfwayDir = normalize(lightDir + ViewDir);
	float distance = length(light.position - fs_in.FragPos);

	float diff = max(dot(Normal, lightDir), 0.0);
	
	float spec = pow(max(dot(Normal, halfwayDir), 0.0), shininess);

	if (diff == 0.0f) {
		spec = 0.0f;
	}

	float attenuation = 1.0 / (light.AttConstant + (light.AttLinear * distance) + (light.AttQuadratic * (distance * distance)));
	
	vec4 textureColour = texture(material.texture_diffuse1, fs_in.TexCoord);
	if(textureColour.a < 0.5) {
		discard;
	}

	vec3 ambient = light.ambient * vec3(textureColour);
	vec3 diffuse = light.diffuse * diff * vec3(textureColour);
	vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, fs_in.TexCoord));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	float shadow = GeneratePointShadows(light);

	diffuse *= (1.0 - shadow);
	specular *= (1.0 - shadow);

	return (ambient + diffuse + specular);
}

float GenerateDirectionalShadow(vec4 fragPosLightSpace) {
	//perspective divide
	vec3 projCoord = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoord = projCoord * 0.5 + 0.5;
	float shadow = 0.0;
	float currentDepth = projCoord.z;
	float spread = 800.0;

	for (int i=0; i<4; i++) {
		int index = int(16.0*randomNumber(fs_in.FragPos.xyz, i))%16;
		float depth = texture(shadowMap, vec2(projCoord.xy + poissonDisk[index].xy/spread)).r;
		shadow += ((currentDepth - bias)> depth) ? 0.2 : 0.0;
	}

	//for (int x=-1; x<=1; ++x) {
	//	for (int y=-1; y<=1; ++y) {
	//		float pcfDepth = texture(shadowMap, projCoord.xy + (vec2(x, y) * texelSize)).r;
	//		shadow += ((currentDepth - bias)> pcfDepth) ? 1.0 : 0.0;
	//	}
	//}

	//shadow /= 9.0;

	if (projCoord.z > 1.0) {
		shadow = 0.0;
	}
	
	return shadow;
}

float randomNumber(vec3 pos, float index) {
	vec4 seed4 = vec4(pos, index);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}

float GeneratePointShadows(PointLight light) {
	vec3 fragToLight = fs_in.FragPos - light.position;
	float closestDepth = texture(shadowCube, fragToLight).r;
	closestDepth *= far_plane;
	float currentDepth = length(fragToLight);

	float bias = 0.05; 
	float shadow = 0.0; //currentDepth -  bias > closestDepth ? 1.0 : 0.0;
	float spread = 100;

	for (int i=0; i<4; i++) {
		int index = int(16.0*randomNumber(fs_in.FragPos.xyz, i))%16;
		float depth = texture(shadowCube, vec3(fragToLight + poissonDisk[index]/spread)).r;
		depth *= far_plane;
		shadow += ((currentDepth - bias) > depth) ? 0.2 : 0.0;
	}

	//FragColor = vec4(vec3(closestDepth / far_plane), 1.0);

	return shadow;
}