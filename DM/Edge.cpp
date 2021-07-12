#include "Edge.h"

Edge::Edge(){
	parent = nullptr;
}

Edge::~Edge(){
}

void Edge::constructCe(double rhoV, double rhoE){
//	if (vertexe1.position - vertexe2.position != glm::vec3(0, 0, 0)) {	应该不会出现向量为0的情况
	glm::vec3 normal = glm::normalize(vertexe1.position - vertexe2.position);
	glm::vec3 rhoVVector = glm::vec3(rhoV, rhoV, rhoV) * normal;
	glm::vec3 rhoEVector = glm::vec3(rhoE, rhoE, rhoE) * normal;
	glm::vec3 pStart = glm::vec3(0, 0, 0);
	pStart = pStart + vertexe1.position + rhoVVector;
	Ce.push_back(pStart);
	//判断该边是否是最短的边，是否可以容纳两个点
	if (length > 2 * rhoV) {
		glm::vec3 pEnd = glm::vec3(0, 0, 0);
		pEnd = pEnd + vertexe2.position - rhoVVector;
		Ce.push_back(pEnd);

		int pCount = (length - 2 * rhoV) / rhoE;
		for (int i = 1; i <= pCount; i++) {
			glm::vec3 p = glm::vec3(0, 0, 0);
			p += pStart;
			for (int j = 0; j < i; j++) {
				p += rhoEVector;
			}
			Ce.push_back(p);
		}
	}


	
}
