#ifndef RESEARCH_Vertex_H
#define RESEARCH_Vertex_H

#include<glm/glm.hpp>
#include<list>
#include<vector>

class Edge;

class Vertex {
public:
    glm::vec3 position;
    int vertexId;
    glm::mat4 Q;
    float eph = 0x7f7fffff;
    int lambda = 0;
    bool deleted = false;
    std::vector<Edge*> incidentEdges;
    Edge* e = nullptr;
    bool typeI = false;
    bool typeII = false;
public:
    Vertex();
    Vertex(float x, float y, float z);
    void init(glm::vec3& p);
    void setId(int id);
    ~Vertex();
};


#endif //RESEARCH_Vertex_H
