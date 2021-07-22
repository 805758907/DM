#include "Edge.h"

Edge::Edge() {
	parent = nullptr;
}

Edge::~Edge() {
}
/*
void Edge::constructCe(double rhoV, double rhoE) {
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
*/
void Edge::constructCe(double rhoV, double rhoE) {
	float rhoVPart = rhoV / length;
	float rhoEPart = rhoE / length;
	Ce.push_back(rhoVPart);
	if (length > 2 * rhoV) {
		int pCount = (length - 2 * rhoV) / rhoE;
		float p = rhoVPart;
		for (int i = 1; i <= pCount; i++) {
			p += rhoEPart;
			Ce.push_back(p);
		}
		Ce.push_back(1 - rhoVPart);
	
	}

}


glm::vec3 Edge::getSplitePosition(glm::vec3& v1, glm::vec3& v2) {
	double len1 = sqrt((vertexe1->position.x - v1.x) * (vertexe1->position.x - v1.x) + (vertexe1->position.y - v1.y) * (vertexe1->position.y - v1.y) + (vertexe1->position.z - v1.z) * (vertexe1->position.z - v1.z));
	double len2 = sqrt((vertexe1->position.x - v2.x) * (vertexe1->position.x - v2.x) + (vertexe1->position.y - v2.y) * (vertexe1->position.y - v2.y) + (vertexe1->position.z - v2.z) * (vertexe1->position.z - v2.z));
	double part1 = len1 / length;	//v1和v2与起点距离在总长度的占比
	double part2 = len2 / length;
	
	//glm::vec3 normal = glm::normalize(vertexe2->position - vertexe1->position);
	glm::vec3 normal = vertexe2->position - vertexe1->position;
	auto it = Ce.begin();
	for (; it != Ce.end(); it++) {
		if ((*it) >= part1) {
			break;
		}
	}
	if (it != Ce.end()) {
		if (*it <= part2) {	//找到位于v1、v2之间的第一个点
			glm::vec3 p = glm::vec3(0, 0, 0);
			glm::vec3 vector = glm::vec3(*it, *it, *it) * normal;
			p = p + vertexe1->position + vector;
			Ce.erase(it);
			return p;
		}
		//不在v1、v2中间，就找离v1或v2最近的点
		double dis2 = *(it) - part2;
		double dis1;
		if (it != Ce.begin()) {
			dis1 = part1 - *(--it);	//v1左边的点
			if (dis1 > dis2) {		//v2右边的点
				it++;
			}
			
		}
		//v1左边没有点，就取v2右边的点
		glm::vec3 p = glm::vec3(0, 0, 0);
		glm::vec3 vector = glm::vec3(*it, *it, *it) * normal;
		p = p + vertexe1->position + vector;
		Ce.erase(it);
		return p;
		
	}
	else {
		glm::vec3 p = glm::vec3(0, 0, 0);
		it--;
		glm::vec3 vector = glm::vec3(*it, *it, *it) * normal;
		p = p + vertexe1->position + vector;
		Ce.erase(it);
		return p;
	}
}
