#version 330 core

//�����
struct DirLight{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
//���Դ
struct PointLight{
	vec3 position;

	//˥��
	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
//����
struct Material{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
uniform vec3 viewPos;
uniform Material material;
uniform PointLight pointLight;
uniform DirLight dirlight;


vec3 CalcDirLight(DirLight light,vec3 normal,vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	//����
	//������
	vec3 norm = normalize(Normal);
	//����
	vec3 viewDir = normalize(viewPos - FragPos);

	vec3 result;
	//�������
	//result = CalcDirLight(dirlight,norm,viewDir);
	//���Դ
	result = CalcPointLight(pointLight,norm,FragPos,viewDir);

	FragColor = vec4(result,1.0);
}

//�����
vec3 CalcDirLight(DirLight light,vec3 normal,vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);
	//������
	float diff = max(dot(normal,lightDir),0.0);
	//�����
	vec3 reflectDir = reflect(-lightDir,normal);
	float spec = pow(max(dot(viewDir,reflectDir),0.0),material.shininess);
	
	//�ϲ�
	vec3 ambient = light.ambient * material.ambient;
	vec3 diffuse = light.diffuse * diff * material.diffuse;
	vec3 specular = light.specular * spec * material.specular;

	return (ambient + diffuse + specular);
}
//���Դ
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);

	//������
	float diff = max(dot(normal,lightDir),0.0);

	//�����
	vec3 reflectDir = reflect(-lightDir,normal);
	float spec = pow(max(dot(viewDir,reflectDir),0.0),material.shininess);

	//˥��
	float distance = length(light.position - fragPos);
	float attenuation = 1.0/(light.constant + light.linear * distance + light.quadratic * (distance * distance));

	//�ϲ����
	vec3 ambient = light.ambient * material.ambient;
	vec3 diffuse = light.diffuse * diff * material.diffuse;
	vec3 specular = light.specular * spec * material.specular;

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}