#include <stdio.h>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <queue>
#include "time.h"//统计时间需添加的头文件
#include "Mesh.h"

float PI = 3.141592653589793115997963468544185161590576171875;

struct ephCmp {
    bool operator()(Vertex* x, Vertex* y) {
        return x->eph > y->eph;     // x小的优先级高
    }
};

//计算单位法向量，v1、v2、v3三点逆时针排布
glm::vec3 calNormal(glm::vec3& v1, glm::vec3& v2, glm::vec3& v3) {
    glm::vec3 n = glm::cross(v2 - v1, v3 - v2);
    if (n != glm::vec3(0, 0, 0)) {
        return glm::normalize(n);
    }
    return n;
}

//边的比较函数，先比较第一个点（下标最小的点），如果相同再比较第二个点
bool compare(const Edge* a, const  Edge* b) {
    if (a->vertexe1->vertexId != b->vertexe1->vertexId) {
        return a->vertexe1->vertexId < b->vertexe1->vertexId;
    }
    return a->vertexe2->vertexId < b->vertexe2->vertexId;
}

//求外接圆圆心
glm::vec3 solveCenterPointOfCircle(glm::vec3& pt0, glm::vec3& pt1, glm::vec3& pt2)
{
    double a1, b1, c1, d1;
    double a2, b2, c2, d2;
    double a3, b3, c3, d3;

    double x1 = pt0.x, y1 = pt0.y, z1 = pt0.z;
    double x2 = pt1.x, y2 = pt1.y, z2 = pt1.z;
    double x3 = pt2.x, y3 = pt2.y, z3 = pt2.z;

    a1 = (y1 * z2 - y2 * z1 - y1 * z3 + y3 * z1 + y2 * z3 - y3 * z2);
    b1 = -(x1 * z2 - x2 * z1 - x1 * z3 + x3 * z1 + x2 * z3 - x3 * z2);
    c1 = (x1 * y2 - x2 * y1 - x1 * y3 + x3 * y1 + x2 * y3 - x3 * y2);
    d1 = -(x1 * y2 * z3 - x1 * y3 * z2 - x2 * y1 * z3 + x2 * y3 * z1 + x3 * y1 * z2 - x3 * y2 * z1);

    a2 = 2 * (x2 - x1);
    b2 = 2 * (y2 - y1);
    c2 = 2 * (z2 - z1);
    d2 = x1 * x1 + y1 * y1 + z1 * z1 - x2 * x2 - y2 * y2 - z2 * z2;

    a3 = 2 * (x3 - x1);
    b3 = 2 * (y3 - y1);
    c3 = 2 * (z3 - z1);
    d3 = x1 * x1 + y1 * y1 + z1 * z1 - x3 * x3 - y3 * y3 - z3 * z3;
    glm::vec3 res;


    res.x = -(b1 * c2 * d3 - b1 * c3 * d2 - b2 * c1 * d3 + b2 * c3 * d1 + b3 * c1 * d2 - b3 * c2 * d1)
        / (a1 * b2 * c3 - a1 * b3 * c2 - a2 * b1 * c3 + a2 * b3 * c1 + a3 * b1 * c2 - a3 * b2 * c1);
    res.y = (a1 * c2 * d3 - a1 * c3 * d2 - a2 * c1 * d3 + a2 * c3 * d1 + a3 * c1 * d2 - a3 * c2 * d1)
        / (a1 * b2 * c3 - a1 * b3 * c2 - a2 * b1 * c3 + a2 * b3 * c1 + a3 * b1 * c2 - a3 * b2 * c1);
    res.z = -(a1 * b2 * d3 - a1 * b3 * d2 - a2 * b1 * d3 + a2 * b3 * d1 + a3 * b1 * d2 - a3 * b2 * d1)
        / (a1 * b2 * c3 - a1 * b3 * c2 - a2 * b1 * c3 + a2 * b3 * c1 + a3 * b1 * c2 - a3 * b2 * c1);

    return res;
}

//外接圆与直线相交于v1，求另一个交点在v1v3这条直线上的位置,如果只有一个交点，则返回0
//获得的是在v1v3上的比例（以v1为起点）
float getAnotherPoint(glm::vec3& v3, glm::vec3& v1, glm::vec3& center) {
    float x31 = v3.x - v1.x;
    float x21 = center.x - v1.x;
    float y31 = v3.y - v1.y;
    float y21 = center.y - v1.y;
    float z31 = v3.z - v1.z;
    float z21 = center.z - v1.z;


    float t = 2 * (x31 * x21 + y31 * y21 + z31 * z21) / (x31 * x31 + y31 * y31 + z31 * z31);

    //return glm::vec3(v1.x + t * x31, v1.y + t * y31, v1.z + t * z31);
    return t;//如果是相切则返回0
}

//在v1v2这条边上，找到一点p使得v1v3垂直于v3p。p的坐标可以表示为（t(x2-x1)+x1,t(y2-y1)+y1,t(z2-z1)+z1）。t是p点在v1->v2向量的比例（以v1为起点）
float getPointOfBoundaryEdge(glm::vec3& v1, glm::vec3& v2, glm::vec3& v3) {
    float x31 = v3.x - v1.x;
    float x21 = v2.x - v1.x;
    float y31 = v3.y - v1.y;
    float y21 = v2.y - v1.y;
    float z31 = v3.z - v1.z;
    float z21 = v2.z - v1.z;

    float t = (x31 * x31 + y31 * y31 + z31 * z31) / (x21 * x31 + y21 * y31 + z21 * z31);

    return t;
}

//各个满足的区间分别为tStart右侧，tEnd左侧，tE右侧，tH右侧，tF左侧，tG左侧，找到其中重叠的最大部分
std::pair<float, float> getSplitRegion(float tStart, float tEnd, float tE, float tH, float tF, float tG) {
    if (tStart > tEnd) {
        printf("debug");
    }
    float min1;           //当tStart > tEnd时出现bug，所以不能取（tStart，tEnd）
    float min2;
    float max1;
    float max2;
    if (tE > tH) {
        min1 = tH;
        min2 = tE;
    }
    else {
        min1 = tE;
        min2 = tH;
    }
    if (tF > tG) {
        max1 = tG;
        max2 = tF;
    }
    else {
        max1 = tF;
        max2 = tG;
    }
    if (min2 <= tStart) {
        if (max1 >= tEnd) {
            return std::pair<float, float>(tStart, tEnd);   //四个满足
        }
        if (max1 >= tStart) {
            return std::pair<float, float>(tStart, max1);   //四个满足
        }
        if (max2 >= tEnd) {
            return std::pair<float, float>(tStart, tEnd);   //三个满足
        }
        if (max2 >= tStart) {
            return std::pair<float, float>(tStart, max2);   //三个满足
        }
        return std::pair<float, float>(tStart, tEnd);       //两个满足
    }
    if (min1 <= tStart) {
        if (min2 <= tEnd) {
            if (max1 >= tEnd) {
                return std::pair<float, float>(min2, tEnd);   //四个满足
            }
            if (max1 >= min2) {
                return std::pair<float, float>(min2, max1);   //四个满足
            }
            if (max1 >= tStart) {
                return std::pair<float, float>(tStart, max1);   //三个满足
            }
            if (max2 >= tEnd) {
                return std::pair<float, float>(min2, tEnd);     //三个满足
            }
            if (max2 >= min2) {
                return std::pair<float, float>(min2, max2);   //三个满足
            }
            return std::pair<float, float>(min2, tEnd);   //两个满足
        }
        if (max1 >= tEnd) {
            return std::pair<float, float>(tStart, tEnd);   //三个满足
        }
        if (max1 >= tStart) {
            return std::pair<float, float>(tStart, max1);   //三个满足
        }
        if (max2 >= tEnd) {
            return std::pair<float, float>(tStart, tEnd);   //两个满足
        }
        if (max2 >= tStart) {
            return std::pair<float, float>(tStart, max2);   //两个满足
        }
        return std::pair<float, float>(tStart, tEnd);       //一个满足
    }
    if (max1 >= tEnd) {
        if (min2 <= tEnd) {
            return std::pair<float, float>(min2, tEnd);     //四个都满足
        }
        if (min1 <= tEnd) {
            return std::pair<float, float>(min1, tEnd);     //三个满足
        }
        return std::pair<float, float>(tStart, tEnd);     //两个满足
    }
    if (max2 >= tEnd) {
        if (max1 >= min2) {
            return std::pair<float, float>(min2, max1);     //四个都满足
        }
        if (min1 <= max1) {
            return std::pair<float, float>(min1, max1);     //三个满足
        }
        if (min2 <= tEnd) {
            return std::pair<float, float>(min2, tEnd);     //三个满足
        }
        if (max1 >= tStart)
            return std::pair<float, float>(tStart, max1);     //两个满足
        if (min1 <= tEnd) {
            return std::pair<float, float>(min1, tEnd);     //两个满足
        }
        return std::pair<float, float>(tStart, tEnd);     //一个满足
    }
    if (min2 <= max1) {
        return std::pair<float, float>(min2, max1);     //四个都满足
    }
    if (min1 <= max1) {
        return std::pair<float, float>(min1, max1);     //三个满足
    }
    if (min2 <= max2) {
        return std::pair<float, float>(min2, max2);     //三个满足
    }
    if (min1 <= max2) {
        return std::pair<float, float>(min1, max2);     //两个满足
    }
    if (min2 <= tEnd) {
        return std::pair<float, float>(min2, tEnd);     //两个满足
    }
    if (max1 >= tStart) {
        return std::pair<float, float>(tStart, max1);     //两个满足
    }
    if (min1 <= tEnd) {
        return std::pair<float, float>(min1, tEnd);   //一个满足
    }
    if (max2 >= tStart) {
        return std::pair<float, float>(tStart, max2);   //一个满足
    }
    return std::pair<float, float>(tStart, tEnd);   //0个满足
    /*
    float min1;         //min1 < min2 < min3
    float min2;
    float min3;
    float max1;         //max1 < max2 < max3
    float max2;
    float max3;
    if (tE > tH) {
        if (tStart > tE) {
            min1 = tH;
            min2 = tE;
            min3 = tStart;
        }
        else if (tStart > tH) {
            min1 = tH;
            min2 = tStart;
            min3 = tE;
        }
        else {
            min1 = tStart;
            min2 = tH;
            min3 = tE;
        }
        
    }
    else {
        if (tStart > tH) {
            min1 = tE;
            min2 = tH;
            min3 = tStart;
        }
        else if (tStart > tE) {
            min1 = tE;
            min2 = tStart;
            min3 = tH;
        }
        else {
            min1 = tStart;
            min2 = tE;
            min3 = tH;
        }
    }
    if (tF > tG) {
        if (tEnd > tF) {
            max1 = tG;
            max2 = tF;
            max3 = tEnd;
        }
        else if (tEnd > tG) {
            max1 = tG;
            max2 = tEnd;
            max3 = tF;
        }
        else {
            max1 = tEnd;
            max2 = tG;
            max3 = tF;
        }
    }
    else {
        if (tEnd > tG) {
            max1 = tF;
            max2 = tG;
            max3 = tEnd;
        }
        else if (tEnd > tF) {
            max1 = tF;
            max2 = tEnd;
            max3 = tG;
        }
        else {
            max1 = tEnd;
            max2 = tF;
            max3 = tG;
        }
    }
    if (min3 <= max1) {
        return  std::pair<float, float>(min3, max1);    //6满足
    }
    else if (min3 <= max2) {
        return  std::pair<float, float>(min3, max2);    //5满足,max1不满足
    }else if()
    */
}

std::pair<float, float> getSplitRegionOfBoundaryEdge(float tStart, float tEnd, float tD, float tE) {
    if (tD <= tStart) {
        if (tE >= tEnd) {
            return std::pair<float, float>(tStart, tEnd);
        }
        if (tE > tStart) {
            std::pair<float, float>(tStart, tE);
        }
        return std::pair<float, float>(tStart, tEnd);
    }
    else if (tD <= tEnd) {
        if (tE >= tEnd) {
            return std::pair<float, float>(tD, tEnd);
        }
        if (tE > tD) {
            std::pair<float, float>(tD, tE);
        }
        return std::pair<float, float>(tD, tEnd);
    }
    else {
        if (tE >= tEnd) {
            return std::pair<float, float>(tStart, tEnd);
        }
        if (tE > tStart) {
            std::pair<float, float>(tStart, tE);
        }
        return std::pair<float, float>(tStart, tEnd);
    }
}

Vertex* getThirdVertexInFace(Face* face, int v1, int v2) {
    for (auto it = face->vertexs.begin(); it != face->vertexs.end(); it++) {
        if ((*it)->vertexId != v1 && (*it)->vertexId != v2) {
            return (*it);
        }
    }
    return nullptr;
}

int getIndexOfVertexInFace(Face* face, Vertex* v) {
    for (int i = 0; i < 3; i++) {
        if (face->vertexs[i]->vertexId == v->vertexId) {
            return i;
        }
    }
    printf("bug: face doesn't have this vertex\n");
    return -1;
}

bool judgeAntiClockWish(Face* face, Vertex* v1, Vertex* v2) {//v1->v2是逆时针
    int index1 = getIndexOfVertexInFace(face, v1);
    int index2 = getIndexOfVertexInFace(face, v2);

    if (index1 < index2) {
        if (index2 - index1 == 1)
            return true;
        return false;
    }
    else {
        if (index1 - index2 == 1)
            return false;
        return true;
    }

}

//旋转pointRotated：以normalFixed作为固定面的法向量，normalRotated为要旋转平面的法向量，edgeNormal作为交线的法向量，startPosition作为交线的起始点
glm::vec3 rotatePoint(glm::vec3& normalFixed, glm::vec3& normalRotated, glm::vec3& edgeNormal, glm::vec3& startPosition, glm::vec3& pointRotated) {
    float cosAngle = glm::dot(normalFixed, normalRotated);   //都是单位向量
    if (cosAngle > 1) {
        cosAngle = 1;
    }
    else if (cosAngle < -1) {
        cosAngle = -1;
    }
    float angle = acos(cosAngle);                           //旋转的弧度
    if (isnan(angle)) {
        printf("nan when compute acos\n");
    }
    glm::vec3 o = glm::normalize(glm::cross(normalFixed, normalRotated));   //判断旋转方向，与交线同向则顺时针旋转；反向则逆时针旋转
    glm::mat4 trans = glm::mat4(1.0f); //创建单位矩阵
    //修改两向量同向、反向的判断
    if (glm::dot(o, edgeNormal) > 0) {  //同向
        angle = -angle;
    }
    else {                              //反向(因为glm::rotate函数本来就是逆时针旋转的）
        ;
    }

    trans = glm::rotate(trans, angle, edgeNormal);          //旋转矩阵
    //把旋转轴移动到原点，再旋转，再移动回来
    glm::mat4 moveTo = glm::mat4(1.0f);
    moveTo = glm::translate(moveTo, -startPosition);

    glm::mat4 moveBack = glm::mat4(1.0f);
    moveBack = glm::translate(moveBack, startPosition);

    glm::vec4 point = glm::vec4(pointRotated, 1);
    return moveBack * trans * moveTo * point;
}

void setMeshFaceOfNewEdge(Face* meshFace, Face* newFace) {  //对新创建的面的新的边，添加其所在的meshface
    for (auto it = newFace->edges.begin(); it != newFace->edges.end(); it++) {
        bool exist = false;
        for (auto faceIt = (*it)->meshFaceId.begin(); faceIt != (*it)->meshFaceId.end(); faceIt++) {
            if ((*faceIt) == meshFace->faceId) {
                exist = true;
                break;
            }
        }
        if (!exist) {
            (*it)->meshFaceId.insert(meshFace->faceId);
        }
    }
}

static inline unsigned int FindNextChar(unsigned int start, const char* str, unsigned int length, char token)
{
    unsigned int result = start;
    while (result < length)
    {
        result++;
        if (str[result] == token)
            break;
    }

    return result;
}

glm::vec3 Mesh::ParseOBJVec3(const std::string& line)
{
    unsigned int tokenLength = line.length();
    const char* tokenString = line.c_str();

    unsigned int vertIndexStart = 2;

    while (vertIndexStart < tokenLength)
    {
        if (tokenString[vertIndexStart] != ' ')
            break;
        vertIndexStart++;
    }

    unsigned int vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, ' ');

    float x = atof(line.substr(vertIndexStart, vertIndexEnd - vertIndexStart).c_str());

    vertIndexStart = vertIndexEnd + 1;
    vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, ' ');

    float y = atof(line.substr(vertIndexStart, vertIndexEnd - vertIndexStart).c_str());

    vertIndexStart = vertIndexEnd + 1;
    vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, ' ');

    float z = atof(line.substr(vertIndexStart, vertIndexEnd - vertIndexStart).c_str());

    return glm::vec3(x, y, z);
}


void Mesh::CreateOBJFace(const std::string& line)
{
    unsigned int tokenLength = line.length();
    const char* tokenString = line.c_str();

    std::vector<Vertex*> vs;
    unsigned int vertIndexStart = 2;

    while (vertIndexStart < tokenLength)
    {
        if (tokenString[vertIndexStart] != ' ')
            break;
        vertIndexStart++;
    }

    unsigned int vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, ' ');

    int v1 = atoi(line.substr(vertIndexStart, vertIndexEnd - vertIndexStart).c_str()) - 1;
    vs.push_back(vertexes[v1]);

    vertIndexStart = vertIndexEnd + 1;
    vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, ' ');

    int v2 = atoi(line.substr(vertIndexStart, vertIndexEnd - vertIndexStart).c_str()) - 1;
    vs.push_back(vertexes[v2]);

    vertIndexStart = vertIndexEnd + 1;
    vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, ' ');

    int v3 = atoi(line.substr(vertIndexStart, vertIndexEnd - vertIndexStart).c_str()) - 1;
    vs.push_back(vertexes[v3]);

    Face* face = new Face();
    face->setId(faces.size());
    /*if (faces.size() == 20605 || faces.size() == 460) {
        printf("debug");
    }*/
    face->setVertex(vs);
    face->children.push_back(face);
    face->isMesh = true;
    generateEdgeOfFace(face, true);
    face->calNormalOfFace();

    faces.push_back(face);
    faceNum++;


    if (vertIndexEnd < tokenLength - 1) {
        Face* face2 = new Face();
        std::vector<Vertex*> vs2;
        vertIndexStart = vertIndexEnd + 1;
        vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, ' ');

        vs2.push_back(vertexes[v1]);
        vs2.push_back(vertexes[v3]);
        int v4 = atoi(line.substr(vertIndexStart, vertIndexEnd - vertIndexStart).c_str()) - 1;
        vs2.push_back(vertexes[v4]);

        face2->setId(faces.size());
        face2->setVertex(vs2);
        face2->children.push_back(face2);
        face2->isMesh = true;
        generateEdgeOfFace(face2, true);
        face2->calNormalOfFace();
        faces.push_back(face2);
        faceNum++;
    }

}

