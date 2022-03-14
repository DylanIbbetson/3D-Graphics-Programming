#include "Renderer.h"
#include "Camera.h"
#include "ImageLoader.h"

Renderer::Renderer() 
{

}

// On exit must clean up any OpenGL resources e.g. the program, the buffers
Renderer::~Renderer()
{
	// TODO: clean up any memory used including OpenGL objects via glDelete* calls
	glDeleteProgram(m_program);
	glDeleteBuffers(1, &m_VAO);
}

// Use IMGUI for a simple on screen GUI
void Renderer::DefineGUI()
{
	// Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	ImGui::Begin("3GP");						// Create a window called "3GP" and append into it.

	ImGui::Text("Visibility.");					// Display some text (you can use a format strings too)	

	ImGui::Checkbox("Wireframe", &m_wireframe);	// A checkbox linked to a member variable

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();
}

// Load, compile and link the shaders and create a program object to host them
GLuint Renderer::CreateProgram(std::string vsPath, std::string fsPath)
{
	// Create a new program (returns a unqiue id)
	GLuint program = glCreateProgram();

	// Load and create vertex and fragment shaders
	GLuint vertex_shader{ Helpers::LoadAndCompileShader(GL_VERTEX_SHADER, vsPath) };
	GLuint fragment_shader{ Helpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, fsPath) };
	if (vertex_shader == 0 || fragment_shader == 0)
		return false;

	// Attach the vertex shader to this program (copies it)
	glAttachShader(program, vertex_shader);

	/* The attibute location 0 maps to the input stream "vertex_position" in the vertex shader
	 Not needed if you use (location=0) in the vertex shader itself
	glBindAttribLocation(m_program, 0, "vertex_position");*/

	// Attach the fragment shader (copies it)
	glAttachShader(program, fragment_shader);

	// Done with the originals of these as we have made copies
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	// Link the shaders, checking for errors
	if (!Helpers::LinkProgramShaders(program))
		return 0;

	return program;
}

