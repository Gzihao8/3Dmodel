#pragma once
#ifndef SPHERECAMERA_H
#define SPHERECAMERA_H

#include<glad/glad.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

#include<iostream>

//#include<vector>

//��ö�����Ͷ��弸�����ܵ�������˶�����
enum Camera_Movement {
	FORWARD,//0
	BACKWARD,//1
	LEFT,//2
	RIGHT,//3
	UP,//4
	DOWN//5
};

//����Ĭ�ϵ��������ֵ
//ƫ����
const float YAW = -90.0f;
//������
const float PITCH = 0.0f;
//�ƶ��ٶ�
const float SPEED = 15.0f;
//������
const float SENSITIVITY = 0.01f;
//����
const float ZOOM = 45.0f;

//�������
class SphereCamera
{
	//����
public:
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	//��һ�˳�
	//float Yaw;
	//float Pitch;

	//������ϵ
	//�춥��,�Ƕ�
	float Zenith;
	//��λ��
	float Azimuth;
	float R;

	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	//���캯��(����������)
	SphereCamera(glm::vec3 position, glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f)) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		R = glm::length(glm::vec3(this->Position.x, this->Position.y, this->Position.z));
		Zenith = glm::degrees(acos(this->Position.y / R));
		Azimuth = glm::degrees(atan(this->Position.x / this->Position.z));

		updataCameraVectors();
	}

	//����

	//������lookAt�ó�����ͼ����
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

	//�ƶ������
	void CameraMove(Camera_Movement direction, float deltaTime)
	{
		//�ٶ�
		float velocity = MovementSpeed * deltaTime;
		//ǰ����������
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

		//�ı��������λ�ú���Ҫ���¼���������ϵ�İ뾶����R
		this->R = glm::length(glm::vec3(this->Position.x, this->Position.y, this->Position.z));
		std::cout << "(" << this->Position.x << "," << this->Position.y << "," << this->Position.z << ")" << std::endl;
	}

	//�����ת������ӽ�
	void ProcessMouseRotate(float xoffset, float yoffset)
	{
		//xoffset *= MouseSensitivity;
		//yoffset *= MouseSensitivity;

		//�޶���λ�Ǻ��춥�Ǳ仯ʱ��ѭ������
		//��һ����+-offsetǰ�������춥����0���һ��ʱ����һ˲���ͼ��ͻ�䣬����һ֡�ͻظ�����
		if (Azimuth < 0)Azimuth = 360 - abs(Azimuth);
		else Azimuth = (int)Azimuth % 360;

		if (Zenith < 0)Zenith = 360 - abs(Zenith);
		else Zenith = (int)Zenith % 360;

		//���춥���Ƿ����180��֪�������ʱ����̬�����������ƶ���������Ƕ�
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

	//����ƶ�������ӽ�
	void ProcessMouseMovement(float xoffset, float yoffset, float deltaTime)
	{

	}
private:
	//�����������front vector
	void updataCameraVectors()
	{
		glm::vec3 front;
		//��һ�˳�
		//front.x = cos(glm::radians(Yaw))*cos(glm::radians(Pitch));
		//front.y = sin(glm::radians(Pitch));
		//front.z = sin(glm::radians(Yaw))*cos(glm::radians(Pitch));

		//�����˳�������ϵ�����
		//�����ʼ�տ�����������ϵԭ�㷽��
		front.x = -(this->Position.x);
		front.y = -(this->Position.y);
		front.z = -(this->Position.z);
		Front = glm::normalize(front);

		//���¼���Right �� Up vector
		//���춥�ǣ�180ʱ��������Ϸ�����ȡ��������������ͻ��
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