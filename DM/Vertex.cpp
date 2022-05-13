#include "Edge.h"

Vertex::Vertex() {
    vertexId = -1;
    e = nullptr;
}

Vertex::Vertex(float newX, float newY, float newZ) {
    position[0] = newX;
    position[1] = newY;
    position[2] = newZ;
    vertexId = -1;
}

void Vertex::init(glm::vec3& p){
    position[0] = p.x;
    position[1] = p.y;
    position[2] = p.z;
}

void Vertex::setId(int id) {
    vertexId = id;
}

Vertex::~Vertex() {}