// Load / create geometry into OpenGL buffers	
bool Renderer::InitialiseGeometry()
{
	//// Load and compile shaders into m_program
	//if (!CreateProgram())
	//	return false;
	m_program = CreateProgram("Data\\Shaders\\vertex_shader.vert", "Data\\Shaders\\fragment_shader.frag");
	m_cubeProgram = CreateProgram("Data\\Shaders\\cube_vertex_shader.vert", "Data\\Shaders\\cube_fragment_shader.frag");
	m_skyboxProgram = CreateProgram("Data\\Shaders\\skybox_vertex_shader.vert", "Data\\Shaders\\skybox_fragment_shader.frag");

	Helpers::ImageLoader GrassTexture;
	if (!GrassTexture.Load("Data\\Textures\\grass.jpg"))
	{

	}

	Helpers::ImageLoader Heightmap;
	if (!Heightmap.Load("Data\\Heightmaps\\curvy.gif"))
	/*if (!Heightmap.Load("Data\\Heightmaps\\testHM.png"))*/
	{

	}



	Model Terrain;
	Terrain.ModelName = "Terrain";
	
	Mesh terrainMesh;




	std::vector<glm::vec3> colours;

	std::vector<glm::vec3> vertices;

	std::vector<GLuint> elements;

	std::vector<glm::vec2> uvCoords;


	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> positions;

	int numCellsX = 250;
	int numCellsZ = 250;

	int numVertX = numCellsX + 1;
	int numVertZ = numCellsZ + 1;

	int numVerts = numVertX * numVertZ;

	float imageX = 0;
	float imageZ = 0;

	positions.resize(numVerts);
	
	normals.resize(numVerts);

	for (int i = 0; i < numVerts; i++)
	{
		normals[i] = glm::vec3(0, 0, 0);
	}
	

	for (int i = 0; i < numVertZ; i++)
	{
		for (int j = 0; j < numVertX; j++)
		{
			vertices.push_back(glm::vec3(i * 8, 0, j * 8));
			normals.push_back(glm::vec3(0, 1, 0));
			colours.push_back(glm::vec3(0, 0, 1));

			uvCoords.push_back(glm::vec2((j / (float)numCellsX), (i / (float)numCellsZ)));

		}
	}
	

	float vertexXtoImage = (float)Heightmap.Width() / numVertX;
	float vertexZtoImage = (float)Heightmap.Height() / numVertZ;

	GLbyte* imageData = (GLbyte*)Heightmap.GetData();



	for (int Z = 0; Z < numVertZ; Z++)
	{
		imageZ = vertexZtoImage * Z;
		
		
		for (int X = 0; X < numVertX; X++)
		{
			imageX = vertexXtoImage * X;
			
			size_t offset = ((size_t)imageX + (size_t)imageZ * Heightmap.Width()) * 4;
			BYTE height = imageData[offset];
			int myvec = (Z * numVertX) + X;
			vertices[myvec].y = (float)height / 2;
		}

	}


	//Tessellation 
	bool toggleDiamondPattern = true;

	for (int cellZ = 0; cellZ < numCellsZ; cellZ++)
	{
		for (int cellX = 0; cellX < numCellsX; cellX++)
		{
			int StartVertIndex = (cellZ * numVertX) + cellX;

			if (toggleDiamondPattern)
			{
				elements.push_back(StartVertIndex);
				elements.push_back(StartVertIndex + 1);
				elements.push_back(StartVertIndex + numVertX + 1);

				elements.push_back(StartVertIndex);
				elements.push_back(StartVertIndex + numVertX + 1);
				elements.push_back(StartVertIndex + numVertX);

			}

			else
			{
				elements.push_back(StartVertIndex);
				elements.push_back(StartVertIndex + 1);
				elements.push_back(StartVertIndex + numVertX);

				elements.push_back(StartVertIndex + 1);
				elements.push_back(StartVertIndex + numVertX + 1);
				elements.push_back(StartVertIndex + numVertX);

			}
			toggleDiamondPattern = !toggleDiamondPattern;
		}
		toggleDiamondPattern = !toggleDiamondPattern;
	}


	//Calculating normals
	for (int e = 0; e < elements.size(); e += 3)
	{
		positions[0] = vertices[elements[e]];
		positions[1] = vertices[elements[(size_t)e + 1]];
		positions[2] = vertices[elements[(size_t)e + 2]];

		glm::vec3 edge1 = positions[1] - positions[0];
		glm::vec3 edge2 = positions[2] - positions[0];


		normals[elements[e]] += glm::cross(edge1, edge2);
		normals[elements[(size_t)e + 1]] += glm::cross(edge1, edge2);
		normals[elements[(size_t)e + 2]] += glm::cross(edge1, edge2);
		
	}

	//normalising the normals
	for (int n = 0; n < normals.size(); n++)
	{
		glm::normalize(normals[n]);
	}

	//Terrain VBOs

	GLuint positionsVBO;
	
	glGenBuffers(1, &positionsVBO);

	glBindBuffer(GL_ARRAY_BUFFER, positionsVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	//Clearing buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint coloursVBO;
	glGenBuffers(1, &coloursVBO);

	glBindBuffer(GL_ARRAY_BUFFER, coloursVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)* colours.size(), colours.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);



	//Terrain texture object

	glGenTextures(1, &terrainMesh.tex);

	glBindTexture(GL_TEXTURE_2D, terrainMesh.tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GrassTexture.Width(), GrassTexture.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, GrassTexture.GetData());

	glGenerateMipmap(GL_TEXTURE_2D);

	GLuint normalsVBO;

	glGenBuffers(1, &normalsVBO);

	glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normals.size(), normals.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint TexVBO;

	glGenBuffers(1, &TexVBO);

	glBindBuffer(GL_ARRAY_BUFFER, TexVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * uvCoords.size(), uvCoords.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Terrain element buffer

	terrainMesh.numElements = elements.size();

	GLuint elementEBO;

	glGenBuffers(1, &elementEBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementEBO);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * elements.size(), elements.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	
	
	//Terrain VAO
	glGenVertexArrays(1, &terrainMesh.vao);
	
	glBindVertexArray(terrainMesh.vao);

	glBindBuffer(GL_ARRAY_BUFFER, positionsVBO);

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);

	glEnableVertexAttribArray(1);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, TexVBO);

	glEnableVertexAttribArray(2);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//Terrain EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementEBO);

	glBindVertexArray(0);


	Terrain.meshVector.push_back(terrainMesh);


