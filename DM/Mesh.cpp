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

struct cmp {
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
bool compare(const Edge* a, const  Edge* b){        
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
//获得的是在v1v3上的比例
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

//在v1v2这条边上，找到一点p使得v1v3垂直于v3p。p的坐标可以表示为（t(x2-x1)+x1,t(y2-y1)+y1,t(z2-z1)+z1）
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

std::pair<float, float> getSplitRegion(float tStart, float tEnd, float tE, float tH, float tF, float tG) {
    float min1;
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
        if(max1 >= tStart)
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
glm::vec3 rotatePoint(glm::vec3& normalFixed, glm::vec3& normalRotated, glm::vec3& edgeNormal, glm::vec3& startPosition, glm::vec3& pointRotated){
    float cosAngle = glm::dot(normalFixed, normalRotated);   //都是单位向量
    float angle = acos(cosAngle);                           //旋转的弧度
    glm::vec3 o = glm::normalize(glm::cross(normalFixed, normalRotated));   //判断旋转方向，与交线同向则顺时针旋转；反向则逆时针旋转
    glm::mat4 trans = glm::mat4(1.0f); //创建单位矩阵
    if ((o.x >= 0 && edgeNormal.x < 0) || (o.x <= 0 && edgeNormal.x > 0)) {  //反向(因为glm::rotate函数本来就是逆时针旋转的）
        ;
    }
    else {                              //同向
        angle = -angle;
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
                exist == true;
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
    face->setVertex(vs);
    face->children.push_back(face);
    generateEdgeOfFace(face, true);
    face->calNormalOfFace();
    faces.push_back(face);
    faceNum++;


    if (vertIndexEnd < tokenLength -1) {
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
        generateEdgeOfFace(face2, true);
        face2->calNormalOfFace();
        faces.push_back(face2);
        faceNum++;
    }
    
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

bool Mesh::readSTL(const char *fileName) {
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
                    Vertex * vertex = new Vertex();
                    vertex->init(position);
                    vertex->setId(vertexes.size());
                    this->vertexes.push_back(vertex);
                }
                    
                break;
            case 'f':
                CreateOBJFace(line);
                break;
            default: break;
            };
        }
    }
    else {
        return false;
    }
    return true;
}

bool Mesh::readSTLASCII(const char *fileName) {
    std::ifstream fileSTL(fileName, std::ios::in | std::ios::binary);//in:只读   binary:二进制模式
    char buf[255];
    char end[] = "endsolid";
    char begin[] = "facet";
    fileSTL >> buf;         //solid
    fileSTL >> partName;    //filenamestl 
    float v1, v2, v3;
    int faceId = 0;
    int vertexId = 0;

    while (fileSTL >> buf) {
        if (strcmp(buf, end) == 0) { //endsolid结束 0表示两个值相等
            break;
        }
        else if (strcmp(buf, begin) == 0) { //facet
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
        generateEdgeOfFace(face,true);

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
    for (int i = 0; i < totalFace; i++) {
        if (faces[i]->isMesh == true) {
            for (auto it = faces[i]->children.begin(); it != faces[i]->children.end(); it++) {
                if ((*it)->deleted == false) {
                    fprintf(fp, "f %d %d %d\n", (*it)->vertexs[0]->vertexId+1, (*it)->vertexs[1]->vertexId+1, (*it)->vertexs[2]->vertexId+1);
                }
            }
        }
    }
}

bool Mesh::saveSTLASCII(const char *fileName) {

    char *fileInf = new char[200];
    int len = 0;
    while (fileName[len] != '\0') {
        len++;
    }
    snprintf(fileInf,len+7, "solid %s", fileName);
    FILE* fp = nullptr;
    int err = fopen_s(&fp, fileName, "w");

    if (err != 0|| fp == nullptr) {
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

bool Mesh::saveSTLBinary(const char * fileName) {

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
    float *data = new float[12];

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

Edge* Mesh::generateEdge(Vertex* v1, Vertex* v2){
    Vertex* pFirst, * pSecond;
    
    if (v1->vertexId < v2->vertexId) {
        pFirst = v1;
        pSecond = v2;
    }
    else {
        pFirst = v2;
        pSecond = v1;
    }
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
    return edge;
}

void Mesh::generateEdgeOfFace(Face* face, bool meshEdge){
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
    
}

void Mesh::computeParameter(){
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

void Mesh::generateDM(){
    printf("edges: %d\n", edges.size());
    computeParameter();
    for (auto it = edges.begin(); it != edges.end(); it++) {
        (*it)->constructCe(rhoV, rhoE);
    } 
    findAllNLDEdges();
   while (!NLDEdges.empty()) {
        Edge* currentEdge = NLDEdges.top();
        NLDEdges.pop();
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



    int count = 0;
    for (auto it = edges.begin(); it != edges.end(); it++) {
        if ((*it)->vertexe1->vertexId == 4972 && (*it)->vertexe2->vertexId == 4974) {
            printf("debug");
        }
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
/*    if (edge->edgeId == 5314) {
        printf("debug");
    }
*/    
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

        float cosAngle = glm::dot(normalABD, normalABC);   //都是单位向量
        float angle = acos(cosAngle);                   //旋转的弧度
        glm::vec3 ve = glm::normalize(vertexA->position - vertexB->position); //该边的向量
        glm::vec3 o = glm::normalize(glm::cross(normalABD, normalABC));   //判断旋转方向，与交线同向则顺时针旋转；反向则逆时针旋转
        if ((o.x >=0 && ve.x < 0) || (o.x <= 0 && ve.x > 0)) {  //反向(因为glm::rotate函数本来就是逆时针旋转的）
            //angle = -angle;
        }
        else {
            angle = -angle;
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
                */            glm::vec3 normalACE;
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
            }
        }
        else {
            tE = 0;
        }

        float tF = 1;
        if (edgeBC != NULL) {
            if (edgeBC->faceId.size() == 2) {
                //printf_s("3\n");
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
                //printf_s("4\n");
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
        }
        else {
            tH = 0;
        }

        float tG = 1;
        if (edgeBD != NULL) {
            if (edgeBD->faceId.size() == 2) {
                //printf_s("5\n");
                glm::vec4 vertexGPosition = glm::vec4(getAnotherVertexPositionByEdge(faceABD, edgeBD), 1);

                glm::vec3 vertexGPos = glm::vec3(vertexGPosition);

                glm::vec3 normalBDG;
                if (antiClockWishOfABInABD) {            //AB在ABD为逆时针，则BD在ABD为逆时针，所以BD在BDG为顺时针
                    normalBDG = calNormal(vertexD->position, vertexB->position, vertexGPos);
                }
                else {
                    normalBDG = calNormal(vertexB->position, vertexD->position, vertexGPos);

                }
                //            glm::vec3 normalBDG = calNormal(vertexB->position, vertexGPos, vertexD->position);
                glm::vec3 newBD = glm::normalize(vertexD->position - vertexB->position); //该边的向量
                vertexGPos = rotatePoint(normalABD, normalBDG, newBD, vertexB->position, vertexGPos);

                center = solveCenterPointOfCircle(vertexB->position, vertexD->position, vertexGPos);
                tG = 1 - getAnotherPoint(vertexA->position, vertexB->position, center);
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
        glm::vec3 splitPosition = parentEdge->getSplitePosition(p1, p2);


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

        //翻转parentFaceOfABC和parentFaceOfABD中所有flippable的NLD
        flipAllNLDEdgeInFace(parentFaceOfABC);
        flipAllNLDEdgeInFace(parentFaceOfABD);

        addNewNonNLDEdge(parentFaceOfABC);
        addNewNonNLDEdge(parentFaceOfABD);
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
        else {
            tD = 0;
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
        else {
            tE = 1;
        }

        std::pair<float, float> tRegion = getSplitRegionOfBoundaryEdge(tStart, tEnd, tD, tE);//相对于AB而言
        glm::vec3 p1 = glm::vec3(vertexA->position.x + tRegion.first * (vertexB->position.x - vertexA->position.x), vertexA->position.y + tRegion.first * (vertexB->position.y - vertexA->position.y), vertexA->position.z + tRegion.first * (vertexB->position.z - vertexA->position.z));
        glm::vec3 p2 = glm::vec3(vertexA->position.x + tRegion.second * (vertexB->position.x - vertexA->position.x), vertexA->position.y + tRegion.second * (vertexB->position.y - vertexA->position.y), vertexA->position.z + tRegion.second * (vertexB->position.z - vertexA->position.z));

        //找到e ∈ E
        Edge* parentEdge = edge->parent;
        glm::vec3 splitPosition = parentEdge->getSplitePosition(p1, p2);

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

        //翻转parentFaceOfABC中所有flippable的NLD
        flipAllNLDEdgeInFace(parentFaceOfABC);

        addNewNonNLDEdge(parentFaceOfABC);
    }
    

}

bool Mesh::isNLD(Edge* edge){
    edge->flippable = false;
    if (edge->deleted == true || edge->edgeId < 0) {
        return false;
    }
    if (edge->faceId.size() == 1) {
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
        //printf_s("9_2\n");
        Face* face1 = faces[*(edge->faceId.begin())];
        Face* face2 = faces[*(++edge->faceId.begin())];
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

        double length31 = glm::distance(a->position, top2->position);
        double length32 = glm::distance(b->position, top2->position);
        cos2 = glm::dot((a->position - top2->position), (b->position - top2->position)) / length31 / length32;
        //求角度之和，用sin(1+2) = sin(1)*cos(2) + cos(1)*sin(2)
        double sin1 = sqrt(1 - cos1 * cos1);
        double sin2 = sqrt(1 - cos2 * cos2);
        double angle1 = acos(cos1);
        double angle2 = acos(cos2);
        double sin12 = (sin1 * cos2 + cos1 * sin2);
        if (sin12 < 0.00001 || (angle1 + angle2) > PI) {
            glm::vec3 normal1 = glm::normalize(face1->normal);
            glm::vec3 normal2 = glm::normalize(face2->normal);
            glm::vec3 dif = normal1 - normal2;
            if (glm::distance(dif, glm::vec3(0, 0, 0)) < 0.000002 || (normal1 + normal2 == glm::vec3(0, 0, 0))) {
                if (top1->vertexId < top2->vertexId) {
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
                if (sin12 < 0) {
                    return true;
                }
                else {
                    double cos3 = glm::dot((top1->position - a->position), (top2->position - a->position)) / length01 / length31;
                    double cos4 = glm::dot((top1->position - b->position), (top2->position - b->position)) / length02 / length32;
                    double sin3 = sqrt(1 - cos3 * cos3);
                    double sin4 = sqrt(1 - cos4 * cos4);
                    double angle3 = acos(cos3);
                    double angle4 = acos(cos4);
                    if ((sin3 * cos4 + cos3 * sin4) <= 0 || (angle3 + angle4) >= PI) {
                        return false;
                    }
                    return true;
                }

            }
            return true;




        }
        return false;

    }
    else {
        printf("empty face of edge %d\n", edge->edgeId);
        return false;
    }
}

void Mesh::findAllNLDEdges() {
    for (auto it = edges.begin(); it != edges.end(); it++) {
        /*        if ((*it)->faceId.size() != 2) {
                    continue;
                }
                else
        */
        if ((*it)->inStack == false && isNLD(*it)) {
            (*it)->inStack = true;
            NLDEdges.push(*it);
        }
    }
}

void Mesh::flipAllNLDEdgeInFace(Face* face) {
    if (face->faceId == 3886) {
        printf("debug");
    }
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
                //if (((*edgeIt)->vertexe1->vertexId == 2373 && (*edgeIt)->vertexe2->vertexId == 2378) || ((*edgeIt)->vertexe1->vertexId == 2375 && (*edgeIt)->vertexe2->vertexId == 2376)){
                if (((*edgeIt)->vertexe1->vertexId == 2109 && (*edgeIt)->vertexe2->vertexId == 2769) || ((*edgeIt)->vertexe1->vertexId == 2768 && (*edgeIt)->vertexe2->vertexId == 2770)) {
                    printf("debug");
                }
                if (isNLD(*edgeIt)) {
                    if ((*edgeIt)->flippable == true) {
                        flipEdge((*edgeIt));
                        goto begin_process;
                    }
                }
            }
        }
    }
}

void Mesh::flipEdge(Edge* edgeAB) {
    /*if (edgeAB->edgeId == 664) {
        printf("debug");
    }
    printf("flip %d", edgeAB->edgeId);*/
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



    Edge* parentEdge = edgeAB->parent;

    Face* parentFaceOfABC = getParentFace(parentEdge, faceABC);
    Face* parentFaceOfABD = getParentFace(parentEdge, faceABD);

    if (parentFaceOfABC->faceId == parentFaceOfABD->faceId) {   //同一个面片分割出来的两个子面片
        parentFaceOfABC->deleteChild(faceABC);
        parentFaceOfABC->deleteChild(faceABD);
        parentFaceOfABC->children.push_back(faceACD);
        parentFaceOfABC->children.push_back(faceBCD);
        setMeshFaceOfNewEdge(parentFaceOfABC, faceACD);
        setMeshFaceOfNewEdge(parentFaceOfABC, faceBCD);
    }
    else {
        parentFaceOfABC->deleteChild(faceABC);
        parentFaceOfABD->deleteChild(faceABD);
        setMeshFaceOfNewEdge(faceACD, faceACD);
        setMeshFaceOfNewEdge(faceBCD, faceBCD);
        faceACD->isMesh = true;
        faceBCD->isMesh = true;
    }



    auto iter = edgeAD->faceId.find(faceABD->faceId);
    edgeAD->faceId.erase(iter);
    iter = edgeBD->faceId.find(faceABD->faceId);
    edgeBD->faceId.erase(iter);
    iter = edgeAC->faceId.find(faceABC->faceId);
    edgeAC->faceId.erase(iter);
    iter = edgeBC->faceId.find(faceABC->faceId);
    edgeBC->faceId.erase(iter);

    
    
    //删除边
    for (auto it = edgeAB->meshFaceId.begin(); it != edgeAB->meshFaceId.end(); it++) {
        addNewNonNLDEdge(faces[*it]);
    }
    std::pair<int, int> edgePair(edgeAB->vertexe1->vertexId, edgeAB->vertexe2->vertexId);
    m_hash_edge.erase(edgePair);
    for (auto it = edges.begin(); it != edges.end(); it++) {
        if ((*it)->edgeId == edgeAB->edgeId) {
            delete(*it);
            edges.erase(it);
            break;
        }
    }

    faceABC->deleted = true;
    faceABD->deleted = true;

    
}

Face* Mesh::getParentFace(Edge* edge, Face* childFace) {
    Face* face1 = faces[*(edge->meshFaceId.begin())];
    for (auto it = face1->children.begin(); it != face1->children.end(); it++) {
        if ((*it)->faceId == childFace->faceId) {
            return face1;
        }
    }
    return faces[*(++edge->meshFaceId.begin())];
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

    face->setVertex(vs);
    face->isMesh = false;
    faces.push_back(face);
    face->children.push_back(face);

    generateEdgeOfFace(face, false);

    return face;
}

void Mesh::addNewNonNLDEdge(Face* face) {
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


void Mesh::init(){
    //找到每个点的临近点
    for (auto it = edges.begin(); it != edges.end(); it++) {
        if ((*it)->splitted == false && (*it) != NULL && (*it)->vertexe1->vertexId != (*it)->vertexe2->vertexId) {
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
        //计算每个点的Q矩阵
        for (auto edgeIt = (*it)->incidentEdges.begin(); edgeIt != (*it)->incidentEdges.end(); edgeIt++) {
            for (auto faceIt = (*edgeIt)->faceId.begin(); faceIt != (*edgeIt)->faceId.end(); faceIt++) {
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
        }
        visitedFace.clear();
    }
}

bool Mesh::isTypeI(Vertex* vertex, std::vector<Face*>& incidentFaces, std::vector<float>& subtendedAngles) {
    //重新排布incidentEdges，使得相邻两条边在同一三角面上

    int n = vertex->incidentEdges.size();
    vertex->incidentVertexes.clear();
    vertex->incidentVertexes.assign(n, nullptr);
    bool notEqual = false;//边的数目与面的数目不一致，非内部点。当不是内部点的时候为true
    if (n == 2) {//n=2
        for (int i = 0; i < n; i++) {
            if (vertex->incidentEdges[i]->vertexe1->vertexId == vertex->vertexId) {
                vertex->incidentVertexes[i] = vertex->incidentEdges[i]->vertexe2;
            }
            else {
                vertex->incidentVertexes[i] = vertex->incidentEdges[i]->vertexe1;
            }
        }
        incidentFaces.push_back(faces[(*vertex->incidentEdges[0]->faceId.begin())]);
        notEqual = true;
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

        //将incidentEdges[0]中的连接点赋值给incidentVertexes[0]
        if (vertex->incidentEdges[0]->vertexe1->vertexId == vertex->vertexId) {
            vertex->incidentVertexes[0] = vertex->incidentEdges[0]->vertexe2;
        }
        else {
            vertex->incidentVertexes[0] = vertex->incidentEdges[0]->vertexe1;
        }
        for (int i = 0; i < n; i++) {
            int faceId = -1;
            for (auto it = vertex->incidentEdges[i]->faceId.begin(); it != vertex->incidentEdges[i]->faceId.end(); it++) {//对每条边，找到它所属的面中没有访问过的，加入到incidentFaces中
                bool visited = false;
                for (auto faceIt = incidentFaces.begin(); faceIt != incidentFaces.end(); faceIt++) {
                    if ((*faceIt)->faceId == (*it)) {
                        visited = true;
                        break;
                    }
                }
                if (!visited) {
                    faceId = (*it);
                    incidentFaces.push_back(faces[*it]);
                    break;
                }
            }
            for (int j = i + 1; j < n; j++) {//通过面id来找相邻
                bool findEdge = false;
                for (auto it = vertex->incidentEdges[j]->faceId.begin(); it != vertex->incidentEdges[j]->faceId.end(); it++) {
                    if ((*it) == faceId) {
                        Vertex* v2;
                        if (vertex->incidentEdges[j]->vertexe1->vertexId == vertex->vertexId) {
                            v2 = vertex->incidentEdges[j]->vertexe2;
                        }
                        else {
                            v2 = vertex->incidentEdges[j]->vertexe1;
                        }
                        Edge* tmp = vertex->incidentEdges[i + 1];
                        vertex->incidentEdges[i + 1] = vertex->incidentEdges[j];
                        vertex->incidentEdges[j] = tmp;
                        vertex->incidentVertexes[i + 1] = v2;
                        findEdge = true;
                        break;
                    }
                }
                if (findEdge) {
                    break;
                }
            }
        }

        for (int i = 0; i < n; i++) {
            for (auto it = vertex->incidentEdges[i]->faceId.begin(); it != vertex->incidentEdges[i]->faceId.end(); it++) {//对每条边，找到它所属的面中没有访问过的，加入到incidentFaces中
                bool visited = false;
                for (auto faceIt = incidentFaces.begin(); faceIt != incidentFaces.end(); faceIt++) {
                    if ((*faceIt)->faceId == (*it)) {
                        visited = true;
                        break;
                    }
                }
                if (!visited) {
                    incidentFaces.push_back(faces[*it]);
                }
            }
        }
        /*if (vertex->incidentEdges[n - 1]->vertexe1->vertexId == vertex->vertexId) {
            vertex->incidentVertexes[n - 1] = vertex->incidentEdges[n - 1]->vertexe2;
        }
        else {
            vertex->incidentVertexes[n - 1] = vertex->incidentEdges[n - 1]->vertexe1;
        }*/
    }


    //找到对应的对角度数
    if (!notEqual)
        for (auto faceIt = incidentFaces.begin(); faceIt != incidentFaces.end(); faceIt++)
        {
            //先找到对边
            Edge* oppositeEdge = nullptr;
            for (auto edgeIt = (*faceIt)->edges.begin(); edgeIt != (*faceIt)->edges.end(); edgeIt++) {
                if ((*edgeIt)->vertexe1->vertexId != vertex->vertexId && (*edgeIt)->vertexe2->vertexId != vertex->vertexId) {
                    oppositeEdge = (*edgeIt);
                    break;
                }
            }
            // printf_s("\n\n");
             //再找到对角度数
            if (oppositeEdge->faceId.size() < 2) {
                subtendedAngles.push_back(-5);
            }
            else {
                subtendedAngles.push_back(getAnotherVertexDegreeByEdge(*faceIt, oppositeEdge));
            }
        }
    //对每个相邻点进行检测（假设把（it，neighborV）压缩到neighborV），检测新的边是否满足nld条件
    //如果当前点是已经是typeI了，那么选择更小的eph作为要压缩的情况
    if (!notEqual) {
        for (int i = 0; i < n; i++) { //i是要压缩到的点
            Vertex* neighborV = vertex->incidentEdges[i]->vertexe1->vertexId == vertex->vertexId ? vertex->incidentEdges[i]->vertexe2 : vertex->incidentEdges[i]->vertexe1;
            bool sat = true;
            //检测相邻的边线
            int firstIndex = (i + 1) % n;
            Vertex* v0 = vertex->incidentEdges[firstIndex]->vertexe1->vertexId == vertex->vertexId ? vertex->incidentEdges[firstIndex]->vertexe2 : vertex->incidentEdges[firstIndex]->vertexe1;
            int secondIndex = (i + 2) % n;
            Vertex* v1 = vertex->incidentEdges[secondIndex]->vertexe1->vertexId == vertex->vertexId ? vertex->incidentEdges[secondIndex]->vertexe2 : vertex->incidentEdges[secondIndex]->vertexe1;
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

            firstIndex = (i + n - 1) % n;
            v0 = vertex->incidentEdges[firstIndex]->vertexe1->vertexId == vertex->vertexId ? vertex->incidentEdges[firstIndex]->vertexe2 : vertex->incidentEdges[firstIndex]->vertexe1;
            secondIndex = (i + n - 2) % n;
            v1 = vertex->incidentEdges[secondIndex]->vertexe1->vertexId == vertex->vertexId ? vertex->incidentEdges[secondIndex]->vertexe2 : vertex->incidentEdges[secondIndex]->vertexe1;
            lengthV1 = glm::distance(neighborV->position, v1->position);
            length01 = glm::distance(v0->position, v1->position);

            cos0 = glm::dot((neighborV->position - v1->position), (v0->position - v1->position)) / lengthV1 / length01;
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
                    if (sinSum < 0.00001) {
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
            int newEdge = n - 2;
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
                if (sinSum < 0.00001) {
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
    }
    else {//找到最小的e
        for (int i = 0; i < n && vertex->incidentVertexes[i]; i++) {
            glm::mat4 QSum = vertex->Q + vertex->incidentVertexes[i]->Q;
            glm::vec4 v4 = glm::vec4(vertex->incidentVertexes[i]->position, 1);
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

    if (vertex->typeI == true) {
        return true;
    }
    else {
        return false;
    }
}


void Mesh::findTypeIAndTypeII() {
    std::vector<Face*> incidentFaces;
    std::vector<float> subtendedAngles;
    for (auto it = vertexes.begin(); it != vertexes.end(); it++) {
        isTypeI(*it, incidentFaces, subtendedAngles);
        /*if (!isTypeI(*it, incidentFaces, subtendedAngles)) {
            isTypeII(*it, incidentFaces, subtendedAngles);
        }*/
        incidentFaces.clear();
        subtendedAngles.clear();
    }
}

//第二类点：在简化后不需要反转洞以外的边即可满足LD
bool Mesh::isTypeII(Vertex* vertex, std::vector<Face*>& incidentFaces, std::vector<float>& subtendedAngles) {
    int edgeCount = vertex->incidentEdges.size();
    int faceCount = incidentFaces.size();
    vertex->typeII = false;
    //Type-II一定在内部
    if (edgeCount != faceCount || edgeCount <= 2) {
        return false;
    }
    std::vector<int> eVertexIndex;
    eVertexIndex.clear();
    //incidentEdges和incidentVertexes经过Type-I的重新排布保证了相邻的组成一个面
    for (int i = 0; i < edgeCount; i++) {//对第i条边检查
        bool encounter = true;
        Vertex* v1 = vertex->incidentVertexes[i];
        int nextIndex = (i + 1) % edgeCount;
        Vertex* v2 = vertex->incidentVertexes[nextIndex];
        int remainingPiontsCount = edgeCount - 2;
        int j = (nextIndex + 1) % edgeCount;
        while (remainingPiontsCount > 0) {
            Vertex* v3 = vertex->incidentVertexes[j];
            float length13 = glm::distance(v1->position, v3->position);
            float length23 = glm::distance(v2->position, v3->position);

            double cos = glm::dot((v2->position - v3->position), (v1->position - v3->position)) / length13 / length23;
            if (subtendedAngles[j] == -5) {    //没有外对角，则判断内对角
                if (cos < -0.000001) {    //不满足条件
                    encounter = false;
                    break;
                }
            }
            else {//外对角存在
                double sinj = sqrt(1 - subtendedAngles[j] * subtendedAngles[j]);
                double sin = sqrt(1 - cos * cos);
                double sinSum = sinj * cos + subtendedAngles[j] * sin;
                if (sinSum < -0.000001) {
                    encounter = false;
                    break;
                }
            }
            j++;
            j %= edgeCount;
            remainingPiontsCount--;
            v2 = v3;//需要重置v2位置
        }
        if (encounter == true) {
            eVertexIndex.push_back(i);
        }
    }
    if (eVertexIndex.size() == 0) {
        return false;
    }
    vertex->typeII = true;
    int num = eVertexIndex.size();
    float minEph = 1000000000;
    int minIndex = 0;
    for (int i = 0; i < num; i++) {
        glm::mat4 QSum = vertex->Q + vertex->incidentVertexes[eVertexIndex[i]]->Q;
        glm::vec4 v4 = glm::vec4(vertex->incidentVertexes[eVertexIndex[i]]->position, 1);
        glm::vec4 temp = v4 * QSum;
        float eph = glm::dot(v4, temp);
        if (eph < minEph) {
            minEph = eph;
            minIndex = eVertexIndex[i];
        }
    }
    /*for (int i = 0; i < edgeCount; i++) {
        glm::mat4 QSum = vertex->Q + vertex->incidentVertexes[i]->Q;
        glm::vec4 v4 = glm::vec4(vertex->incidentVertexes[i]->position, 1);
        glm::vec4 temp = v4 * QSum;
        float eph = glm::dot(v4, temp);
        if (eph < minEph) {
            minEph = eph;
            minIndex = i;
        }
    }*/
    vertex->e = vertex->incidentEdges[minIndex];
    vertex->eIndex = minIndex;
    double dH = 1000;
    vertex->eph = minEph + (vertex->lambda + vertex->incidentVertexes[minIndex]->lambda) * dH;

    return true;
}

void Mesh::simplification(float scale) {
    init();
    findTypeIAndTypeII();
    std::vector<Vertex*> vertexQueue;
    for (auto it = vertexes.begin(); it != vertexes.end(); it++) {
        vertexQueue.push_back(*it);
    }
    make_heap(vertexQueue.begin(), vertexQueue.end(), cmp());
    int totalVertexCount = vertexQueue.size();
    int targetVertexCount = totalVertexCount * scale;
    //开始简化
    while (totalVertexCount > targetVertexCount) {
        //取出最小点
        pop_heap(vertexQueue.begin(), vertexQueue.end(), cmp());
        Vertex* removeVertex = vertexQueue.back();
        int removeVertexId = removeVertex->vertexId;
        vertexQueue.pop_back();
        Edge* contractEdge = removeVertex->e;
        if (contractEdge == nullptr) {//找第一个相邻点来压缩
            printf("debug");
        }
        Vertex* resultVertex = contractEdge->vertexe1->vertexId == removeVertex->vertexId ? contractEdge->vertexe2 : contractEdge->vertexe1;
        resultVertex->Q += removeVertex->Q;
        resultVertex->lambda += removeVertex->lambda;
        int incidentCount = removeVertex->incidentVertexes.size();
        //压缩edge，先生成新的面，再更改未删除边的面信息，删除原来的面和边，更改removeVertex的相邻点的情况，计算resultVertex的类型，最后删除点
        int nextIndex = (removeVertex->eIndex + 1) % incidentCount;
        int secondIndex = (nextIndex + 1) % incidentCount;
        Vertex* nextVertex = removeVertex->incidentVertexes[nextIndex];
        Vertex* secondVertex;

        int newFaceCount = incidentCount - 2;
        while (newFaceCount > 0) {
            //找到原来的面
            Edge* oldEdge = removeVertex->incidentEdges[nextIndex];
            nextVertex = removeVertex->incidentVertexes[nextIndex];
            secondVertex = removeVertex->incidentVertexes[secondIndex];
            Face* oldFace = nullptr;
            for (auto it = oldEdge->faceId.begin(); it != oldEdge->faceId.end(); it++) {
                if (VertexBelongToFace(secondVertex, faces[*it])) {
                    oldFace = faces[*it];
                    break;
                }
            }
            Edge* parentEdge = oldEdge->parent;
            Face* parentFace = getParentFace(parentEdge, oldFace);
            parentFace->deleteChild(oldFace);
            bool antiClockWish = judgeAntiClockWish(oldFace, secondVertex, nextVertex);
            //生成新的面
            Face* newFace = nullptr;
            if (antiClockWish) {
                newFace = generateNewFace(secondVertex, nextVertex, resultVertex);
            }
            else {
                newFace = generateNewFace(secondVertex, resultVertex, nextVertex);
            }
            //更改未删除边的面信息
            Edge* remainingEdge = nullptr;
            for (auto it = oldFace->edges.begin(); it != oldFace->edges.end(); it++) {
                if ((*it)->vertexe1->vertexId != removeVertexId && (*it)->vertexe2->vertexId != removeVertexId) {
                    remainingEdge = (*it);
                    break;
                }
            }
            auto iter1 = remainingEdge->faceId.find(oldFace->faceId);
            remainingEdge->faceId.erase(iter1);
            Edge* newEdge = nullptr;
            int firstVertexOfNewEdgeIndex, secondVertexOfNewEdgeIndex;
            if (nextVertex->vertexId > resultVertex->vertexId) {
                firstVertexOfNewEdgeIndex = resultVertex->vertexId;
                secondVertexOfNewEdgeIndex = nextVertex->vertexId;
            }
            else {
                secondVertexOfNewEdgeIndex = resultVertex->vertexId;
                firstVertexOfNewEdgeIndex = nextVertex->vertexId;
            }
            for (auto it = newFace->edges.begin(); it != newFace->edges.end(); it++) {
                if ((*it)->vertexe1->vertexId == firstVertexOfNewEdgeIndex && (*it)->vertexe2->vertexId == secondVertexOfNewEdgeIndex) {
                    newEdge = *it;
                    break;
                }
            }
            //更改非合并点的相邻点,删除removeVertex，添加resultVertex，修改相邻边
            for (auto it = nextVertex->incidentVertexes.begin(); it != nextVertex->incidentVertexes.end(); it++) {
                if ((*it)->vertexId == removeVertexId) {
                    it = nextVertex->incidentVertexes.erase(it);
                    nextVertex->incidentVertexes.insert(it, resultVertex);
                    break;
                }
            }

            for (auto it = nextVertex->incidentEdges.begin(); it != nextVertex->incidentEdges.end(); it++) {
                if ((*it)->edgeId == oldEdge->edgeId) {
                    it = nextVertex->incidentEdges.erase(it);
                    nextVertex->incidentEdges.insert(it, newEdge);//插入新的边
                    break;
                }
            }
            //result点也添加新的边
            resultVertex->incidentEdges.push_back(newEdge);
            //删除原来的面和边
            oldFace->deleted = true;
            std::pair<int, int> edgePair(oldEdge->vertexe1->vertexId, oldEdge->vertexe2->vertexId);
            m_hash_edge.erase(edgePair);
            for (auto it = edges.begin(); it != edges.end(); it++) {
                if ((*it)->edgeId == oldEdge->edgeId) {
                    delete(*it);
                    edges.erase(it);
                    break;
                }
            }
            newFaceCount--;
            nextIndex = secondIndex;
            secondIndex = (nextIndex + 1) % incidentCount;
        }
        //删除与resultVertex相邻的面和边
        //找到原来的面
        nextIndex = (removeVertex->eIndex + 1) % incidentCount;
        nextVertex = removeVertex->incidentVertexes[nextIndex];
        secondIndex = (removeVertex->eIndex + incidentCount - 1) % incidentCount;
        secondVertex = removeVertex->incidentVertexes[secondIndex];
        Face* oldFace = nullptr;
        for (auto it = contractEdge->faceId.begin(); it != contractEdge->faceId.end(); it++) {
            if (VertexBelongToFace(nextVertex, faces[*it])) {
                oldFace = faces[*it];
                break;
            }
        }
        Edge* parentEdge = contractEdge->parent;
        Face* parentFace = getParentFace(parentEdge, oldFace);
        parentFace->deleteChild(oldFace);
        //更改未删除边的面信息
        Edge* remainingEdge = nullptr;
        for (auto it = oldFace->edges.begin(); it != oldFace->edges.end(); it++) {
            if ((*it)->vertexe1->vertexId != removeVertexId && (*it)->vertexe2->vertexId != removeVertexId) {
                remainingEdge = (*it);
                break;
            }
        }
        auto iter1 = remainingEdge->faceId.find(oldFace->faceId);
        remainingEdge->faceId.erase(iter1);

        //更改相邻点以及相邻边
        for (auto it = resultVertex->incidentVertexes.begin(); it != resultVertex->incidentVertexes.end(); it++) {
            if ((*it)->vertexId == removeVertexId) {
                resultVertex->incidentVertexes.erase(it);
                break;
            }
        }
        for (auto it = resultVertex->incidentEdges.begin(); it != resultVertex->incidentEdges.end(); it++) {
            if ((*it)->edgeId == contractEdge->edgeId) {
                resultVertex->incidentEdges.erase(it);
                break;
            }
        }

        for (auto it = contractEdge->faceId.begin(); it != contractEdge->faceId.end(); it++) {
            if (VertexBelongToFace(secondVertex, faces[*it])) {
                oldFace = faces[*it];
                break;
            }
        }
        parentEdge = contractEdge->parent;
        parentFace = getParentFace(parentEdge, oldFace);
        parentFace->deleteChild(oldFace);
        for (auto it = oldFace->edges.begin(); it != oldFace->edges.end(); it++) {
            if ((*it)->vertexe1->vertexId != removeVertexId && (*it)->vertexe2->vertexId != removeVertexId) {
                remainingEdge = (*it);
                break;
            }
        }
        iter1 = remainingEdge->faceId.find(oldFace->faceId);
        remainingEdge->faceId.erase(iter1);
        //删除原来的边
        oldFace->deleted = true;
        std::pair<int, int> edgePair(contractEdge->vertexe1->vertexId, contractEdge->vertexe2->vertexId);
        m_hash_edge.erase(edgePair);
        for (auto it = edges.begin(); it != edges.end(); it++) {
            if ((*it)->edgeId == contractEdge->edgeId) {
                delete(*it);
                edges.erase(it);
                break;
            }
        }
        //删除点
        removeVertex->deleted = true;

        //更新result点的类型
        std::vector<Face*> incidentFaces;
        std::vector<float> subtendedAngles;
        isTypeI(resultVertex, incidentFaces, subtendedAngles);
        /*if (!isTypeI(resultVertex, incidentFaces, subtendedAngles)) {
            isTypeII(resultVertex, incidentFaces, subtendedAngles);
        }*/
        incidentFaces.clear();
        subtendedAngles.clear();

        for (auto it = resultVertex->incidentVertexes.begin(); it != resultVertex->incidentVertexes.end(); it++) {
            isTypeI(*it, incidentFaces, subtendedAngles);
            /*if (!isTypeI(*it, incidentFaces, subtendedAngles)) {
                isTypeII(*it, incidentFaces, subtendedAngles);
            }*/
            incidentFaces.clear();
            subtendedAngles.clear();
        }
    }


}