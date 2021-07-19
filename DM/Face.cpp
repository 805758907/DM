#include "Face.h"
Face::Face() {
}

void Face::setNormal(float i, float j, float k) {
    glm::vec3 nv(i, j, k);
    normal = glm::normalize(nv);
}

void Face::setNormal(glm::vec3& vec) {
    glm::vec3 n(vec.x, vec.y, vec.z);
    normal = n;
}

void Face::setVertex(std::vector<Vertex*>& vs){
    vertexs = vs;
}

void Face::setId(int id){
    faceId = id;
}

Face::~Face() {
//    delete vertexs;
}