///////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////JEEP MODEL///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

	Model jeep;
	
	jeep.ModelName = "jeep";


	// Load in the jeep
	Helpers::ModelLoader JeepLoad;
	if (!JeepLoad.LoadFromFile("Data\\Models\\Jeep\\jeep.obj"))
		return false;

	//Load in the jeep texture
	Helpers::ImageLoader JeepTexture;
	if (JeepTexture.Load("Data\\Models\\Jeep\\jeep_army.jpg"))
	{



	}




	// Now we can loop through all the mesh in the loaded model:
	for (const Helpers::Mesh& mesh : JeepLoad.GetMeshVector())
	{
		// We can extract from the mesh via:
		//mesh.vertices  - a vector of glm::vec3 (3 floats) giving the position of each vertex
		//mesh.elements - a vector of unsigned int defining which vert make up each triangle
		// TODO: create VBO for the vertices and a EBO for the elements

		Mesh jeepMesh;

		//Jeep VBOs
		GLuint positionsVBO;

		glGenBuffers(1, &positionsVBO);

		glBindBuffer(GL_ARRAY_BUFFER, positionsVBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.vertices.size(), mesh.vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);


		GLuint normalsVBO;

		glGenBuffers(1, &normalsVBO);

		glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.normals.size(), mesh.normals.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		jeepMesh.numElements = mesh.elements.size();

		GLuint elementsEBO;

		glGenBuffers(1, &elementsEBO);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsEBO);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh.elements.size(), mesh.elements.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);



		//Jeep texture object
		glGenTextures(1, &jeepMesh.tex);

		glBindTexture(GL_TEXTURE_2D, jeepMesh.tex);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, JeepTexture.Width(), JeepTexture.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, JeepTexture.GetData());

		glGenerateMipmap(GL_TEXTURE_2D);


		GLuint TexVBO;

		glGenBuffers(1, &TexVBO);

		glBindBuffer(GL_ARRAY_BUFFER, TexVBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * mesh.uvCoords.size(), mesh.uvCoords.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);



		//Jeep VAO

		glGenVertexArrays(1, &jeepMesh.vao);

		glBindVertexArray(jeepMesh.vao);

		glBindBuffer(GL_ARRAY_BUFFER, positionsVBO);

		glEnableVertexAttribArray(0);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);

		glEnableVertexAttribArray(1);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, TexVBO);

		glEnableVertexAttribArray(2);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);


		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsEBO);

		glBindVertexArray(0);


		jeep.meshVector.emplace_back(jeepMesh);
	}


//////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////CUBE//////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

	Model cube;

	cube.ModelName = "cube";

	Mesh theCube;

	//Creating cube vertices
	glm::vec3 Corners[24] =
	{
		{-10, -10, 10}, //0
		{10, -10, 10}, //1
		{-10, 10, 10}, //2
		{10, 10, 10},  //3	

		{-10, -10, -10}, //4
		{10, -10, -10}, //5
		{-10, 10, -10}, //6
		{10, 10, -10}, //7

		{-10, -10, 10}, //8
		{-10, 10, 10}, //9
		{-10, -10, -10}, //10
		{-10, 10, -10}, //11

		{10, -10, 10}, //12
		{10, 10, 10},  //13
		{10, -10, -10}, //14
		{10, 10, -10}, //15

		{-10, 10, 10}, //16
		{10, 10, 10},  //17
		{-10, 10, -10}, //18
		{10, 10, -10}, //19

		{-10, -10, 10}, //20
		{10, -10, 10}, //21
		{-10, -10, -10}, //22
		{10, -10, -10}, //23
	};

	//Setting the cube face colours
	std::vector<GLfloat> cubeColours =
	{
		//R    G     B
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,

		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,

		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f
	};


	//Pushing back all of the vertices
	std::vector<glm::vec3> cubeVertices;
	cubeVertices.push_back(Corners[0]); //0
	cubeVertices.push_back(Corners[1]); //1
	cubeVertices.push_back(Corners[2]); //2
	cubeVertices.push_back(Corners[3]); //3

	cubeVertices.push_back(Corners[4]); //4
	cubeVertices.push_back(Corners[5]); //5
	cubeVertices.push_back(Corners[6]); //6
	cubeVertices.push_back(Corners[7]); //7

	cubeVertices.push_back(Corners[8]); //0
	cubeVertices.push_back(Corners[9]); //2
	cubeVertices.push_back(Corners[10]); //4
	cubeVertices.push_back(Corners[11]); //6

	cubeVertices.push_back(Corners[12]); //1
	cubeVertices.push_back(Corners[13]); //3
	cubeVertices.push_back(Corners[14]); //5
	cubeVertices.push_back(Corners[15]); //7

	cubeVertices.push_back(Corners[16]); //2
	cubeVertices.push_back(Corners[17]); //3
	cubeVertices.push_back(Corners[18]); //6
	cubeVertices.push_back(Corners[19]); //7

	cubeVertices.push_back(Corners[20]); //0
	cubeVertices.push_back(Corners[21]); //1
	cubeVertices.push_back(Corners[22]); //4
	cubeVertices.push_back(Corners[23]); //5



	//Pushing back each triangle making up each face in an anti-clockwise order
	std::vector<GLuint> cubeElements;
	//front face triangles
	cubeElements.push_back(0);
	cubeElements.push_back(1);
	cubeElements.push_back(2);
