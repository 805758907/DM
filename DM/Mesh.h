#ifndef RESEARCH_Mesh_H
#define RESEARCH_Mesh_H

#include "Face.h"
#include "hash.h"
#include <stack>

class Mesh {
public:
    char partName[80] = {};          //文件名称
    int faceNum = 0;                //面的数目
    int vertexNum = 0;
    double lMin = 10000000000;
    double lMax = 0;
    double sinThetaMin = 1;
    double rhoV;
    double rhoE;
    std::vector<Face*> faces;
    std::list<Edge*> edges;
    std::vector<Vertex*> vertexes;
    std::stack<Edge*> NLDEdges;
    std::vector<glm::vec3> normals;
    std::vector< glm::vec3> colors;
    int edgeIndex = 0;

    std::unordered_map<glm::vec3, int, hash_point>	m_hash_point;
    std::unordered_map<std::pair<int, int>, Edge*, hash_edge>	m_hash_edge;

public:
    Mesh();
    ~Mesh();
    bool readSTL(const char* fileName);
    bool readOBJ(const char* fileName);
    bool readSTLASCII(const char *fileName);
    bool readSTLBinary(const char* fileName);
    bool saveOBJ(const char* fileName);
    bool saveSTLBinary(const char * fileName);
    bool saveSTLASCII(const char * fileName);

    Edge* generateEdge(Vertex* v1, Vertex* v2);
    void generateEdgeOfFace(Face* face, bool meshEdge);   //bool值表示是否在构建Mesh的边
    void computeParameter();        //计算最长边、最短边和最小夹角
    void generateDM();              //对每条边生成Ce
    void handleNonFlippableNLDEdge(Edge* edge);
    bool isNLD(Edge* edge);         //判断是否是NLD边
    void findAllNLDEdges();
    void flipAllNLDEdgeInFace(Face* face);
    void flipEdge(Edge* edge);                                        //返回翻转后生成的面
    Face* getParentFace(Edge* edge, Face* childFace);                               //获取childFace所在的meshFace
    Vertex* generateNewVertex(glm::vec3&);
    Face* generateNewFace(Vertex* v1, Vertex* v2, Vertex* v3);
    Face* generateNewFaceFromOldFace(Face* parentFace, Vertex* v1, Vertex* v2, Vertex* v3);
    void addNewNonFlippableNLDEdge(Face* face);
    glm::vec3 getAnotherVertexPositionByEdge(Face* face, Edge* edge);
    float getAnotherVertexDegreeByEdge(Face* face, Edge* edge);
    glm::vec3 ParseOBJVec3(const std::string& line);
    void CreateOBJFace(const std::string& line);
    void deleteFace(Face* face, bool deleteEdgeFromList);
    float getPerpendicularPoint(glm::vec3& v1, glm::vec3& v2, glm::vec3& v3);

    void init();
    bool isTypeI(Vertex* vertex, std::vector<float>& subtendedAngles);
    bool isTypeII(Vertex* vertex, std::vector<float>& subtendedAngles);
    void findTypeIAndTypeII();
    std::vector<Face*> flipEdgeWhenTestingTypeII(Edge* edgeAB, Face* faceABC, Face* faceABD);
    bool flipAllEdgesOfTypeII(std::vector<Face*>& faceSet, Vertex* vertex, std::list<int>& borderEdge);//在typeII检测的时候，用于翻转边
    void flipEdgeWhenSimplifying(Vertex* vertexA, Vertex* vertexB, std::list<Face*>& newFaces);           //在压缩TYPE-II顶点的过程中翻转边，需要修改邻边关系
    bool resortIncidentEdge(Vertex* vertex);            //重新排布incidentEdges，使得相邻两条边在同一三角面上。incidentEdges另一个顶点就对应incidentVertexes（下标相同）

    void simplification(float scale);

    Edge* findEdgeByPoints(Vertex* v1, Vertex* v2);
    int findVertexByPoint(glm::vec3& p);
    void ToSet(Vertex* v);//Debug过程中发现简化后出现incident边点重复的情况，添加进行边和点的去重

    void printData();
};


#endif //RESEARCH_Mesh_H