void Mesh::deleteFace(Face* face, bool deleteEdgeFromList) {
    int faceId = face->faceId;

    for (auto edgeIt = face->edges.begin(); edgeIt != face->edges.end(); edgeIt++) {    //所有边的face列表删去这个面
        auto iter = (*edgeIt)->faceId.find(faceId);
        (*edgeIt)->faceId.erase(iter);

        //meshFaceId也删去这个面（如果有的话）
        iter = (*edgeIt)->meshFaceId.find(faceId);
        if (iter != (*edgeIt)->meshFaceId.end()) {
            (*edgeIt)->meshFaceId.erase(iter);
        }
        
        if ((*edgeIt)->faceId.size() == 0) {    //如果这条边不在任何面上了，那么把它删除
            std::pair<int, int> edgePair((*edgeIt)->vertexe1->vertexId, (*edgeIt)->vertexe2->vertexId);
            m_hash_edge.erase(edgePair);
            (*edgeIt)->deleted = true;
            (*edgeIt)->meshFaceId.clear();
            //TODO是否要delete？
            if (deleteEdgeFromList) {
                auto it = edges.end();
                --it;
                bool edgeDeleted = false;
                for (; it != edges.begin(); it--) {
                    if ((*it)->edgeId == (*edgeIt)->edgeId) {
                        (*it)->deleted = true;
                        delete(*it);
                        edges.erase(it);
                        edgeDeleted = true;
                        break;
                    }
                }
                if (!edgeDeleted && (*it)->edgeId == (*edgeIt)->edgeId) {//遍历到第一个元素
                    (*it)->deleted = true;
                    delete(*it);
                    edges.erase(it);
                }
            }
            
        }
    }
    face->deleted = true;
    face->vertexs.clear();
    face->edges.clear();
}


//在v1v2这条边上，找到一点p使得v1p垂直于v3p。p的坐标可以表示为（t(x2-x1)+x1,t(y2-y1)+y1,t(z2-z1)+z1）。t是p点在v1->v2向量的比例（以v1为起点）
//向量v1->v2 · p->v3 = 0
float Mesh::getPerpendicularPoint(glm::vec3& v1, glm::vec3& v2, glm::vec3& v3) {
    float x31 = v3.x - v1.x;
    float x21 = v2.x - v1.x;
    float y31 = v3.y - v1.y;
    float y21 = v2.y - v1.y;
    float z31 = v3.z - v1.z;
    float z21 = v2.z - v1.z;

    float t = (x31 * x21 + y31 * y21 + z31 * z21) / (x21 * x21 + y21 * y21 + z21 * z21);

    return t;
}

bool VertexBelongToFace(Vertex* v, Face* f) {
    for (auto it = f->vertexs.begin(); it != f->vertexs.end(); it++) {
        if ((*it)->vertexId == v->vertexId) {
            return true;
        }
    }
    return false;
}

Mesh::Mesh() {
}

Mesh::~Mesh() {
}

bool Mesh::readSTL(const char* fileName) {
    //判断类型，调用不同的函数来生成
    FILE* fStl = nullptr;
    int err = fopen_s(&fStl, fileName, "r");//r表示只允许对文件进行读取而不能改写文件内容   r+则允许读写
    if (fStl == nullptr) {
        return false;
    }
    if (err != 0) {
        return false;
    }//文件打开成功是0反之非0

    char buf[6] = {};
    if (fread(buf, 5, 1, fStl) != 1) {
        fclose(fStl);
        return false;
    }
    fclose(fStl);

    if (strcmp(buf, "solid") == 0) { //是ASCII STL   STL Ascll开头为solid filename stl
        return readSTLASCII(fileName);
    }
    else {                         //STL Binary文件开头为UNIT8[80]   Header
        return readSTLBinary(fileName);
    }



}

bool Mesh::readOBJ(const char* fileName)
{
    std::ifstream file;
    file.open(fileName);
    int bla1;
    std::string line;

    if (file.is_open()) {
        while (file.good())
        {
            getline(file, line);
            unsigned int lineLength = line.length();
            if (lineLength < 2)
                continue;

            const char* lineCStr = line.c_str();

            switch (lineCStr[0])
            {
            case 'v':
                if (lineCStr[1] == 't')
                {
                    //                    this->uvs.push_back(ParseOBJVec2(line));
                }
                else if (lineCStr[1] == 'n')
                {
                    this->normals.push_back(ParseOBJVec3(line));
                    this->colors.push_back((normals.back() + glm::vec3(1)) * 0.5f);
                }
                else if (lineCStr[1] == ' ' || lineCStr[1] == '\t') {
                    glm::vec3 position = ParseOBJVec3(line);
                    int id = findVertexByPoint(position);
                    Vertex* vertex = nullptr;
                    if (id == -1) {			//没找到，该point是新的
                        vertex = new Vertex();
                        vertex->init(position);
                        vertex->setId(vertexNum);
                        vertexNum++;
                        m_hash_point[position] = vertexes.size();
                        
                    }
                    else {
                        vertex = vertexes[id];
                    }
                    vertexes.push_back(vertex); //在obj中，可能存在两个顶点实质为同一个点的情况,所以为了能正常创建面片
                                                //需要先把顶点指针存入
                }

                break;
            case 'f':
                CreateOBJFace(line);
                break;
            default: break;
            };
        }

        //删除冗余顶点
        for (auto it = vertexes.begin(); it != vertexes.end(); ) {
            if ((*it)->visited == false) {  //第一个被访问到的点
                (*it)->visited = true;
                it++;
            }
            else {
                it = vertexes.erase(it);
            }
        }
    }
    else {
        return false;
    }
    return true;

    
}

bool Mesh::readSTLASCII(const char* fileName) {
    std::ifstream fileSTL(fileName, std::ios::in | std::ios::binary);//in:只读   binary:二进制模式
    char buf[255];
    char end[] = "endsolid";
    char begin[] = "facet";
    fileSTL >> buf;         //solid
    fileSTL >> partName;    //filenamestl 
    float v1, v2, v3;
    int faceId = 0;
    int vertexId = 0;
    if (strcmp(partName, begin) == 0) {
        goto begin_read;
    }

    while (fileSTL >> buf) {
        if (strcmp(buf, end) == 0) { //endsolid结束 0表示两个值相等
            break;
        }
        else if (strcmp(buf, begin) == 0) { //facet
begin_read:
            Face* face = new Face();
            fileSTL >> buf;     // normal 

            fileSTL >> v1;
            fileSTL >> v2;
            fileSTL >> v3;
            face->setNormal(v1, v2, v3);
            std::vector<Vertex*> vs;

            fileSTL >> buf;     //outer 
            fileSTL >> buf;     //loop

            for (unsigned j = 0; j < 3; j++) {
                fileSTL >> buf; //vertex 

                fileSTL >> v1;
                fileSTL >> v2;
                fileSTL >> v3;
                glm::vec3 p = glm::vec3(v1, v2, v3);

                Vertex* vertex = generateNewVertex(p);
                vs.push_back(vertex);
                /*                int id = findVertexByPoint(p);
                                if (id == -1) {			//没找到，该point是新的
                                    Vertex* vertex = new Vertex();
                                    vertex->init(p);
                                    vertex->setId(vertexId);
                                    m_hash_point[p] = vertexId;
                                    vertexId++;
                                    vertexes.push_back(vertex);
                                    vs.push_back(vertex);
                                }
                                else {
                                    vs.push_back(vertexes[id]);
                                }
                */
            }
            fileSTL >> buf;     //endloop
            fileSTL >> buf;     //endfacet

            face->setId(faceId);
            face->setVertex(vs);
            generateEdgeOfFace(face, true);

            face->children.push_back(face);
            faces.push_back(face);
            faceId++;
            faceNum++;
        }

    }
    return true;
}

bool Mesh::readSTLBinary(const char* fileName) {
    FILE* fStl = nullptr;
    int err = fopen_s(&fStl, fileName, "rb");
    if (fStl == nullptr) {
        return false;
    }
    if (err != 0) {
        return false;
    }

    if (fread(partName, 80, 1, fStl) != 1) {
        return false;
    }
    if (fread(&faceNum, 4, 1, fStl) != 1) {
        return false;
    }
    int vertexId = 0;
    int faceId = 0;
    float v1, v2, v3;
    char buf[2] = {};
    clock_t start, finish;
    double duration;
    for (int i = 0; i < faceNum; i++) {
        Face* face = new Face();
        fread(&v1, 4, 1, fStl);//读取法线数据
        fread(&v2, 4, 1, fStl);
        fread(&v3, 4, 1, fStl);
        face->setNormal(v1, v2, v3);
        std::vector<Vertex*> vs;
        for (int j = 0; j < 3; j++) {
            fread(&v1, 4, 1, fStl);//读取顶点的数据
            fread(&v2, 4, 1, fStl);
            fread(&v3, 4, 1, fStl);
            glm::vec3 p = glm::vec3((float)v1, (float)v2, (float)v3);

            Vertex* vertex = generateNewVertex(p);
            vs.push_back(vertex);
            /*          int id = findVertexByPoint(p);
                        if (id == -1) {			//没找到，该point是新的
                            Vertex* vertex = new Vertex();
                            vertex->init(p);
                            vertex->setId(vertexId);
                            m_hash_point[p] = vertexId;
                            vertexId++;
                            vertexes.push_back(vertex);
                            vs.push_back(vertex);
                        }else {
                            vs.push_back(vertexes[id]);
                        }
            */
        }
        //        start = clock();
        fread(face->buf, 2, 1, fStl);//读取保留项数据，这一项一般没什么用，这里选择读取是为了移动文件指针
        face->setId(faceId);
        face->setVertex(vs);
        face->children.push_back(face);
        generateEdgeOfFace(face, true);

        faces.push_back(face);
        faceId++;
    }

    return true;

}

bool Mesh::saveOBJ(const char* fileName)
{
    FILE* fp = nullptr;
    int err = fopen_s(&fp, fileName, "w");
    if (fp == nullptr || err != 0) {
        return false;
    }
    int vertexCount = vertexes.size();
    for (int i = 0; i < vertexCount; i++) {
        glm::vec3 position = vertexes[i]->position;
        fprintf(fp, "v %f %f %f\n", position.x, position.y, position.z);
    }
    fprintf(fp, "\n");
    int totalFace = faces.size();
    int resultFaceCount = 0;
    for (int i = 0; i < totalFace; i++) {   //遍历faces，找到Mesh面
        if (faces[i]->isMesh == true) {
            for (auto it = faces[i]->children.begin(); it != faces[i]->children.end(); it++) {//把mesh的children全输出了
                if ((*it)->deleted == false) {
                    fprintf(fp, "f %d %d %d\n", (*it)->vertexs[0]->vertexId + 1, (*it)->vertexs[1]->vertexId + 1, (*it)->vertexs[2]->vertexId + 1);
                    resultFaceCount++;
                }
            }
        }
    }
    printf("resultFaceCount: %d\n", resultFaceCount);
    fclose(fp);
    return true;
}

bool Mesh::saveSTLASCII(const char* fileName) {

    char* fileInf = new char[200];
    int len = 0;
    while (fileName[len] != '\0') {
        len++;
    }
    snprintf(fileInf, len + 7, "solid %s", fileName);
    FILE* fp = nullptr;
    int err = fopen_s(&fp, fileName, "w");

    if (err != 0 || fp == nullptr) {
        return false;
    }

    fprintf(fp, "%s\n", fileInf);

    int totalFace = faces.size();
    for (int i = 0; i < totalFace; i++) {
        if (faces[i]->isMesh == true) {
            for (auto it = faces[i]->children.begin(); it != faces[i]->children.end(); it++) {
                glm::vec3 v1 = (*((*it)->vertexs.begin()))->position;
                float v1x = v1.x;
                float v1y = v1.y;
                float v1z = v1.z;

                glm::vec3 v2 = (*(++(*it)->vertexs.begin()))->position;
                float v2x = v2.x;
                float v2y = v2.y;
                float v2z = v2.z;

                glm::vec3 v3 = (*(++(++(*it)->vertexs.begin())))->position;
                float v3x = v3.x;
                float v3y = v3.y;
                float v3z = v3.z;


                float nx = (v1y - v3y) * (v2z - v3z) - (v1z - v3z) * (v2y - v3y);
                float ny = (v1z - v3z) * (v2x - v3x) - (v2z - v3z) * (v1x - v3x);
                float nz = (v1x - v3x) * (v2y - v3y) - (v2x - v3x) * (v1y - v3y);

                float nxyz = sqrt(nx * nx + ny * ny + nz * nz);
                //fprintf(fp, "facet normal %f %f %f\n", (*it)->normal.x, (*it) -> normal.y, (*it)->normal.z);
                fprintf(fp, "facet normal %f %f %f\n", nx / nxyz, ny / nxyz, nz / nxyz);
                fprintf(fp, "outer loop\n");
                fprintf(fp, "vertex %f %f %f\n", v1x, v1y, v1z);
                fprintf(fp, "vertex %f %f %f\n", v2x, v2y, v2z);
                fprintf(fp, "vertex %f %f %f\n", v3x, v3y, v3z);
                fprintf(fp, "endloop\n");
                fprintf(fp, "endfacet\n");
            }
        }
    }
    snprintf(fileInf, len + 10, "endsolid %s", fileName);
    fprintf(fp, "%s\n", fileInf);
    fclose(fp);

    delete[]fileInf;
    return true;
}

bool Mesh::saveSTLBinary(const char* fileName) {

    //char *saveName = new char[100];
    //sprintf(saveName, "%s%s.stl", pathName, fileName);

    //FILE *fp = fopen(saveName, "wb");
    //delete[]savename;
    int count = 0;
    int totalFace = faces.size();
    for (int i = 0; i < totalFace; i++) {
        if (faces[i]->isMesh == true) {
            for (auto it = faces[i]->children.begin(); it != faces[i]->children.end(); it++) {
                if ((*it)->deleted == false) {
                    count++;
                }
            }
        }

    }
    printf("origin: %d\n", faceNum);
    printf("current: %d\n", count);

    FILE* fp = nullptr;
    int err = fopen_s(&fp, fileName, "wb");
    if (fp == nullptr) {
        return false;
    }
    if (err != 0) {
        return false;
    }
    float* data = new float[12];

    fwrite(partName, sizeof(char), 80, fp);
    //fwrite(&faceNum, sizeof(int), 1, fp);
    fwrite(&count, sizeof(int), 1, fp);

    for (int i = 0; i < totalFace; i++) {
        if (faces[i]->isMesh == true) {
            for (auto it = faces[i]->children.begin(); it != faces[i]->children.end(); it++) {
                if ((*it)->deleted == false) {
                    data[0] = (*it)->normal.x;
                    data[1] = (*it)->normal.y;
                    data[2] = (*it)->normal.z;

                    data[3] = (*it)->vertexs[0]->position.x;
                    data[4] = (*it)->vertexs[0]->position.y;
                    data[5] = (*it)->vertexs[0]->position.z;
                    data[6] = (*it)->vertexs[1]->position.x;
                    data[7] = (*it)->vertexs[1]->position.y;
                    data[8] = (*it)->vertexs[1]->position.z;
                    data[9] = (*it)->vertexs[2]->position.x;
                    data[10] = (*it)->vertexs[2]->position.y;
                    data[11] = (*it)->vertexs[2]->position.z;

                    fwrite(data, sizeof(float), 12, fp);
                    fwrite((*it)->buf, sizeof(char), 2, fp);
                }
            }
        }
        /*        data[0] = faces[i]->normal.x;
                data[1] = faces[i]->normal.y;
                data[2] = faces[i]->normal.z;

                data[3] = faces[i]->vertexs[0]->position.x;
                data[4] = faces[i]->vertexs[0]->position.y;
                data[5] = faces[i]->vertexs[0]->position.z;
                data[6] = faces[i]->vertexs[1]->position.x;
                data[7] = faces[i]->vertexs[1]->position.y;
                data[8] = faces[i]->vertexs[1]->position.z;
                data[9] = faces[i]->vertexs[2]->position.x;
                data[10] = faces[i]->vertexs[2]->position.y;
                data[11] = faces[i]->vertexs[2]->position.z;

                fwrite(data, sizeof(float), 12, fp);
                fwrite(faces[i]->buf, sizeof(char), 2, fp);

        */
    }

    fclose(fp);

    delete[]data;
    return true;

}


int Mesh::findVertexByPoint(glm::vec3& p) {
    auto it = m_hash_point.find(p);
    if (it != m_hash_point.end()) {
        return it->second;
    }
    else {
        return -1;
    }
}

Edge* Mesh::findEdgeByPoints(Vertex* v1, Vertex* v2) {
    std::pair<int, int> p(v1->vertexId, v2->vertexId);
    auto it = m_hash_edge.find(p);
    if (it != m_hash_edge.end()) {
        return it->second;
    }
    else {
        return nullptr;
    }
}

Edge* Mesh::generateEdge(Vertex* v1, Vertex* v2) {
    Vertex* pFirst, * pSecond;

    if (v1->vertexId < v2->vertexId) {
        pFirst = v1;
        pSecond = v2;
    }
    else {
        pFirst = v2;
        pSecond = v1;
    }
    //if (pFirst->vertexId == 174 && pSecond->vertexId == 175) {
    //    printf("debug");
    //}
    Edge* edge = findEdgeByPoints(pFirst, pSecond);
    if (edge == nullptr) {			//没找到，该edge是新的
        edge = new Edge();
        edge->vertexe1 = pFirst;
        edge->vertexe2 = pSecond;
        edge->length = glm::distance(edge->vertexe1->position, edge->vertexe2->position);
        if (edge->length > lMax) {
            lMax = edge->length;
        }
        if (edge->length < lMin) {
            lMin = edge->length;
        }
        edge->edgeId = edgeIndex;
        edgeIndex++;
        edge->parent = edge;
        std::pair<int, int> pair1(pFirst->vertexId, pSecond->vertexId);
        m_hash_edge[pair1] = edge;
        edges.push_back(edge);
    }
    else if (edge->deleted == true) {
        printf("deleted Edge\n");
    }
    return edge;
}

