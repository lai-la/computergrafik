// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/constants.hpp>

using namespace glm;
using namespace std;

#include "shader.hpp"
#include "Joint.h"

// Wuerfel und Kugel
#include "objects.hpp"
#include"Obj3D.h"

#include "objloader.hpp"

#include "texture.hpp"

float angleX = 0.0f;
float angleY = 0.0f;
float angleZ = 0.0f;
//variables to implement camera movement
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -2.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

#define KEY_MOVE_STEP 5.0f
#define JOINT_ROTATE_STEP 5.0f

#define WORLD_LIGHT_COORDINATES -2.5, 2.5, 2.//-1., 3., 1.
#define CAMERA_COORDINATES -2.7, 1., 3.8

#define TEXTURE_FILE "wood_texture.bmp"
#define GRASS_TEXTURE_FILE "grass_texture.bmp"
#define WINDOW_X 1024
#define WINDOW_Y 768

void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

/*
void cursor_position_callback(GLFWwindow* window, double xPos, double yPos) ;

void cursor_enter_callback(GLFWwindow* window, int entered);

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
*/
vector<Joint*> all_joints;
int selected_joint = 0;

void set_selected_joint(int num) {
	if (num < 0) num = all_joints.size() - 1; //makes sure 0 is the lowest selectable joint
	selected_joint = num % all_joints.size(); // "" 10 is the highest
	Joint *joint = all_joints[selected_joint]; // selects joint
	cout << "Selected joint nr " << selected_joint << ": " << joint->name << endl;
}


void move_selected_joint(int sign) {
	if (all_joints.size() == 0) return; // avoids program crash when list of joints not initialized
	Joint *joint = all_joints[selected_joint];
	if(joint->counter + sign <= joint->max && joint->counter + sign >= joint->min) {
		joint->rotate(sign * JOINT_ROTATE_STEP);
		joint->counter += sign;
	}
}



void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_RELEASE) return;
	float cameraSpeed = 0.05f; // adjust accordingly
		
	switch (key)
	{
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, GL_TRUE);
		break;

	case GLFW_KEY_X:
		angleX += KEY_MOVE_STEP;
		break;
		////////////////////////////////////////////////
		// Y and Z are somehow inverted on my keyboard//
		///////////////////////////////////////////////
	case GLFW_KEY_Y:
		angleZ += KEY_MOVE_STEP;
		break;

	case GLFW_KEY_Z:
		angleY += KEY_MOVE_STEP;
		break;

	case GLFW_KEY_UP:
		set_selected_joint(selected_joint + 1);
		break;
	case GLFW_KEY_DOWN:
		set_selected_joint(selected_joint - 1);
		break;
		
	case GLFW_KEY_RIGHT:
		move_selected_joint(1);
		break;
	case GLFW_KEY_LEFT:
		move_selected_joint(-1);
		break;

	//camera
	case GLFW_KEY_W:
		cameraPos += cameraSpeed * cameraFront;
		break;
	case GLFW_KEY_S:
		cameraPos -= cameraSpeed * cameraFront;
		break;
	
	/*
	case GLFW_KEY_A:
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		break;
	case GLFW_KEY_D:
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		break;
	*/
	default:
		break;
	}
}

// Diese Drei Matrizen global (Singleton-Muster), damit sie jederzeit modifiziert und
// an die Grafikkarte geschickt werden koennen
glm::mat4 Projection;
glm::mat4 View;
glm::mat4 Model;
GLuint programID;

