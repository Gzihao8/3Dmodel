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


//设置窗口大小
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

//摄像机
//FPSCamera camera(glm::vec3(0.0f, 0.0f, 3.0f));
SphereCamera camera(glm::vec3(1.0f, 1.0f, 0.0f));
glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
glm::mat4 view = camera.GetViewMatrix();
glm::mat4 model = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)), glm::vec3(1.0f, 1.0f, 1.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

//模型
extern Model ourModel;
//模型内的顶点
vector<Vertex> modelVertices;
//模型内的网格
vector<Mesh>modelMeshes;
//鼠标选中的顶点，用于绘制选中效果，以及绘制折线
vector<Vertex> mouseVertices;

//时间
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//光源位置
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

//模式开关
//鼠标吸附拣选模式
bool mousePickAbsorbMode = false;
//视角旋转模式
bool rotateMode = false;
//视角移动模式
bool moveMode = false;
//鼠标自由拣选模式
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
	//让GLFW捕捉鼠标
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	//glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//启动深度测试
	glEnable(GL_DEPTH_TEST);
	//启用深度偏移避免深度冲突问题
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);

	//着色器小程序
	//绘制物体的,将物体绘制成白色
	//若只有名字没有具体路径，只会在main.cpp同级目录寻找
	Shader objectShader("../Shader/vs/object.vs", "../Shader/fs/object.fs");
	//绘制顶点的，将顶点绘制成蓝色
	Shader blueVertexShader("../Shader/vs/object.vs", "../Shader/fs/blueVertex.fs");
	//绘制鼠标点击的顶点，绘制成红色
	Shader redVertexShader("../Shader/vs/object.vs", "../Shader/fs/redVertex.fs");

	//导入模型
	//Model ourModel("../Model/building/building.obj");
	//Model ourModel("../Model/baseboard.stl");
	//Model ourModel("../Model/wanxiangjie.stl");
	Model ourModel("../Model/chamber_4.0.stl");
	//Model ourModel("../Model/nanosuit.obj");
	//Model ourModel("../Model/colorized.ply");

	//将模型内的顶点保存出来，用于鼠标点击中判断点中哪个点
	for (int i = 0; i < ourModel.meshes.size(); i++)
		for (int j = 0; j < ourModel.meshes[i].vertices.size(); j++)
		{
			Vertex modelVertex;
			modelVertex.Position.x = ourModel.meshes[i].vertices[j].Position.x;
			modelVertex.Position.y = ourModel.meshes[i].vertices[j].Position.y;
			modelVertex.Position.z = ourModel.meshes[i].vertices[j].Position.z;
			modelVertices.push_back(modelVertex);
		}

	//将模型内的网格保存出来，用于鼠标点击中判断交点
	for (int i = 0; i < ourModel.meshes.size(); i++)
	{
		modelMeshes.push_back(ourModel.meshes[i]);
	}

	//渲染
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//输入
		processInput(window);

		//黑色背景
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//渲染加载的模型
		//每次渲染前需要更新一下视图矩阵，因为假若摄像机存在移动，视图矩阵会发生变化
		glm::mat4 view = camera.GetViewMatrix();
		//用objectShader渲染模型的网格
		objectShader.use();
		//投影转换有关的设置
		objectShader.setMat4("projection", projection);
		objectShader.setMat4("view", view);
		objectShader.setMat4("model", model);
		//渲染有关的设置
		//用于渲染的摄像机位置
		objectShader.setVec3("viewPos", camera.Position);
		//材质有关的设置,目前是铬合金
		objectShader.setVec3("material.ambient", glm::vec3(0.25f, 0.25f, 0.25f));
		objectShader.setVec3("material.diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
		objectShader.setVec3("material.specular", glm::vec3(0.774597f, 0.774507f, 0.774597f));
		objectShader.setFloat("material.shininess", 0.6f);
		//定向光设置
		objectShader.setVec3("dirlight.direction", glm::vec3(0.0f, 1.0f, 0.0f));
		objectShader.setVec3("dirlight.ambient", glm::vec3(2.0f, 2.0f, 2.0f));
		objectShader.setVec3("dirlight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
		objectShader.setVec3("dirlight.specular", glm::vec3(0.5f, 0.5f, 0.5f));
		//点光源设置
		objectShader.setVec3("pointLight.position", camera.Position);
		objectShader.setVec3("pointLight.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
		objectShader.setVec3("pointLight.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
		objectShader.setVec3("pointLight.specular", glm::vec3(0.5f, 0.5f, 0.5f));
		objectShader.setFloat("pointLight.constant", 1.0f);
		objectShader.setFloat("pointLight.linear", 0.09f);
		objectShader.setFloat("pointLight.quadratic", 0.032f);
		//渲染模型，包括光影效果
		ourModel.DrawMesh(objectShader);

		//用blueVertexShader渲染模型的顶点
		blueVertexShader.use();
		blueVertexShader.setMat4("projection", projection);
		blueVertexShader.setMat4("view", view);
		blueVertexShader.setMat4("model", model);
		//ourModel.DrawVertex(blueVertexShader);

		//渲染鼠标点击选中的顶点
		if ((mousePickAbsorbMode || mousePickFreeMode) && mouseVertices.size() != 0)
		{
			//顶点缓冲对象，顶点数组对象,索引缓冲对象
			unsigned int VBO, VAO;
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);

			//先绑定顶点数组对象
			glBindVertexArray(VAO);

			//绑定顶点缓冲对象
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			//将数据存入顶点缓冲对象所指的内存中
			glBufferData(GL_ARRAY_BUFFER, mouseVertices.size() * sizeof(Vertex), &mouseVertices[0], GL_STATIC_DRAW);

			//告诉OpenGL该如何解析顶点坐标数据
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
			//启用顶点位置属性
			glEnableVertexAttribArray(0);//这里的0应该是顶点着色器中location=0中的0

			//解绑顶点缓冲对象
			//glBindBuffer(GL_ARRAY_BUFFER, 0);

			//解绑顶点数组对象
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
	//Esc关闭窗口
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	//SD控制摄像机移动
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.CameraMove(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.CameraMove(RIGHT, deltaTime);

	//O键控制进入鼠标吸附拣选模式
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
		mousePickAbsorbMode = true;
	//P键退出鼠标吸附拣选模式，并清空选中的顶点
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
	{
		mousePickAbsorbMode = false;
		//清空元素，但不回收空间
		mouseVertices.clear();
	}
	//U键控制进入自由绘制模式
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
		mousePickFreeMode = true;
	//I键退出自由绘制模式，并清空选中的顶点
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
	{
		mousePickFreeMode = false;
		//清空元素，但不回收空间
		mouseVertices.clear();
	}

	//鼠标左键点击进入视角旋转模式
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		rotateMode = true;
	//鼠标左键松开退出视角旋转模式
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
		rotateMode = false;

	//鼠标中键点击进入视角平移模式
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
		moveMode = true;
	//鼠标中键松开退出视角平移模式
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

	//为鼠标点击函数更新鼠标现在的位置
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
		//需要根据当前视图矩阵来进行逆投影，所以也需要更新视图矩阵
		glm::mat4 view = camera.GetViewMatrix();

		glm::mat4 modelView = view * model;
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		{
			//逆投影得到近平面上的点
			glm::vec3 objectPosition = glm::unProject(glm::vec3(lastX, SCR_HEIGHT - lastY, 0.0), view, projection, glm::vec4(0.0f, 0.0f, SCR_WIDTH, SCR_HEIGHT));
			//逆投影得到远平面上的点
			glm::vec3 objectPosition2 = glm::unProject(glm::vec3(lastX, SCR_HEIGHT - lastY, 1.0), view, projection, glm::vec4(0.0f, 0.0f, SCR_WIDTH, SCR_HEIGHT));

			//鼠标点击后渲染上颜色的距离直线最近的顶点
			double minDistance = DBL_MAX;
			glm::vec3 closestPoint;
			//遍历所有顶点，寻找与直线距离最近的点
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
			//将找到的顶点的坐标添加进全局变量中，准备用于渲染成其他颜色
			mouseVertices.push_back(mouseVertex);
		}
	}
	else if (mousePickFreeMode)
	{
		//需要根据当前视图矩阵来进行逆投影，所以也需要更新视图矩阵
		glm::mat4 view = camera.GetViewMatrix();

		glm::mat4 modelView = view * model;
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		{
			glm::vec3 objectPosition = glm::unProject(glm::vec3(lastX, SCR_HEIGHT - lastY, 0.0), view, projection, glm::vec4(0.0f, 0.0f, SCR_WIDTH, SCR_HEIGHT));
			//射线原点
			glm::vec3 O = camera.Position;
			//射线方向向量
			glm::vec3 D = glm::normalize(objectPosition - O);

			//用于进行深度测试，需要取深度最小的点
			double minDistance = DBL_MAX;
			glm::vec3 closestPoint;
			//鼠标点击后求解射线与三角形相交处的交点，通过遍历求得
			//每一个Mesh
			for (int i = 0; i < modelMeshes.size(); i++)
			{
				unsigned int* vertexPointer = &modelMeshes[i].indices[0];
				//每一个Mesh里的每一个三角形
				for (int j = 0; j < modelMeshes[i].indices.size(); j = j + 3)
				{
					//每一个三角形所在平面与射线进行相交得到交点，并判断交点是否在三角形内
					//三角形的三个顶点
					glm::vec3 V1 = glm::vec3(modelMeshes[i].vertices[*(vertexPointer + j)].Position.x, modelMeshes[i].vertices[*(vertexPointer + j)].Position.y, modelMeshes[i].vertices[*(vertexPointer + j)].Position.z);
					glm::vec3 V2 = glm::vec3(modelMeshes[i].vertices[*(vertexPointer + j + 1)].Position.x, modelMeshes[i].vertices[*(vertexPointer + j + 1)].Position.y, modelMeshes[i].vertices[*(vertexPointer + j + 1)].Position.z);
					glm::vec3 V3 = glm::vec3(modelMeshes[i].vertices[*(vertexPointer + j + 2)].Position.x, modelMeshes[i].vertices[*(vertexPointer + j + 2)].Position.y, modelMeshes[i].vertices[*(vertexPointer + j + 2)].Position.z);
					//交点
					glm::vec3 I;

					//如果存在交点，且交点在三角形内
					if (RayTriangleIntersection(V1, V2, V3, camera.Position, D, &I))
					{
						//对交点进行深度测试。深度=交点与摄像机位置之间的距离。
						double distance = glm::length(I - camera.Position);
						if (distance < minDistance)
						{
							minDistance = distance;
							closestPoint = I;
						}
					}

				}
			}
			//将交点添加进容器内，用于渲染
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
	//纹理对象
	unsigned int textureID;
	//texture
	glGenTextures(1, &textureID);
	//加载并生成纹理
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

		//绑定纹理对象
		glBindTexture(GL_TEXTURE_2D, textureID);
		//生成纹理贴图
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//为当前绑定的纹理对象设置环绕
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		//为当前绑定的纹理对象设置过滤方式
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//释放内存
		stbi_image_free(data);
	}
	else
	{
		std::cout << "Failed to load texture at path " << path << std::endl;
		//释放图像的内存
		stbi_image_free(data);
	}

	return textureID;
}
/*
函数功能：求解C点距直线AB的距离
参数说明：AB直线，C点
返回值：C点距直线AB的距离
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
函数功能：求解射线与三角形的交点
参数说明：V1,V2,V3,三角形三点；O,射线原点；D,射线方向。
返回值：
	ture：交点存在且位于三角形内部，交点坐标存在I内。
	false：交点不存在或交点不位于三角形内部
*/
bool RayTriangleIntersection(glm::vec3 V1, glm::vec3 V2, glm::vec3 V3, glm::vec3 O, glm::vec3 D, glm::vec3 *I)
{
	glm::vec3 E1 = V2 - V1;
	glm::vec3 E2 = V3 - V1;

	//开始计算行列式
	glm::vec3 P = glm::cross(D, E2);
	//若行列式接近零，则射线与三角形平面平行
	double det = glm::dot(P, E1);

	if (det > -EPSILON && det < EPSILON)
	{
		return false;
	}

	double inv_det = 1.f / det;

	glm::vec3 T = O - V1;

	//计算参数u
	double u = glm::dot(P, T) * inv_det;
	//判断交点是否在三角形内
	if (u < 0.f || u > 1.f)
	{
		return false;
	}

	glm::vec3 Q = glm::cross(T, E1);

	//计算参数v
	double v = glm::dot(Q, D)* inv_det;

	//判断交点是否在三角形内
	if (v < 0.f || u + v  > 1.f)
	{
		return false;
	}

	//计算参数t
	double t = glm::dot(Q, E2)* inv_det;

	//计算交点
	if (t > EPSILON)
	{
		*I = O + D * glm::vec3(t, t, t);
		return true;
	}

	return false;
}