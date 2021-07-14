#include <stdio.h>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Mesh.h"


//边的比较函数，先比较第一个点（下标最小的点），如果相同再比较第二个点
bool compare(const Edge& a, const  Edge& b){        
    if (a.vertexe1.vertexId != b.vertexe1.vertexId) {
        return a.vertexe1.vertexId < b.vertexe1.vertexId;
    }
    return a.vertexe2.vertexId < b.vertexe2.vertexId;
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
            Face face;
            fileSTL >> buf;     // normal 

            fileSTL >> v1;
            fileSTL >> v2;
            fileSTL >> v3;
            face.setNormal(v1, v2, v3);
            std::vector<Vertex> vs;

            fileSTL >> buf;     //outer 
            fileSTL >> buf;     //loop

            for (unsigned j = 0; j < 3; j++) {
                fileSTL >> buf; //vertex 

                fileSTL >> v1;
                fileSTL >> v2;
                fileSTL >> v3;
                glm::vec3 p = glm::vec3(v1, v2, v3);

                int id = findVertexByPoint(p);
                if (id == -1) {			//没找到，该point是新的
                    Vertex vertex;
                    vertex.init(p);
                    vertex.setId(vertexId);
                    m_hash_point[p] = vertexId;
                    vertexId++;
                    vertexes.push_back(vertex);
                    vs.push_back(vertex);
                }
                else {
                    vs.push_back(vertexes[id]);
                }
            }
            fileSTL >> buf;     //endloop
            fileSTL >> buf;     //endfacet

            face.setId(faceId);
            face.setVertex(vs);
            generateEdge(face);


            faces.push_back(face);
            faceId++;
            faceNum++;
        }
        
    }
    
    computeParameter();
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
    for (int i = 0; i < faceNum; i++) {
        Face face;
        fread(&v1, 4, 1, fStl);//读取法线数据
        fread(&v2, 4, 1, fStl);
        fread(&v3, 4, 1, fStl);
        face.setNormal(v1, v2, v3);
        std::vector<Vertex> vs;
        for (int j = 0; j < 3; j++) {
            fread(&v1, 4, 1, fStl);//读取顶点的数据
            fread(&v2, 4, 1, fStl);
            fread(&v3, 4, 1, fStl);
            glm::vec3 p = glm::vec3((float)v1, (float)v2, (float)v3);

            int id = findVertexByPoint(p);
            if (id == -1) {			//没找到，该point是新的
                Vertex vertex;
                vertex.init(p);
                vertex.setId(vertexId);
                m_hash_point[p] = vertexId;
                vertexId++;
                vertexes.push_back(vertex);
                vs.push_back(vertex);
            }else {
                vs.push_back(vertexes[id]);
            }
        }

        fread(face.buf, 2, 1, fStl);//读取保留项数据，这一项一般没什么用，这里选择读取是为了移动文件指针
        face.setId(faceId);
        face.setVertex(vs);
        generateEdge(face);


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
        glm::vec3 v1 = (faces[i].vertexs.begin())->position;
        float v1x = v1.x;
        float v1y = v1.y;
        float v1z = v1.z;

        glm::vec3 v2 = (++faces[i].vertexs.begin())->position;
        float v2x = v2.x;
        float v2y = v2.y;
        float v2z = v2.z;

        glm::vec3 v3 = (++(++faces[i].vertexs.begin()))->position;
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
    fwrite(&faceNum, sizeof(int), 1, fp);

    for (int i = 0; i < faceNum; i++) {
        data[0] = faces[i].normal.x;
        data[1] = faces[i].normal.y;
        data[2] = faces[i].normal.z;

        data[3] = faces[i].vertexs[0].position.x;
        data[4] = faces[i].vertexs[0].position.y;
        data[5] = faces[i].vertexs[0].position.z;
        data[6] = faces[i].vertexs[1].position.x;
        data[7] = faces[i].vertexs[1].position.y;
        data[8] = faces[i].vertexs[1].position.z;
        data[9] = faces[i].vertexs[2].position.x;
        data[10] = faces[i].vertexs[2].position.y;
        data[11] = faces[i].vertexs[2].position.z;

        fwrite(data, sizeof(float), 12, fp);
        fwrite(faces[i].buf, sizeof(char), 2, fp);

    }

    fclose(fp);

    delete[]data;
    return true;

}


