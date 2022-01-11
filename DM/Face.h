#ifndef RESEARCH_Face_H
#define RESEARCH_Face_H

#include "Edge.h"

class Face {
public:
    std::vector<Vertex*> vertexs;   //面上包含的顶点
    std::vector<Edge*> edges;       //面上包含的边
    glm::vec3 normal;               //平面法向量
    glm::vec4 formula;              //记录下平面的表达式（ax+by+cz+d=0）
    std::vector<float> angles;      //面上的角度
    char buf[2];
    int faceId;
    std::list<Face*> children;      //Mesh的面只包含其他face，非Mesh的面只包含自己
    bool isMesh = true;
    std::list<Edge*> borders;       //Mesh的面的边界线（可能是原来的边拆成了几段）
    bool deleted = false;
public:
    Face();
    void setNormal(float i, float j, float k);  //给平面法向量赋值并且将向量单位化（使用glm函数库）  下同，通过vec3直接赋值
    void setNormal(glm::vec3& vec);
    void calNormalOfFace();                     //通过叉乘，选取平面上三点，确定平面的法向量
    void calFormula();                          //点乘计算平面的解析方程
    void setVertex(std::vector<Vertex*>& vs);   //修改vertexs向量组的值
    void setId(int id);                         //设置平面ID的函数
    void deleteChild(Face* child);
    void deleteBorder(Edge* edge);              //删除面的某个边界线
    
    ~Face();
};


#endif //RESEARCH_Face_H