void Mesh::generateEdgeOfFace(Face* face, bool meshEdge) {  //在建立面的时候，就会求对应的角度
    Edge* edge01 = generateEdge(face->vertexs[0], face->vertexs[1]);
    edge01->faceId.insert(face->faceId);
    
    if (meshEdge) {
        edge01->meshFaceId.insert(face->faceId);
    }
    face->edges.push_back(edge01);
    face->borders.push_back(edge01);
    double length01 = edge01->length;

    Edge* edge12 = generateEdge(face->vertexs[1], face->vertexs[2]);
    edge12->faceId.insert(face->faceId);
    if (meshEdge) {
        edge12->meshFaceId.insert(face->faceId);
    }
    face->edges.push_back(edge12);
    face->borders.push_back(edge12);
    double length12 = edge12->length;

    Edge* edge02 = generateEdge(face->vertexs[0], face->vertexs[2]);
    edge02->faceId.insert(face->faceId);
    if (meshEdge) {
        edge02->meshFaceId.insert(face->faceId);
    }
    face->edges.push_back(edge02);
    face->borders.push_back(edge02);
    double length02 = edge02->length;

    //求角度
//    start = clock();
//    float cosTheta0 = glm::dot((face->vertexs[1]->position - face->vertexs[0]->position), (face->vertexs[2]->position - face->vertexs[0]->position)) / length01 / length02;
    double cosTheta0 = (length01 * length01 + length02 * length02 - length12 * length12) / 2 / length01 / length02;
    double sinTheta0 = sqrt(1 - cosTheta0 * cosTheta0);
    if (sinTheta0 < sinThetaMin) {
        sinThetaMin = sinTheta0;
    }
    face->angles.push_back(cosTheta0);
    double cosTheta1 = (length01 * length01 + length12 * length12 - length02 * length02) / 2 / length01 / length12;
    //    float cosTheta1 = glm::dot((face->vertexs[0]->position - face->vertexs[1]->position), (face->vertexs[2]->position - face->vertexs[1]->position)) / length01 / length12;
    double sinTheta1 = sqrt(1 - cosTheta1 * cosTheta1);
    if (sinTheta1 < sinThetaMin) {
        sinThetaMin = sinTheta1;
    }
    face->angles.push_back(cosTheta1);
    double cosTheta2 = (length12 * length12 + length02 * length02 - length01 * length01) / 2 / length12 / length02;
    //    float cosTheta2 = glm::dot((face->vertexs[0]->position - face->vertexs[2]->position), (face->vertexs[1]->position - face->vertexs[2]->position)) / length12 / length02;
    double sinTheta2 = sqrt(1 - cosTheta2 * cosTheta2);
    if (sinTheta2 < sinThetaMin) {
        sinThetaMin = sinTheta2;
    }

    face->angles.push_back(cosTheta2);
    //if (edge01->faceId.size() > 3) {
    //    printf("4 face of edge\n;");
    //}if (edge02->faceId.size() > 3) {
    //    printf("4 face of edge\n;");
    //}if (edge12->faceId.size() > 3) {
    //    printf("4 face of edge\n;");
    //}

}

void Mesh::computeParameter() {
    float v1 = lMin * sinThetaMin / (0.5 + sinThetaMin);
    float v2 = lMin / 2;

    if (v1 < v2) {
        rhoV = v1;
    }
    else {
        rhoV = v2;
    }
    rhoE = 2 * rhoV * sinThetaMin;
}

void Mesh::generateDM() {
    //printf("edges: %d\n", edges.size());
    computeParameter();
    /*for (auto it = edges.begin(); it != edges.end(); it++) {
        (*it)->constructCe(rhoV, rhoE);
    }
    */
    findAllNLDEdges();
    while (!NLDEdges.empty()) {
        Edge* currentEdge = NLDEdges.top();
        NLDEdges.pop();
        if (currentEdge && currentEdge->deleted == false) {
            if (isNLD(currentEdge)) {
                if (currentEdge->flippable == false) {
                    handleNonFlippableNLDEdge(currentEdge);
                }
                else {
                    flipEdge(currentEdge);
                }
            }
            currentEdge->inStack = false;
        }
        
    }


    int count = 0;
    for (auto it = edges.begin(); it != edges.end(); it++) {
        if (isNLD(*it)) {
            count++;
        }
    }
    printf("NLD:%d\n", count);
}


/*
* 先把这条边所属的两个三角面以及该三角面的另外四条边对应的三角面（总共6个面）旋转到同一个平面上,再求I0-I4,找到公共部分
* 分割，添加四条边，四个面
* 遍历这四个面的所有边，翻NLD，把不能翻转的加入栈中
*/

void Mesh::handleNonFlippableNLDEdge(Edge* edge) {//edge就是edgeAB
    //saveOBJ("beforeSplite.obj");
    if (edge->edgeId == 66) {
        printf("debug");
    }
    Vertex* vertexA = edge->vertexe1;
    Vertex* vertexB = edge->vertexe2;
    if (edge->faceId.size() == 2) {
        Face* faceABD = faces[*(edge->faceId.begin())];
        Face* faceABC = faces[*(++edge->faceId.begin())];
        //以faceABD为基准平面，找到faceABC顶点旋转后的位置v',以此来构建外接圆与Is
        Vertex* vertexC = nullptr;
        vertexC = getThirdVertexInFace(faceABC, vertexA->vertexId, vertexB->vertexId);
        Vertex* vertexD = nullptr;
        vertexD = getThirdVertexInFace(faceABD, vertexA->vertexId, vertexB->vertexId);

        glm::vec3 normalABD = faceABD->normal;
        glm::vec3 normalABC = faceABC->normal;
        glm::vec4 vertexCPosition = glm::vec4(vertexC->position, 1);

        Edge* edgeAC = nullptr, * edgeBC = nullptr;
        for (auto it = faceABC->edges.begin(); it != faceABC->edges.end(); it++) {
            //printf_s("!!!%d\n", (*it)->edgeId);
            if ((*it)->edgeId != edge->edgeId) {
                if ((*it)->vertexe1->vertexId == vertexA->vertexId || (*it)->vertexe2->vertexId == vertexA->vertexId) {
                    edgeAC = (*it);
                }
                else {
                    edgeBC = (*it);
                }
            }
        }

        float cosAngle = glm::dot(normalABD, normalABC);                        //两个法向量的夹角cos，法向量都是单位向量
        if (cosAngle > 1) {
            cosAngle = 1;
        }
        else if (cosAngle < -1) {
            cosAngle = -1;
        }
        float angle = acos(cosAngle);                                           //旋转的弧度
        glm::vec3 ve = glm::normalize(vertexA->position - vertexB->position);   //该边的向量B->A
        glm::vec3 o = glm::normalize(glm::cross(normalABD, normalABC));         //o是法向量的叉乘，此结果与交线AB平行。用于判断旋转方向，与BA同向则绕BA顺时针旋转；反向则逆时针旋转
        //if ((o.x >= 0 && ve.x < 0) || (o.x <= 0 && ve.x > 0)) {                 //反向(因为glm::rotate函数本来就是逆时针旋转的）
        //    //angle = -angle;
        //}
        //else {
        //    angle = -angle;
        //}

        //修改两向量同向、反向的判断，点乘>0为同向，<0为反向
        if (glm::dot(o, ve) > 0) {  //同向
            angle = -angle;
        }
        else {                              //反向(因为glm::rotate函数本来就是逆时针旋转的）
            ;
        }

        glm::mat4 trans = glm::mat4(1.0f);              //创建单位矩阵
        trans = glm::rotate(trans, angle, ve);          //旋转矩阵

//        glm::mat4 trans2 = glm::mat4(1.0f);              //创建单位矩阵
//        trans2 = glm::rotate(trans2, -angle, ve);          //旋转矩阵

        //把旋转轴移动到原点，再旋转，再移动回来
        glm::mat4 moveTo = glm::mat4(1.0f);
        moveTo = glm::translate(moveTo, -vertexB->position);

        glm::mat4 moveBack = glm::mat4(1.0f);
        moveBack = glm::translate(moveBack, vertexB->position);

        bool antiClockWishOfABInABD = judgeAntiClockWish(faceABD, vertexA, vertexB);

        //旋转对角
        glm::vec4 newVertexCPosition = moveBack * trans * moveTo * vertexCPosition;
        glm::vec3 newVertexCPos = glm::vec3(newVertexCPosition);
        Edge* parent = edge->parent;
        Vertex* pStart = parent->vertexe1;
        Vertex* pEnd = parent->vertexe2;

        //        glm::vec4 newVertexCPosition2 = moveBack * trans2 * moveTo * vertexCPosition;

        //tX是指X点的可用区间的端点在AB上的比例，以A为起点
        glm::vec3 center = solveCenterPointOfCircle(vertexA->position, vertexD->position, newVertexCPos);
        float tEnd = getAnotherPoint(vertexB->position, vertexA->position, center);
        center = solveCenterPointOfCircle(vertexB->position, vertexD->position, newVertexCPos);
        float tStart = 1 - getAnotherPoint(vertexA->position, vertexB->position, center);

        if (tEnd > 1) {
            tEnd = 1;
        }
        if (tStart < 0) {
            tStart = 0;
        }

        float tE = 0;
        //还要把相关的另两个三角面的对角也旋转了
        if (edgeAC != NULL) {
            if (edgeAC->faceId.size() == 2) {
                //    printf_s("2\n");
                glm::vec4 vertexEPosition = glm::vec4(getAnotherVertexPositionByEdge(faceABC, edgeAC), 1);
                glm::vec4 newVertexEPosition = moveBack * trans * moveTo * vertexEPosition;

                //再旋转另外两个三角面到达在同一平面
                glm::vec3 newVertexEPos = glm::vec3(newVertexEPosition);

                /*            Face* faceACE = nullptr;
                            if (*(edgeAC->faceId.begin()) != faceABC->faceId) {
                                faceACE = faces[*(edgeAC->faceId.begin())];
                            }
                            else {
                                faceACE = faces[*(++edgeAC->faceId.begin())];
                            }
                            bool antiClockWish = judgeAntiClockWish(faceACE, vertexA, vertexC);
                */
                glm::vec3 normalACE;
                if (antiClockWishOfABInABD) {            //AB在ABD为逆时针，则在ABC为顺时针，所以AC在ABC为逆时针，在ACE为顺时针
                    normalACE = calNormal(newVertexCPos, vertexA->position, newVertexEPos);
                }
                else {
                    normalACE = calNormal(vertexA->position, newVertexCPos, newVertexEPos);

                }

                glm::vec3 newAC = glm::normalize(newVertexCPos - vertexA->position); //该边的向量
                newVertexEPos = rotatePoint(normalABD, normalACE, newAC, vertexA->position, newVertexEPos);

                center = solveCenterPointOfCircle(vertexA->position, newVertexEPos, newVertexCPos);
                tE = getAnotherPoint(vertexB->position, vertexA->position, center);
                if (tE < 0) {
                //    tE = 0;
                }
            }
            else {  //需要切割产生的对角小于等于90度。在AB上找到一点S，使得CS垂直于BS
                tE = getPerpendicularPoint(vertexA->position, vertexB->position, newVertexCPos);
            }
        }
        else {
            tE = 0;
        }

        float tF = 1;
        if (edgeBC != NULL) {
            if (edgeBC->faceId.size() == 2) {
                glm::vec4 vertexFPosition = glm::vec4(getAnotherVertexPositionByEdge(faceABC, edgeBC), 1);
                glm::vec4 newVertexFPosition = moveBack * trans * moveTo * vertexFPosition;

                //再旋转另外两个三角面到达在同一平面
                glm::vec3 newVertexFPos = glm::vec3(newVertexFPosition);
                glm::vec3 normalBCF;
                if (antiClockWishOfABInABD) {            //AB在ABD为逆时针，则在ABC为顺时针，所以BC在ABC为顺时针，在BCF为逆时针
                    normalBCF = calNormal(vertexB->position, newVertexCPos, newVertexFPos);
                }
                else {
                    normalBCF = calNormal(newVertexCPos, vertexB->position, newVertexFPos);

                }
                //            glm::vec3 normalBCF = calNormal(vertexB->position, newVertexCPos, newVertexFPos);
                glm::vec3 newBC = glm::normalize(newVertexCPos - vertexB->position); //该边的向量
                newVertexFPos = rotatePoint(normalABD, normalBCF, newBC, vertexB->position, newVertexFPos);

                center = solveCenterPointOfCircle(vertexB->position, newVertexCPos, newVertexFPos);
                tF = 1 - getAnotherPoint(vertexA->position, vertexB->position, center);
                if (tF > 1) {
                //    tF = 1;
                }
            }
            else {  //需要切割产生的对角小于等于90度。在AB上找到一点S，使得CS垂直于BS
                tF = getPerpendicularPoint(vertexA->position, vertexB->position, newVertexCPos);
            }

        }
        else {
            tF = 1;
        }


        //再旋转与ABD相邻的两个三角面到达在同一平面

        Edge* edgeAD = nullptr, * edgeBD = nullptr;
        for (auto it = faceABD->edges.begin(); it != faceABD->edges.end(); it++) {
            if ((*it)->edgeId != edge->edgeId) {
                if ((*it)->vertexe1->vertexId == vertexA->vertexId || (*it)->vertexe2->vertexId == vertexA->vertexId) {
                    edgeAD = (*it);
                }
                else {
                    edgeBD = (*it);
                }
            }
        }
        //求另两个三角面的顶点
        float tH = 0;
        if (edgeAD != NULL) {
            if (edgeAD->faceId.size() == 2) {
                glm::vec4 vertexHPosition = glm::vec4(getAnotherVertexPositionByEdge(faceABD, edgeAD), 1);
                glm::vec3 vertexHPos = glm::vec3(vertexHPosition);

                glm::vec3 normalADH;
                if (antiClockWishOfABInABD) {            //AB在ABD为逆时针，则AD在ABD为顺时针，所以AD在ADH为逆时针
                    normalADH = calNormal(vertexA->position, vertexD->position, vertexHPos);
                }
                else {
                    normalADH = calNormal(vertexD->position, vertexA->position, vertexHPos);

                }

                //            glm::vec3 normalADH = calNormal(vertexA->position, vertexD->position, vertexHPos);
                glm::vec3 newAD = glm::normalize(vertexD->position - vertexA->position); //该边的向量
                vertexHPos = rotatePoint(normalABD, normalADH, newAD, vertexA->position, vertexHPos);


                center = solveCenterPointOfCircle(vertexA->position, vertexD->position, vertexHPos);
                tH = getAnotherPoint(vertexB->position, vertexA->position, center);
            }
            else {  //需要切割产生的对角小于等于90度。在AB上找到一点S，使得DS垂直于BS
                tH = getPerpendicularPoint(vertexA->position, vertexB->position, vertexD->position);
            }
        }
        else {
            tH = 0;
        }

        float tG = 1;
        if (edgeBD != NULL) {
            if (edgeBD->faceId.size() == 2) {
                glm::vec4 vertexGPosition = glm::vec4(getAnotherVertexPositionByEdge(faceABD, edgeBD), 1);

                glm::vec3 vertexGPos = glm::vec3(vertexGPosition);

                glm::vec3 normalBDG;
                if (antiClockWishOfABInABD) {            //AB在ABD为逆时针，则BD在ABD为逆时针，所以BD在BDG为顺时针
                    normalBDG = calNormal(vertexD->position, vertexB->position, vertexGPos);
                }
                else {
                    normalBDG = calNormal(vertexB->position, vertexD->position, vertexGPos);

                }
                glm::vec3 newBD = glm::normalize(vertexD->position - vertexB->position); //该边的向量
                vertexGPos = rotatePoint(normalABD, normalBDG, newBD, vertexB->position, vertexGPos);

                center = solveCenterPointOfCircle(vertexB->position, vertexD->position, vertexGPos);
                tG = 1 - getAnotherPoint(vertexA->position, vertexB->position, center);
                
                if (tG > 1) {
                    tG = 1;
                }
            }
            else {  //需要切割产生的对角小于等于90度。在AB上找到一点S，使得DS垂直于BS
                tG = getPerpendicularPoint(vertexA->position, vertexB->position, vertexD->position);
            }
        }
        else {
            tG = 1;
        }




        std::pair<float, float> tRegion = getSplitRegion(tStart, tEnd, tE, tH, tF, tG);//相对于AB而言
        glm::vec3 p1 = glm::vec3(vertexA->position.x + tRegion.first * (vertexB->position.x - vertexA->position.x), vertexA->position.y + tRegion.first * (vertexB->position.y - vertexA->position.y), vertexA->position.z + tRegion.first * (vertexB->position.z - vertexA->position.z));
        glm::vec3 p2 = glm::vec3(vertexA->position.x + tRegion.second * (vertexB->position.x - vertexA->position.x), vertexA->position.y + tRegion.second * (vertexB->position.y - vertexA->position.y), vertexA->position.z + tRegion.second * (vertexB->position.z - vertexA->position.z));

        //找到e ∈ E
        Edge* parentEdge = edge->parent;
        //glm::vec3 splitPosition = parentEdge->getSplitePosition(p1, p2);
        glm::vec3 splitPosition = parentEdge->getSplitePosition2(p1, p2, rhoV, rhoE, &m_hash_point);

        //printf("split edge %d;", edge->edgeId, splitPosition.x, splitPosition.y, splitPosition.z);

        Face* parentFaceOfABC = getParentFace(parentEdge, faceABC);
        Face* parentFaceOfABD = getParentFace(parentEdge, faceABD);

        //分割：添加点
        Vertex* vertexS = generateNewVertex(splitPosition);
        edge->splitted = true;

        //        glm::vec3 t1 = glm::normalize(parentEdge->vertexe1->position - splitPosition);
        //        glm::vec3 t2 = glm::normalize(splitPosition - parentEdge->vertexe2->position);


        bool antiClockWish = judgeAntiClockWish(faceABC, vertexA, vertexB);
        Face* faceACS = nullptr;
        Face* faceBCS = nullptr;
        Face* faceADS = nullptr;
        Face* faceBDS = nullptr;

        //添加四个面
        if (antiClockWish) {    //A、B是逆时针(在ABC这个面上）
            faceACS = generateNewFaceFromOldFace(faceABC, vertexA, vertexS, vertexC);
            faceBCS = generateNewFaceFromOldFace(faceABC, vertexS, vertexB, vertexC);
            faceADS = generateNewFaceFromOldFace(faceABD, vertexS, vertexA, vertexD);
            faceBDS = generateNewFaceFromOldFace(faceABD, vertexB, vertexS, vertexD);
        }
        else {
            faceACS = generateNewFaceFromOldFace(faceABC, vertexS, vertexA, vertexC);
            faceBCS = generateNewFaceFromOldFace(faceABC, vertexB, vertexS, vertexC);
            faceADS = generateNewFaceFromOldFace(faceABD, vertexA, vertexS, vertexD);
            faceBDS = generateNewFaceFromOldFace(faceABD, vertexS, vertexB, vertexD);
        }

        setMeshFaceOfNewEdge(parentFaceOfABC, faceACS);
        setMeshFaceOfNewEdge(parentFaceOfABC, faceBCS);
        setMeshFaceOfNewEdge(parentFaceOfABD, faceADS);
        setMeshFaceOfNewEdge(parentFaceOfABD, faceBDS);

        parentFaceOfABC->deleteChild(faceABC);
        parentFaceOfABD->deleteChild(faceABD);


        parentFaceOfABC->children.push_back(faceACS);
        parentFaceOfABC->children.push_back(faceBCS);
        parentFaceOfABD->children.push_back(faceADS);
        parentFaceOfABD->children.push_back(faceBDS);

        //更新ABC和ABD两个面原来边的面所属信息
        auto iter = edgeAD->faceId.find(faceABD->faceId);
        edgeAD->faceId.erase(iter);
        iter = edgeBD->faceId.find(faceABD->faceId);
        edgeBD->faceId.erase(iter);
        iter = edgeAC->faceId.find(faceABC->faceId);
        edgeAC->faceId.erase(iter);
        iter = edgeBC->faceId.find(faceABC->faceId);
        edgeBC->faceId.erase(iter);
        iter = edge->faceId.find(faceABC->faceId);
        edge->faceId.erase(iter);
        iter = edge->faceId.find(faceABD->faceId);
        edge->faceId.erase(iter);

        parentFaceOfABC->deleteBorder(edge);
        parentFaceOfABD->deleteBorder(edge);

        for (auto it = faceACS->edges.begin(); it != faceACS->edges.end(); it++) {
            if (((*it)->vertexe1->vertexId == vertexA->vertexId && (*it)->vertexe2->vertexId == vertexS->vertexId) ||
                ((*it)->vertexe1->vertexId == vertexS->vertexId && (*it)->vertexe2->vertexId == vertexA->vertexId)) {
                (*it)->parent = parentEdge;
                parentFaceOfABC->borders.push_back((*it));
                parentFaceOfABD->borders.push_back((*it));
                break;
            }
        }
        for (auto it = faceBCS->edges.begin(); it != faceBCS->edges.end(); it++) {
            if (((*it)->vertexe1->vertexId == vertexB->vertexId && (*it)->vertexe2->vertexId == vertexS->vertexId) ||
                ((*it)->vertexe1->vertexId == vertexS->vertexId && (*it)->vertexe2->vertexId == vertexB->vertexId)) {
                (*it)->parent = parentEdge;
                parentFaceOfABC->borders.push_back((*it));
                parentFaceOfABD->borders.push_back((*it));
                break;
            }
        }
        //saveOBJ("afterSplit.obj");
        //翻转parentFaceOfABC和parentFaceOfABD中所有flippable的NLD
        flipAllNLDEdgeInFace(parentFaceOfABC);
        flipAllNLDEdgeInFace(parentFaceOfABD);
        //saveOBJ("afterFlip.obj");
        addNewNonFlippableNLDEdge(parentFaceOfABC);
        addNewNonFlippableNLDEdge(parentFaceOfABD);
    }
    else {
        Face* faceABC = faces[*(edge->faceId.begin())];
        Vertex* vertexC = nullptr; 
        vertexC = getThirdVertexInFace(faceABC, vertexA->vertexId, vertexB->vertexId);
        Edge* edgeAC = nullptr, * edgeBC = nullptr;
        for (auto it = faceABC->edges.begin(); it != faceABC->edges.end(); it++) {
            if ((*it)->edgeId != edge->edgeId) {
                if ((*it)->vertexe1->vertexId == vertexA->vertexId || (*it)->vertexe2->vertexId == vertexA->vertexId) {
                    edgeAC = (*it);
                }
                else {
                    edgeBC = (*it);
                }
            }
        }
        float tEnd = getPointOfBoundaryEdge(vertexA->position, vertexB->position, vertexC->position);
        if (tEnd > 1) {
            tEnd = 1;
        }

        float tStart = 1 - getPointOfBoundaryEdge(vertexB->position, vertexA->position, vertexC->position);
        if (tStart < 0) {
            tStart = 0;
        }

        float tD;
        glm::vec3 center;
        if (edgeAC->faceId.size() == 2) {
            //旋转三角面到达在同一平面
            glm::vec4 vertexDPosition = glm::vec4(getAnotherVertexPositionByEdge(faceABC, edgeAC), 1);
            glm::vec3 newVertexDPos = glm::vec3(vertexDPosition);
            glm::vec3 normalACD = calNormal(vertexA->position, vertexC->position, newVertexDPos);
            glm::vec3 newAC = glm::normalize(vertexC->position - vertexA->position); //该边的向量
            newVertexDPos = rotatePoint(faceABC->normal, normalACD, newAC, vertexA->position, newVertexDPos);

            center = solveCenterPointOfCircle(vertexA->position, vertexC->position, newVertexDPos);
            tD = getAnotherPoint(vertexB->position, vertexA->position, center);
        }
        else {  //需要切割产生的对角小于等于90度。在AB上找到一点S，使得CS垂直于BS
            tD = getPerpendicularPoint(vertexA->position, vertexB->position, vertexC->position);
         }

        float tE = 1;
        if (edgeBC->faceId.size() == 2) {
            //printf_s("8\n");
            //旋转三角面到达在同一平面
            glm::vec4 vertexEPosition = glm::vec4(getAnotherVertexPositionByEdge(faceABC, edgeBC), 1);
            glm::vec3 newVertexEPos = glm::vec3(vertexEPosition);
            glm::vec3 normalBCE = calNormal(vertexB->position, vertexC->position, newVertexEPos);
            glm::vec3 newBC = glm::normalize(vertexC->position - vertexB->position); //该边的向量
            newVertexEPos = rotatePoint(faceABC->normal, normalBCE, newBC, vertexA->position, newVertexEPos);

            center = solveCenterPointOfCircle(vertexB->position, vertexC->position, newVertexEPos);
            tE = 1 - getAnotherPoint(vertexA->position, vertexB->position, center);

        }
        else {  //需要切割产生的对角小于等于90度。在AB上找到一点S，使得CS垂直于BS
            tE = getPerpendicularPoint(vertexA->position, vertexB->position, vertexC->position);
        }

        std::pair<float, float> tRegion = getSplitRegionOfBoundaryEdge(tStart, tEnd, tD, tE);//相对于AB而言
        glm::vec3 p1 = glm::vec3(vertexA->position.x + tRegion.first * (vertexB->position.x - vertexA->position.x), vertexA->position.y + tRegion.first * (vertexB->position.y - vertexA->position.y), vertexA->position.z + tRegion.first * (vertexB->position.z - vertexA->position.z));
        glm::vec3 p2 = glm::vec3(vertexA->position.x + tRegion.second * (vertexB->position.x - vertexA->position.x), vertexA->position.y + tRegion.second * (vertexB->position.y - vertexA->position.y), vertexA->position.z + tRegion.second * (vertexB->position.z - vertexA->position.z));

        //找到e ∈ E
        Edge* parentEdge = edge->parent;
        //glm::vec3 splitPosition = parentEdge->getSplitePosition(p1, p2);
        glm::vec3 splitPosition = parentEdge->getSplitePosition2(p1, p2, rhoV, rhoE, &m_hash_point);


        Face* parentFaceOfABC = getParentFace(parentEdge, faceABC);

        //分割：添加点
        Vertex* vertexS = generateNewVertex(splitPosition);
        edge->splitted = true;

        bool antiClockWish = judgeAntiClockWish(faceABC, vertexA, vertexB);
        Face* faceACS = nullptr;
        Face* faceBCS = nullptr;

        //添加两个面
        if (antiClockWish) {    //A、B是逆时针(在ABC这个面上）
            faceACS = generateNewFaceFromOldFace(faceABC, vertexA, vertexS, vertexC);
            faceBCS = generateNewFaceFromOldFace(faceABC, vertexS, vertexB, vertexC);
        }
        else {
            faceACS = generateNewFaceFromOldFace(faceABC, vertexS, vertexA, vertexC);
            faceBCS = generateNewFaceFromOldFace(faceABC, vertexB, vertexS, vertexC);
        }

        setMeshFaceOfNewEdge(parentFaceOfABC, faceACS);
        setMeshFaceOfNewEdge(parentFaceOfABC, faceBCS);

        parentFaceOfABC->deleteChild(faceABC);

        parentFaceOfABC->children.push_back(faceACS);
        parentFaceOfABC->children.push_back(faceBCS);

        //更新ABC原来边的面所属信息
        auto iter = edgeAC->faceId.find(faceABC->faceId);
        edgeAC->faceId.erase(iter);
        iter = edgeBC->faceId.find(faceABC->faceId);
        edgeBC->faceId.erase(iter);
        iter = edge->faceId.find(faceABC->faceId);
        edge->faceId.erase(iter);

        edge->deleted = true;

        parentFaceOfABC->deleteBorder(edge);

        for (auto it = faceACS->edges.begin(); it != faceACS->edges.end(); it++) {
            if (((*it)->vertexe1->vertexId == vertexA->vertexId && (*it)->vertexe2->vertexId == vertexS->vertexId) ||
                ((*it)->vertexe1->vertexId == vertexS->vertexId && (*it)->vertexe2->vertexId == vertexA->vertexId)) {
                (*it)->parent = parentEdge;
                parentFaceOfABC->borders.push_back((*it));
                break;
            }
        }
        for (auto it = faceBCS->edges.begin(); it != faceBCS->edges.end(); it++) {
            if (((*it)->vertexe1->vertexId == vertexB->vertexId && (*it)->vertexe2->vertexId == vertexS->vertexId) ||
                ((*it)->vertexe1->vertexId == vertexS->vertexId && (*it)->vertexe2->vertexId == vertexB->vertexId)) {
                (*it)->parent = parentEdge;
                parentFaceOfABC->borders.push_back((*it));
                break;
            }
        }
        //saveOBJ("afterSplit.obj");
        //翻转parentFaceOfABC中所有flippable的NLD
        flipAllNLDEdgeInFace(parentFaceOfABC);

        addNewNonFlippableNLDEdge(parentFaceOfABC);
    }


}

