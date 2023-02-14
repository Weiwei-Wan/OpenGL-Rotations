// Windows includes (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>
#include <map>
#include <vector> // STL dynamic memory.
//#include <glad/glad.h>
//#include <GLFW/glfw3.h>

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

//glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/quaternion.hpp> 
#include <glm/gtx/quaternion.hpp>

// Project includes
#include "maths_funcs.h"
#define GLT_IMPLEMENTATION
#include "gltext.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
#define MESH_PLANE "./models/plane.dae"
#define MESH_PROPE "./models/propeller2.dae"

/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/

struct ModelData
{
	size_t mPointCount = 0;
	std::vector<vec3> mVertices;
	std::vector<vec3> mNormals;
	std::vector<vec2> mTextureCoords;
};

ModelData mesh_plane;
ModelData mesh_prope;

using namespace std;
GLuint SkyBoxID, PhongID;

ModelData mesh_data;
unsigned int mesh_vao = 0;
int width = 1600;
int height = 1200;

glm::mat4 persp_proj = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
glm::mat4 model = glm::mat4(1.0f);

// Camera pos
glm::vec3 camera_pos = glm::vec3(8.0f, 0.0f, 0.0f);
glm::vec3 camera_dir = glm::vec3(-8.0f, 0.0f, 0.0f);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
GLfloat plane_pos_x = 0.0f;
GLfloat plane_pos_z = 0.0f;
GLfloat plane_pos_y = 0.0f;
GLfloat camYaw = 0.0f;
GLfloat camPitch = 0.0f;

GLfloat shininess = 20.0f;
GLfloat ks = 0.5f;

GLuint loc1, loc2;
GLfloat Yaw = 0.0f;
GLfloat Pitch = 0.0f;
GLfloat Roll = 0.0f;
GLfloat Delta = 5.0f;
boolean eular = true;
boolean first_direction = false;
glm::mat4 rotation;
GLfloat rotate_x = 0.0f;
GLfloat rotate_delta = 20.0f;

// ------------ SKYBOX ------------
unsigned int skyboxVAO, skyboxVBO;
unsigned int cubemapTexture;
vector<std::string> faces
{
	"./skybox/px2.jpg",
	"./skybox/nx2.jpg",
	"./skybox/py2.jpg",
	"./skybox/ny2.jpg",
	"./skybox/pz2.jpg",
	"./skybox/nz2.jpg"
};

float skyboxVertices[] = {
	-200.0f,  200.0f, -200.0f,
	-200.0f, -200.0f, -200.0f,
	 200.0f, -200.0f, -200.0f,
	 200.0f, -200.0f, -200.0f,
	 200.0f,  200.0f, -200.0f,
	-200.0f,  200.0f, -200.0f,

	-200.0f, -200.0f,  200.0f,
	-200.0f, -200.0f, -200.0f,
	-200.0f,  200.0f, -200.0f,
	-200.0f,  200.0f, -200.0f,
	-200.0f,  200.0f,  200.0f,
	-200.0f, -200.0f,  200.0f,

	 200.0f, -200.0f, -200.0f,
	 200.0f, -200.0f,  200.0f,
	 200.0f,  200.0f,  200.0f,
	 200.0f,  200.0f,  200.0f,
	 200.0f,  200.0f, -200.0f,
	 200.0f, -200.0f, -200.0f,

	-200.0f, -200.0f,  200.0f,
	-200.0f,  200.0f,  200.0f,
	 200.0f,  200.0f,  200.0f,
	 200.0f,  200.0f,  200.0f,
	 200.0f, -200.0f,  200.0f,
	-200.0f, -200.0f,  200.0f,

	-200.0f,  200.0f, -200.0f,
	 200.0f,  200.0f, -200.0f,
	 200.0f,  200.0f,  200.0f,
	 200.0f,  200.0f,  200.0f,
	-200.0f,  200.0f,  200.0f,
	-200.0f,  200.0f, -200.0f,

	-200.0f, -200.0f, -200.0f,
	-200.0f, -200.0f,  200.0f,
	 200.0f, -200.0f, -200.0f,
	 200.0f, -200.0f, -200.0f,
	-200.0f, -200.0f,  200.0f,
	 200.0f, -200.0f,  200.0f
};
#pragma region MESH LOADING
/*----------------------------------------------------------------------------
MESH LOADING FUNCTION
----------------------------------------------------------------------------*/

ModelData load_mesh(const char* file_name) {
	ModelData modelData;

	const aiScene* scene = aiImportFile(
		file_name,
		aiProcess_Triangulate | aiProcess_PreTransformVertices
	);

	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		return modelData;
	}

	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);

	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		const aiMesh* mesh = scene->mMeshes[m_i];
		printf("    %i vertices in mesh\n", mesh->mNumVertices);
		modelData.mPointCount += mesh->mNumVertices;
		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				modelData.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
			}
			if (mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
				modelData.mTextureCoords.push_back(vec2(vt->x, vt->y));
			}
			if (mesh->HasTangentsAndBitangents()) {
			}
		}
	}

	aiReleaseImport(scene);
	return modelData;
}

