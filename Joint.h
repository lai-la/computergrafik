#pragma once

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

//Include GLEW
#include <GL/glew.h>

//Include GLFW
#include <GLFW/glfw3.h>

//Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
using namespace glm;
using namespace std;

#include "Obj3D.h"

class Joint
{
    // Main transform of this joint.
    // Relative to parent (origin parameter in displayRecursive).
    glm::mat4 transform = glm::mat4(1.f);
    
    // Optional extra transform, not propagated to children, for scaling etc.
    // Relative to 'transform'.
    glm::mat4 extraTransform = glm::mat4(1.f);
    
    vector<Joint*> children;
    Obj3D *obj;
    
public:
    Joint(Obj3D *obj, Joint *parent);
    
    bool visible = true;
    string name = "unknown";
    vec3 rotate_axis = vec3(1., 0., 0.); // Used for rotate(float)
	
	int counter = 0;
	int max = 0;
	int min = 0;
    
    void displayRecursive(glm::mat4 origin, void (*sendModel)(glm::mat4 model)); // Display self and children
    void getAllChildren(vector<Joint*> *target);
    
    void scale(vec3 scale); // Applies to extraTransform, instead of transform
    void rotate(float degrees); // rotate() is prepended, so it happends before all other transforms
    void rotate(float degrees, vec3 axis);
    void translate(vec3 trans); // Append translation
    void translate(vec3 baseTrans, float parentLen, vec3 parentScale); // Move to the end of the parent

};