bool Mesh::isNLD(Edge* edge) {
    edge->flippable = false;
    if (edge->splitted == true || edge->deleted == true) {
        return false;
    }
    if (edge->faceId.size() == 1) { //如果是边界边，则看对角是否大于90度
        Face* face = faces[*(edge->faceId.begin())];
        Vertex* top = nullptr;
        Vertex* a = edge->vertexe1;
        Vertex* b = edge->vertexe2;
        float cos = 0.0;
        for (int i = 0; i < 3; i++) {
            if (face->vertexs[i]->vertexId != edge->vertexe1->vertexId && face->vertexs[i]->vertexId != edge->vertexe2->vertexId) {
                top = face->vertexs[i];
                //               cos = face->angles[i];
                break;
            }
        }
        float length01 = glm::distance(a->position, top->position);
        float length02 = glm::distance(b->position, top->position);

        cos = glm::dot((a->position - top->position), (b->position - top->position)) / length01 / length02;
        if (cos < -0.000001) {
            return true;
        }
        else {
            return false;
        }
    }
    else if (edge->faceId.size() == 2) {
        Face* face1 = faces[*(edge->faceId.begin())];
        Face* face2 = faces[*(++edge->faceId.begin())];
        Face* parentFaceOf1 = getParentFace(edge, face1);
        Face* parentFaceOf2 = getParentFace(edge, face2);
        

        Vertex* a = edge->vertexe1;
        Vertex* b = edge->vertexe2;
        Vertex* top1 = nullptr;
        Vertex* top2 = nullptr;
        double cos1 = 0.0;
        double cos2 = 0.0;
        for (int i = 0; i < 3; i++) {
            if (face1->vertexs[i]->vertexId != edge->vertexe1->vertexId && face1->vertexs[i]->vertexId != edge->vertexe2->vertexId) {
                top1 = face1->vertexs[i];
                break;
            }
        }
        for (int i = 0; i < 3; i++) {
            if (face2->vertexs[i]->vertexId != edge->vertexe1->vertexId && face2->vertexs[i]->vertexId != edge->vertexe2->vertexId) {
                top2 = face2->vertexs[i];
                break;
            }
        }
        double length01 = glm::distance(a->position, top1->position);
        double length02 = glm::distance(b->position, top1->position);
        cos1 = glm::dot((a->position - top1->position), (b->position - top1->position)) / length01 / length02;
        if (cos1 > 1) {
            cos1 = 1;
        }else if (cos1 < -1) {
            cos1 = -1;
        }

        double length31 = glm::distance(a->position, top2->position);
        double length32 = glm::distance(b->position, top2->position);
        cos2 = glm::dot((a->position - top2->position), (b->position - top2->position)) / length31 / length32;
        if (cos2 > 1) {
            cos2 = 1;
        }
        else if (cos2 < -1) {
            cos2 = -1;
        }

        //求角度之和，用sin(1+2) = sin(1)*cos(2) + cos(1)*sin(2)
        double sin1 = sqrt(1 - cos1 * cos1);
        double sin2 = sqrt(1 - cos2 * cos2);
        double angle1 = acos(cos1);
        double angle2 = acos(cos2);
        if (isnan(angle1) || isnan(angle2)) {

            printf("nan when compute acos\n");
        }
        double sin12 = (sin1 * cos2 + cos1 * sin2);
        if ((angle1 + angle2) > PI || (sin12 < -0.000001 && ((angle1 + angle2)> PI/2))) { //如果角度和大于180度，判断是否可翻转
            glm::vec3 normal1 = glm::normalize(face1->normal);
            glm::vec3 normal2 = glm::normalize(face2->normal);
            glm::vec3 dif = normal1 - normal2;
            if ((glm::dot(normal1,normal2) == 1) || glm::distance(dif, glm::vec3(0, 0, 0)) < 0.000002) {//如果两个法向量平行（此处用向量差模长小于0.000002近似平行）
                if (top1->vertexId < top2->vertexId) {      //该边两个对点如果是在同一直线上，则不是可翻转的
                    if (findEdgeByPoints(top1, top2) != nullptr) {
                        return true;
                    }
                }
                else {
                    if (findEdgeByPoints(top2, top1) != nullptr) {
                        return true;
                    }
                }
                
                edge->flippable = true;
                //为了防止处理两个面不属于同一父面的情况（翻转会导致父子关系出现问题）
                if (parentFaceOf1->faceId != parentFaceOf2->faceId) {
                    edge->flippable = false;
                }
                
                if (sin12 < 0) {
                    return true;
                }
                else {  //如果翻转后生成的边也是NLD，则原边不是可翻转的边，但是也是一条NLD边
                    double cos3 = glm::dot((top1->position - a->position), (top2->position - a->position)) / length01 / length31;
                    if (cos3 > 1) {
                        cos3 = 1;
                    }
                    else if (cos3 < -1) {
                        cos3 = -1;
                    }
                    double cos4 = glm::dot((top1->position - b->position), (top2->position - b->position)) / length02 / length32;
                    if (cos4 > 1) {
                        cos4 = 1;
                    }
                    else if (cos4 < -1) {
                        cos4 = -1;
                    }
                    double sin3 = sqrt(1 - cos3 * cos3);
                    double sin4 = sqrt(1 - cos4 * cos4);
                    double angle3 = acos(cos3);
                    double angle4 = acos(cos4);
                    if (isnan(angle3) || isnan(angle4)) {
                        printf("nan when compute acos\n");
                    }
                    if ((sin3 * cos4 + cos3 * sin4) <= 0 || (angle3 + angle4) >= PI) {
                        edge->flippable = false;
                        return true;
                    }
                    return true;
                }

            }
            return true;




        }
        return false;

    }
    else {
        if (edge->faceId.size() == 4) {
            printf("debug");
        }
        printf("%d face of edge %d\n", edge->faceId.size(), edge->edgeId);
        return false;
    }
}

void Mesh::findAllNLDEdges() {
    for (auto it = edges.begin(); it != edges.end(); it++) {
        if ((*it)->inStack == false && isNLD(*it)) {
            (*it)->inStack = true;
            NLDEdges.push(*it);
        }
    }
}

void Mesh::flipAllNLDEdgeInFace(Face* face) {
begin_process:
    std::list<int> visitedEdge;
    for (auto childIt = face->children.begin(); childIt != face->children.end(); childIt++) {
        for (auto edgeIt = (*childIt)->edges.begin(); edgeIt != (*childIt)->edges.end(); edgeIt++) {
            bool visited = false;
            auto it = visitedEdge.begin();
            while (it != visitedEdge.end()) {
                if ((*it) < (*edgeIt)->edgeId) {
                    it++;
                }
                else if ((*it) == (*edgeIt)->edgeId) {   //该边已经访问过
                    visited = true;
                    break;
                }
                else {
                    break;
                }
            }
            if (!visited && ((*edgeIt)->edgeId >= 0)) {
                visitedEdge.insert(it, (*edgeIt)->edgeId);
                if (isNLD(*edgeIt)) {
                    if ((*edgeIt)->flippable == true) {
                        flipEdge((*edgeIt));
                        visitedEdge.clear();
                        goto begin_process;
                    }
                }
            }
        }
    }
}



