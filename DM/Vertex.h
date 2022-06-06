#ifndef RESEARCH_Vertex_H
#define RESEARCH_Vertex_H

#include<glm/glm.hpp>
#include<list>
#include<vector>

class Edge;
class Face;

class Vertex {
public:
    glm::vec3 position = glm::vec3(0,0,0);     //三维的向量，用来存储点的坐标
    int vertexId = -1;           //存储点在点集中的ID
    glm::mat4 Q = glm::mat4(0.0f);
    float eph = 0x7f7fffff;
    int lambda = 0;
    bool deleted = false;                   //判断在生成DM时是否需要删除
    std::vector<Edge*> incidentEdges;       //储存与该点连接的边
    std::vector<Vertex*> incidentVertexes;  //储存与该点有边相连的点
    Edge* e = nullptr;                      //要简化的边
    int eIndex = -1;                        //e在incidentEdges的序号
    bool typeI = false;
    bool typeII = false;
    std::vector<std::pair<int, int>> flippedEdge;
    std::vector<Face*> incidentFaces;       //incidentFaces和incidentEdges没有联系
    bool boundary = false;
    bool visited = false;

public:
    Vertex();                           //不赋予坐标，仅初始化编号为-1
    Vertex(float x, float y, float z);  //创建新的坐标为(x,y,z)编号初始化为-1的点
    void init(glm::vec3& p);            //初始化点的坐标或者给点的坐标赋予新的值
    void setId(int id);                 //设置点的ID
    ~Vertex();
};


#endif //RESEARCH_Vertex_H
