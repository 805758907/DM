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
    calFormula();
}

void Face::calFormula(){
    float d = -glm::dot(normal, vertexs[0]->position);
    formula = glm::vec4(normal, d);
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
    bool faceDeleted = false;
    for (auto it = children.begin(); it != children.end(); it++) {
        if ((*it)->faceId == child->faceId) {
            children.erase(it);
            faceDeleted = true;
            break;
        }
    }
    if (!faceDeleted) {
        printf("deleteChild error\n");
    }

}

void Face::deleteBorder(Edge* edge){
    bool borderDeleted = false;
    for (auto it = borders.begin(); it != borders.end(); it++) {
        if ((*it)->edgeId == edge->edgeId) {
            borders.erase(it);
            borderDeleted = true;
            break;
        }
    }
    if (!borderDeleted) {
        printf("deleteBorder error\n");
    }
}

glm::vec3 Face::getMassCenter()
{
    glm::vec3 centerOfV0V1 = vertexs[0]->position + vertexs[1]->position;
    centerOfV0V1 = glm::vec3(centerOfV0V1.x/2, centerOfV0V1.y / 2, centerOfV0V1.z / 2);
    glm::vec3 direction = vertexs[2]->position - centerOfV0V1;
    glm::vec3 massCenter = centerOfV0V1 + glm::vec3(direction.x / 3, direction.y / 3, direction.z / 3);

    return massCenter;
}