void Mesh::flipEdge(Edge* edgeAB) {       //翻转边，删除原三角面ABC、ABD，从parentFace中创建新的三角面ACD、BCD
    /*
    printf("flip %d", edgeAB->edgeId);*/
    //寻找顶点和边
    //saveOBJ("beforeFilp.obj");
    Vertex* vertexA = edgeAB->vertexe1;
    Vertex* vertexB = edgeAB->vertexe2;
    Face* faceABC = faces[*(edgeAB->faceId.begin())];
    Face* faceABD = faces[*(++edgeAB->faceId.begin())];

    Vertex* vertexC = getThirdVertexInFace(faceABC, vertexA->vertexId, vertexB->vertexId);
    Vertex* vertexD = getThirdVertexInFace(faceABD, vertexA->vertexId, vertexB->vertexId);

    Edge* edgeAC = nullptr, * edgeBC = nullptr;
    for (auto it = faceABC->edges.begin(); it != faceABC->edges.end(); it++) {
        if ((*it)->edgeId != edgeAB->edgeId) {
            if ((*it)->vertexe1->vertexId == vertexA->vertexId || (*it)->vertexe2->vertexId == vertexA->vertexId) {
                edgeAC = (*it);
            }
            else {
                edgeBC = (*it);
            }
        }
    }
    Edge* edgeAD = nullptr, * edgeBD = nullptr;
    for (auto it = faceABD->edges.begin(); it != faceABD->edges.end(); it++) {
        if ((*it)->edgeId != edgeAB->edgeId) {
            if ((*it)->vertexe1->vertexId == vertexA->vertexId || (*it)->vertexe2->vertexId == vertexA->vertexId) {
                edgeAD = (*it);
            }
            else {
                edgeBD = (*it);
            }
        }
    }
    //判断原三角面顶点A、C的顺序，创建新的面ACD、BCD
    /*bool antiClockWish = judgeAntiClockWish(faceABC, vertexA, vertexC);
    Face* faceACD = nullptr;
    Face* faceBCD = nullptr;
    if (antiClockWish) {
        faceACD = generateNewFaceFromOldFace(faceABC, vertexA, vertexC, vertexD);
        faceBCD = generateNewFaceFromOldFace(faceABC, vertexC, vertexB, vertexD);
    }
    else {
        faceACD = generateNewFaceFromOldFace(faceABC, vertexC, vertexA, vertexD);
        faceBCD = generateNewFaceFromOldFace(faceABC, vertexB, vertexC, vertexD);
    }
    if (faceACD->faceId == 26) {
        printf("debug");
    }*/

    //从parentFace中删除原来的面，添加新的面
    Edge* parentEdge = edgeAB->parent;

    Face* parentFaceOfABC = getParentFace(parentEdge, faceABC);
    Face* parentFaceOfABD = getParentFace(parentEdge, faceABD);

    if (parentFaceOfABC->faceId == parentFaceOfABD->faceId) {   //同一个面片分割出来的两个子面片，则设置parentFace,parentFace也删除原面片，添加新的面作为子面片
        bool antiClockWish = judgeAntiClockWish(faceABC, vertexA, vertexC);
        Face* faceACD = nullptr;
        Face* faceBCD = nullptr;
        if (antiClockWish) {
            faceACD = generateNewFaceFromOldFace(faceABC, vertexA, vertexC, vertexD);
            faceBCD = generateNewFaceFromOldFace(faceABC, vertexC, vertexB, vertexD);
        }
        else {
            faceACD = generateNewFaceFromOldFace(faceABC, vertexC, vertexA, vertexD);
            faceBCD = generateNewFaceFromOldFace(faceABC, vertexB, vertexC, vertexD);
        }
        
        parentFaceOfABC->deleteChild(faceABC);
        parentFaceOfABC->deleteChild(faceABD);
        parentFaceOfABC->children.push_back(faceACD);
        parentFaceOfABC->children.push_back(faceBCD);
        setMeshFaceOfNewEdge(parentFaceOfABC, faceACD);
        setMeshFaceOfNewEdge(parentFaceOfABC, faceBCD);

        //原来的边删去faceId
        auto iter = edgeAD->faceId.find(faceABD->faceId);
        edgeAD->faceId.erase(iter);
        iter = edgeBD->faceId.find(faceABD->faceId);
        edgeBD->faceId.erase(iter);
        iter = edgeAC->faceId.find(faceABC->faceId);
        edgeAC->faceId.erase(iter);
        iter = edgeBC->faceId.find(faceABC->faceId);
        edgeBC->faceId.erase(iter);


        /*
        //检测
        for (auto it = edgeAB->meshFaceId.begin(); it != edgeAB->meshFaceId.end(); it++) {
            addNewNonFlippableNLDEdge(faces[*it]);
        }*/

        //删除边
        edgeAB->meshFaceId.clear();
        edgeAB->faceId.clear();
        std::pair<int, int> edgePair(edgeAB->vertexe1->vertexId, edgeAB->vertexe2->vertexId);
        m_hash_edge.erase(edgePair);
        for (auto it = edges.begin(); it != edges.end(); it++) {
            if ((*it)->edgeId == edgeAB->edgeId) {
                (*it)->deleted = true;
                delete(*it);
                edges.erase(it);
                break;
            }
        }

        faceABC->deleted = true;
        faceABC->vertexs.clear();
        faceABD->deleted = true;
        faceABD->vertexs.clear();

        std::vector<Face*> newFaces;
        newFaces.push_back(faceACD);
        newFaces.push_back(faceBCD);

        if (faceACD->faceId == 26) {
            //saveOBJ("progress.obj");
        }
        //saveOBJ("afterFilp.obj");
        //return newFaces;
    }
    else {                                                      //这两个面不是同一个面片分割出来的，只是法向量相同而已。则把新的面作为meshFace，也是自己的parentFace，原来的边的meshFace删去原来的面
        /*parentFaceOfABC->deleteChild(faceABC);
        parentFaceOfABD->deleteChild(faceABD);
        setMeshFaceOfNewEdge(faceACD, faceACD);
        setMeshFaceOfNewEdge(faceBCD, faceBCD);
        if (parentFaceOfABC->faceId == faceABC->faceId) {
            auto faceIt1 = edgeAC->meshFaceId.find(faceABC->faceId);
            edgeAC->meshFaceId.erase(faceIt1);
            faceIt1 = edgeBC->meshFaceId.find(faceABC->faceId);
            edgeBC->meshFaceId.erase(faceIt1);
        }
        if (parentFaceOfABD->faceId == faceABD->faceId) {
            auto faceIt2 = edgeAD->meshFaceId.find(faceABD->faceId);
            edgeAD->meshFaceId.erase(faceIt2);
            faceIt2 = edgeBD->meshFaceId.find(faceABD->faceId);
            edgeBD->meshFaceId.erase(faceIt2);
        }
        faceACD->isMesh = true;
        faceBCD->isMesh = true;*/ 
        edgeAB->flippable = false;

/* 
        parentFaceOfABC->deleteChild(faceABC);
        parentFaceOfABD->deleteChild(faceABD);
        setMeshFaceOfNewEdge(faceACD, faceACD);
        setMeshFaceOfNewEdge(faceBCD, faceBCD);
        faceACD->isMesh = true;
        faceBCD->isMesh = true;*/
    }


    ////原来的边删去faceId
    //auto iter = edgeAD->faceId.find(faceABD->faceId);
    //edgeAD->faceId.erase(iter);
    //iter = edgeBD->faceId.find(faceABD->faceId);
    //edgeBD->faceId.erase(iter);
    //iter = edgeAC->faceId.find(faceABC->faceId);
    //edgeAC->faceId.erase(iter);
    //iter = edgeBC->faceId.find(faceABC->faceId);
    //edgeBC->faceId.erase(iter);


    ///*
    ////检测
    //for (auto it = edgeAB->meshFaceId.begin(); it != edgeAB->meshFaceId.end(); it++) {
    //    addNewNonFlippableNLDEdge(faces[*it]);
    //}*/
    //
    ////删除边
    //edgeAB->meshFaceId.clear();
    //edgeAB->faceId.clear();
    //std::pair<int, int> edgePair(edgeAB->vertexe1->vertexId, edgeAB->vertexe2->vertexId);
    //m_hash_edge.erase(edgePair);
    //for (auto it = edges.begin(); it != edges.end(); it++) {
    //    if ((*it)->edgeId == edgeAB->edgeId) {
    //        (*it)->deleted = true;
    //        delete(*it);
    //        edges.erase(it);
    //        break;
    //    }
    //}

    //faceABC->deleted = true;
    //faceABC->vertexs.clear();
    //faceABD->deleted = true;
    //faceABD->vertexs.clear();

    //std::vector<Face*> newFaces;
    //newFaces.push_back(faceACD);
    //newFaces.push_back(faceBCD);

    //if (faceACD->faceId == 26) {
    //    saveOBJ("progress.obj");
    //}
    //saveOBJ("afterFilp.obj");
    //return newFaces;
}

Face* Mesh::getParentFace(Edge* edge, Face* childFace) {    //根据边edge来找meshFace
    if (edge->meshFaceId.size() == 0) {
        printf("emptyMeshFace\n");
        return nullptr;
    }
    if (edge->meshFaceId.size() == 4) {
        printf("debug");
    }
    for (auto faceIdIt = edge->meshFaceId.begin(); faceIdIt != edge->meshFaceId.end(); faceIdIt++) {
        Face* face1 = faces[*(faceIdIt)];
        for (auto it = face1->children.begin(); it != face1->children.end(); it++) {
            if ((*it)->faceId == childFace->faceId) {
                return face1;
            }
        }
    }
    printf("no face contain this childFace\n");//如果
//    if ((*childFace->children.begin())->faceId == childFace->faceId) {
//        return childFace;
//    }

    return faces[*(edge->meshFaceId.begin())];
}

Vertex* Mesh::generateNewVertex(glm::vec3& position) {
    int id = findVertexByPoint(position);
    Vertex* vertex = nullptr;
    if (id == -1) {			//没找到，该point是新的
        vertex = new Vertex();
        vertex->init(position);
        vertex->setId(vertexes.size());
        m_hash_point[position] = vertexes.size();
        vertexes.push_back(vertex);
    }
    else {
        vertex = vertexes[id];
    }
    return vertex;
}



Face* Mesh::generateNewFace(Vertex* v1, Vertex* v2, Vertex* v3) {//v1,v2,v3是逆时针
    Face* face = new Face();
    std::vector<Vertex*> vs;
    vs.push_back(v1);
    vs.push_back(v2);
    vs.push_back(v3);

    face->setId(faces.size());
    if (faces.size() == 80630 || faces.size() == 31048) {
        printf("debug");
    }
    //if (face->faceId == 9057) {
    //    printf("debug");
    //}
    //if (face->faceId == 20165) {//faceId数等于4的边多出来的面id
    //    printf("debug");
    //}
    face->setVertex(vs);
    face->isMesh = true;
    faces.push_back(face);
    face->children.push_back(face);

    generateEdgeOfFace(face, true);
    face->calNormalOfFace();
    return face;
}


Face* Mesh::generateNewFaceFromOldFace(Face* parentFace, Vertex* v1, Vertex* v2, Vertex* v3) {
    Face* face = new Face();

    face->setNormal(parentFace->normal);
    std::vector<Vertex*> vs;
    vs.push_back(v1);
    vs.push_back(v2);
    vs.push_back(v3);

    face->setId(faces.size());
    if (faces.size() == 20605 || faces.size() == 460) {
        printf("debug");
    }
    //if (face->faceId == 9057) {
    //    printf("debug");
    //}
    //if (face->faceId == 20165) {
    //    printf("debug");
    //}
    face->setVertex(vs);
    face->isMesh = false;
    faces.push_back(face);
    face->children.push_back(face);

    generateEdgeOfFace(face, false);

    return face;
}

void Mesh::addNewNonFlippableNLDEdge(Face* face) {
    for (auto it = face->borders.begin(); it != face->borders.end(); it++) {
        if ((*it)->inStack == false && isNLD(*it) && (*it)->flippable == false) {
            (*it)->inStack = true;
            NLDEdges.push(*it);

        }
    }
}

//找到edge所属的除face的另一个面的对角E的坐标
glm::vec3 Mesh::getAnotherVertexPositionByEdge(Face* face, Edge* edge) {
    Vertex* vertexA = edge->vertexe1;
    Vertex* vertexC = edge->vertexe2;

    //求另一个三角面
    Face* faceACE = nullptr;
    if (*(edge->faceId.begin()) != face->faceId) {
        faceACE = faces[*(edge->faceId.begin())];
    }
    else {
        faceACE = faces[*(++edge->faceId.begin())];
    }

    //求顶点
    Vertex* vertexE = nullptr;
    vertexE = getThirdVertexInFace(faceACE, vertexA->vertexId, vertexC->vertexId);
    return glm::vec3(vertexE->position);
}

//找到edge所属的除face的另一个面的对角的度数
float Mesh::getAnotherVertexDegreeByEdge(Face* face, Edge* edge) {
    Vertex* vertexA = edge->vertexe1;
    Vertex* vertexC = edge->vertexe2;

    //求另一个三角面
    Face* faceACE = nullptr;
    if (*(edge->faceId.begin()) != face->faceId) {
        faceACE = faces[*(edge->faceId.begin())];
    }
    else {
        faceACE = faces[*(++edge->faceId.begin())];
    }

    //求顶点
    Vertex* vertexE = nullptr;
    vertexE = getThirdVertexInFace(faceACE, vertexA->vertexId, vertexC->vertexId);
    if (vertexE)
        for (int i = 0; i < 3; i++) {
            if (faceACE->vertexs[i]->vertexId == vertexE->vertexId) {
                return faceACE->angles[i];
            }
        }
    return -5;
}


void Mesh::init() {
    //找到每个点的临近点
    for (auto it = edges.begin(); it != edges.end(); it++) {
        if ((*it)->faceId.size() > 2) {
            printf("edge face > 2\n");
        }
        if ((*it)->deleted == false && (*it)->splitted == false && (*it)->vertexe1->vertexId != (*it)->vertexe2->vertexId) {
            (*it)->vertexe1->incidentEdges.push_back(*it);
            (*it)->vertexe2->incidentEdges.push_back(*it);

            //added elgce  添加了每个点的邻接点
            (*it)->vertexe1->incidentVertexes.push_back((*it)->vertexe2);
            (*it)->vertexe2->incidentVertexes.push_back((*it)->vertexe1);
        }
    }

    //计算每个面的方程
    for (auto it = faces.begin(); it != faces.end(); it++) {
        if ((*it)->deleted == false) {
            (*it)->calFormula();
        }
    }

    std::unordered_map<int, bool> visitedFace;//STL中的哈希容器
    for (auto it = vertexes.begin(); it != vertexes.end(); it++) {
        //if ((*it)->vertexId == 899) {
        //    printf("debug");
        //}
        //计算每个点的Q矩阵
        for (auto edgeIt = (*it)->incidentEdges.begin(); edgeIt != (*it)->incidentEdges.end(); edgeIt++) {//对于该点，遍历它所有邻边
            for (auto faceIt = (*edgeIt)->faceId.begin(); faceIt != (*edgeIt)->faceId.end(); faceIt++) {        //看看该边在哪些面上，计算Q值
                if (visitedFace.find(*faceIt) == visitedFace.end()) {   //没有访问过
                    visitedFace[*faceIt] = true;
                    //Q矩阵加上该平面的Q值
                    glm::vec4 formula = faces[*faceIt]->formula;
                    for (int i = 0; i < 4; i++) {
                        for (int j = 0; j < 4; j++) {
                            (*it)->Q[i][j] += formula[i] * formula[j];
                        }
                    }
                    (*it)->lambda++;//关联的面数

                }
            }
            if ((*edgeIt)->faceId.size() == 1) {                                                            //如果这条边是边界边，那么按照QEM方法，为这条边添加一个垂直面
                auto faceIt = (*edgeIt)->faceId.begin();                                                    // TODO 可以在遍历边的时候就进行计算，这样每一条边（两个顶点）只用计算一次。而在这计算，则需要计算两次
                auto formalface = faces[*faceIt];
                Face* newface = new Face();
                glm::vec3 fornormal = (formalface)->normal;
                glm::vec3 newnormal;
                if (fornormal.x != 0) {
                    newnormal.y = newnormal.z = 1;
                    newnormal.x = -(fornormal.y + fornormal.z) / fornormal.x;
                }
                else if (fornormal.y != 0) {
                    newnormal.x = newnormal.z = 1;
                    newnormal.y = -(fornormal.x + fornormal.z) / fornormal.y;
                }
                else {
                    newnormal.x = newnormal.y = 1;
                    newnormal.z = -(fornormal.x + fornormal.y) / fornormal.z;
                }
                newnormal = glm::normalize(newnormal);
                newface->setNormal(newnormal);
                newface->vertexs.push_back((*it));
                newface->calFormula();
                glm::vec4 formula = newface->formula;
                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 4; j++) {
                        (*it)->Q[i][j] += 1000 * formula[i] * formula[j];                                   //惩罚因子为1000
                    }
                }
                //(*it)->lambda++;                                                                          //该面不是真实的面，不属于顶点的关联面
                delete newface;
            }
        }
        visitedFace.clear();
    }
}




