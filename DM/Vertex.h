#ifndef RESEARCH_Vertex_H
#define RESEARCH_Vertex_H

#include<glm/glm.hpp>

class Vertex {
public:
    glm::vec3 position;
    int vertexId;
    float q;
    float eph = 0x7f7fffff;
    int lambda = 0;

public:
    Vertex();
    Vertex(float  x, float  y, float  z);
    void init(glm::vec3& p);
    void setId(int id);
    ~Vertex();
};


#endif //RESEARCH_Vertex_H
