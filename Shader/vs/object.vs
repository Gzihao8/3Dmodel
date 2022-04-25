#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;
	//物体坐标系——>世界坐标系——>摄像机坐标系——>屏幕坐标系
	//三维点坐标——————————————————>屏幕像素点坐标
    gl_Position = projection * view * model * vec4(aPos, 1.0);
	FragPos = vec3(model * vec4(aPos,1));
	Normal = mat3(transpose(inverse(model))) * aNormal;
}