bool Mesh::isTypeI(Vertex* vertex, std::vector<float>& subtendedAngles) {
    if (vertex->vertexId == 439) {
        printf("dubug");
    }
    if (vertex->deleted) {
        printf("deleted vertex\n");
        return false;
    }


    vertex->boundary = false;
    int n = vertex->incidentEdges.size();
    vertex->eph = 0x7f7fffff;
    vertex->typeI = false;
    vertex->typeII = false;
    vertex->e = nullptr;
    vertex->eIndex = -1;
    vertex->flippedEdge.clear();
    bool notEqual = resortIncidentEdge(vertex);     //true表示邻边数与面数不相等，即这个点是边界点。false表示为内部点
    if (notEqual) {
        vertex->boundary = true;
    }
    
    std::vector<Edge*> oppositeEdges;
    //if (vertex->vertexId == 5251) {
    //    printf("debug");
    //}
    //oppositeEdges.assign(n, nullptr);
    if (!notEqual) {                                //内部点
        //找到对应的对角度数 subtendedAngles和incidentFaces对应。incidentFaces[i]是incidentEdges[i]与incidentEdges[i+1]所组成的面（对应incidentVertexes[i]和incidentVertexes[i+1]）
        for (auto faceIt = vertex->incidentFaces.begin(); faceIt != vertex->incidentFaces.end(); faceIt++) {
            if ((*faceIt)->deleted == false) {
                //先找到对边
                Edge* oppositeEdge = nullptr;
                for (auto edgeIt = (*faceIt)->edges.begin(); edgeIt != (*faceIt)->edges.end(); edgeIt++) {
                    if ((*edgeIt)->vertexe1->vertexId != vertex->vertexId && (*edgeIt)->vertexe2->vertexId != vertex->vertexId) {
                        oppositeEdge = (*edgeIt);
                        oppositeEdges.push_back(oppositeEdge);
                        break;
                    }
                }
                // printf_s("\n\n");
                 //再找到对角度数（cos）
                if (oppositeEdge->faceId.size() < 2) {
                    subtendedAngles.push_back(-5);
                }
                else {
                    subtendedAngles.push_back(getAnotherVertexDegreeByEdge(*faceIt, oppositeEdge));
                }
            }
        }
        for (int i = 0; i < n; i++) {       //TODO 删除
            if (!VertexBelongToFace(vertex->incidentVertexes[i], vertex->incidentFaces[i]) && !VertexBelongToFace(vertex->incidentVertexes[(i+1)%n], vertex->incidentFaces[i])) {
                printf("vertex does not belong to face");
                break;
            }
        }
        //对每个相邻点进行检测（假设把（it，neighborV）压缩到neighborV），检测新的边是否满足nld条件
        //如果当前点是已经是typeI了，那么选择更小的eph作为要压缩的情况
        for (int i = 0; i < n; i++) { //i是要压缩到的点
            Vertex* neighborV = vertex->incidentVertexes[i];
            bool sat = true;
            //检测相邻的边线
            int firstIndex = (i + 1) % n;
            Vertex* v0 = vertex->incidentVertexes[firstIndex];
            int secondIndex = (i + 2) % n;
            Vertex* v1 = vertex->incidentVertexes[secondIndex];
            float lengthV1 = glm::distance(neighborV->position, v1->position);
            float length01 = glm::distance(v0->position, v1->position);

            float cos1 = glm::dot((neighborV->position - v1->position), (v0->position - v1->position)) / lengthV1 / length01;
            if (subtendedAngles[i] == -5) {    //没有外对角，则判断内对角
                if (cos1 < -0.000001) {    //不满足条件
                    continue;
                }
            }
            else {
                float sinj = sqrt(1 - subtendedAngles[i] * subtendedAngles[i]);

                float sin1 = sqrt(1 - cos1 * cos1);
                float sinSum = sinj * cos1 + subtendedAngles[i] * sin1;
                if (sinSum < -0.00001) {
                    continue;
                }
            }

            //另一侧的相邻边线
            firstIndex = (i + n - 1) % n;
            v0 = vertex->incidentEdges[firstIndex]->vertexe1->vertexId == vertex->vertexId ? vertex->incidentEdges[firstIndex]->vertexe2 : vertex->incidentEdges[firstIndex]->vertexe1;
            secondIndex = (i + n - 2) % n;
            v1 = vertex->incidentEdges[secondIndex]->vertexe1->vertexId == vertex->vertexId ? vertex->incidentEdges[secondIndex]->vertexe2 : vertex->incidentEdges[secondIndex]->vertexe1;
            lengthV1 = glm::distance(neighborV->position, v1->position);
            length01 = glm::distance(v0->position, v1->position);

            cos1 = glm::dot((neighborV->position - v1->position), (v0->position - v1->position)) / lengthV1 / length01;
            if (subtendedAngles[firstIndex] == -5) {    //没有外对角，则判断内对角
                if (cos1 < -0.000001) {    //不满足条件
                    continue;
                }
            }
            else {
                float sinj = sqrt(1 - subtendedAngles[firstIndex] * subtendedAngles[firstIndex]);
                float sin1 = sqrt(1 - cos1 * cos1);
                float sinSum = sinj * cos1 + subtendedAngles[firstIndex] * sin1;
                if (sinSum < -0.00001) {
                    continue;
                }
            }
            //检测非相邻的边线
            int remainingEdge = n - 2;
            int j = (i + 1) % n;
            //TODO：优化算法，下一轮可以使用上一轮的点以及边长
            while (remainingEdge > 0) {
                v0 = vertex->incidentEdges[j]->vertexe1->vertexId == vertex->vertexId ? vertex->incidentEdges[j]->vertexe2 : vertex->incidentEdges[j]->vertexe1;
                int nextIndex = (j + 1) % n;
                v1 = vertex->incidentEdges[nextIndex]->vertexe1->vertexId == vertex->vertexId ? vertex->incidentEdges[nextIndex]->vertexe2 : vertex->incidentEdges[nextIndex]->vertexe1;
                float length1 = glm::distance(neighborV->position, v0->position);
                float length2 = glm::distance(neighborV->position, v1->position);

                float cos = glm::dot((v1->position - neighborV->position), (v0->position - neighborV->position)) / length1 / length2;
                if (subtendedAngles[j] == -5) {    //没有外对角，则判断内对角
                    if (cos < -0.000001) {    //不满足条件
                        sat = false;
                        break;
                    }

                }
                else {
                    float sinj = sqrt(1 - subtendedAngles[j] * subtendedAngles[j]);
                    float sin = sqrt(1 - cos * cos);
                    float sinSum = sinj * cos + subtendedAngles[j] * sin;
                    if (sinSum < -0.00001) {
                        sat = false;
                        break;
                    }
                }
                j++;
                j %= n;
                remainingEdge--;
            }
            if (!sat) {
                continue;
            }

            //检测新生成的边
            int newEdge = n - 3;
            j = (i + 2) % n;
            Vertex* v2;
            while (newEdge > 0) {
                int preIndex = (j + n - 1) % n;
                v0 = vertex->incidentEdges[preIndex]->vertexe1->vertexId == vertex->vertexId ? vertex->incidentEdges[preIndex]->vertexe2 : vertex->incidentEdges[preIndex]->vertexe1;
                v1 = vertex->incidentEdges[j]->vertexe1->vertexId == vertex->vertexId ? vertex->incidentEdges[j]->vertexe2 : vertex->incidentEdges[j]->vertexe1;
                int nextIndex = (j + 1) % n;
                v2 = vertex->incidentEdges[nextIndex]->vertexe1->vertexId == vertex->vertexId ? vertex->incidentEdges[nextIndex]->vertexe2 : vertex->incidentEdges[nextIndex]->vertexe1;

                float lengthV0 = glm::distance(neighborV->position, v0->position);
                //            float lengthV1 = glm::distance(neighborV->position, v1->position);
                float lengthV2 = glm::distance(neighborV->position, v2->position);
                float length01 = glm::distance(v0->position, v1->position);
                float length12 = glm::distance(v1->position, v2->position);

                float cos0 = glm::dot((v1->position - v0->position), (neighborV->position - v0->position)) / lengthV0 / length01;
                float cos2 = glm::dot((v1->position - v2->position), (neighborV->position - v2->position)) / lengthV2 / length12;
                float sin0 = sqrt(1 - cos0 * cos0);
                float sin2 = sqrt(1 - cos2 * cos2);
                float sinSum = sin0 * cos2 + cos0 * sin2;
                if (sinSum < -0.00001) {
                    sat = false;
                    break;
                }
                j++;
                j %= n;
                newEdge--;
            }
            if (!sat) {
                continue;
            }
            else {
                glm::mat4 QSum = vertex->Q + neighborV->Q;
                glm::vec4 v4 = glm::vec4(neighborV->position, 1);
                glm::vec4 temp = v4 * QSum;
                float res = glm::dot(v4, temp);
                if (vertex->eph > res) {
                    vertex->eph = res;
                    vertex->e = vertex->incidentEdges[i];
                    vertex->typeI = true;
                    vertex->eIndex = i;

                }
            }
        }
        if (vertex->typeI != false) {
            return true;
        }
        else {
            return false;
        }
    }else {
    /*
    //假设把（it，neighborV）压缩到neighborV
        for (int i = 0; i < n; i++) {
            Vertex* neighborV = vertex->incidentVertexes[i];
            bool sat = true;
            //检测非相邻的边线（如果有，就能形成三角形；如果没有，则
            int firstIndex = (i + 1) % n;
            Vertex* v0 = vertex->incidentVertexes[firstIndex];
            int secondIndex = (i + 2) % n;
            Vertex* v1 = vertex->incidentVertexes[secondIndex];
            float lengthV1 = glm::distance(neighborV->position, v1->position);
            float length01 = glm::distance(v0->position, v1->position);

            float cos0 = glm::dot((neighborV->position - v1->position), (v0->position - v1->position)) / lengthV1 / length01;
            if (subtendedAngles[firstIndex] == -5) {    //没有外对角，则判断内对角
                if (cos0 < -0.000001) {    //不满足条件
                    continue;
                }
            }
            else {
                float sinj = sqrt(1 - subtendedAngles[firstIndex] * subtendedAngles[firstIndex]);
                float sin = sqrt(1 - cos0 * cos0);
                float sinSum = sinj * cos0 + subtendedAngles[firstIndex] * sin;
                if (sinSum < 0.00001) {
                    continue;
                }
            }
        }*/
        return false;
    }
        
            
    
    //if (!notEqual) {
    //    
    //}
    //else {//找到最小的e
    //    for (int i = 0; i < n && vertex->incidentVertexes[i]; i++) {
    //        glm::mat4 QSum = vertex->Q + vertex->incidentVertexes[i]->Q;
    //        glm::vec4 v4 = glm::vec4(vertex->incidentVertexes[i]->position, 1);
    //        glm::vec4 temp = v4 * QSum;
    //        float res = glm::dot(v4, temp);
    //        if (vertex->eph > res) {
    //            vertex->eph = res;
    //            vertex->e = vertex->incidentEdges[i];
    //            vertex->typeI = true;
    //            vertex->eIndex = i;
    //        }
    //    }
    //}

    if (vertex->typeI == true) {
        return true;
    }
    else {
        return false;
    }
    return false;
}


void Mesh::findTypeIAndTypeII() {
    std::vector<float> subtendedAngles;
    for (auto it = vertexes.begin(); it != vertexes.end(); it++) {
        (*it)->incidentFaces.clear();
        subtendedAngles.clear();
        if (!isTypeI(*it, subtendedAngles)) {
            isTypeII(*it, subtendedAngles);
        }
    }
}

std::vector<Face*> Mesh::flipEdgeWhenTestingTypeII(Edge* edgeAB, Face* faceABC, Face* faceABD) {
    //寻找顶点和边
    if (edgeAB->faceId.size() != 2) {
        printf("face count != 2\n");
    }
    Vertex* vertexA = edgeAB->vertexe1;
    Vertex* vertexB = edgeAB->vertexe2;
    //Face* faceABC = faces[*(edgeAB->faceId.begin())];
    //Face* faceABD = faces[*(++edgeAB->faceId.begin())];

    Vertex* vertexC = getThirdVertexInFace(faceABC, vertexA->vertexId, vertexB->vertexId);
    Vertex* vertexD = getThirdVertexInFace(faceABD, vertexA->vertexId, vertexB->vertexId);

    Edge* edgeAC = nullptr, * edgeBC = nullptr;
    for (auto it = faceABC->edges.begin(); it != faceABC->edges.end(); it++) {
        if ((*it)->edgeId != edgeAB->edgeId) {
            if ((*it)->vertexe1->vertexId == vertexA->vertexId || (*it)->vertexe2->vertexId == vertexA->vertexId) {
                edgeAC = (*it);
            }
            else {
                edgeBC = (*it);
            }
        }
    }
    Edge* edgeAD = nullptr, * edgeBD = nullptr;
    for (auto it = faceABD->edges.begin(); it != faceABD->edges.end(); it++) {
        if ((*it)->edgeId != edgeAB->edgeId) {
            if ((*it)->vertexe1->vertexId == vertexA->vertexId || (*it)->vertexe2->vertexId == vertexA->vertexId) {
                edgeAD = (*it);
            }
            else {
                edgeBD = (*it);
            }
        }
    }
    //判断原三角面顶点A、C的顺序，创建新的面ACD、BCD
    bool antiClockWish = judgeAntiClockWish(faceABC, vertexA, vertexC);
    Face* faceACD = nullptr;
    Face* faceBCD = nullptr;
    if (antiClockWish) {
        faceACD = generateNewFace(vertexA, vertexC, vertexD);
        faceBCD = generateNewFace(vertexC, vertexB, vertexD);
    }
    else {
        faceACD = generateNewFace(vertexC, vertexA, vertexD);
        faceBCD = generateNewFace(vertexB, vertexC, vertexD);
    }

    //从parentFace中删除原来的面
    Edge* parentEdge = edgeAB->parent;
    Face* parentFaceOfABC = getParentFace(parentEdge, faceABC);
    Face* parentFaceOfABD = getParentFace(parentEdge, faceABD);

    parentFaceOfABC->deleteChild(faceABC);
    parentFaceOfABD->deleteChild(faceABD);

    deleteFace(faceABC, true);
    deleteFace(faceABD, true);

    std::vector<Face*> newFaces;
    newFaces.push_back(faceACD);
    newFaces.push_back(faceBCD);
    return newFaces;
}

bool Mesh::flipAllEdgesOfTypeII(std::vector<Face*>& faceSet, Vertex* vertex, std::list<int>& borderEdge) {
    if (vertex->vertexId == 3340) {
        printf("debug");    //翻转的边出现边界边情况
    }
    //if (vertex->vertexId == 173) {
    //    printf("debug");    //一条边有四个邻面。在简化(175,2825) to 175后检测相邻边出现该问题，因为有一个面片是边界面片（faces[20605]）
    //}
    auto borderEnd = borderEdge.end();
    borderEnd--;
typeII_process:
    for (auto faceIt = faceSet.begin(); faceIt != faceSet.end(); faceIt++) {
        for (auto edgeIt = (*faceIt)->edges.begin(); edgeIt != (*faceIt)->edges.end(); edgeIt++) {
            bool isVisited = false;
            auto it = borderEdge.begin();       //在borderEdge中插入访问过的边，便于计算
            while (it != borderEdge.end()) {
                if ((*it) == (*edgeIt)->edgeId) {   //该边已经访问过
                    isVisited = true;
                    break;
                }
                it++;
            }
            if (!isVisited && ((*edgeIt)->edgeId >= 0)) {         //只考虑空洞内部的边
                borderEdge.push_back((*edgeIt)->edgeId);
                if ((*edgeIt)->faceId.size() > 3) {
                    //borderEdge 还原
                    auto visitedEdge = borderEnd;
                    visitedEdge++;
                    borderEdge.erase(visitedEdge, borderEdge.end());
                    return false;
                }
                if (isNLD(*edgeIt)) {
                    if ((*edgeIt)->faceId.size() != 2) {
                        printf("debug");
                    }
                    Face* face1 = *faceIt;
                    Face* face2 = nullptr;
                    int anotherFaceId = -1;
                    for (auto it = (*edgeIt)->faceId.begin(); it != (*edgeIt)->faceId.end(); it++) {//寻找另一个包含该边的面片
                        if ((*it) != (*faceIt)->faceId) {
                            anotherFaceId = (*it);
                            face2 = faces[anotherFaceId];
                            break;
                        }
                    }
                    std::pair<int, int> flippedEdge((*edgeIt)->vertexe1->vertexId, (*edgeIt)->vertexe2->vertexId);
                    vertex->flippedEdge.push_back(flippedEdge);
                    
                    //删除原来的面
                    faceSet.erase(faceIt);//因为删除了这条边，所以该迭代器指向野地址
                    if (anotherFaceId != -1) {
                        for (faceIt = faceSet.begin(); faceIt != faceSet.end(); faceIt++) {
                            if ((*faceIt)->faceId == anotherFaceId) {
                                faceSet.erase(faceIt);
                                break;
                            }
                        }
                    }
                    std::vector<Face*> newFace = flipEdgeWhenTestingTypeII((*edgeIt), face1, face2);   
                    
                    

                    //添加新的面
                    faceSet.push_back(newFace[0]);
                    faceSet.push_back(newFace[1]);

                    //borderEdge 还原
                    auto visited = borderEnd;
                    visited++;
                    borderEdge.erase(visited, borderEdge.end());
                    if (vertex->flippedEdge.size() > 30) {
                        return false;
                    }
                    //排除一条边有四个邻面的情况
                    for (int j = 0; j < 2; j++) {
                        for (auto newEdgeIt = newFace[j]->edges.begin(); newEdgeIt != newFace[j]->edges.end(); newEdgeIt++) {
                            if ((*newEdgeIt)->faceId.size() > 3) {
                                return false;
                            }
                        }
                    }
                    goto typeII_process;
                }
            }
        }
    }
    auto visited = borderEnd;
    visited++;
    borderEdge.erase(visited, borderEdge.end());
    return true;
}