#
	cubeElements.push_back(1);
	cubeElements.push_back(3);
	cubeElements.push_back(2);

	//back face triangles
	cubeElements.push_back(6);
	cubeElements.push_back(5);
	cubeElements.push_back(4);

	cubeElements.push_back(7);
	cubeElements.push_back(5);
	cubeElements.push_back(6);

	//left face triangles
	cubeElements.push_back(9);
	cubeElements.push_back(10);
	cubeElements.push_back(8);

	cubeElements.push_back(9);
	cubeElements.push_back(11);
	cubeElements.push_back(10);

	//right face triangles
	cubeElements.push_back(12);
	cubeElements.push_back(14);
	cubeElements.push_back(13);

	cubeElements.push_back(14);
	cubeElements.push_back(15);
	cubeElements.push_back(13);

	//top face triangles
	cubeElements.push_back(16);
	cubeElements.push_back(17);
	cubeElements.push_back(18);

	cubeElements.push_back(17);
	cubeElements.push_back(19);
	cubeElements.push_back(18);

	//bottom face triangles
	cubeElements.push_back(22);
	cubeElements.push_back(21);
	cubeElements.push_back(20);

	cubeElements.push_back(22);
	cubeElements.push_back(23);
	cubeElements.push_back(21);


	//Cube VBO

	GLuint CubePositionsVBO;

	glGenBuffers(1, &CubePositionsVBO);

	glBindBuffer(GL_ARRAY_BUFFER, CubePositionsVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)* cubeVertices.size(), cubeVertices.data(), GL_STATIC_DRAW);

	//Clearing buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint CubeColoursVBO;
	glGenBuffers(1, &CubeColoursVBO);

	glBindBuffer(GL_ARRAY_BUFFER, CubeColoursVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* cubeColours.size(), cubeColours.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Cube element buffer

	theCube.numElements = elements.size();

	GLuint CubeElementEBO;

	glGenBuffers(1, &CubeElementEBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CubeElementEBO);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)* cubeElements.size(), cubeElements.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);



	/*
		TODO 4: Finally create a VAO to wrap the buffers. You need to specify the streams for the positions
		(attribute 0) and colours (attribute 1). You also need to bind the element buffer.
		Use the member variable m_VAO
	*/

	//VBO
	glGenVertexArrays(1, &theCube.vao);

	glBindVertexArray(theCube.vao);

	glBindBuffer(GL_ARRAY_BUFFER, CubePositionsVBO);

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, CubeColoursVBO);

	glEnableVertexAttribArray(1);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CubeElementEBO);

	glBindVertexArray(0);


	cube.meshVector.push_back(theCube);

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////SKYBOX///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////


	Model skybox;

	skybox.ModelName = "skybox";


	Helpers::ModelLoader skyboxLoad;
	

	if (!skyboxLoad.LoadFromFile("Data\\Models\\Sky\\Clouds\\skybox.x"))
		return false;
	
	int TextureIndex = 0;

	for (const Helpers::Mesh& mesh : skyboxLoad.GetMeshVector())
	{

		Mesh skyboxMesh;

		GLuint positionsVBO;

		//Skybox VBO
		glGenBuffers(1, &positionsVBO);

		glBindBuffer(GL_ARRAY_BUFFER, positionsVBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.vertices.size(), mesh.vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);


		GLuint normalsVBO;

		glGenBuffers(1, &normalsVBO);

		glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.normals.size(), mesh.normals.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		skyboxMesh.numElements = mesh.elements.size();

		GLuint elementsEBO;

		glGenBuffers(1, &elementsEBO);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsEBO);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh.elements.size(), mesh.elements.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		GLuint TexVBO;

		glGenBuffers(1, &TexVBO);

		glBindBuffer(GL_ARRAY_BUFFER, TexVBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * mesh.uvCoords.size(), mesh.uvCoords.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);


		//Skybox VAO
		glGenVertexArrays(1, &skyboxMesh.vao);

		glBindVertexArray(skyboxMesh.vao);

		glBindBuffer(GL_ARRAY_BUFFER, positionsVBO);

		glEnableVertexAttribArray(0);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);

		glEnableVertexAttribArray(1);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, TexVBO);

		glEnableVertexAttribArray(2);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);


		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsEBO);

		glBindVertexArray(0);


		//Switching between each texture for each face of the skybox
		Helpers::ImageLoader hillsTexture;
		switch (TextureIndex)
		{
		case 0:
			if (!hillsTexture.Load("Data\\Models\\Sky\\Clouds\\SkyBox_Top.tga"))
			{
				return false;
			}
			break;
		case 1:
			if (!hillsTexture.Load("Data\\Models\\Sky\\Clouds\\SkyBox_Right.tga"))
			{
				return false;
			}
			break;
		case 2:
			if (!hillsTexture.Load("Data\\Models\\Sky\\Clouds\\SkyBox_Left.tga"))
			{
				return false;
			}
			break;
		case 3:
			if (!hillsTexture.Load("Data\\Models\\Sky\\Clouds\\SkyBox_Front.tga"))
			{
				return false;
			}
			break;
		case 4:
			if (!hillsTexture.Load("Data\\Models\\Sky\\Clouds\\SkyBox_Back.tga"))
			{
				return false;
			}
			break;
		case 5:
			if (!hillsTexture.Load("Data\\Models\\Sky\\Clouds\\SkyBox_Bottom.tga"))
			{
				return false;
			}
			break;
		}

		//Skybox texture object
		glGenTextures(1, &skyboxMesh.tex);

		glBindTexture(GL_TEXTURE_2D, skyboxMesh.tex);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, hillsTexture.Width(), hillsTexture.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, hillsTexture.GetData());

		glGenerateMipmap(GL_TEXTURE_2D);



		skybox.meshVector.emplace_back(skyboxMesh);

		TextureIndex++;
	}

	//Pushing back each model created above
	modelVector.emplace_back(skybox);
	modelVector.emplace_back(Terrain);
	modelVector.emplace_back(jeep);
	modelVector.emplace_back(cube);

	


	return true;
}