int Mesh::findVertexByPoint(glm::vec3 p) {
    auto it = m_hash_point.find(p);
    if (it != m_hash_point.end()) {
        return it->second;
    }
    else {
        return -1;
    }
}

void Mesh::generateEdge(Face& face){
    Edge edge1;
    Edge edge2;
    Edge edge3;
    double length01;
    double length12;
    double length02;
    //std::sort(edges.begin(), edges.end(), compare);
    if (face.vertexs[0].vertexId < face.vertexs[1].vertexId) {
        edge1.vertexe1 = face.vertexs[0];
        edge1.vertexe2 = face.vertexs[1];
    }
    else {
        edge1.vertexe1 = face.vertexs[1];
        edge1.vertexe2 = face.vertexs[0];
    }
    auto it = edges.begin();
    for (; it != edges.end(); it++) {   //第一个大于或等于查找值的迭代器
        if (!compare(*it, edge1)) {
            break;
        }
    }
    if (!(it == edges.end()) && ((*it).vertexe1.vertexId == edge1.vertexe1.vertexId)
        && ((*it).vertexe2.vertexId == edge1.vertexe2.vertexId)) {     //已存在的edge
        length01 = (*it).length;
        (*it).faceId.insert(face.faceId);
    }
    else {
        
        length01 = sqrt(pow((edge1.vertexe1.position.x - edge1.vertexe2.position.x), 2)
            + pow((edge1.vertexe1.position.y - edge1.vertexe2.position.y), 2) + pow((edge1.vertexe1.position.z - edge1.vertexe2.position.z), 2));
        if (length01 > lMax) {
            lMax = length01;
        }
        if (length01 < lMin) {
            lMin = length01;
        }
        edge1.length = length01;
        edge1.edgeId = edges.size();
        edge1.faceId.insert(face.faceId);
        edge1.parent = &edge1;
        edges.insert(it, edge1);
    }
    face.edges.push_back(edge1);
    

    if (face.vertexs[1].vertexId < face.vertexs[2].vertexId) {
        edge2.vertexe1 = face.vertexs[1];
        edge2.vertexe2 = face.vertexs[2];
    }
    else {
        edge2.vertexe1 = face.vertexs[2];
        edge2.vertexe2 = face.vertexs[1];
    }
    it = edges.begin(); //第一个大于或等于查找值的迭代器
    for (; it != edges.end(); it++) {   //第一个大于或等于查找值的迭代器
        if (!compare(*it, edge2)) {
            break;
        }
    }
    if (!(it == edges.end()) && ((*it).vertexe1.vertexId == edge2.vertexe1.vertexId)
        && ((*it).vertexe2.vertexId == edge2.vertexe2.vertexId)) {     //已存在的edge
        length12 = (*it).length;
        (*it).faceId.insert(face.faceId);
    }
    else {
        length12 = sqrt(pow((edge2.vertexe1.position.x - edge2.vertexe2.position.x), 2)
            + pow((edge2.vertexe1.position.y - edge2.vertexe2.position.y), 2) + pow((edge2.vertexe1.position.z - edge2.vertexe2.position.z), 2));
        if (length12 > lMax) {
            lMax = length12;
        }
        if (length12 < lMin) {
            lMin = length12;
        }
        edge2.length = length12;
        edge2.edgeId = edges.size();
        edge2.faceId.insert(face.faceId);
        edge2.parent = &edge2;
        edges.insert(it, edge2);

    }
    face.edges.push_back(edge2);
    

    if (face.vertexs[0].vertexId < face.vertexs[2].vertexId) {
        edge3.vertexe1 = face.vertexs[0];
        edge3.vertexe2 = face.vertexs[2];
    }
    else {
        edge3.vertexe1 = face.vertexs[2];
        edge3.vertexe2 = face.vertexs[0];
    }
    it = edges.begin(); //第一个大于或等于查找值的迭代器
    for (; it != edges.end(); it++) {   //第一个大于或等于查找值的迭代器
        if (!compare(*it, edge3)) {
            break;
        }
    }
    if (!(it == edges.end()) && ((*it).vertexe1.vertexId == edge3.vertexe1.vertexId)
        && ((*it).vertexe2.vertexId == edge3.vertexe2.vertexId)) {     //已存在的edge
        length02 = (*it).length;
        (*it).faceId.insert(face.faceId);
    }
    else {
        length02 = sqrt(pow((edge3.vertexe1.position.x - edge3.vertexe2.position.x), 2)
            + pow((edge3.vertexe1.position.y - edge3.vertexe2.position.y), 2) + pow((edge3.vertexe1.position.z - edge3.vertexe2.position.z), 2));
        if (length02 > lMax) {
            lMax = length02;
        }
        if (length02 < lMin) {
            lMin = length02;
        }
        edge3.length = length02;
        edge3.edgeId = edges.size();
        edge3.faceId.insert(face.faceId);
        edge3.parent = &edge3;
        edges.insert(it, edge3);

    }
    face.edges.push_back(edge2);
    
    
    //求角度
    double cosTheta0 = glm::dot((face.vertexs[1].position - face.vertexs[0].position), (face.vertexs[2].position - face.vertexs[0].position)) / length01 / length02;
    double sinTheta0 = sqrt(1 - cosTheta0 * cosTheta0);
    if (sinTheta0 < sinThetaMin) {
        sinThetaMin = sinTheta0;
    }
    face.angles.push_back(cosTheta0);
    double cosTheta1 = glm::dot((face.vertexs[0].position - face.vertexs[1].position), (face.vertexs[2].position - face.vertexs[1].position)) / length01 / length12;
    double sinTheta1 = sqrt(1 - cosTheta1 * cosTheta1);
    if (sinTheta1 < sinThetaMin) {
        sinThetaMin = sinTheta1;
    }
    face.angles.push_back(cosTheta1);
    double cosTheta2 = glm::dot((face.vertexs[0].position - face.vertexs[2].position), (face.vertexs[1].position - face.vertexs[2].position)) / length12 / length02;
    double sinTheta2 = sqrt(1 - cosTheta2 * cosTheta2);
    if (sinTheta2 < sinThetaMin) {
        sinThetaMin = sinTheta2;
    }
    face.angles.push_back(cosTheta2);
    if (sinThetaMin < 0.001) {
        //printf("11");
    }
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
/*   for (auto it = edges.begin(); it != edges.end(); it++) {
        (*it).constructCe(rhoV, rhoE);
    }*/ 
    findAllNLDEdges();
    while (!NLDEdges.empty()) {
        Edge currentEdge = NLDEdges.top();
        NLDEdges.pop();
        while (currentEdge.flippable == true && !NLDEdges.empty()) {
            currentEdge = NLDEdges.top();
            NLDEdges.pop();
        }
        handleNonFlippableNLDEdge(currentEdge);

    }
}