void Mesh::flipEdgeWhenSimplifying(Vertex* vertexA, Vertex* vertexB, std::list<Face*>& newFaces) {  //找到AB边对应的边和面，翻转直接按照新的面来生成，再更改邻边关系
    //寻找顶点和边
    Face* faceABC = nullptr;
    Face* faceABD = nullptr;
    bool firstFace = true;
    for (auto it = newFaces.begin(); it != newFaces.end();) {
        if (VertexBelongToFace(vertexA, *it) && VertexBelongToFace(vertexB, *it)) {
            if (firstFace) {
                faceABC = *it;
                firstFace = false;
                it = newFaces.erase(it);
            }
            else {
                faceABD = *it;
                newFaces.erase(it);
                break;
            }
        }
        else {
            it++;
        }

    }
    Edge* edgeAB = nullptr;
    for (auto it = faceABC->edges.begin(); it != faceABC->edges.end(); it++) {
        if ((*it)->vertexe1->vertexId == vertexA->vertexId && (*it)->vertexe2->vertexId == vertexB->vertexId) {
            edgeAB = *it;
            break;
        }
    }


    Vertex* vertexC = getThirdVertexInFace(faceABC, vertexA->vertexId, vertexB->vertexId);
    Vertex* vertexD = getThirdVertexInFace(faceABD, vertexA->vertexId, vertexB->vertexId);

    Edge* edgeAC = nullptr, * edgeBC = nullptr;
    for (auto it = faceABC->edges.begin(); it != faceABC->edges.end(); it++) {
        if ((*it)->edgeId != edgeAB->edgeId) {
            if ((*it)->vertexe1->vertexId == vertexA->vertexId || (*it)->vertexe2->vertexId == vertexA->vertexId) {
                edgeAC = (*it);
            }
            else {
                edgeBC = (*it);
            }
        }
    }
    Edge* edgeAD = nullptr, * edgeBD = nullptr;
    for (auto it = faceABD->edges.begin(); it != faceABD->edges.end(); it++) {
        if ((*it)->edgeId != edgeAB->edgeId) {
            if ((*it)->vertexe1->vertexId == vertexA->vertexId || (*it)->vertexe2->vertexId == vertexA->vertexId) {
                edgeAD = (*it);
            }
            else {
                edgeBD = (*it);
            }
        }
    }

    //判断原三角面顶点A、C的顺序，创建新的面ACD、BCD
    bool antiClockWish = judgeAntiClockWish(faceABC, vertexA, vertexC);
    Face* faceACD = nullptr;
    Face* faceBCD = nullptr;
    bool faceACDExist = false;
    bool faceBCDExist = false;
    if (edgeAD != nullptr) {
        for (auto it = edgeAD->faceId.begin(); it != edgeAD->faceId.end(); it++) {
            if (VertexBelongToFace(vertexC, faces[*it])) {
                faceACD = faces[*it];
                faceACDExist = true;
                break;
            }
        }
    }
    if (edgeBD != nullptr) {
        for (auto it = edgeBD->faceId.begin(); it != edgeBD->faceId.end(); it++) {
            if (VertexBelongToFace(vertexC, faces[*it])) {
                faceBCD = faces[*it];
                faceBCDExist = true;
                break;
            }
        }
    }

    if (antiClockWish) {
        if (!faceACDExist) {
            faceACD = generateNewFace(vertexA, vertexC, vertexD);
        }
        if (!faceBCDExist) {
            faceBCD = generateNewFace(vertexC, vertexB, vertexD);
        }
    }
    else {
        if (!faceACDExist) {
            faceACD = generateNewFace(vertexC, vertexA, vertexD);
        }
        if (!faceBCDExist) {
            faceBCD = generateNewFace(vertexB, vertexC, vertexD);
        }
    }

    //从parentFace中删除原来的面
    Edge* parentEdge = edgeAB->parent;
    Face* parentFaceOfABC = getParentFace(parentEdge, faceABC);
    Face* parentFaceOfABD = getParentFace(parentEdge, faceABD);

    parentFaceOfABC->deleteChild(faceABC);
    parentFaceOfABD->deleteChild(faceABD);

    //找到新的边CD
    Edge* edgeCD = nullptr;
    for (auto edgeIt = faceACD->edges.begin(); edgeIt != faceACD->edges.end(); edgeIt++) {
        if ((*edgeIt)->vertexe1->vertexId != vertexA->vertexId && (*edgeIt)->vertexe2->vertexId != vertexA->vertexId) {
            edgeCD = (*edgeIt);
            break;
        }
    }

    //更新邻边关系：（面的可以不用更新）
    //A、B两点的邻边删除AB，邻点删去B、A，C、D两点的邻边添加CD，邻点添加D、C
    //A
    for (auto it = vertexA->incidentEdges.begin(); it != vertexA->incidentEdges.end(); it++) {
        if ((*it)->edgeId == edgeAB->edgeId) {
            vertexA->incidentEdges.erase(it);
            break;
        }
    }
    for (auto it = vertexA->incidentVertexes.begin(); it != vertexA->incidentVertexes.end(); it++) {
        if ((*it)->vertexId == vertexB->vertexId) {
            vertexA->incidentVertexes.erase(it);
            break;
        }
    }
    //B
    for (auto it = vertexB->incidentEdges.begin(); it != vertexB->incidentEdges.end(); it++) {
        if ((*it)->edgeId == edgeAB->edgeId) {
            vertexB->incidentEdges.erase(it);
            break;
        }
    }
    for (auto it = vertexB->incidentVertexes.begin(); it != vertexB->incidentVertexes.end(); it++) {
        if ((*it)->vertexId == vertexA->vertexId) {
            vertexB->incidentVertexes.erase(it);
            break;
        }
    }
    
    bool isNew = true;      //判断是否该边已经被记录（其他面包含这条边）
    for (auto it = vertexC->incidentVertexes.begin(); it != vertexC->incidentVertexes.end(); it++) {
        if ((*it)->vertexId == vertexD->vertexId) {
            isNew = false;
            break;
        }
    }

    if (isNew) {
        //C
        vertexC->incidentEdges.push_back(edgeCD);
        vertexC->incidentVertexes.push_back(vertexD);
        //D
        vertexD->incidentEdges.push_back(edgeCD);
        vertexD->incidentVertexes.push_back(vertexC);
    }
    

    deleteFace(faceABC, false);
    deleteFace(faceABD, false);

    newFaces.push_back(faceACD);
    newFaces.push_back(faceBCD);
}

bool Mesh::resortIncidentEdge(Vertex* vertex) {     //根据incidentEdges来确定incidentVertexes和incidentFaces，并且调整两个数组的顺序
    //if (vertex->vertexId == 3350 && vertex->incidentEdges.size() == 8) {
    //    printf("debug");
    //}


    //subtendedAngles是同下标incidentFaces的对角
    int n = vertex->incidentEdges.size();
    vertex->incidentFaces.clear();
    vertex->incidentVertexes.clear();
    vertex->incidentVertexes.assign(n, nullptr);
    bool notEqual = false;//边的数目与面的数目不一致，非内部点。当不是内部点的时候为true
    if (n < 2) {
        printf("点的邻边少于2\n");
        notEqual = true;
    }
    else if (n == 2) {//n=2 说明是一个边界点
        for (int i = 0; i < n; i++) {
            if (vertex->incidentEdges[i]->vertexe1->vertexId == vertex->vertexId) {
                vertex->incidentVertexes[i] = vertex->incidentEdges[i]->vertexe2;
            }
            else {
                vertex->incidentVertexes[i] = vertex->incidentEdges[i]->vertexe1;
            }
        }
        vertex->incidentFaces.push_back(faces[(*vertex->incidentEdges[0]->faceId.begin())]);
        notEqual = true;

        //TODO 仅验证边界边是否在同一个面上，之后删除
        bool sameFace = false;
        for (auto it = vertex->incidentFaces[0]->edges.begin(); it != vertex->incidentFaces[0]->edges.end(); it++) {
            if ((*it)->edgeId == vertex->incidentEdges[1]->edgeId) {
                sameFace = true;
                break;
            }
        }
        if (!sameFace) {
            printf("边界点的两条边不在同一面上\n");
        }
    }
    else {
        for (int i = 0; i < n; i++) {
            if (vertex->incidentEdges[i]->faceId.size() == 1) {//换到第一个位置       边缘处的边
                Edge* tmp = vertex->incidentEdges[i];
                vertex->incidentEdges[i] = vertex->incidentEdges[0];
                vertex->incidentEdges[0] = tmp;
                notEqual = true;//表明这个点不是内部点
                break;
            }
        }


        //对每条边，找到它所属的面中没有访问过的，加入到incidentFaces中。同时把incidentEdges更新
        for (int i = 0; i < n; i++) {
            int faceId = -1;
            for (auto it = vertex->incidentEdges[i]->faceId.begin(); it != vertex->incidentEdges[i]->faceId.end(); it++) {
                bool visited = false;                                                                                     //当点为边界点时，incidentFaces和incidentEdges没有联系。如果是内部点，则有联系
                for (auto faceIt = vertex->incidentFaces.begin(); faceIt != vertex->incidentFaces.end(); faceIt++) {      //内部点，incidentFaces[i]与incidentFaces[i+1]的公共边为incidentEdges[i+1](incidentFaces[i]包含incidentEdges[i])
                    if ((*faceIt)->faceId == (*it)) {                                                                     //或者说，incidentEdges[i]与incidentEdges[i+1]所组成的面是incidentFaces[i]
                        visited = true;
                        break;
                    }
                }
                if (!visited) {         //没访问过这个面
                    faceId = (*it);
                    vertex->incidentFaces.push_back(faces[*it]);
                    break;
                }
            }
            if (faceId == -1) { //如果这条边没有未访问过的面了（可能是边界边，可能是最后一条边），那么就把剩下的边中，边界边j挪到边i的后一个
                for (int j = i + 1; j < n; j++) {
                    if (vertex->incidentEdges[j]->faceId.size() < 2) {
                        int index = (i + 1);
                        Edge* tmp = vertex->incidentEdges[index];
                        vertex->incidentEdges[index] = vertex->incidentEdges[j];
                        vertex->incidentEdges[j] = tmp;
                        break;
                    }
                }
            }
            else {
                for (int j = i + 1; j < n; j++) {//还有未访问过的面，则通过面id来找相邻，把相邻的这条边j挪到边i的后一个
                    bool findEdge = false;
                    for (auto it = vertex->incidentEdges[j]->faceId.begin(); it != vertex->incidentEdges[j]->faceId.end(); it++) {
                        if ((*it) == faceId) {
                            int index = (i + 1) % n;                             //%n是否有必要？
                            Edge* tmp = vertex->incidentEdges[index];
                            vertex->incidentEdges[index] = vertex->incidentEdges[j];
                            vertex->incidentEdges[j] = tmp;
                            findEdge = true;
                            break;
                        }
                    }
                    if (findEdge) {
                        break;
                    }
                }
            }
            
        }

        //incidentEdges与incidentVertexes对应
        //将incidentEdges[i]中的连接点赋值给incidentVertexes[i]
        for (int indexOfEdge = 0; indexOfEdge < n; indexOfEdge++) {
            if (vertex->incidentEdges[indexOfEdge]->vertexe1->vertexId == vertex->vertexId) {
                vertex->incidentVertexes[indexOfEdge] = vertex->incidentEdges[indexOfEdge]->vertexe2;
            }
            else {
                vertex->incidentVertexes[indexOfEdge] = vertex->incidentEdges[indexOfEdge]->vertexe1;
            }
        }
    }
    return notEqual;    
}

//第二类点：在简化后不需要反转洞以外的边即可满足LD
bool Mesh::isTypeII(Vertex* vertex, std::vector<float>& subtendedAngles) {
    if (vertex->vertexId == 5 && vertex->incidentEdges.size() == 7) {
        printf("debug");
    }
    int edgeCount = vertex->incidentEdges.size();
    int faceCount = vertex->incidentFaces.size();
    vertex->typeII = false;
    vertex->flippedEdge.clear();
    if (vertex->boundary) {
        return false;
    }
    //Type-II一定在内部
    if (edgeCount != faceCount || edgeCount <= 2) {
        return false;
    }
    std::vector<int> eVertexIndex;
    eVertexIndex.clear();
    //incidentEdges和incidentVertexes经过Type-I的重新排布保证了相邻的组成一个面
    for (int i = 0; i < edgeCount; i++) {//对第i条边检查,看是否无需翻转
        //bool encounter = true;
        Vertex* v1 = vertex->incidentVertexes[i];
        int nextIndex = (i + 1) % edgeCount;
        Vertex* v2 = vertex->incidentVertexes[nextIndex];
        int remainingPiontsCount = edgeCount - 2;
        int j = (nextIndex + 1) % edgeCount;
        while (remainingPiontsCount > 0) {  //每个对顶点都需要检查
            Vertex* v3 = vertex->incidentVertexes[j];
            float length13 = glm::distance(v1->position, v3->position);
            float length23 = glm::distance(v2->position, v3->position);

            double cos = glm::dot((v2->position - v3->position), (v1->position - v3->position)) / length13 / length23;
            if (subtendedAngles[i] == -5) {    //没有外对角，则判断内对角
                if (cos < -0.000001) {    //不满足条件(角度大于90度）
                    return false;
                }
            }
            else {//外对角存在
                double sini = sqrt(1 - subtendedAngles[i] * subtendedAngles[i]);
                double sin = sqrt(1 - cos * cos);
                double sinSum = sini * cos + subtendedAngles[j] * sin;
                if (sinSum < -0.000001) {   //不满足条件(角度和大于180度）
                    return false;
                }
            }
            j++;
            j %= edgeCount;
            remainingPiontsCount--;
            //v2 = v3;//需要重置v2位置
        }
    }
    vertex->typeII = true;
    //int num = eVertexIndex.size();
    int num = edgeCount;
    float minEph = 1000000000;
    int minIndex = 0;
    for (int i = 0; i < edgeCount; i++) {
        glm::mat4 QSum = vertex->Q + vertex->incidentVertexes[i]->Q;
        glm::vec4 v4 = glm::vec4(vertex->incidentVertexes[i]->position, 1);
        glm::vec4 temp = v4 * QSum;
        float eph = glm::dot(v4, temp);
        if (eph < minEph) {
            minEph = eph;
            minIndex = i;
        }
    }
    vertex->e = vertex->incidentEdges[minIndex];
    vertex->eIndex = minIndex;
    Vertex* vAfter = vertex->incidentVertexes[minIndex];//边收缩后的顶点
    //squared Hausdorff distance
    //先得到压缩后的形状，计算各个面的质心（中线交点），再检测需要翻转哪几条边，最后算出dH
    int LTaTriangleCount = edgeCount - 2;
    if (LTaTriangleCount == 1) {
        vertex->typeII = false;
        return false;       //压缩后，只有1个三角形，此时没有内部边，无需翻转，因此不是TypeII顶点
    }
    else {
        
        int remainingVertexCount = LTaTriangleCount;
        int firstIndex = (minIndex + 1) % edgeCount;
        int secondIndex;
        Vertex* firstVertex = vertex->incidentVertexes[firstIndex];

        //判断顺逆时针
        Face* oldFace = vertex->incidentFaces[minIndex];
        /*for (auto faceIt = vertex->incidentFaces.begin(); faceIt != vertex->incidentFaces.end(); faceIt++) {
            if (VertexBelongToFace(vAfter, *faceIt) && VertexBelongToFace(firstVertex, *faceIt)) {
                oldFace = *faceIt;
                break;
            }
        }
        if (oldFace == nullptr) {
            printf("can't find face\n");
        }*/

        bool antiClockwish = judgeAntiClockWish(oldFace, vAfter, firstVertex);

        std::vector<Face*> newFaces;     //空洞三角化的结果（翻转后）
        std::list<int> borderEdgeIdList;    //构成空洞边缘的边
        std::vector<glm::vec3> massCenters;  //翻转前的质心集合
        //Edge* edge;
        /*
        if (vAfter->vertexId > vertex->incidentVertexes[firstIndex]->vertexId) {
            edge = findEdgeByPoints(vertex->incidentVertexes[firstIndex], vAfter);
        }
        else {
            edge = findEdgeByPoints(vAfter, vertex->incidentVertexes[firstIndex]);
        }
        borderEdgeIdList.push_back(edge->edgeId);*/
        
        if (vertex->vertexId == 3340) {
            printf("debug");    //翻转的边出现边界边情况
        }
        if (vertex->vertexId == 439 && vertex->e->vertexe1->vertexId == 427) {
            printf("debug");    //翻转的边出现边界边情况
        }
        if (vertex->vertexId == 439) {
            printf("debug");    //翻转的边出现边界边情况
        }

        int testVertexId = vertex->vertexId;
        for (int i = 0; i < faceCount; i++) {
            for (auto edgeIt = vertex->incidentFaces[i]->edges.begin(); edgeIt != vertex->incidentFaces[i]->edges.end(); edgeIt++) {
                if ((*edgeIt)->vertexe1->vertexId != testVertexId && (*edgeIt)->vertexe2->vertexId != testVertexId) {
                    borderEdgeIdList.push_back((*edgeIt)->edgeId);
                    break;
                }
            }
        }
        while (remainingVertexCount > 0) {                  //创建三角面片（不删除以前的面片）
            secondIndex = (firstIndex + 1) % edgeCount;
            Face* face; 
            if (antiClockwish) {
                face = generateNewFace(vAfter, vertex->incidentVertexes[firstIndex], vertex->incidentVertexes[secondIndex]);
            }
            else {
                face = generateNewFace(vAfter, vertex->incidentVertexes[secondIndex], vertex->incidentVertexes[firstIndex]);
            }
            /*
            if (vertex->incidentVertexes[secondIndex]->vertexId > vertex->incidentVertexes[firstIndex]->vertexId) {
                edge = findEdgeByPoints(vertex->incidentVertexes[firstIndex], vertex->incidentVertexes[secondIndex]);
            }
            else {
                edge = findEdgeByPoints(vertex->incidentVertexes[secondIndex], vertex->incidentVertexes[firstIndex]);
            }
            borderEdgeIdList.push_back(edge->edgeId);*/
            newFaces.push_back(face);
            firstIndex += 1;
            firstIndex = firstIndex % edgeCount;
            remainingVertexCount--;
            massCenters.push_back(face->getMassCenter());
        }
        /*
        if (vAfter->vertexId > vertex->incidentVertexes[secondIndex]->vertexId) {
            edge = findEdgeByPoints(vertex->incidentVertexes[secondIndex], vAfter);
        }
        else {
            edge = findEdgeByPoints(vAfter, vertex->incidentVertexes[secondIndex]);
        }
        borderEdgeIdList.push_back(edge->edgeId);*/
        //对于每个面片，检测边(vertex->incidentVertexes[minIndex]，vertex->incidentVertexes[vertexIndex])是否满足LD性质，不满足就翻转
 /*       int vertexIndex = (minIndex + 2) % edgeCount;
        for (int faceIndex = 0; faceIndex < LTaTriangleCount-1; faceIndex++) {
            //按照顶点序号来找这条边
            std::pair<int, int> idPair;
            int vertexIdMin = vertex->incidentVertexes[minIndex]->vertexId;
            int vertexIdCurrent = vertex->incidentVertexes[vertexIndex]->vertexId;
            if (vertexIdMin > vertexIdCurrent) {
                idPair.first = vertexIdCurrent;
                idPair.second = vertexIdMin;
            }
            else {
                idPair.first = vertexIdMin;
                idPair.second = vertexIdCurrent;
            }
            Edge* edgeToBeCheck;
            auto hashResult = m_hash_edge.find(idPair);
            if (hashResult != m_hash_edge.end()) {
                edgeToBeCheck = hashResult->second;
            }
            else {
                printf("找不到这条边#isTypeII\n");
                vertexIndex = (vertexIndex + 1) % edgeCount;
                continue;
            }
            int edgeId = edgeToBeCheck->edgeId;
            
            //检测这条边是否在这两个面片上，如果不在，就报错(仅在初次实现中使用，发现无误之后就可以删除）
            Face* currentFace = newFace[faceIndex];
            bool notIn = true;
            for (auto edgeIt = currentFace->edges.begin(); edgeIt != currentFace->edges.end(); edgeIt++) {
                if (edgeId == (*edgeIt)->edgeId) {
                    notIn = false;
                    break;
                }
            }
            if (notIn) {
                printf("该边不在面片上#isTypeII\n");
                vertexIndex = (vertexIndex + 1) % edgeCount;
                continue;
            }
            else {
                currentFace = newFace[faceIndex+1];
                notIn = true;
                for (auto edgeIt = currentFace->edges.begin(); edgeIt != currentFace->edges.end(); edgeIt++) {
                    if (edgeId == (*edgeIt)->edgeId) {
                        notIn = false;
                        break;
                    }
                }
                if (notIn) {
                    printf("该边不在面片上#isTypeII\n");
                    vertexIndex = (vertexIndex + 1) % edgeCount;
                    continue;
                }
            }
            //在面片上，看是否满足LD性质
            if (isNLD(edgeToBeCheck)) {
                flipEdge(edgeToBeCheck);
            }

        }
*/
        if (flipAllEdgesOfTypeII(newFaces, vertex, borderEdgeIdList) == false) {
            for (int i = 0; i < newFaces.size(); i++) {
                deleteFace(newFaces[i], true);
            }
            vertex->flippedEdge.clear();
            vertex->typeII = false;
            return false;
        };
        
        //计算dH
        std::vector<int> LTaVertexIds;              //按照id从小到大排序
        std::vector<glm::vec3> LTaVertexPositions;  //存放翻转结束后顶点位置
        for (auto faceIt = newFaces.begin(); faceIt != newFaces.end(); faceIt++) {  //把所有面上的顶点加入集合中
            for (auto vertexIt = (*faceIt)->vertexs.begin(); vertexIt != (*faceIt)->vertexs.end(); vertexIt++) {
                bool exist = false;
                auto idIt = LTaVertexIds.begin();
                for (; idIt != LTaVertexIds.end(); idIt++) {
                    if ((*idIt) == (*vertexIt)->vertexId) {
                        exist = true;
                        break;
                    }
                    else if ((*idIt) > (*vertexIt)->vertexId) {
                        break;
                    }
                }
                if (!exist) {
                    LTaVertexIds.insert(idIt, (*vertexIt)->vertexId);
                    LTaVertexPositions.push_back((*vertexIt)->position);
                }
            }
            
        }
        float maxDistance = 0.0;        //也就是dH
        int vertexCount = LTaVertexIds.size();
        for (int i = 0; i < LTaTriangleCount; i++) {        //对于每个质心，计算LTa到它的距离，找到最小值
            float minDistance = 1000000000;
            for (int j = 0; j < vertexCount; j++) {
                float distance = 0;
                float minus = LTaVertexPositions[j].x - massCenters[i].x;
                distance += minus * minus;
                minus = LTaVertexPositions[j].y - massCenters[i].y;
                distance += minus * minus;
                minus = LTaVertexPositions[j].z - massCenters[i].z;
                distance += minus * minus;

                if (distance < minDistance) {
                    minDistance = distance;
                }
            }

            if (minDistance > maxDistance) {
                maxDistance = minDistance;
            }
        }
        vertex->eph = minEph + (vertex->lambda + vAfter->lambda) * maxDistance;

        
        //最后还原，删除压缩和翻转生成的边和面
        for (int i = 0; i < LTaTriangleCount; i++) {
            deleteFace(newFaces[i], true);
        }
        return true;
    }



}

