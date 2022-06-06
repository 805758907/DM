#ifndef RESEARCH_Vertex_H
#define RESEARCH_Vertex_H

#include<glm/glm.hpp>
#include<list>
#include<vector>

class Edge;
class Face;

class Vertex {
public:
    glm::vec3 position = glm::vec3(0,0,0);     //��ά�������������洢�������
    int vertexId = -1;           //�洢���ڵ㼯�е�ID
    glm::mat4 Q = glm::mat4(0.0f);
    float eph = 0x7f7fffff;
    int lambda = 0;
    bool deleted = false;                   //�ж�������DMʱ�Ƿ���Ҫɾ��
    std::vector<Edge*> incidentEdges;       //������õ����ӵı�
    std::vector<Vertex*> incidentVertexes;  //������õ��б������ĵ�
    Edge* e = nullptr;                      //Ҫ�򻯵ı�
    int eIndex = -1;                        //e��incidentEdges�����
    bool typeI = false;
    bool typeII = false;
    std::vector<std::pair<int, int>> flippedEdge;
    std::vector<Face*> incidentFaces;       //incidentFaces��incidentEdgesû����ϵ
    bool boundary = false;
    bool visited = false;

public:
    Vertex();                           //���������꣬����ʼ�����Ϊ-1
    Vertex(float x, float y, float z);  //�����µ�����Ϊ(x,y,z)��ų�ʼ��Ϊ-1�ĵ�
    void init(glm::vec3& p);            //��ʼ�����������߸�������긳���µ�ֵ
    void setId(int id);                 //���õ��ID
    ~Vertex();
};


#endif //RESEARCH_Vertex_H