/*
*先把这条边所属的两个三角面以及该三角面的另外四条边对应的三角面（总共6个面）旋转到同一个平面上,再求I0-I4,找到公共部分
*
*/
void Mesh::handleNonFlippableNLDEdge(Edge& edge){   
    Vertex vertexA = edge.vertexe1;                 
    Vertex vertexB = edge.vertexe2;
    Face faceABD = faces[*(edge.faceId.begin())];
    Face faceABC = faces[*(++edge.faceId.begin())];
    
    //以faceABD为基准平面，找到faceABC顶点旋转后的位置v',以此来构建外接圆与Is
    Vertex vertexC;
    for (auto it = faceABC.vertexs.begin(); it != faceABC.vertexs.end(); it++) {        
        if ((*it).vertexId != vertexA.vertexId && (*it).vertexId != vertexB.vertexId) {
            vertexC = *it;
            break;
        }
    }
    Vertex vertexD;
    for (auto it = faceABD.vertexs.begin(); it != faceABD.vertexs.end(); it++) {
        if ((*it).vertexId != vertexA.vertexId && (*it).vertexId != vertexB.vertexId) {
            vertexD = *it;
            break;
        }
    }
    glm::vec3 normalABD = faceABD.normal;
    glm::vec3 normalABC = faceABC.normal;
    glm::vec4 vertexCPosition = glm::vec4(vertexC.position, 1);

    Edge edgeAC, edgeBC;
    for (auto it = faceABC.edges.begin(); it != faceABC.edges.end(); it++) {
        if ((*it).edgeId != edge.edgeId) {
            if ((*it).vertexe1.vertexId == vertexA.vertexId || (*it).vertexe2.vertexId == vertexA.vertexId) {
                edgeAC = (*it);
            }
            else {
                edgeBC = (*it);
            }
        }
    }
    glm::vec4 vertexEPosition = glm::vec4(getAnotherVertexPositionByEdge(faceABC, edgeAC), 1);
    glm::vec4 vertexFPosition = glm::vec4(getAnotherVertexPositionByEdge(faceABC, edgeBC), 1);
 /*   //求另两个三角面
    Face faceACE, faceBCF;
    if (*(edgeAC.faceId.begin()) != faceABC.faceId) {
        faceACE = faces[*(edgeAC.faceId.begin())];
    }
    else {
        faceACE = faces[*(++edgeAC.faceId.begin())];
    }
    if (*(edgeBC.faceId.begin()) != faceABC.faceId) {
        faceBCF = faces[*(edgeBC.faceId.begin())];
    }
    else {
        faceBCF = faces[*(++edgeBC.faceId.begin())];
    }
    //求另两个三角面的顶点
    Vertex vertexE, vertexF;
    for (auto it = faceACE.vertexs.begin(); it != faceACE.vertexs.end(); it++) {
        if ((*it).vertexId != vertexA.vertexId && (*it).vertexId != vertexC.vertexId) {
            vertexE = *it;
            break;
        }
    }
    for (auto it = faceBCF.vertexs.begin(); it != faceBCF.vertexs.end(); it++) {
        if ((*it).vertexId != vertexB.vertexId && (*it).vertexId != vertexC.vertexId) {
            vertexF = *it;
            break;
        }
    }
    glm::vec4 vertexEPosition = glm::vec4(vertexE.position, 1);
    glm::vec4 vertexFPosition = glm::vec4(vertexF.position, 1);
*/
    float cosAngle = glm::dot(normalABD, normalABC);   //都是单位向量
    float angle = acos(cosAngle);                   //旋转的弧度
    glm::vec3 ve = glm::normalize(vertexA.position - vertexB.position); //该边的向量
    glm::mat4 trans = glm::mat4(1.0f);              //创建单位矩阵
    trans = glm::rotate(trans,angle, ve);          //旋转矩阵

    //把旋转轴移动到原点，再旋转，再移动回来
    glm::mat4 moveTo = glm::mat4(1.0f);
    moveTo = glm::translate(moveTo, -vertexB.position);

    glm::mat4 moveBack = glm::mat4(1.0f);
    moveBack = glm::translate(moveBack, vertexB.position);

    //不只是旋转对角，还要把相关的另两个三角面的对角也旋转了
    glm::vec4 newVertexCPosition = moveBack * trans * moveTo* vertexCPosition;
    glm::vec4 newVertexEPosition = moveBack * trans * moveTo * vertexEPosition;
    glm::vec4 newVertexFPosition = moveBack * trans * moveTo * vertexFPosition;

    //再旋转另外两个三角面到达在同一平面
    glm::vec3 newVertexEPos = glm::vec3(newVertexEPosition);
    glm::vec3 newVertexCPos = glm::vec3(newVertexCPosition);
    glm::vec3 normalACE = calNormal(vertexA.position, newVertexCPos, newVertexEPos);
    float cosAngleE = glm::dot(normalABD, normalACE);   //都是单位向量
    float angleE = acos(cosAngleE);                   //旋转的弧度
    glm::vec3 newAC = glm::normalize(newVertexCPos - vertexA.position); //该边的向量
    trans = glm::mat4(1.0f);              //创建单位矩阵
    trans = glm::rotate(trans, angleE, newAC);          //旋转矩阵
    moveTo = glm::mat4(1.0f);
    moveTo = glm::translate(moveTo, -vertexA.position);
    moveBack = glm::mat4(1.0f);
    moveBack = glm::translate(moveBack, vertexA.position);
    newVertexEPosition = moveBack * trans * moveTo * newVertexEPosition;

    glm::vec3 newVertexFPos = glm::vec3(newVertexFPosition);
    glm::vec3 normalBCF = calNormal(vertexB.position, newVertexCPos, newVertexEPos);
    float cosAngleF = glm::dot(normalABD, normalBCF);   //都是单位向量
    float angleF = acos(cosAngleF);                   //旋转的弧度
    glm::vec3 newBC = glm::normalize(newVertexCPos - vertexB.position); //该边的向量
    trans = glm::mat4(1.0f);              //创建单位矩阵
    trans = glm::rotate(trans, -angleF, newBC);          //旋转矩阵
    moveTo = glm::mat4(1.0f);
    moveTo = glm::translate(moveTo, -vertexB.position);
    moveBack = glm::mat4(1.0f);
    moveBack = glm::translate(moveBack, vertexB.position);
    newVertexFPosition = moveBack * trans * moveTo * newVertexFPosition;


    //再旋转与ABD相邻的两个三角面到达在同一平面
    
    Edge edgeAD, edgeBD;
    for (auto it = faceABD.edges.begin(); it != faceABD.edges.end(); it++) {
        if ((*it).edgeId != edge.edgeId) {
            if ((*it).vertexe1.vertexId == vertexA.vertexId || (*it).vertexe2.vertexId == vertexA.vertexId) {
                edgeAD = (*it);
            }
            else {
                edgeBD = (*it);
            }
        }
    }
    //求另两个三角面
    glm::vec4 vertexHPosition = glm::vec4(getAnotherVertexPositionByEdge(faceABD, edgeAD), 1);
    glm::vec4 vertexGPosition = glm::vec4(getAnotherVertexPositionByEdge(faceABD, edgeBD), 1);
    glm::vec3 vertexHPos = glm::vec3(vertexHPosition);

    glm::vec3 normalADH = calNormal(vertexA.position, vertexD.position, vertexHPos);
    float cosAngleH = glm::dot(normalABD, normalADH);   //都是单位向量
    float angleH = acos(cosAngleH);                   //旋转的弧度
    glm::vec3 newAD = glm::normalize(vertexD.position - vertexA.position); //该边的向量
    trans = glm::mat4(1.0f);              //创建单位矩阵
    trans = glm::rotate(trans, -angleH, newAD);          //旋转矩阵
    moveTo = glm::mat4(1.0f);
    moveTo = glm::translate(moveTo, -vertexA.position);
    moveBack = glm::mat4(1.0f);
    moveBack = glm::translate(moveBack, vertexA.position);
    vertexHPosition = moveBack * trans * moveTo * vertexHPosition;

    glm::vec3 newVertexGPos = glm::vec3(vertexGPosition);
    glm::vec3 normalBDG = calNormal(vertexB.position, newVertexGPos, vertexD.position);
    float cosAngleG = glm::dot(normalABD, normalBDG);   //都是单位向量
    float angleG = acos(cosAngleG);                   //旋转的弧度
    glm::vec3 newBD = glm::normalize(vertexD.position - vertexB.position); //该边的向量
    trans = glm::mat4(1.0f);              //创建单位矩阵
    trans = glm::rotate(trans, -angleG, newBC);          //旋转矩阵
    moveTo = glm::mat4(1.0f);
    moveTo = glm::translate(moveTo, -vertexB.position);
    moveBack = glm::mat4(1.0f);
    moveBack = glm::translate(moveBack, vertexB.position);
    vertexGPosition = moveBack * trans * moveTo * vertexGPosition;

}

