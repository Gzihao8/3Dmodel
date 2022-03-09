#pragma once
#ifndef SHADER_H
#define SHADER_H

#include<glad/glad.h>

#include<string>
#include <fstream>
#include <sstream>

#include <iostream>

class Shader
{
public:
	//��ɫ�������ID
	unsigned int ID;

	//���������������ɫ��������Ҫ������ɫ��Դ������ļ�·����Ƭ����ɫ��Դ������ļ�·��
	Shader(const char* verterPath, const char* fragmentPath)
	{
		//1.���ļ�·���л�ȡ�����Ƭ����ɫ��
		std::string vertexCode;
		std::string fragmentCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		//��֤ifstream��������׳��쳣
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			//���ļ�
			vShaderFile.open(verterPath);
			fShaderFile.open(fragmentPath);
			std::stringstream vShaderStream, fShaderStream;
			//��ȡ�ļ��Ļ������ݵ���������
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			//�ر��ļ�
			vShaderFile.close();
			fShaderFile.close();
			//ת����������string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();

		}
		catch (std::ifstream::failure e)
		{
			std::cout << "Error::shader file not succesfully read" << std::endl;
		}
		//��C++��stringת����C���ַ�����
		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();

		//������ɫ�������ӳ���ɫ������
		unsigned int vertex, fragment;
		int success;
		char infolog[512];
		//������ɫ��
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		//�������Ƿ�ɹ�
		checkCompileErrors(vertex, "vertex");
		//Ƭ����ɫ��
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		//�������Ƿ�ɹ�
		checkCompileErrors(fragment, "fragment");

		//��ɫ������
		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		glLinkProgram(ID);
		//��������Ƿ�ɹ�
		checkCompileErrors(ID, "program");
		//ɾ����ɫ��������Ϊ��ɫ�������Ѿ����Ӻ���
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	//������������ɫ������
	void use()
	{
		glUseProgram(ID);
	}

	//������uniform���ߺ���,��ѯuniform��locationֵ�����ã�name�Ǹ�uniform�����֣�value�ǽ�Ҫ���óɵ�ֵ
	//����������ɫ���ĳ��������ɷ���uniform�����Բ��������������ĸ���ɫ����
	void setBool(const std::string &name, bool value)const
	{
		//glGetUniformLocation��ѯuniform��λ��
		//glUniform����ֵ
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}
	void setInt(const std::string &name, int value)const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
	void setFloat(const std::string &name, float value)const
	{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}
	void setMat4(const std::string &name, const glm::mat4 &mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	void setVec3(const std::string &name, const glm::vec3 &value) const
	{
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}


private:

	//������������/�����Ƿ�ɹ�
	void checkCompileErrors(unsigned int ID, std::string type)
	{
		int success;
		char infolog[1024];
		if (type != "program")//������Ǽ�������Ƿ�ɹ����Ǿ��Ǽ����ɫ�������Ƿ�ɹ�
		{
			glGetShaderiv(ID, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(ID, 1024, NULL, infolog);
				std::cout << "Error:shader compilation error of type:" << type << "\n" << infolog << std::endl;
			}
		}
		else
		{
			glGetProgramiv(ID, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(ID, 1024, NULL, infolog);
				std::cout << "Error:program linking error\n" << infolog << std::endl;
			}
		}
	}
};

#endif