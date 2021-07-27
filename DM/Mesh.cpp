#include <stdio.h>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "time.h"//统计时间需添加的头文件
#include "Mesh.h"

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
float getAnotherPoint(glm::vec3& v3, glm::vec3& v1, glm::vec3& center) {
    float x31 = v3.x - v1.x;
    float x21 = center.x - v1.x;
    float y31 = v3.y - v1.y;
    float y21 = center.y - v1.y;
    float z31 = v3.z - v1.z;
    float z21 = center.z - v1.z;


    float t = 2 * (x31 * x21 + y31 * y21 + z31 * z21) / (x31 * x31 + y31 * y31 + z31 * z31);

    //return glm::vec3(v1.x + t * x31, v1.y + t * y31, v1.z + t * z31);
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
            if (max2 >= min2) {
                return std::pair<float, float>(min2, max2);   //三个满足
            }
            return std::pair<float, float>(tStart, min2);   //两个满足
        }
        if (max1 >= tEnd) {
            return std::pair<float, float>(tStart, tEnd);   //三个满足
        }
        if (max1 >= tStart) {
            return std::pair<float, float>(tStart, max1);   //三个满足
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
            return std::pair<float, float>(min1, tEnd);     //三个都满足
        }
        return std::pair<float, float>(tStart, tEnd);     //两个都满足
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
        return std::pair<float, float>(tStart, tEnd);     //两个满足
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

bool judgeAntiClockWish(Face* face, Vertex* v1, Vertex* v2) {
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
    if (o.x == edgeNormal.x) {//同向
        trans = glm::rotate(trans, angle, edgeNormal);          //旋转矩阵
    }
    else {
        trans = glm::rotate(trans, -angle, edgeNormal);
    }

    //把旋转轴移动到原点，再旋转，再移动回来
    glm::mat4 moveTo = glm::mat4(1.0f);
    moveTo = glm::translate(moveTo, -startPosition);

    glm::mat4 moveBack = glm::mat4(1.0f);
    moveBack = glm::translate(moveBack, startPosition);

    glm::vec4 point = glm::vec4(pointRotated, 1);
    return moveBack * trans * moveTo * point;
}

Mesh::Mesh() {
}

Mesh::~Mesh() {
}

bool Mesh::readSTL(const char *fileName) {
    //判断类型，调用不同的函数来生成
    FILE* fStl = nullptr;
    int err = fopen_s(&fStl, fileName, "r");
    if (fStl == nullptr) {
        return false;
    }
    if (err != 0) {
        return false;
    }

    char buf[6] = {};
    if (fread(buf, 5, 1, fStl) != 1) {
        fclose(fStl);
        return false;
    }
    fclose(fStl);

    if (strcmp(buf, "solid") == 0) { //是ASCII STL
        return readSTLASCII(fileName);
    } else {
        return readSTLBinary(fileName);
    }

    

}

