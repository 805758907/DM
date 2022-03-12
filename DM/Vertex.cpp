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
    position = p;
}

void Vertex::setId(int id) {
    vertexId = id;
}

Vertex::~Vertex() {}