#pragma endregion MESH LOADING

// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS
char* readShaderSource(const char* shaderFile) {
	FILE* fp;
	fopen_s(&fp, shaderFile, "rb");

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		std::cerr << "Error creating shader..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024] = { '\0' };
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error compiling "
			<< (ShaderType == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< " shader program: " << InfoLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders(const char* vshadername, const char* fshadername)
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	GLuint shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		std::cerr << "Error creating shader program..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, vshadername, GL_VERTEX_SHADER);
	AddShader(shaderProgramID, fshadername, GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { '\0' };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}

unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int skyboxTextureID;
	glGenTextures(1, &skyboxTextureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return skyboxTextureID;
}
#pragma endregion SHADER_FUNCTIONS

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS
void generateObjectBufferMesh(GLuint& ID, ModelData mesh_data) {
	unsigned int vp_vbo = 0;
	loc1 = glGetAttribLocation(ID, "vertex_position");
	loc2 = glGetAttribLocation(ID, "vertex_normal");

	glGenBuffers(1, &vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mVertices[0], GL_STATIC_DRAW);
	unsigned int vn_vbo = 0;
	glGenBuffers(1, &vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mNormals[0], GL_STATIC_DRAW);

	unsigned int vao = 0;
	glBindVertexArray(vao);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}

void generateSkybox() {
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}
#pragma endregion VBO_FUNCTIONS

void drawText(const char* str, GLfloat size, glm::vec3 pos) {
	// Initialize glText
	gltInit();
	// Creating text
	GLTtext* text = gltCreateText();
	gltSetText(text, str);
	// Begin text drawing (this for instance calls glUseProgram)
	gltBeginDraw();
	// Draw any amount of text between begin and end
	gltColor(1.0f, 0.0f, 0.0f, 1.0f);
	gltDrawText2DAligned(text, 70 * (pos.x + 1), 450 - pos.y * 70, size, GLT_CENTER, GLT_CENTER);
	// Finish drawing text
	gltEndDraw();
	// Deleting text
	gltDeleteText(text);
	// Destroy glText
	gltTerminate();
}

void display() {
	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	char array[10];
	rotate_x += rotate_delta;

	if (first_direction) {
		camera_pos = glm::vec3(plane_pos_x, plane_pos_y + 0.6f, plane_pos_z);
		camera_dir.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		camera_dir.y = sin(glm::radians(Pitch));
		camera_dir.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		camera_dir = glm::normalize(camera_dir);
	}
	else {
		camera_pos = glm::vec3(8.0f, 0.0f, 0.0f);
		camera_dir = glm::vec3(-8.0f, 0.0f, 0.0f);
	}
	
	//Declare your uniform variables that will be used in your shader
	glm::mat4 view = glm::lookAt(camera_pos, // Camera is at (x,y,z), in World Space
		                         camera_pos + camera_dir, // and looks at the origin 
		                         glm::vec3(0, 1, 0));  // Head is up (set to 0,-1,0 to look upside-down)

	// skybox
	cubemapTexture = loadCubemap(faces);
	glDepthFunc(GL_LEQUAL);
	glUseProgram(SkyBoxID);
	generateSkybox();
	glUniformMatrix4fv(glGetUniformLocation(SkyBoxID, "view"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(SkyBoxID, "proj"), 1, GL_FALSE, &persp_proj[0][0]);
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	//plane
	glUseProgram(PhongID);
	generateObjectBufferMesh(PhongID, mesh_plane);	
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(plane_pos_x, plane_pos_y, plane_pos_z));
	model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f)); 
	if (eular) {
		rotation = glm::eulerAngleYXZ(glm::radians(Yaw), glm::radians(Pitch), glm::radians(Roll));
	}
	else {
		glm::quat rotateYQuat = glm::angleAxis(glm::radians(Yaw), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::quat rotateZQuat = glm::angleAxis(glm::radians(Roll), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::quat rotateXQuat = glm::angleAxis(glm::radians(Pitch), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::quat quaternion = rotateYQuat * rotateZQuat * rotateXQuat;
		rotation = glm::toMat4(quaternion);
	} 
	model = rotation * model;
	glUniformMatrix4fv(glGetUniformLocation(PhongID, "proj"), 1, GL_FALSE, &persp_proj[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(PhongID, "view"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(PhongID, "model"), 1, GL_FALSE, &model[0][0]);
	glUniform3f(glGetUniformLocation(PhongID, "color"), 0.9f, 0.3f, 0.1f);
	glUniform1f(glGetUniformLocation(PhongID, "shininess"), shininess);
	glUniform1f(glGetUniformLocation(PhongID, "ks"), ks);
	glUniform3f(glGetUniformLocation(PhongID, "cameraPos"), camera_pos.x, camera_pos.y, camera_pos.z);
	glDrawArrays(GL_TRIANGLES, 0, mesh_plane.mPointCount);

	// propeller
	generateObjectBufferMesh(PhongID, mesh_prope);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(plane_pos_x, plane_pos_y+0.52f, plane_pos_z));
	model = glm::scale(model, glm::vec3(0.005f, 0.005f, 0.005f));
	model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rotate_x), glm::vec3(0.0f, 1.0f, 0.0f));

	model = rotation * model;
	glUniformMatrix4fv(glGetUniformLocation(PhongID, "model"), 1, GL_FALSE, &model[0][0]);
	glUniform3f(glGetUniformLocation(PhongID, "color"), 0.9f, 0.9f, 0.9f);
	glDrawArrays(GL_TRIANGLES, 0, mesh_prope.mPointCount);
	
	drawText("Yaw:", 2, glm::vec3(3.0f, -5.0f, 0.0f));
	snprintf(array, sizeof(array), "%1.1f", Yaw);
	drawText(array, 2, glm::vec3(5.5f, -5.0f, 0.0f));
	drawText("Pitch:", 2, glm::vec3(3.0f, -6.0f, 0.0f));
	snprintf(array, sizeof(array), "%1.1f", Pitch);
	drawText(array, 2, glm::vec3(5.5f, -6.0f, 0.0f));
	drawText("Roll:", 2, glm::vec3(3.0f, -7.0f, 0.0f));
	snprintf(array, sizeof(array), "%1.1f", Roll);
	drawText(array, 2, glm::vec3(5.5f, -7.0f, 0.0f));

	if (eular) {
		drawText("Euler Angle Mode", 4, glm::vec3(3.0f, 4.0f, 0.0f));
	}
	else {
		drawText("Quaternion Mode", 4, glm::vec3(3.0f, 4.0f, 0.0f));
	}

	glutPostRedisplay();
	glutSwapBuffers();
}

void init()
{
	mesh_plane = load_mesh(MESH_PLANE);
	mesh_prope = load_mesh(MESH_PROPE);
	// Set up the shaders
	PhongID = CompileShaders("./shaders/PhongVertexShader.txt", "./shaders/simpleFragmentShader.txt");
	SkyBoxID = CompileShaders("./shaders/skyboxVertexShader.txt", "./shaders/skyboxFragmentShader.txt");
}

// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {
	printf("KEYBOARD");
	if (key == 'x') {
		Yaw += Delta;
	}
	else if (key == 'a') {
		Yaw -= Delta;
	}
	else if (key == 'y') {
		Pitch += Delta;
	}
	else if (key == 'b') {
		Pitch -= Delta;
	}
	else if (key == 'z') {
		Roll += Delta;
	}
	else if (key == 'c') {
		Roll -= Delta;
	}
	else if (key == 'q') {
		eular = !eular;
		Yaw = 0.0f;
		Pitch = 0.0f;
		Roll = 0.0f;
	}
	else if (key == 'f') {
		first_direction = !first_direction;
	}
}

void mousePress(int button, int state, int xpos, int ypos) {
	// Wheel reports as button 3(scroll up) and button 4(scroll down)
	if (button == 3) // It's a wheel event
	{
		// Each wheel event reports like a button click, GLUT_DOWN then GLUT_UP
		if (state == GLUT_UP) return; // Disregard redundant GLUT_UP events
		printf("Scroll %s At %d %d\n", (button == 3) ? "Up" : "Down", xpos, ypos);
		camera_pos.x += 1.0f;
	}
	else if (button == 4) // It's a wheel event
	{
		if (state == GLUT_UP) return; // Disregard redundant GLUT_UP events
		printf("Scroll %s At %d %d\n", (button == 4) ? "Up" : "Down", xpos, ypos);
		camera_pos.x -= 1.0f;
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {  // normal button event
		camYaw += (xpos - float(width) / 2.0) / width;
		camYaw = glm::mod(camYaw + 180.0f, 360.0f) - 180.0f;
		camPitch -= (ypos - float(height) / 2.0) / height;
		camPitch = glm::clamp(camPitch, -89.0f, 89.0f);
		camera_dir.x = cos(camPitch) * sin(camYaw);
		camera_dir.y = sin(camPitch);
		camera_dir.z = -cos(camPitch) * cos(camYaw);
	}
}

int main(int argc, char** argv) {
	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("lab1");

	//texure
	glEnable(GL_DEPTH_TEST);

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutKeyboardFunc(keypress);
	glutMouseFunc(mousePress);

	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}