bool Mesh::readSTLASCII(const char *fileName) {
    std::ifstream fileSTL(fileName, std::ios::in | std::ios::binary);
    char buf[255];
    char end[] = "endsolid";
    char begin[] = "facet";
    fileSTL >> buf;         //solid
    fileSTL >> partName;    //filenamestl 
    float v1, v2, v3;
    int faceId = 0;
    int vertexId = 0;

    while (fileSTL >> buf) {
        if (strcmp(buf, end) == 0) { //endsolid结束
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

//        finish = clock();
 //       duration = (double)(finish - start) / CLOCKS_PER_SEC;
//        printf("%f seconds of 2\n", duration);
//        printf("%d faceIndex\n", i);
        faces.push_back(face);
        faceId++;
    }
    
    return true;

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


    for (int i = 0; i < faceNum; i++) {
        glm::vec3 v1 = (*(faces[i]->vertexs.begin()))->position;
        float v1x = v1.x;
        float v1y = v1.y;
        float v1z = v1.z;

        glm::vec3 v2 = (*(++faces[i]->vertexs.begin()))->position;
        float v2x = v2.x;
        float v2y = v2.y;
        float v2z = v2.z;

        glm::vec3 v3 = (*(++(++faces[i]->vertexs.begin())))->position;
        float v3x = v3.x;
        float v3y = v3.y;
        float v3z = v3.z;


        float nx = (v1y - v3y) * (v2z - v3z) - (v1z - v3z) * (v2y - v3y);
        float ny = (v1z - v3z) * (v2x - v3x) - (v2z - v3z) * (v1x - v3x);
        float nz = (v1x - v3x) * (v2y - v3y) - (v2x - v3x) * (v1y - v3y);

        float nxyz = sqrt(nx * nx + ny * ny + nz * nz);

        fprintf(fp, "facet normal %f %f %f\n", nx / nxyz, ny / nxyz, nz / nxyz);
        fprintf(fp, "outer loop\n");
        fprintf(fp, "vertex %f %f %f\n", v1x, v1y, v1z);
        fprintf(fp, "vertex %f %f %f\n", v2x, v2y, v2z);
        fprintf(fp, "vertex %f %f %f\n", v3x, v3y, v3z);
        fprintf(fp, "endloop\n");
        fprintf(fp, "endfacet\n");

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
    for (int i = 0; i < faceNum; i++) {
        for (auto it = faces[i]->children.begin(); it != faces[i]->children.end(); it++) {
            count++;
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

    for (int i = 0; i < faceNum; i++) {
        for (auto it = faces[i]->children.begin(); it != faces[i]->children.end(); it++) {
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
        edge->length = sqrt(pow((edge->vertexe1->position.x - edge->vertexe2->position.x), 2)
            + pow((edge->vertexe1->position.y - edge->vertexe2->position.y), 2) + pow((edge->vertexe1->position.z - edge->vertexe2->position.z), 2));
        if (edge->length > lMax) {
            lMax = edge->length;
        }
        if (edge->length < lMin) {
            lMin = edge->length;
        }
        edge->edgeId = edges.size();
        edge->parent = edge;
        std::pair<int, int> pair1(pFirst->vertexId, pSecond->vertexId);
        m_hash_edge[pair1] = edge;
        edges.push_back(edge);
    }
    return edge;
}

void Mesh::generateEdgeOfFace(Face* face, bool meshEdge){
    clock_t start, finish;
    double duration;
    start = clock();
   
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
    double cosTheta0 = glm::dot((face->vertexs[1]->position - face->vertexs[0]->position), (face->vertexs[2]->position - face->vertexs[0]->position)) / length01 / length02;
    double sinTheta0 = sqrt(1 - cosTheta0 * cosTheta0);
    if (sinTheta0 < sinThetaMin) {
        sinThetaMin = sinTheta0;
    }
    face->angles.push_back(cosTheta0);
    double cosTheta1 = glm::dot((face->vertexs[0]->position - face->vertexs[1]->position), (face->vertexs[2]->position - face->vertexs[1]->position)) / length01 / length12;
    double sinTheta1 = sqrt(1 - cosTheta1 * cosTheta1);
    if (sinTheta1 < sinThetaMin) {
        sinThetaMin = sinTheta1;
    }
    face->angles.push_back(cosTheta1);
    double cosTheta2 = glm::dot((face->vertexs[0]->position - face->vertexs[2]->position), (face->vertexs[1]->position - face->vertexs[2]->position)) / length12 / length02;
    double sinTheta2 = sqrt(1 - cosTheta2 * cosTheta2);
    if (sinTheta2 < sinThetaMin) {
        sinThetaMin = sinTheta2;
    }
    if (sinThetaMin < 0.0001) {
        printf("small!!!");
    }
    face->angles.push_back(cosTheta2);
    
}

void Mesh::computeParameter(){
    double v1 = lMin * sinThetaMin / (0.5 + sinThetaMin);
    double v2 = lMin / 2;
    if (v1 < v2) {
        rhoV = v1;
    }
    else {
        rhoV = v2;
    }
    rhoE = 2 * rhoV * sinThetaMin;
}

void Mesh::generateDM(){
    computeParameter();
    for (auto it = edges.begin(); it != edges.end(); it++) {
        (*it)->constructCe(rhoV, rhoE);
    } 
    findAllNLDEdges();
   while (!NLDEdges.empty()) {
        Edge* currentEdge = NLDEdges.top();
        NLDEdges.pop();
        if (currentEdge->flippable == false) {
            handleNonFlippableNLDEdge(currentEdge);
        }
        currentEdge->inStack = false;


    }
/*
    Edge* currentEdge = NLDEdges.top();
    NLDEdges.pop();
    if (currentEdge->flippable == false) {
        handleNonFlippableNLDEdge(currentEdge);
    }
    currentEdge->inStack = false;
*/}


/*
* 先把这条边所属的两个三角面以及该三角面的另外四条边对应的三角面（总共6个面）旋转到同一个平面上,再求I0-I4,找到公共部分
* 分割，添加四条边，四个面
* 遍历这四个面的所有边，翻NLD，把不能翻转的加入栈中
*/

void Mesh::handleNonFlippableNLDEdge(Edge* edge) {//edge就是edgeAB
    Vertex* vertexA = edge->vertexe1;
    Vertex* vertexB = edge->vertexe2;
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
    if (o.x != ve.x) {//同向
        angle = -angle;
    }
    glm::mat4 trans = glm::mat4(1.0f);              //创建单位矩阵
    trans = glm::rotate(trans, angle, ve);          //旋转矩阵

    //把旋转轴移动到原点，再旋转，再移动回来
    glm::mat4 moveTo = glm::mat4(1.0f);
    moveTo = glm::translate(moveTo, -vertexB->position);

    glm::mat4 moveBack = glm::mat4(1.0f);
    moveBack = glm::translate(moveBack, vertexB->position);

    //旋转对角
    glm::vec4 newVertexCPosition = moveBack * trans * moveTo * vertexCPosition;
    glm::vec3 newVertexCPos = glm::vec3(newVertexCPosition);
    Edge* parent = edge->parent;
    Vertex* pStart = parent->vertexe1;
    Vertex* pEnd = parent->vertexe2;

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
    if (edgeAC->faceId.size() == 2) {
        glm::vec4 vertexEPosition = glm::vec4(getAnotherVertexPositionByEdge(faceABC, edgeAC), 1);      
        glm::vec4 newVertexEPosition = moveBack * trans * moveTo * vertexEPosition;
        
        //再旋转另外两个三角面到达在同一平面
        glm::vec3 newVertexEPos = glm::vec3(newVertexEPosition);
        glm::vec3 normalACE = calNormal(vertexA->position, newVertexCPos, newVertexEPos);
        glm::vec3 newAC = glm::normalize(newVertexCPos - vertexA->position); //该边的向量
        newVertexEPos = rotatePoint(normalABD, normalACE, newAC, vertexA->position, newVertexEPos);

        center = solveCenterPointOfCircle(vertexA->position, newVertexEPos, newVertexCPos);
        tE = getAnotherPoint(vertexB->position, vertexA->position, center);
    }
    else {
        tE = 0;
    }

    float tF = 1;
    if (edgeBC->faceId.size() == 2) {
        glm::vec4 vertexFPosition = glm::vec4(getAnotherVertexPositionByEdge(faceABC, edgeBC), 1);
        glm::vec4 newVertexFPosition = moveBack * trans * moveTo * vertexFPosition;

        //再旋转另外两个三角面到达在同一平面
        glm::vec3 newVertexFPos = glm::vec3(newVertexFPosition);
        glm::vec3 normalBCF = calNormal(vertexB->position, newVertexCPos, newVertexFPos);
        glm::vec3 newBC = glm::normalize(newVertexCPos - vertexB->position); //该边的向量
        newVertexFPos = rotatePoint(normalABD, normalBCF, newBC, vertexB->position, newVertexFPos);

        center = solveCenterPointOfCircle(vertexB->position, newVertexCPos, newVertexFPos);
        tF = 1 - getAnotherPoint(vertexA->position, vertexB->position, center);

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
    if (edgeAD->faceId.size() == 2) {
        glm::vec4 vertexHPosition = glm::vec4(getAnotherVertexPositionByEdge(faceABD, edgeAD), 1);
        glm::vec3 vertexHPos = glm::vec3(vertexHPosition);

        glm::vec3 normalADH = calNormal(vertexA->position, vertexD->position, vertexHPos);
        glm::vec3 newAD = glm::normalize(vertexD->position - vertexA->position); //该边的向量
        vertexHPos = rotatePoint(normalABD, normalADH, newAD, vertexA->position, vertexHPos);
        

        center = solveCenterPointOfCircle(vertexA->position, vertexD->position, vertexHPos);
        tH = getAnotherPoint(vertexB->position, vertexA->position, center);
    }
    else {
        tH = 0;
    }

    float tG = 1;
    if (edgeBD->faceId.size() == 2) {
        glm::vec4 vertexGPosition = glm::vec4(getAnotherVertexPositionByEdge(faceABD, edgeBD), 1);

        glm::vec3 vertexGPos = glm::vec3(vertexGPosition);
        glm::vec3 normalBDG = calNormal(vertexB->position, vertexGPos, vertexD->position);
        glm::vec3 newBD = glm::normalize(vertexD->position - vertexB->position); //该边的向量
        vertexGPos = rotatePoint(normalABD, normalBDG, newBD, vertexB->position, vertexGPos);

        center = solveCenterPointOfCircle(vertexB->position, vertexD->position, vertexGPos);
        tG = 1 - getAnotherPoint(vertexA->position, vertexB->position, center);
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

    Face* parentFaceOfABC = getParentFace(parentEdge, faceABC);
    Face* parentFaceOfABD = getParentFace(parentEdge, faceABD);

    //分割：添加点
    Vertex* vertexS = generateNewVertex(splitPosition);

    glm::vec3 t1 = glm::normalize(parentEdge->vertexe1->position - splitPosition);
    glm::vec3 t2 = glm::normalize(splitPosition - parentEdge->vertexe2->position);

    bool antiClockWish = judgeAntiClockWish(faceABC, vertexA, vertexB);
    Face* faceACS = nullptr;
    Face* faceBCS = nullptr;
    Face* faceADS = nullptr;
    Face* faceBDS = nullptr;

    //添加四个面
    if (antiClockWish) {    //A、B是逆时针
        faceACS = generateNewFace(faceABC->normal, vertexA, vertexS, vertexC);
        faceBCS = generateNewFace(faceABC->normal, vertexS, vertexB, vertexC);
        faceADS = generateNewFace(faceABD->normal, vertexS, vertexA, vertexD);
        faceBDS = generateNewFace(faceABD->normal, vertexB, vertexS, vertexD);
    }
    else {
        faceACS = generateNewFace(faceABC->normal, vertexS, vertexA, vertexC);
        faceBCS = generateNewFace(faceABC->normal, vertexB, vertexS, vertexC);
        faceADS = generateNewFace(faceABD->normal, vertexA, vertexS, vertexD);
        faceBDS = generateNewFace(faceABD->normal, vertexS, vertexB, vertexD);
    }
    

    parentFaceOfABC->deleteChild(faceABC);
/*    for (auto faceIt = faces.rbegin(); faceIt != faces.rend(); faceIt++) {
        if ((*faceIt)->faceId == faceABC->faceId) {
            faces.erase((++faceIt).base());
            break;
        }
    }
*/    parentFaceOfABD->deleteChild(faceABD);
/*    for (auto faceIt = faces.rbegin(); faceIt != faces.rend(); faceIt++) {
        if ((*faceIt)->faceId == faceABD->faceId) {
            faces.erase((++faceIt).base());
            break;
        }
    }
*/
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

    for (auto it = faceACS->edges.begin(); it != faceACS->edges.begin(); it++) {
        if (((*it)->vertexe1->vertexId == vertexA->vertexId && (*it)->vertexe2->vertexId == vertexS->vertexId) ||
            ((*it)->vertexe1->vertexId == vertexS->vertexId && (*it)->vertexe2->vertexId == vertexA->vertexId)) {
            (*it)->parent = parentEdge;
            parentFaceOfABC->borders.push_back((*it));
            parentFaceOfABD->borders.push_back((*it));
            break;
        }
    }
    for (auto it = faceBCS->edges.begin(); it != faceBCS->edges.begin(); it++) {
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

bool Mesh::isNLD(Edge* edge){
    edge->flippable = false;
    if (edge->faceId.size() != 2) {
        printf("edge with face not equal to 2: %d\n", edge->edgeId);
        return false;
    }
    else {
        Face* face1 = faces[*(edge->faceId.begin())];
        Face* face2 = faces[*(++edge->faceId.begin())];

        Vertex* top1;
        Vertex* top2;
        double cos1 = 0.0;
        double cos2 = 0.0;
        for (int i = 0; i < 3; i++) {
            if (face1->vertexs[i]->vertexId != edge->vertexe1->vertexId && face1->vertexs[i]->vertexId != edge->vertexe2->vertexId) {
                top1 = face1->vertexs[i];
                cos1 = face1->angles[i];
                break;
            }
        }
        for (int i = 0; i < 3; i++) {
            if (face2->vertexs[i]->vertexId != edge->vertexe1->vertexId && face2->vertexs[i]->vertexId != edge->vertexe2->vertexId) {
                top2 = face2->vertexs[i];
                cos2 = face2->angles[i];
                break;
            }
        }
        //求角度之和，用sin(1+2) = sin(1)*cos(1) + cos(1)*sin(1)
        double sin1 = sqrt(1 - cos1 * cos1);
        double sin2 = sqrt(1 - cos2 * cos2);

        if ((sin1 * cos1 + cos1 * sin1) < 0) {
            glm::vec3 normal1 = glm::normalize(face1->normal);
            glm::vec3 normal2 = glm::normalize(face2->normal);

            if ((normal1 == normal2) || (normal1 + normal2 == glm::vec3(0, 0, 0))) {
                edge->flippable = true;
                
            }
            return true;
        }
        return false;

    }
}

void Mesh::findAllNLDEdges(){
    for (auto it = edges.begin(); it != edges.end(); it++) {
        if ((*it)->faceId.size() != 2) {
            continue;
        }
        else if ((*it)->inStack == false && isNLD(*it)) {
            (*it)->inStack = true;
            NLDEdges.push(*it);
        }
    }
}

void Mesh::flipAllNLDEdgeInFace(Face* face){
    std::list<int> visitedEdge;
    for (auto childIt = face->children.begin(); childIt != face->children.end(); childIt++) {
        for (auto edgeIt = (*childIt)->edges.begin(); edgeIt != (*childIt)->edges.end(); edgeIt++) {
            bool visited = false;
            auto it = visitedEdge.begin();
            while(it != visitedEdge.end()) {
                if ((*it) < face->faceId) {
                    it++;
                }
                else if ((*it) == face->faceId) {   //该边已经访问过
                    visited = true;
                    break;
                }
                else {
                    break;
                }
            }
            if (!visited) {
                visitedEdge.insert(it, face->faceId);
                if (isNLD(*edgeIt)) {
                    if ((*edgeIt)->flippable == true) {
                        flipEdge((*edgeIt));
                    }
                }
            }
        }
    }
}

void Mesh::flipEdge(Edge* edgeAB){
    printf("flip %d", edgeAB->edgeId);
    Vertex* vertexA = edgeAB->vertexe1;
    Vertex* vertexB = edgeAB->vertexe2;
    Face* faceABC = faces[*(edgeAB->faceId.begin())];
    Face* faceABD = faces[*(++edgeAB->faceId.begin())];

    Vertex* vertexC = getThirdVertexInFace(faceABC, vertexA->vertexId,vertexB->vertexId);
    Vertex* vertexD = getThirdVertexInFace(faceABD, vertexA->vertexId, vertexB->vertexId);
    
    Edge* edgeAC = nullptr,* edgeBC = nullptr;
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
    Edge* edgeAD = nullptr,* edgeBD = nullptr;
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

    Face* faceACD = nullptr;
    faceACD = generateNewFace(faceABC->normal, vertexA, vertexD, vertexC);
    Face* faceBCD = nullptr;
    faceBCD = generateNewFace(faceABC->normal, vertexC, vertexD, vertexB);

    Edge* parentEdge = edgeAB->parent;

    Face* parentFaceOfABC = getParentFace(parentEdge, faceABC);
    Face* parentFaceOfABD = getParentFace(parentEdge, faceABD);

    if (parentFaceOfABC->faceId == parentFaceOfABD->faceId) {
        parentFaceOfABC->deleteChild(faceABC);
        parentFaceOfABC->deleteChild(faceABD);
        parentFaceOfABC->children.push_back(faceACD);
        parentFaceOfABC->children.push_back(faceBCD);
    }
    else {
//        parentFaceOfABC
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
    for (auto it = edges.begin(); it != edges.end(); it++) {
        if ((*it)->edgeId == edgeAB->edgeId) {
            delete(*it);
            edges.erase(it);
            break;
        }
    }
/*    //删除面
    for (auto it = faces.begin(); it != faces.end();){
        if ((*it)->faceId == faceABD->faceId || (*it)->faceId == faceABC->faceId){
            delete(*it);
            it = faces.erase(it);  //在这里获取下一个元素
        }
        else
           ++it;
    }
*/
}

Face* Mesh::getParentFace(Edge* edge, Face* childFace){
    Face* face1 = faces[*(edge->meshFaceId.begin())];
    for (auto it = face1->children.begin(); it != face1->children.end(); it++) {
        if ((*it)->faceId == childFace->faceId) {
            return face1;
        }
    }
    return faces[*(++edge->meshFaceId.begin())];
}

Vertex* Mesh::generateNewVertex(glm::vec3& position){
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


Face* Mesh::generateNewFace(glm::vec3& normal, Vertex* v1, Vertex* v2, Vertex* v3){
    Face* face = new Face();

    face->setNormal(normal);
    std::vector<Vertex*> vs;
    vs.push_back(v1);
    vs.push_back(v2);
    vs.push_back(v3);

    face->setId(faces.size());
    face->setVertex(vs);
    face->isMesh = false;
    faces.push_back(face);

    generateEdgeOfFace(face, false);

    return face;
}

void Mesh::addNewNonNLDEdge(Face* face){
    for (auto it = face->borders.begin(); it != face->borders.end(); it++) {
        if ((*it)->inStack == false && isNLD(*it)) {
            (*it)->inStack = true;
            NLDEdges.push(*it);
            
        }
    }
}

//找到edge所属的除face的另一个面的对角E
glm::vec3 Mesh::getAnotherVertexPositionByEdge(Face* face, Edge* edge){
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