// Render the scene. Passed the delta time since last called.
void Renderer::Render(const Helpers::Camera& camera, float deltaTime)
{			
	// Configure pipeline settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Wireframe mode controlled by ImGui
	if (m_wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Clear buffers from previous frame
	glClearColor(0.0f, 0.0f, 0.0f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Compute viewport and projection matrix
	GLint viewportSize[4];
	glGetIntegerv(GL_VIEWPORT, viewportSize);
	const float aspect_ratio = viewportSize[2] / (float)viewportSize[3];
	glm::mat4 projection_xform = glm::perspective(glm::radians(45.0f), aspect_ratio, 0.1f, 4000.0f);

	// Compute camera view matrix and combine with projection matrix for passing to shader
	/*glm::mat4 view_xform = glm::lookAt(camera.GetPosition(), camera.GetPosition() + camera.GetLookVector(), camera.GetUpVector());
	glm::mat4 combined_xform = projection_xform * view_xform;*/


	// Send the combined matrix to the shader in a uniform
	
	glm::mat4 model_xform = glm::mat4(1);

	
	//Looping through each mesh of each model 
	for (Model& model : modelVector)
	{
		for (Mesh& mesh : model.meshVector)
		{
			if (model.ModelName == "skybox")
			{
				//Disabling the depth mask and depth test for the skybox
				glDepthMask(GL_FALSE);
				glDisable(GL_DEPTH_TEST);
				


				glm::mat4 view_xform = glm::lookAt(camera.GetPosition(), camera.GetPosition() + camera.GetLookVector(), camera.GetUpVector());
				glm::mat4 view_xform2 = glm::mat4(glm::mat3(view_xform));
				glm::mat4 combined_xform = projection_xform * view_xform2;

				glUseProgram(m_skyboxProgram);

				GLuint combined_xform_id = glGetUniformLocation(m_skyboxProgram, "combined_xform");
				glUniformMatrix4fv(combined_xform_id, 1, GL_FALSE, glm::value_ptr(combined_xform));
			}

			else if (model.ModelName == "cube")
			{
				glDepthMask(GL_TRUE);
				glEnable(GL_DEPTH_TEST);

				glm::mat4 view_xform = glm::lookAt(camera.GetPosition(), camera.GetPosition() + camera.GetLookVector(), camera.GetUpVector());
				glm::mat4 combined_xform = projection_xform * view_xform;

				glUseProgram(m_cubeProgram);

				GLuint combined_xform_id = glGetUniformLocation(m_cubeProgram, "combined_xform");
				glUniformMatrix4fv(combined_xform_id, 1, GL_FALSE, glm::value_ptr(combined_xform));

				model_xform = glm::scale(model_xform, glm::vec3{ 10,10,10 });
				model_xform = glm::translate(model_xform, glm::vec3(100, 25, 0));

				static float angle = 0;
				static bool rotateY = true;

				if (rotateY) // Rotate around y axis		
					model_xform = glm::rotate(model_xform, angle, glm::vec3{ 0 ,1,0 });
				else // Rotate around x axis		
					model_xform = glm::rotate(model_xform, angle, glm::vec3{ 1 ,0,0 });

				angle += 0.001f;
				if (angle > glm::two_pi<float>())
				{
					angle = 0;
					rotateY = !rotateY;
				}


			}

			else if (model.ModelName == "jeep")
			{
				glDepthMask(GL_TRUE);
				glEnable(GL_DEPTH_TEST);

				glm::mat4 view_xform = glm::lookAt(camera.GetPosition(), camera.GetPosition() + camera.GetLookVector(), camera.GetUpVector());
				glm::mat4 combined_xform = projection_xform * view_xform;

				glUseProgram(m_program);

				GLuint combined_xform_id = glGetUniformLocation(m_program, "combined_xform");
				glUniformMatrix4fv(combined_xform_id, 1, GL_FALSE, glm::value_ptr(combined_xform));

				model_xform = glm::scale(model_xform, glm::vec3{0.5,0.5,0.5});
				model_xform = glm::translate(model_xform, glm::vec3(2000, 10, 2500));


			}

			else if (model.ModelName != "cube" && model.ModelName != "jeep" && model.ModelName != "skybox")
			{
				glDepthMask(GL_TRUE);
				glEnable(GL_DEPTH_TEST);

				glm::mat4 view_xform = glm::lookAt(camera.GetPosition(), camera.GetPosition() + camera.GetLookVector(), camera.GetUpVector());
				glm::mat4 combined_xform = projection_xform * view_xform;

				glUseProgram(m_program);

				GLuint combined_xform_id = glGetUniformLocation(m_program, "combined_xform");
				glUniformMatrix4fv(combined_xform_id, 1, GL_FALSE, glm::value_ptr(combined_xform));
			}



			// Send the model matrix to the shader in a uniform
			GLuint model_xform_id = glGetUniformLocation(m_program, "model_xform");
			glUniformMatrix4fv(model_xform_id, 1, GL_FALSE, glm::value_ptr(model_xform));

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mesh.tex);
			glUniform1i(glGetUniformLocation(m_program, "sampler_tex"), 0);

			// Bind our VAO and render
			glBindVertexArray(mesh.vao);
			glDrawElements(GL_TRIANGLES, mesh.numElements, GL_UNSIGNED_INT, (void*)0);
		}
	}





	

}