bool Mesh::isNLD(Edge& edge){   
    if (edge.faceId.size() != 2) {
        return false;
    }
    else {
        Face face1 = faces[*(edge.faceId.begin())];
        Face face2 = faces[*(++edge.faceId.begin())];

        Vertex top1;
        Vertex top2;
        double cos1 = 0.0;
        double cos2 = 0.0;
        for (int i = 0; i < 3; i++) {
            if (face1.vertexs[i].vertexId != edge.vertexe1.vertexId && face1.vertexs[i].vertexId != edge.vertexe2.vertexId) {
                top1 = face1.vertexs[i];
                cos1 = face1.angles[i];
                break;
            }
        }
        for (int i = 0; i < 3; i++) {
            if (face2.vertexs[i].vertexId != edge.vertexe1.vertexId && face2.vertexs[i].vertexId != edge.vertexe2.vertexId) {
                top2 = face2.vertexs[i];
                cos2 = face2.angles[i];
                break;
            }
        }
        //求角度之和，用sin(1+2) = sin(1)*cos(1) + cos(1)*sin(1)
        double sin1 = sqrt(1 - cos1 * cos1);
        double sin2 = sqrt(1 - cos2 * cos2);

        if ((sin1 * cos1 + cos1 * sin1) < 0) {
            glm::vec3 normal1 = glm::normalize(face1.normal);
            glm::vec3 normal2 = glm::normalize(face2.normal);

            if ((normal1 == normal2) || (normal1 + normal2 == glm::vec3(0, 0, 0))) {
                edge.flippable = true;
                
            }
            return true;
        }
        return false;

    }
}

