#pragma once
#ifndef SPHERECAMERA_H
#define SPHERECAMERA_H

#include<glad/glad.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

#include<iostream>

//#include<vector>

//用枚举类型定义几个可能的摄像机运动方向
enum Camera_Movement {
	FORWARD,//0
	BACKWARD,//1
	LEFT,//2
	RIGHT,//3
	UP,//4
	DOWN//5
};

//定义默认的摄像机数值
//偏航角
const float YAW = -90.0f;
//俯仰角
const float PITCH = 0.0f;
//移动速度
const float SPEED = 15.0f;
//灵敏度
const float SENSITIVITY = 0.01f;
//缩放
const float ZOOM = 45.0f;

//摄像机类
class SphereCamera
{
	//属性
public:
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	//第一人称
	//float Yaw;
	//float Pitch;

	//球坐标系
	//天顶角,角度
	float Zenith;
	//方位角
	float Azimuth;
	float R;

	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	//构造函数(用向量构造)
	SphereCamera(glm::vec3 position, glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f)) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		R = glm::length(glm::vec3(this->Position.x, this->Position.y, this->Position.z));
		Zenith = glm::degrees(acos(this->Position.y / R));
		Azimuth = glm::degrees(atan(this->Position.x / this->Position.z));

		updataCameraVectors();
	}

	//方法

	//计算用lookAt得出的视图矩阵
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

	//移动摄像机
	void CameraMove(Camera_Movement direction, float deltaTime)
	{
		//速度
		float velocity = MovementSpeed * deltaTime;
		//前后左右上下
		if (direction == FORWARD)
			Position += Front * velocity;
		if (direction == BACKWARD)
			Position -= Front * velocity;
		if (direction == LEFT)
			Position -= Right * velocity;
		if (direction == RIGHT)
			Position += Right * velocity;
		if (direction == UP)
			Position += Up * velocity;
		if (direction == DOWN)
			Position -= Up * velocity;

		//改变完摄像机位置后需要重新计算球坐标系的半径参数R
		this->R = glm::length(glm::vec3(this->Position.x, this->Position.y, this->Position.z));
		std::cout << "(" << this->Position.x << "," << this->Position.y << "," << this->Position.z << ")" << std::endl;
	}

	//鼠标旋转摄像机视角
	void ProcessMouseRotate(float xoffset, float yoffset)
	{
		//xoffset *= MouseSensitivity;
		//yoffset *= MouseSensitivity;

		//限定方位角和天顶角变化时的循环过程
		//这一步在+-offset前做会在天顶角由0最后一减时出现一瞬间的图像突变，但下一帧就回复正常
		if (Azimuth < 0)Azimuth = 360 - abs(Azimuth);
		else Azimuth = (int)Azimuth % 360;

		if (Zenith < 0)Zenith = 360 - abs(Zenith);
		else Zenith = (int)Zenith % 360;

		//由天顶角是否大于180可知摄像机此时的姿态，决定鼠标的移动如何增减角度
		if (Zenith > 180)
		{
			Azimuth += xoffset;
			Zenith += yoffset;
		}
		else
		{
			Azimuth -= xoffset;
			Zenith += yoffset;
		}

		//std::cout << "xoffset:" << xoffset << std::endl;
		//std::cout << "yoffset:" << yoffset << std::endl;

		//std::cout << "Azimuth:" << Azimuth << std::endl;
		//std::cout << "Zenith:" << Zenith << std::endl;

		this->Position.x = this->R * glm::sin(glm::radians(Zenith)) * glm::sin(glm::radians(Azimuth));
		this->Position.y = this->R * glm::cos(glm::radians(Zenith));
		this->Position.z = this->R * glm::sin(glm::radians(Zenith)) * glm::cos(glm::radians(Azimuth));
		//std::cout << "(" << this->Position.x << "," << this->Position.y << "," << this->Position.z << ")" << std::endl;

		updataCameraVectors();

	}

	//鼠标移动摄像机视角
	void ProcessMouseMovement(float xoffset, float yoffset, float deltaTime)
	{

	}
private:
	//计算摄像机的front vector
	void updataCameraVectors()
	{
		glm::vec3 front;
		//第一人称
		//front.x = cos(glm::radians(Yaw))*cos(glm::radians(Pitch));
		//front.y = sin(glm::radians(Pitch));
		//front.z = sin(glm::radians(Yaw))*cos(glm::radians(Pitch));

		//第三人称球坐标系摄像机
		//摄像机始终看向世界坐标系原点方向
		front.x = -(this->Position.x);
		front.y = -(this->Position.y);
		front.z = -(this->Position.z);
		Front = glm::normalize(front);

		//重新计算Right 和 Up vector
		//当天顶角＞180时，摄像机上方向需取反，否则画面会产生突变
		if (Zenith > 180)
		{
			Right = glm::normalize(glm::cross(Front, WorldUp));
			Up = -glm::normalize(glm::cross(Right, Front));
		}
		else
		{
			Right = glm::normalize(glm::cross(Front, WorldUp));
			Up = glm::normalize(glm::cross(Right, Front));
		}

	}
};
#endif