void Mesh::ToSet(Vertex* v) {
    std::vector<Vertex*> temp_vertex;
    std::vector<Edge*> temp_edge;
    for (auto it = v->incidentVertexes.begin(); it != v->incidentVertexes.end()-1; it++) {
        bool isOK = true;
        for (auto tmp = it + 1; tmp != v->incidentVertexes.end(); tmp++) {
            if ((*tmp)->vertexId == (*it)->vertexId) {
                isOK = false;
                printf("same incidentVertexes#ToSet");
                break;
            }
        }
        if (isOK && (*it)!=nullptr && !(*it)->deleted) {
            temp_vertex.push_back((*it));
        }

    }
    v->incidentVertexes.clear();
    for (auto it = temp_vertex.begin(); it != temp_vertex.end(); it++) {
        v->incidentVertexes.push_back(*it);
    }
    temp_vertex.clear();

    for (auto it = v->incidentEdges.begin(); it != v->incidentEdges.end() - 1; it++) {
        bool isOK = true;
        for (auto tmp = it + 1; tmp != v->incidentEdges.end(); tmp++) {
            if ((*tmp)->edgeId == (*it)->edgeId) {
                isOK = false;
                break;
            }
        }
        if (isOK && (*it)!=nullptr && !(*it)->deleted) {
            temp_edge.push_back(*it);
        }
    }
    v->incidentEdges.clear();
    for (auto it = temp_edge.begin(); it != temp_edge.end(); it++) {
        v->incidentEdges.push_back(*it);
    }
    temp_edge.clear();
}

void Mesh::printData(){
    printf("faces count: %d\n", faces.size());
    printf("vertexes count: %d\n", vertexes.size());

}

void Mesh::simplification(float scale) {
    init();
    
    ////todo
    //for (auto it = edges.begin(); it != edges.end(); it++) {
    //    if ((*it)->faceId.size() > 2) {
    //        printf("edge face > 2\n");
    //    }
    //}
    findTypeIAndTypeII();
    //todo
    //for (auto it = edges.begin(); it != edges.end(); it++) {
    //    if ((*it)->faceId.size() > 2) {
    //        printf("edge face > 2\n");
    //    }
    //}
    

    std::vector<Vertex*> vertexQueue;
    for (auto it = vertexes.begin(); it != vertexes.end(); it++) {
        /*if ((*it)->vertexId == 2826) {
            printf("debug");
        }*/
        vertexQueue.push_back(*it);
    }
    make_heap(vertexQueue.begin(), vertexQueue.end(), ephCmp());
    int totalVertexCount = vertexQueue.size();
    int targetVertexCount = totalVertexCount * scale;
    //开始简化
    while (totalVertexCount > targetVertexCount) {
        //saveOBJ("beforeSimp.obj");
        //取出最小点
        totalVertexCount--;         //TODO 检测是否需要重新push所有点
/*        vertexQueue.clear();
        for (auto it = vertexes.begin(); it != vertexes.end(); it++) {
            if ((*it)->deleted == false) {
                vertexQueue.push_back(*it);
            }
        }*/
        make_heap(vertexQueue.begin(), vertexQueue.end(), ephCmp());
        pop_heap(vertexQueue.begin(), vertexQueue.end(), ephCmp());
        Vertex* removeVertex = vertexQueue.back();
        int removeVertexId = removeVertex->vertexId;
        vertexQueue.pop_back();
        //if (removeVertex->vertexId == 439) {
        //    printf("dubug");
        //}

        if (removeVertex->typeI || removeVertex->typeII) {
            //if (removeVertex->typeII == true) {
            //    printf("typeII");
            //}
            Edge* contractEdge = removeVertex->e;
            if (contractEdge == nullptr) {//找第一个相邻点来压缩TODO
                printf("debug");
            }
            Vertex* resultVertex = contractEdge->vertexe1->vertexId == removeVertex->vertexId ? contractEdge->vertexe2 : contractEdge->vertexe1;    //压缩后的点
            int resultVertexId = resultVertex->vertexId;
            //printf("simplification(%d,%d) to %d\n", contractEdge->vertexe1->vertexId, contractEdge->vertexe2->vertexId, resultVertexId);
            
            std::set<Vertex*> influencedVertexes;   //所有与resultVertex和removeVertex有关联的点，用于之后更新邻点情况（如果不设置这个，会在翻转包含resultVertex的边时，把另一个点从resultVertex的incidentVertexes中删除，导致没法更新）
            for (auto it = resultVertex->incidentVertexes.begin(); it != resultVertex->incidentVertexes.end(); it++) {  //先把resultVertex的所有incidentVertexes添加进来（除了removeVertex）
                if ((*it)->vertexId != removeVertexId) {        
                    influencedVertexes.insert(*it);
                }
            }
            for (auto it = removeVertex->incidentVertexes.begin(); it != removeVertex->incidentVertexes.end(); it++) {  //再把removeVertex中除了resultVertex的所有incidentVertexes添加进来（不重复）
                if ((*it)->vertexId != resultVertexId) {
                    if (influencedVertexes.find(*it) == influencedVertexes.end()) { //找不到，说明不重复
                        influencedVertexes.insert(*it);
                    }
                }
            }
            
            

            resultVertex->Q += removeVertex->Q;
            resultVertex->lambda += removeVertex->lambda;
            int incidentCount = removeVertex->incidentVertexes.size();
            //压缩edge，先生成新的面，再更改未删除边的面信息，更改未删除顶点的incidentFace，删除原来的面和边，更改removeVertex的相邻边和相邻点的情况，删除点，计算resultVertex的类型，最后更新未删除顶点的类型
            //removeVertex->eIndex表示的是resultVertex
            int nextIndex = (removeVertex->eIndex + 1) % incidentCount;
            int secondIndex = (nextIndex + 1) % incidentCount;
            Vertex* nextVertex;
            Vertex* secondVertex;
            Edge* oldEdge;
            int newFaceCount = incidentCount - 2;
            //while循环中生成新的面，在同时把incidentCount-2个点的相邻点集删去removeVertex
            std::list<Face*> newFaces;
            while (newFaceCount > 0) {
                //找到原来的面
                oldEdge = removeVertex->incidentEdges[nextIndex];
                nextVertex = removeVertex->incidentVertexes[nextIndex];
                secondVertex = removeVertex->incidentVertexes[secondIndex];
                Face* oldFace = nullptr;

                for (auto it = oldEdge->faceId.begin(); it != oldEdge->faceId.end(); it++) {    //TODO 
                    if (VertexBelongToFace(secondVertex, faces[*it])) {
                        oldFace = faces[*it];
                        break;
                    }
                }

                //生成新的面
                Face* newFace = nullptr;
                if (oldFace != nullptr) {
                    //检测是否存在一个面包含这三个顶点
                    Edge* edge12 = nullptr;
                    if (secondVertex->vertexId > nextVertex->vertexId) {
                        edge12 = findEdgeByPoints(nextVertex, secondVertex);
                    }
                    else {
                        edge12 = findEdgeByPoints(secondVertex, nextVertex);
                    }
                    bool findFace = false;
                    if (edge12 != nullptr) {
                        for (auto it = edge12->faceId.begin(); it != edge12->faceId.end(); it++) {
                            if (VertexBelongToFace(resultVertex, faces[*it])) {
                                newFace = faces[*it];
                                findFace = true;
                                break;
                            }
                        }
                    }
                    if (!findFace) {
                        bool antiClockWish = judgeAntiClockWish(oldFace, secondVertex, nextVertex);
                    
                        if (antiClockWish) {
                            newFace = generateNewFace(secondVertex, nextVertex, resultVertex);
                        }
                        else {
                            newFace = generateNewFace(secondVertex, resultVertex, nextVertex);
                        }
                    }
                    newFaces.push_back(newFace);
                    
                }
                else {
                    printf("empty face\n");
                }

                //创建newFace的过程中已经创建好了新的两条边，因此可以直接寻找到他们（有可能是原来就存在的边，是resultVertex所在的边）
                Edge* newEdge;
                if (nextVertex->vertexId > resultVertex->vertexId) {
                    //newEdge = findEdgeByPoints(resultVertex, nextVertex);
                    for (auto it = newFace->edges.begin(); it != newFace->edges.end(); it++) {
                        if ((*it)->vertexe1->vertexId == resultVertex->vertexId && (*it)->vertexe2->vertexId == nextVertex->vertexId) {
                            newEdge = *it;
                            break;
                        }
                    }
                }
                else {
                    //newEdge = findEdgeByPoints(nextVertex, resultVertex);
                    for (auto it = newFace->edges.begin(); it != newFace->edges.end(); it++) {
                        if ((*it)->vertexe1->vertexId == nextVertex->vertexId && (*it)->vertexe2->vertexId == resultVertex->vertexId) {
                            newEdge = *it;
                            break;
                        }
                    }
                }

                bool isNew = true;
                //更改非resultVertex的相邻点,删除removeVertex，添加resultVertex
                for (auto it = nextVertex->incidentVertexes.begin(); it != nextVertex->incidentVertexes.end(); it++) {
                    if ((*it)->vertexId == removeVertexId) {
                        it = nextVertex->incidentVertexes.erase(it);

                        //存在一种可能：只是删除了三角形内部的边但没有形成新的边，此时不需要再在incidentVertexes中添加点（因为已经存在)
                        for (auto tmp = nextVertex->incidentVertexes.begin(); tmp != nextVertex->incidentVertexes.end(); tmp++) {
                            if ((*tmp)->vertexId == resultVertexId) {
                                isNew = false;
                                break;
                            }
                        }
                        if (isNew) {
                            nextVertex->incidentVertexes.insert(it, resultVertex);
                        }
                        break;
                    }
                }

                //更改非resultVertex的相邻边
                for (auto it = nextVertex->incidentEdges.begin(); it != nextVertex->incidentEdges.end(); it++) {
                    if ((*it)->edgeId == oldEdge->edgeId) {
                        it = nextVertex->incidentEdges.erase(it);
                        if (isNew) {
                            nextVertex->incidentEdges.insert(it, newEdge);//插入新的边
                        }
                        break;
                    }
                }


                //result点也添加新的边     TODO 插入顺序是否需要考虑
                if (isNew) {
                    resultVertex->incidentEdges.push_back(newEdge);
                    resultVertex->incidentVertexes.push_back(nextVertex);
                }
                newFaceCount--;
                nextIndex = secondIndex;
                secondIndex = (nextIndex + 1) % incidentCount;
            }
            //还剩最后一个与resultVertex相邻的点未处理邻居关系
            oldEdge = removeVertex->incidentEdges[nextIndex];
            nextVertex = removeVertex->incidentVertexes[nextIndex];
            for (auto it = nextVertex->incidentVertexes.begin(); it != nextVertex->incidentVertexes.end(); it++) {
                if ((*it)->vertexId == removeVertexId) {
                    it = nextVertex->incidentVertexes.erase(it);
                    break;
                }
            }

            for (auto it = nextVertex->incidentEdges.begin(); it != nextVertex->incidentEdges.end(); it++) {
                if ((*it)->edgeId == oldEdge->edgeId) {
                    it = nextVertex->incidentEdges.erase(it);
                    break;
                }
            }
            
            //修改resultVertex的邻居关系，删除removeVertex
            //邻点
            for (auto it = resultVertex->incidentVertexes.begin(); it != resultVertex->incidentVertexes.end(); it++) {
                if ((*it)->vertexId == removeVertexId) {
                    it = resultVertex->incidentVertexes.erase(it);
                    break;
                }
            }
            //邻边
            for (auto it = resultVertex->incidentEdges.begin(); it != resultVertex->incidentEdges.end(); it++) {
                if ((*it)->edgeId == contractEdge->edgeId) {
                    it = resultVertex->incidentEdges.erase(it);
                    break;
                }
            }


            //删除原来的面和边(更改未删除顶点的incidentFace在resort函数里)
            for (auto faceIt = removeVertex->incidentFaces.begin(); faceIt != removeVertex->incidentFaces.end(); faceIt++) {
                deleteFace(*faceIt, false);
            }

            

            if (removeVertex->typeII) {
                for (auto edgeIt = removeVertex->flippedEdge.begin(); edgeIt != removeVertex->flippedEdge.end(); edgeIt++) {
                    Vertex* v1 = vertexes[(*edgeIt).first];
                    Vertex* v2 = vertexes[(*edgeIt).second];
                    
                    //翻转之后需要修改邻边关系！！！
                    flipEdgeWhenSimplifying(v1, v2, newFaces);

                }
                removeVertex->flippedEdge.clear();
                
            }

            //删除点
            removeVertex->deleted = true;



            //操作，确保变化过的点中没有重复的内容
    /*        for (auto it = vertexes.begin(); it != vertexes.end(); it++) {
                ToSet(*it);
            }*/
            /*ToSet(resultVertex);
            for (auto it = resultVertex->incidentVertexes.begin(); it != resultVertex->incidentVertexes.end(); it++) {
                ToSet(*it);
            }*/

            //更新result点的类型
            std::vector<float> subtendedAngles;
            if (!isTypeI(resultVertex, subtendedAngles)) {
                isTypeII(resultVertex, subtendedAngles);
            }
            subtendedAngles.clear();

            for (auto it = influencedVertexes.begin(); it != influencedVertexes.end(); it++) {
                if (!isTypeI(*it, subtendedAngles)) {
                    isTypeII(*it, subtendedAngles);
                }
                subtendedAngles.clear();
            }
            //for (auto it = resultVertex->incidentVertexes.begin(); it != resultVertex->incidentVertexes.end();) {
            //    if ((*it) == nullptr || (*it)->deleted) {
            //        //从中删除
            //        it = resultVertex->incidentVertexes.erase(it);
            //        continue;
            //    }
            //    //if ((*it)->vertexId == 1260) {
            //    //    printf("debug");
            //    //}
            //    if (!isTypeI(*it, subtendedAngles)) {
            //        
            //        isTypeII(*it, subtendedAngles);
            //    }
            //    subtendedAngles.clear();
            //    it++;
            //}
        }else {
            printf("abort, no typeI or II, totalVertexCount: %d\n", totalVertexCount);
            break;
        }
    }

    printf("totalVertexCount: %d\n", totalVertexCount);
    
        
}