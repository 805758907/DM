#include <stdio.h>
#include <cstring>
#include <algorithm>
#include "Mesh.h"


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
    FILE* fStl = nullptr;
    int err = fopen_s(&fStl, fileName, "r");
    if (fStl == nullptr) {
        return false;
    }
    if (err != 0) {
        return false;
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
    char buf[2];
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
    computeParameter();
    return true;

}
/*
bool Mesh::saveSTLASCII(char *pathName, char *fileName) {
    bool suc = true;
    char *saveName = new char[100];
    sprintf(savename, "%s%s.stl", pathname, filename);

    char *fileInf = new char[200];
    sprintf(fileInf, "solid %s.stl  %s", filename, "by master");

    FILE *fp = fopen(savename, "w");
    fprintf(fp, "%s\n", fileInf);
    delete[]savename;

    for (int i = 0; i < m_TriaNum; i++) {
        int id = triaV1[i];
        float v1x = vx[id];
        float v1y = vy[id];
        float v1z = vz[id];

        id = triaV2[i];
        float v2x = vx[id];
        float v2y = vy[id];
        float v2z = vz[id];

        id = triaV3[i];
        float v3x = vx[id];
        float v3y = vy[id];
        float v3z = vz[id];

        float nx = (v1y - v3y) * (v2z - v3z) - (v1z - v3z) * (v2y - v3y);
        float ny = (v1z - v3z) * (v2x - v3x) - (v2z - v3z) * (v1x - v3x);
        float nz = (v1x - v3x) * (v2y - v3y) - (v2x - v3x) * (v1y - v3y);

        float nxyz = sqrt(nx * nx + ny * ny + nz * nz);

        fprintf(fp, "Face normal %f %f %f\n", nx / nxyz, ny / nxyz, nz / nxyz);
        fprintf(fp, "outer loop\n");
        fprintf(fp, "vertex %f %f %f\n", v1x, v1y, v1z);
        fprintf(fp, "vertex %f %f %f\n", v2x, v2y, v2z);
        fprintf(fp, "vertex %f %f %f\n", v3x, v3y, v3z);
        fprintf(fp, "endloop\n");
        fprintf(fp, "endFace\n");

    }
    sprintf(fileInf, "endsolid %s.ast  %s", filename, "by master");
    fprintf(fp, "%s\n", fileInf);
    fclose(fp);

    delete[]fileInf;


    return suc;
}
*/
bool Mesh::saveSTLBinary(const char *fileName) {

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

bool Mesh::saveSTLASCII(char* pathname, char* filename){
    return false;
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
        edge1.faceId.insert(face.faceId);
        edges.insert(it, edge1);
    }
    

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
        edge2.faceId.insert(face.faceId);
        edges.insert(it, edge2);

    }
    
    

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
        edge3.faceId.insert(face.faceId);
        edges.insert(it, edge3);

    }
    
    
    
    //求角度
    double cosTheta0 = glm::dot((face.vertexs[1].position - face.vertexs[0].position), (face.vertexs[2].position - face.vertexs[0].position)) / length01 / length02;
    double sinTheta0 = sqrt(1 - cosTheta0 * cosTheta0);
    if (sinTheta0 < sinThetaMin) {
        sinThetaMin = sinTheta0;
    }
    double cosTheta1 = glm::dot((face.vertexs[0].position - face.vertexs[1].position), (face.vertexs[2].position - face.vertexs[1].position)) / length01 / length12;
    double sinTheta1 = sqrt(1 - cosTheta1 * cosTheta1);
    if (sinTheta1 < sinThetaMin) {
        sinThetaMin = sinTheta1;
    }
    double cosTheta2 = glm::dot((face.vertexs[0].position - face.vertexs[2].position), (face.vertexs[1].position - face.vertexs[2].position)) / length12 / length02;
    double sinTheta2 = sqrt(1 - cosTheta2 * cosTheta2);
    if (sinTheta2 < sinThetaMin) {
        sinThetaMin = sinTheta2;
    }
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
    for (auto it = edges.begin(); it != edges.end(); it++) {
        (*it).constructCe(rhoV, rhoE);
    }
}

