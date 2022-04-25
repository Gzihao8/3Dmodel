#include<glad/glad.h>

#include<GLFW/glfw3.h>

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include"shader.h"
//#include"FPScamera.h"
#include"sphereCamera.h"
#include"model.h"

#include<iostream>


#define EPSILON 0.0001

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow *window);
unsigned int loadTexture(char const* path);
double DistanceOfPointToLine(glm::vec3 A, glm::vec3 B, glm::vec3 C);
bool RayTriangleIntersection(glm::vec3 V1, glm::vec3 V2, glm::vec3 V3, glm::vec3 O, glm::vec3 D, glm::vec3 *I);


//���ô��ڴ�С
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

//�����
//FPSCamera camera(glm::vec3(0.0f, 0.0f, 3.0f));
SphereCamera camera(glm::vec3(1.0f, 1.0f, 0.0f));
glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
glm::mat4 view = camera.GetViewMatrix();
glm::mat4 model = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)), glm::vec3(1.0f, 1.0f, 1.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

//ģ��
extern Model ourModel;
//ģ���ڵĶ���
vector<Vertex> modelVertices;
//ģ���ڵ�����
vector<Mesh>modelMeshes;
//���ѡ�еĶ��㣬���ڻ���ѡ��Ч�����Լ���������
vector<Vertex> mouseVertices;

//ʱ��
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//��Դλ��
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

//ģʽ����
//���������ѡģʽ
bool mousePickAbsorbMode = false;
//�ӽ���תģʽ
bool rotateMode = false;
//�ӽ��ƶ�ģʽ
bool moveMode = false;
//������ɼ�ѡģʽ
bool mousePickFreeMode = false;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "learnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	//��GLFW��׽���
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	//glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//������Ȳ���
	glEnable(GL_DEPTH_TEST);
	//�������ƫ�Ʊ�����ȳ�ͻ����
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);

	//��ɫ��С����
	//���������,��������Ƴɰ�ɫ
	//��ֻ������û�о���·����ֻ����main.cppͬ��Ŀ¼Ѱ��
	Shader objectShader("../Shader/vs/object.vs", "../Shader/fs/object.fs");
	//���ƶ���ģ���������Ƴ���ɫ
	Shader blueVertexShader("../Shader/vs/object.vs", "../Shader/fs/blueVertex.fs");
	//����������Ķ��㣬���Ƴɺ�ɫ
	Shader redVertexShader("../Shader/vs/object.vs", "../Shader/fs/redVertex.fs");

	//����ģ��
	//Model ourModel("../Model/building/building.obj");
	//Model ourModel("../Model/baseboard.stl");
	//Model ourModel("../Model/wanxiangjie.stl");
	Model ourModel("../Model/chamber_4.0.stl");
	//Model ourModel("../Model/nanosuit.obj");
	//Model ourModel("../Model/colorized.ply");

	//��ģ���ڵĶ��㱣�������������������жϵ����ĸ���
	for (int i = 0; i < ourModel.meshes.size(); i++)
		for (int j = 0; j < ourModel.meshes[i].vertices.size(); j++)
		{
			Vertex modelVertex;
			modelVertex.Position.x = ourModel.meshes[i].vertices[j].Position.x;
			modelVertex.Position.y = ourModel.meshes[i].vertices[j].Position.y;
			modelVertex.Position.z = ourModel.meshes[i].vertices[j].Position.z;
			modelVertices.push_back(modelVertex);
		}

	//��ģ���ڵ����񱣴������������������жϽ���
	for (int i = 0; i < ourModel.meshes.size(); i++)
	{
		modelMeshes.push_back(ourModel.meshes[i]);
	}

	//��Ⱦ
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//����
		processInput(window);

		//��ɫ����
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//��Ⱦ���ص�ģ��
		//ÿ����Ⱦǰ��Ҫ����һ����ͼ������Ϊ��������������ƶ�����ͼ����ᷢ���仯
		glm::mat4 view = camera.GetViewMatrix();
		//��objectShader��Ⱦģ�͵�����
		objectShader.use();
		//ͶӰת���йص�����
		objectShader.setMat4("projection", projection);
		objectShader.setMat4("view", view);
		objectShader.setMat4("model", model);
		//��Ⱦ�йص�����
		//������Ⱦ�������λ��
		objectShader.setVec3("viewPos", camera.Position);
		//�����йص�����,Ŀǰ�Ǹ��Ͻ�
		objectShader.setVec3("material.ambient", glm::vec3(0.25f, 0.25f, 0.25f));
		objectShader.setVec3("material.diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
		objectShader.setVec3("material.specular", glm::vec3(0.774597f, 0.774507f, 0.774597f));
		objectShader.setFloat("material.shininess", 0.6f);
		//���������
		objectShader.setVec3("dirlight.direction", glm::vec3(0.0f, 1.0f, 0.0f));
		objectShader.setVec3("dirlight.ambient", glm::vec3(2.0f, 2.0f, 2.0f));
		objectShader.setVec3("dirlight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
		objectShader.setVec3("dirlight.specular", glm::vec3(0.5f, 0.5f, 0.5f));
		//���Դ����
		objectShader.setVec3("pointLight.position", camera.Position);
		objectShader.setVec3("pointLight.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
		objectShader.setVec3("pointLight.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
		objectShader.setVec3("pointLight.specular", glm::vec3(0.5f, 0.5f, 0.5f));
		objectShader.setFloat("pointLight.constant", 1.0f);
		objectShader.setFloat("pointLight.linear", 0.09f);
		objectShader.setFloat("pointLight.quadratic", 0.032f);
		//��Ⱦģ�ͣ�������ӰЧ��
		ourModel.DrawMesh(objectShader);

		//��blueVertexShader��Ⱦģ�͵Ķ���
		blueVertexShader.use();
		blueVertexShader.setMat4("projection", projection);
		blueVertexShader.setMat4("view", view);
		blueVertexShader.setMat4("model", model);
		//ourModel.DrawVertex(blueVertexShader);

		//��Ⱦ�����ѡ�еĶ���
		if ((mousePickAbsorbMode || mousePickFreeMode) && mouseVertices.size() != 0)
		{
			//���㻺����󣬶����������,�����������
			unsigned int VBO, VAO;
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);

			//�Ȱ󶨶����������
			glBindVertexArray(VAO);

			//�󶨶��㻺�����
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			//�����ݴ��붥�㻺�������ָ���ڴ���
			glBufferData(GL_ARRAY_BUFFER, mouseVertices.size() * sizeof(Vertex), &mouseVertices[0], GL_STATIC_DRAW);

			//����OpenGL����ν���������������
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
			//���ö���λ������
			glEnableVertexAttribArray(0);//�����0Ӧ���Ƕ�����ɫ����location=0�е�0

			//��󶥵㻺�����
			//glBindBuffer(GL_ARRAY_BUFFER, 0);

			//��󶥵��������
			glBindVertexArray(0);

			redVertexShader.use();

			redVertexShader.setMat4("projection", projection);
			redVertexShader.setMat4("view", view);
			redVertexShader.setMat4("model", model);

			glBindVertexArray(VAO);
			glPointSize(15);
			glDrawArrays(GL_POINTS, 0, mouseVertices.size());
			glLineWidth(10);
			glDrawArrays(GL_LINE_STRIP, 0, mouseVertices.size());
		}
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window)
{
	//Esc�رմ���
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	//SD����������ƶ�
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.CameraMove(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.CameraMove(RIGHT, deltaTime);

	//O�����ƽ������������ѡģʽ
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
		mousePickAbsorbMode = true;
	//P���˳����������ѡģʽ�������ѡ�еĶ���
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
	{
		mousePickAbsorbMode = false;
		//���Ԫ�أ��������տռ�
		mouseVertices.clear();
	}
	//U�����ƽ������ɻ���ģʽ
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
		mousePickFreeMode = true;
	//I���˳����ɻ���ģʽ�������ѡ�еĶ���
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
	{
		mousePickFreeMode = false;
		//���Ԫ�أ��������տռ�
		mouseVertices.clear();
	}

	//��������������ӽ���תģʽ
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		rotateMode = true;
	//�������ɿ��˳��ӽ���תģʽ
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
		rotateMode = false;

	//����м���������ӽ�ƽ��ģʽ
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
		moveMode = true;
	//����м��ɿ��˳��ӽ�ƽ��ģʽ
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_RELEASE)
		moveMode = false;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	//Ϊ�������������������ڵ�λ��
	lastX = xpos;
	lastY = ypos;

	if (rotateMode)
		camera.ProcessMouseRotate(xoffset, yoffset);
	if (moveMode)
		camera.ProcessMouseMovement(xoffset, yoffset, deltaTime);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (yoffset > 0)
		camera.CameraMove(FORWARD, deltaTime);
	else
		camera.CameraMove(BACKWARD, deltaTime);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (mousePickAbsorbMode)
	{
		//��Ҫ���ݵ�ǰ��ͼ������������ͶӰ������Ҳ��Ҫ������ͼ����
		glm::mat4 view = camera.GetViewMatrix();

		glm::mat4 modelView = view * model;
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		{
			//��ͶӰ�õ���ƽ���ϵĵ�
			glm::vec3 objectPosition = glm::unProject(glm::vec3(lastX, SCR_HEIGHT - lastY, 0.0), view, projection, glm::vec4(0.0f, 0.0f, SCR_WIDTH, SCR_HEIGHT));
			//��ͶӰ�õ�Զƽ���ϵĵ�
			glm::vec3 objectPosition2 = glm::unProject(glm::vec3(lastX, SCR_HEIGHT - lastY, 1.0), view, projection, glm::vec4(0.0f, 0.0f, SCR_WIDTH, SCR_HEIGHT));

			//���������Ⱦ����ɫ�ľ���ֱ������Ķ���
			double minDistance = DBL_MAX;
			glm::vec3 closestPoint;
			//�������ж��㣬Ѱ����ֱ�߾�������ĵ�
			for (int i = 0; i < modelVertices.size(); i++)
			{
				glm::vec3 point;
				point.x = modelVertices[i].Position.x;
				point.y = modelVertices[i].Position.y;
				point.z = modelVertices[i].Position.z;

				double distance = DistanceOfPointToLine(objectPosition, objectPosition2, point);
				if (distance < minDistance)
				{
					minDistance = distance;
					closestPoint = point;
				}
			}
			Vertex mouseVertex;
			mouseVertex.Position.x = closestPoint.x;
			mouseVertex.Position.y = closestPoint.y;
			mouseVertex.Position.z = closestPoint.z;
			cout << "(" << closestPoint.x << "," << closestPoint.y << "," << closestPoint.z << ")" << endl;
			//���ҵ��Ķ����������ӽ�ȫ�ֱ����У�׼��������Ⱦ��������ɫ
			mouseVertices.push_back(mouseVertex);
		}
	}
	else if (mousePickFreeMode)
	{
		//��Ҫ���ݵ�ǰ��ͼ������������ͶӰ������Ҳ��Ҫ������ͼ����
		glm::mat4 view = camera.GetViewMatrix();

		glm::mat4 modelView = view * model;
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		{
			glm::vec3 objectPosition = glm::unProject(glm::vec3(lastX, SCR_HEIGHT - lastY, 0.0), view, projection, glm::vec4(0.0f, 0.0f, SCR_WIDTH, SCR_HEIGHT));
			//����ԭ��
			glm::vec3 O = camera.Position;
			//���߷�������
			glm::vec3 D = glm::normalize(objectPosition - O);

			//���ڽ�����Ȳ��ԣ���Ҫȡ�����С�ĵ�
			double minDistance = DBL_MAX;
			glm::vec3 closestPoint;
			//�����������������������ཻ���Ľ��㣬ͨ���������
			//ÿһ��Mesh
			for (int i = 0; i < modelMeshes.size(); i++)
			{
				unsigned int* vertexPointer = &modelMeshes[i].indices[0];
				//ÿһ��Mesh���ÿһ��������
				for (int j = 0; j < modelMeshes[i].indices.size(); j = j + 3)
				{
					//ÿһ������������ƽ�������߽����ཻ�õ����㣬���жϽ����Ƿ�����������
					//�����ε���������
					glm::vec3 V1 = glm::vec3(modelMeshes[i].vertices[*(vertexPointer + j)].Position.x, modelMeshes[i].vertices[*(vertexPointer + j)].Position.y, modelMeshes[i].vertices[*(vertexPointer + j)].Position.z);
					glm::vec3 V2 = glm::vec3(modelMeshes[i].vertices[*(vertexPointer + j + 1)].Position.x, modelMeshes[i].vertices[*(vertexPointer + j + 1)].Position.y, modelMeshes[i].vertices[*(vertexPointer + j + 1)].Position.z);
					glm::vec3 V3 = glm::vec3(modelMeshes[i].vertices[*(vertexPointer + j + 2)].Position.x, modelMeshes[i].vertices[*(vertexPointer + j + 2)].Position.y, modelMeshes[i].vertices[*(vertexPointer + j + 2)].Position.z);
					//����
					glm::vec3 I;

					//������ڽ��㣬�ҽ�������������
					if (RayTriangleIntersection(V1, V2, V3, camera.Position, D, &I))
					{
						//�Խ��������Ȳ��ԡ����=�����������λ��֮��ľ��롣
						double distance = glm::length(I - camera.Position);
						if (distance < minDistance)
						{
							minDistance = distance;
							closestPoint = I;
						}
					}

				}
			}
			//��������ӽ������ڣ�������Ⱦ
			Vertex mouseVertex;
			mouseVertex.Position.x = closestPoint.x;
			mouseVertex.Position.y = closestPoint.y;
			mouseVertex.Position.z = closestPoint.z;
			cout << "(" << closestPoint.x << "," << closestPoint.y << "," << closestPoint.z << ")" << endl;
			mouseVertices.push_back(mouseVertex);
		}
	}
}
unsigned int loadTexture(char const* path)
{
	//�������
	unsigned int textureID;
	//texture
	glGenTextures(1, &textureID);
	//���ز���������
	int width, height, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (data)
	{
		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		if (nrChannels == 3)
			format = GL_RGB;
		if (nrChannels == 4)
			format = GL_RGBA;

		//���������
		glBindTexture(GL_TEXTURE_2D, textureID);
		//����������ͼ
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//Ϊ��ǰ�󶨵�����������û���
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		//Ϊ��ǰ�󶨵�����������ù��˷�ʽ
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//�ͷ��ڴ�
		stbi_image_free(data);
	}
	else
	{
		std::cout << "Failed to load texture at path " << path << std::endl;
		//�ͷ�ͼ����ڴ�
		stbi_image_free(data);
	}

	return textureID;
}
/*
�������ܣ����C���ֱ��AB�ľ���
����˵����ABֱ�ߣ�C��
����ֵ��C���ֱ��AB�ľ���
*/
double DistanceOfPointToLine(glm::vec3 A, glm::vec3 B, glm::vec3 C)
{
	glm::vec3 AB = B - A;
	glm::vec3 AC = C - A;

	glm::vec3 ABcrossAC = glm::cross(AB, AC);

	double distance = glm::length(ABcrossAC) / glm::length(AB);

	return distance;
}
/*
�������ܣ���������������εĽ���
����˵����V1,V2,V3,���������㣻O,����ԭ�㣻D,���߷���
����ֵ��
	ture�����������λ���������ڲ��������������I�ڡ�
	false�����㲻���ڻ򽻵㲻λ���������ڲ�
*/
bool RayTriangleIntersection(glm::vec3 V1, glm::vec3 V2, glm::vec3 V3, glm::vec3 O, glm::vec3 D, glm::vec3 *I)
{
	glm::vec3 E1 = V2 - V1;
	glm::vec3 E2 = V3 - V1;

	//��ʼ��������ʽ
	glm::vec3 P = glm::cross(D, E2);
	//������ʽ�ӽ��㣬��������������ƽ��ƽ��
	double det = glm::dot(P, E1);

	if (det > -EPSILON && det < EPSILON)
	{
		return false;
	}

	double inv_det = 1.f / det;

	glm::vec3 T = O - V1;

	//�������u
	double u = glm::dot(P, T) * inv_det;
	//�жϽ����Ƿ�����������
	if (u < 0.f || u > 1.f)
	{
		return false;
	}

	glm::vec3 Q = glm::cross(T, E1);

	//�������v
	double v = glm::dot(Q, D)* inv_det;

	//�жϽ����Ƿ�����������
	if (v < 0.f || u + v  > 1.f)
	{
		return false;
	}

	//�������t
	double t = glm::dot(Q, E2)* inv_det;

	//���㽻��
	if (t > EPSILON)
	{
		*I = O + D * glm::vec3(t, t, t);
		return true;
	}

	return false;
}