#include "Face.h"
Face::Face() {
}

void Face::setNormal(float i, float j, float k) {
    normal[0] = i;
    normal[1] = j;
    normal[2] = k;
}

void Face::setVertex(std::vector<Vertex>& vs){
    vertexs = vs;
}

void Face::setId(int id){
    faceId = id;
}

Face::~Face() {
//    delete vertexs;
}