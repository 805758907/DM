#ifndef RESEARCH_Face_H
#define RESEARCH_Face_H

#include "Edge.h"

class Face {
public:
    std::vector<Vertex> vertexs;
    std::vector<Edge> edges;
    glm::vec3 normal;
    char buf[2];
    int faceId;
public:
    Face();
    void setNormal(float i, float j, float k);
    void setVertex(std::vector<Vertex>& vs);
    void setId(int id);
    ~Face();
};


#endif //RESEARCH_Face_H
