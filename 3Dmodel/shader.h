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
	//着色器程序的ID
	unsigned int ID;

	//方法：构造这个着色器程序需要顶点着色器源代码的文件路径和片段着色器源代码的文件路径
	Shader(const char* verterPath, const char* fragmentPath)
	{
		//1.从文件路径中获取顶点和片段着色器
		std::string vertexCode;
		std::string fragmentCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		//保证ifstream对象可以抛出异常
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			//打开文件
			vShaderFile.open(verterPath);
			fShaderFile.open(fragmentPath);
			std::stringstream vShaderStream, fShaderStream;
			//读取文件的缓冲内容到数据流中
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			//关闭文件
			vShaderFile.close();
			fShaderFile.close();
			//转换数据流到string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();

		}
		catch (std::ifstream::failure e)
		{
			std::cout << "Error::shader file not succesfully read" << std::endl;
		}
		//将C++的string转换成C的字符数组
		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();

		//编译着色器并连接成着色器程序
		unsigned int vertex, fragment;
		int success;
		char infolog[512];
		//顶点着色器
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		//检查编译是否成功
		checkCompileErrors(vertex, "vertex");
		//片段着色器
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		//检查编译是否成功
		checkCompileErrors(fragment, "fragment");

		//着色器程序
		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		glLinkProgram(ID);
		//检查连接是否成功
		checkCompileErrors(ID, "program");
		//删除着色器对象，因为着色器程序已经连接好了
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	//方法：激活着色器程序
	void use()
	{
		glUseProgram(ID);
	}

	//方法：uniform工具函数,查询uniform的location值并设置，name是该uniform的名字，value是将要设置成的值
	//由于整个着色器的程序都能自由访问uniform，所以并不关心它是在哪个着色器中
	void setBool(const std::string &name, bool value)const
	{
		//glGetUniformLocation查询uniform的位置
		//glUniform设置值
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

	//方法：检查编译/连接是否成功
	void checkCompileErrors(unsigned int ID, std::string type)
	{
		int success;
		char infolog[1024];
		if (type != "program")//如果不是检查连接是否成功，那就是检查着色器编译是否成功
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