void sendMVP()
{
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP = Projection * View * Model; 

	glUniformMatrix4fv(glGetUniformLocation(programID, "M"), 1, GL_FALSE, &Model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(programID, "V"), 1, GL_FALSE, &View[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(programID, "P"), 1, GL_FALSE, &Projection[0][0]);
	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform, konstant fuer alle Eckpunkte
	glUniformMatrix4fv(glGetUniformLocation(programID, "MVP"), 1, GL_FALSE, &MVP[0][0]);
}

void sendModel(glm::mat4 model) {
	glm::mat4 Save = Model;
	Model = model;
	sendMVP();
	Model = Save;
}

#define PI 180.f
#define IDENT mat4(1.f)


void makeLeg(Obj3D *obj1, Obj3D *obj2, Obj3D *obj3, Joint *body, string name, vec3 legTranslate1, vec3 legTranslate2, vec3 legTranslate3) {
	Joint *upper = new Joint(obj1, body);
	auto upperLegScale = vec3(0.19, 0.19, 0.19);
	upper->translate(legTranslate1);
	upper->scale(upperLegScale);
	upper->rotate(0.5f * PI, vec3(1., 0., 0.));
	upper->name = name;
	if (name == "front left leg" || name == "front right leg") {
		upper->min = -5;
		upper->max = 10;
	}
	else {
		upper->min = -15;
		upper->max = 8;
	}
	
	Joint *lower = new Joint(obj2, upper);
	auto lowerLegScale = vec3(0.19, 0.19, 0.19);
	lower->scale(lowerLegScale);
	lower->translate(legTranslate2);
	lower->name = "lower " + name;
	if (name == "front left leg" || name == "front right leg") {
		lower->min = -2;
		lower->max = 15;
	}
	else {
		lower->min = -20;
		lower->max = 3;
	}

	Joint *hoof = new Joint(obj3, lower);
	auto hoofScale = vec3(0.19, 0.19, 0.19);
	hoof->scale(hoofScale);
	hoof->translate(legTranslate3);
	hoof->name = "hoof " + name;
	hoof->min = -2;
	hoof->max = 15;
}

Joint *makeHorseSkeleton() {
	// Default orientation: horse body starts at origin, grows towards Z-axis. The end ("top") of the body will be the front of the horse.

 	auto SCALE = vec3(0.19, 0.19, 0.19);
	auto BODY_W = 0.37;
	auto BODY_L = 0.9;

	Joint *body = new Joint(new Obj3D("horse_body.obj"), nullptr);
	body->scale(SCALE);
	body->name = "body";
	
	makeLeg(new Obj3D("horse_front_upper_leg.obj"), new Obj3D("horse_front_lower_leg.obj"), new Obj3D("horse_front_hoof.obj"), body, "front left leg", vec3(0.15, -0.2, 0.6), vec3(0., 0., 0.65), vec3(0., 0., 0.35));
	makeLeg(new Obj3D("horse_front_upper_leg.obj"), new Obj3D("horse_front_lower_leg.obj"), new Obj3D("horse_front_hoof.obj"), body, "front right leg", vec3(-0.15, -0.2, 0.6), vec3(0., 0., 0.65), vec3(0., 0., 0.35));
	makeLeg(new Obj3D("horse_rear_upper2_leg.obj"), new Obj3D("horse_rear_lower2_leg.obj"), new Obj3D("horse_rear_hoof2.obj"), body, "rear left leg", vec3(0.1, -0.2, -0.64), vec3(0., -0.27, 0.5), vec3(0., -0.05, 0.46));
	makeLeg(new Obj3D("horse_rear_upper2_leg.obj"), new Obj3D("horse_rear_lower2_leg.obj"), new Obj3D("horse_rear_hoof2.obj"), body, "rear right leg", vec3(-0.1, -0.2, -0.64), vec3(0., -0.27, 0.5), vec3(0., -0.05, 0.46));

	
	Joint *neck = new Joint(new Obj3D("horse_neck.obj"), body);
	neck->scale(SCALE);
	neck->translate(vec3(0.05, 0.2, 0.6));
	neck->rotate(-0.3f * PI, vec3(1., 0., 0.));
	neck->name = "neck";
	neck->counter = 0;
	neck->max = 21;
	neck->min = -3;
	
	Joint *head = new Joint(new Obj3D("horse_head2.obj"), neck);
	head->scale(SCALE);
	head->rotate(0.42f * PI, vec3(1., 0., 0.));
	head->translate(vec3(-0.05, -0.02, 0.18), 3.5f, SCALE);
	head->name = "head";
	head->counter = 0;
	head->max = 11;
	head->min = -10;
	
	Joint *tail = new Joint(new Obj3D("horse_tail.obj"), body);
	auto tailScale = vec3(0.3, 0.3, 0.3);
	tail->translate(vec3(0., 0.23, -0.92));
	tail->rotate(0.58f * PI, vec3(1., 0., 0.));
	tail->rotate(0.5f * PI, vec3(0., 0., 1.));
	tail->scale(tailScale);
	tail->name = "tail";
	tail->rotate_axis = vec3(0., 1., 0.);
	tail->counter = 0;
	tail->max = 0;
	tail->min = -17;
	
	return body;
}

void paintCoordinateSystem() { 
	glm::mat4 csSave = Model;
	Model = glm::scale(mat4(1.f), glm::vec3(5, 0.01, 0.01));
	sendMVP();
	drawCube();
	Model = glm::scale(mat4(1.f), glm::vec3(0.01, 5.0, 0.01));
	sendMVP();
	drawCube();
	Model = glm::scale(mat4(1.f), glm::vec3(0.01, 0.01, 5.0));
	sendMVP();
	drawCube();
	Model = csSave;
}


void drawGround(Obj3D* groundObj) {
	glm::mat4 groundSave = Model;
	Model = glm::scale(Model, glm::vec3(10000.0, 0.01, 10000.0));
	Model = glm::translate(Model, vec3(0., -150., 0.));
	sendMVP();
	groundObj->display();
	Model = groundSave;
}

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		exit(EXIT_FAILURE);
	}

	// Fehler werden auf stderr ausgegeben, s. o.
	glfwSetErrorCallback(error_callback);

	// Open a window and create its OpenGL context
	// glfwWindowHint vorher aufrufen, um erforderliche Resourcen festzulegen
	GLFWwindow* window = glfwCreateWindow(WINDOW_X, // Breite
										  WINDOW_Y,  // Hoehe
										  "Horse Simulation", // Ueberschrift
										  NULL,  // windowed mode
										  NULL); // shared window

	/*
	// Mouse
	//sets Cursor Pos Callback, method cursor_position_callback displays mouse position coordinates on screen
	glfwSetCursorPosCallback(window, cursor_position_callback);
	//Makes sure mous cursor is visible in window
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	//detecs when cursor exits and enters window
	glfwSetCursorEnterCallback(window, cursor_enter_callback);

	//detects when action on mouse button e.. left button pressed and released
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	//ensure pressed button gets detected by next poll
	glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, 1);
	*/

	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// Make the window's context current (wird nicht automatisch gemacht)
    glfwMakeContextCurrent(window);

	// Initialize GLEW
	// GLEW erm�glicht Zugriff auf OpenGL-API > 1.1
	glewExperimental = true; // Needed for core profile

	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Auf Keyboard-Events reagieren
	glfwSetKeyCallback(window, key_callback);

	// Background
	glClearColor(0.529f, 0.807f, 0.980f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
		
	// Create and compile our GLSL program from the shaders
	//programID = LoadShaders("TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader");
///Achtung die folgende Zeile ersetzt eine andere ######
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");

	glUseProgram(programID);


	//double xPos = 0;
	//double yPos = 0;

	
	// Build skeleton
	Joint *horse = makeHorseSkeleton();
	horse->getAllChildren(&all_joints);
	set_selected_joint(0);
	GLuint WoodTexture = loadBMP_custom(TEXTURE_FILE);
	GLuint GrassTexture = loadBMP_custom(GRASS_TEXTURE_FILE);
	cout << "Texture ids: " << WoodTexture << ", " << GrassTexture << endl;

	Obj3D* groundObj = new Obj3D("cube2.obj");

	// Eventloop
	while (!glfwWindowShouldClose(window))
	{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // |Z-Buffer muss gel�scht werden	


		//glfwGetCursorPos(window, &xPos, &yPos);

		// Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
		Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
		
		//Camera Projection
		//Projection = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);
		// Camera matrix
		/*
		View = glm::lookAt(glm::vec3(CAMERA_COORDINATES), // Camera in World Space
						   glm::vec3(0,0,0),  // Camera look-at target
						   glm::vec3(0,1,0)); // World upwards direction (Y goes upwards)
		*/
		View = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

		Model = mat4(1.f);
		//paintCoordinateSystem();
		

		Model = glm::rotate(Model, angleX, glm::vec3(1.0f, 0.0f, 0.0f));
		Model = glm::rotate(Model, angleY, glm::vec3(0.0f, 1.0f, 0.0f));
		Model = glm::rotate(Model, angleZ, glm::vec3(0.0f, 0.0f, 1.0f));
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, GrassTexture);
		glUniform1i(glGetUniformLocation(programID, "myTextureSampler"), 0);

		drawGround(groundObj);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, WoodTexture);

		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(glGetUniformLocation(programID, "myTextureSampler"), 0);

		// Draw the skeleton at the origin
		horse->displayRecursive(Model, &sendModel);
				
		glUniform3f(glGetUniformLocation(programID, "LightPosition_worldspace"), WORLD_LIGHT_COORDINATES);
		
		// Swap buffers
		glfwSwapBuffers(window);
		// Poll for and process events 
		glfwPollEvents();
	} 

	glDeleteProgram(programID);
	glDeleteTextures(1, &WoodTexture);
	glDeleteTextures(1, &GrassTexture);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	return 0;
}

/*
void cursor_position_callback(GLFWwindow* window, double xPos, double yPos)
{
	std::cout << xPos << " : " << yPos << std::endl;
}

void cursor_enter_callback(GLFWwindow* window, int entered)
{
	if (entered)
	{
		std::cout << "Entered Window" << std::endl;
	}
	else
	{
		std::cout << "Left Window" << std::endl;
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		std::cout << "Left button pressed" << std::endl;
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		std::cout << "Left button released" << std::endl;
	}
}
*/
/*void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	//avoid big jump when mouse first enters the screen
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	//calculate the offset movement between the last and current frame:
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.05f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	//add the offset values to globally declared pitch and yaw values: 
	yaw += xoffset;
	pitch += yoffset;
	// some constraints to avoid weird camera movement
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;
	// calculate the actual direction vector from the resulting yaw and pitch value
	glm::vec3 front;
	front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	front.y = sin(glm::radians(pitch));
	front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}
*/

