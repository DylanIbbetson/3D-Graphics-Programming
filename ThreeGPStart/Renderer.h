#pragma once

#include "ExternalLibraryHeaders.h"

#include "Helper.h"
#include "Mesh.h"
#include "Camera.h"




struct Mesh
{
	GLuint vao;
	GLuint numElements;
	glm::vec3 rotation = glm::vec3(0, 0, 0);
	std::string name;
	GLuint tex;
};


struct Model
{
	std::vector<Mesh> meshVector;
	GLuint numCubeElements = 0;
	std::string ModelName;
};


class Renderer
{
private:
	// Program object - to host shaders
	GLuint m_program{ 0 };
	GLuint m_cubeProgram{ 0 };
	GLuint m_skyboxProgram{ 0 };
	
	// Vertex Array Object to wrap all render settings
	GLuint m_VAO{ 0 };

	// Number of elments to use when rendering
	GLuint m_numElements{ 0 };

	std::vector<Model> modelVector;



	//Texture
	GLuint tex;

	bool m_wireframe{ false };

	GLuint CreateProgram(std::string, std::string);
public:
	Renderer();
	~Renderer();

	// Draw GUI
	void DefineGUI();

	// Create and / or load geometry, this is like 'level load'
	bool InitialiseGeometry();

	// Render the scene
	void Render(const Helpers::Camera& camera, float deltaTime);
};

