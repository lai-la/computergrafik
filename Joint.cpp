
#include "Joint.h"
#include <iostream>

using namespace std;
#define GLM_ENABLE_EXPERIMENTAL
//#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/quaternion.hpp>
//#include <glm/gtx/matrix_decompose.hpp> 

Joint::Joint(Obj3D *obj, Joint *parent) : obj(obj) {
  if (parent) parent->children.push_back(this);
}

void Joint::displayRecursive(glm::mat4 origin, void (*sendModel)(glm::mat4 model)) {
  // Set our own transformation and display
  glm::mat4 transformed = origin * transform;
  if (visible) {
    (*sendModel)(transformed * extraTransform);
    obj->display();
  }
  
  // Iterate children and display them in the new transform
  for (std::vector<Joint*>::iterator it = this->children.begin(); it != this->children.end(); ++it) {
    (*it)->displayRecursive(transformed, sendModel);
  }
}

void Joint::getAllChildren(vector<Joint*> *target) {
  for (std::vector<Joint*>::iterator it = this->children.begin(); it != this->children.end(); ++it) {
    target->push_back((*it));
    (*it)->getAllChildren(target);
  }
}

void Joint::rotate(float degrees) {
  this->rotate(degrees, this->rotate_axis);
}

void Joint::rotate(float degrees, vec3 axis) {
  // Let rotation happen first
  this->transform = this->transform * glm::rotate(mat4(1.f), degrees, axis);
}

void Joint::scale(vec3 scale) {
  this->extraTransform = glm::scale(mat4(1.), scale) * this->extraTransform;
}

void Joint::translate(vec3 trans) {
  this->transform = glm::translate(mat4(1.f), trans) * this->transform;
}

void Joint::translate(vec3 baseTrans, float parentLen, vec3 parentScale) {
  this->translate(parentScale * vec3(0., 0., parentLen) + baseTrans);
}
