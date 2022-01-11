#ifndef RESEARCH_Face_H
#define RESEARCH_Face_H

#include "Edge.h"

class Face {
public:
    std::vector<Vertex*> vertexs;   //���ϰ����Ķ���
    std::vector<Edge*> edges;       //���ϰ����ı�
    glm::vec3 normal;               //ƽ�淨����
    glm::vec4 formula;              //��¼��ƽ��ı��ʽ��ax+by+cz+d=0��
    std::vector<float> angles;      //���ϵĽǶ�
    char buf[2];
    int faceId;
    std::list<Face*> children;      //Mesh����ֻ��������face����Mesh����ֻ�����Լ�
    bool isMesh = true;
    std::list<Edge*> borders;       //Mesh����ı߽��ߣ�������ԭ���ı߲���˼��Σ�
    bool deleted = false;
public:
    Face();
    void setNormal(float i, float j, float k);  //��ƽ�淨������ֵ���ҽ�������λ����ʹ��glm�����⣩  ��ͬ��ͨ��vec3ֱ�Ӹ�ֵ
    void setNormal(glm::vec3& vec);
    void calNormalOfFace();                     //ͨ����ˣ�ѡȡƽ�������㣬ȷ��ƽ��ķ�����
    void calFormula();                          //��˼���ƽ��Ľ�������
    void setVertex(std::vector<Vertex*>& vs);   //�޸�vertexs�������ֵ
    void setId(int id);                         //����ƽ��ID�ĺ���
    void deleteChild(Face* child);
    void deleteBorder(Edge* edge);              //ɾ�����ĳ���߽���
    
    ~Face();
};


#endif //RESEARCH_Face_H