void Mesh::findAllNLDEdges(){
    for (auto it = edges.begin(); it != edges.end(); it++) {
        if (isNLD(*it)) {
            NLDEdges.push(*it);
        }
    }
}

glm::vec3 Mesh::calNormal(glm::vec3& v1, glm::vec3& v2, glm::vec3& v3){
    glm::vec3 n = glm::cross(v2 - v1, v3 - v2);
    if (n != glm::vec3(0, 0, 0)) {
        return glm::normalize(n);
    }
    return n;
}

//找到edge所属的除face的另一个面的对角E
glm::vec3 Mesh::getAnotherVertexPositionByEdge(Face& face, Edge& edge){
    Vertex vertexA = edge.vertexe1;
    Vertex vertexC = edge.vertexe2;

    //求另一个三角面
    Face faceACE;
    if (*(edge.faceId.begin()) != face.faceId) {
        faceACE = faces[*(edge.faceId.begin())];
    }
    else {
        faceACE = faces[*(++edge.faceId.begin())];
    }
    
    //求顶点
    Vertex vertexE;
    for (auto it = faceACE.vertexs.begin(); it != faceACE.vertexs.end(); it++) {
        if ((*it).vertexId != vertexA.vertexId && (*it).vertexId != vertexC.vertexId) {
            vertexE = *it;
            break;
        }
    }
    return glm::vec3(vertexE.position);
}



