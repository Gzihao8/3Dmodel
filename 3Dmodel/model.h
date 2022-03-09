#pragma once
#ifndef MODEL_H
#define MODEL_H

#include<glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
//������ͼ���õ�ͷ�ļ�
#include"stb_image.h"

#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

#include<iostream>

#include"shader.h"
#include"mesh.h"

#include<vector>

using namespace std;

unsigned int TextureFromFile(const char *path, const string &directory);

class Model
{
public:
	//ģ������
	vector<Mesh> meshes;
	vector<Texture> textures_loaded;
	string directory;

	//���캯��
	Model(string const &path)
	{
		loadModel(path);
	}

	//������ƺ���
	void DrawMesh(Shader &shader)
	{
		for (unsigned int i = 0; i < meshes.size(); i++)
			meshes[i].DrawMesh(shader);
	}
	//������ƺ���
	void DrawVertex(Shader &shader)
	{
		for (unsigned int i = 0; i < meshes.size(); i++)
			meshes[i].DrawVertex(shader);
	}

private:
	//����
	void loadModel(string const &path)
	{
		Assimp::Importer import;
		const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

		//��鳡���͸��ڵ㲻ΪNULL��ͨ�������Flag���鿴���ص������Ƿ�����
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			cout << "Error::ASSIMP::" << import.GetErrorString() << endl;
			return;
		}
		//��ȡ�ļ�·����Ŀ¼·��
		directory = path.substr(0, path.find_last_of('/'));

		//��scene�еĸ��ڵ㴫��ݹ�processNode����
		processNode(scene->mRootNode, scene);
	}
	void processNode(aiNode *node, const aiScene *scene)
	{
		//����ڵ����е���������еĻ���
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		//�Ըýڵ�������ӽڵ��ظ���һ����
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}
	Mesh processMesh(aiMesh *mesh, const aiScene *scene)
	{
		vector<Vertex> vertices;
		vector<unsigned int> indices;
		vector<Texture> textures;

		//������λ�á�����������������
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vector;
			//vector.x = mesh->mVertices[i].x;
			//vector.y = mesh->mVertices[i].y;
			//vector.z = mesh->mVertices[i].z;

			//��Сʮ��
			vector.x = mesh->mVertices[i].x / 100;
			vector.y = mesh->mVertices[i].y / 100;
			vector.z = mesh->mVertices[i].z / 100;
			vertex.Position = vector;

			if (mesh->HasNormals())
			{
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;
			}

			if (mesh->mTextureCoords[0])//�����Ƿ�����������
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else//���������û�������������Ϣ���򽫶����������������Ϊ��0��0��
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);

			vertices.push_back(vertex);
		}
		//��������
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		//�������
		//��������Ƿ��������
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
			vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			//��vector<Texture> textures�в���
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

			vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

		}
		return Mesh(vertices, indices, textures);
	}
	vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
	{
		vector<Texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			//��ȡ������ļ�·��
			aiString str;
			mat->GetTexture(type, i, &str);

			bool skip = false;
			//��texture_loaded������Ƚ�
			for (unsigned int j = 0; j < textures_loaded.size(); j++)
			{
				//������
				if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
				{
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}
			//��������
			if (!skip)
			{
				//�������û�м��أ�������
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), directory);
				texture.type = typeName;
				texture.path = str.C_Str();
				textures.push_back(texture);
				//��ӵ��Ѽ��ص�������
				textures_loaded.push_back(texture);
			}
		}
		return textures;
	}

};

unsigned int TextureFromFile(const char *path, const string &directory)
{
	string filename = string(path);
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}


#endif