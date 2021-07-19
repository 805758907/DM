#ifndef RESEARCH_Face_H
#define RESEARCH_Face_H

#include "Edge.h"

class Face {
public:
    std::vector<Vertex*> vertexs;
    std::vector<Edge*> edges;
    glm::vec3 normal;
    std::vector<double> angles;
    char buf[2];
    int faceId;
public:
    Face();
    void setNormal(float i, float j, float k);
    void setNormal(glm::vec3& vec);
    void setVertex(std::vector<Vertex*>& vs);
    void setId(int id);
    ~Face();
};


#endif //RESEARCH_Face_H
