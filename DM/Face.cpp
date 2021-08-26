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

void Face::calNormalOfFace(){
    glm::vec3 n1 = vertexs[1]->position - vertexs[0]->position;
    glm::vec3 n2 = vertexs[2]->position - vertexs[0]->position;
    normal = glm::normalize(glm::cross(n1, n2));
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

void Face::deleteChild(Face* child){
    for (auto it = children.begin(); it != children.end(); it++) {
        if ((*it)->faceId == child->faceId) {
            children.erase(it);
            break;
        }
    }
}

void Face::deleteBorder(Edge* edge){
    for (auto it = borders.begin(); it != borders.end(); it++) {
        if ((*it)->edgeId == edge->edgeId) {
            borders.erase(it);
            break;
        }
    }
}
