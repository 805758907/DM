#ifndef RESEARCH_Mesh_H
#define RESEARCH_Mesh_H

#include "Face.h"
#include <unordered_map>
#include <stack>

template<typename T>
inline void hash_combine(std::size_t& seed, const T& val)
{
	seed ^= std::hash<T>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<typename T>
inline void hash_val(std::size_t& seed, const T& val)
{
	hash_combine(seed, val);
}

template<typename T, typename... Types>
inline void hash_val(std::size_t& seed, const T& val, const Types&... args)
{
	hash_combine(seed, val);
	hash_val(seed, args...);
}

template<typename... Types>
inline std::size_t hash_val(const Types& ...args)
{
	std::size_t seed = 0;
	hash_val(seed, args...);
	return seed;
}
class hash_point {
public:
	size_t operator()(const glm::vec3& p) const {
		return hash_val(p.x, p.y, p.z);
	}
};


class Mesh {
public:
    char partName[80];          //文件名称
    int faceNum;                //面的数目
    double lMin = 10000000000;
    double lMax = 0;
    double sinThetaMin = 1;
    double rhoV;
    double rhoE;
    std::vector<Face> faces;
    std::list<Edge> edges;
    std::vector<Vertex> vertexes;
    std::stack<Edge> NLDEdges;

    std::unordered_map<glm::vec3, int, hash_point>	m_hash_point;

public:
    Mesh();
    ~Mesh();
    bool readSTL(const char* fileName);
    bool readSTLASCII(const char *fileName);
    bool readSTLBinary(const char* fileName);
    bool saveSTLBinary(const char *pathname);
    bool saveSTLASCII(char *pathname,char * filename);

    void generateEdge(Face& face);
    void computeParameter();
    void generateDM();
    bool isNLD(Edge& edge);
    void findAllNLDEdges();
    int findVertexByPoint(glm::vec3 p);
};


#endif //RESEARCH_Mesh_H
