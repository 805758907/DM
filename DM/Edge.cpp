#include "Edge.h"

Edge::Edge() {
	parent = nullptr;
}

Edge::~Edge() {
}

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
/*
	Ce.push_back(1 - rhoVPart);
	if (length > 2 * rhoV) {
		int pCount = (length - 2 * rhoV) / rhoE;
		float p = 1 - rhoVPart;
		for (int i = 1; i <= pCount; i++) {
			p -= rhoEPart;
			Ce.push_front(p);
		}
		Ce.push_front(rhoVPart);


	}
	*/
}


glm::vec3 Edge::getSplitePosition(glm::vec3& v1, glm::vec3& v2) {//v1，v2为区间的两个端点
	double len1 = glm::distance(v1, vertexe1->position);
	double len2 = glm::distance(v2, vertexe1->position);
	double part1 = len1 / length;	//v1和v2与起点距离在总长度的占比
	double part2 = len2 / length;
	if (part2 > 1 || part1 > 1) {
		printf("区间端点超过线段端点");
	}


	double center = (part1 + part2) / 2;
	
	//glm::vec3 normal = glm::normalize(vertexe2->position - vertexe1->position);
	glm::vec3 normal = vertexe2->position - vertexe1->position;
	auto it = Ce.begin();
	for (; it != Ce.end(); it++) {
		if ((*it) >= part1) {
			break;
		}
	}
	if (it != Ce.end()) {
/*		if ((*it) < part2) {	//找到位于v1、v2之间的最后一个点
			while (it != Ce.end() && (*it) < part2) {
				it++;
			}
			it--;
			glm::vec3 p = glm::vec3(0, 0, 0);
			glm::vec3 vector = glm::vec3(*it, *it, *it) * normal;
			p = p + vertexe1->position + vector;
			Ce.erase(it);
			return p;
		}
		if ((*it) <= part2) {	//找到位于v1、v2之间的第一个点
			glm::vec3 p = glm::vec3(0, 0, 0);
			glm::vec3 vector = glm::vec3(*it, *it, *it) * normal;
			p = p + vertexe1->position + vector;
			Ce.erase(it);
			return p;
		}*/
	//找到位于v1、v2之间的中间点

		if ((*it) <= part2) {
			while (it != Ce.end()&& (*it) < center) {
				it++;
			}
			if (it != Ce.begin()) {
				it--;

			}
			if ((*it) < part1) {
				it++;
			}
			glm::vec3 p = glm::vec3(0, 0, 0);
			glm::vec3 vector = glm::vec3(*it, *it, *it) * normal;
			p = p + vertexe1->position + vector;
			Ce.erase(it);
			return p;
		}
/**/		//不在v1、v2中间，就找离v1或v2最近的点
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
	else {//到最尾部还未找到在区间内的点，则选取最后一个点
		glm::vec3 p = glm::vec3(0, 0, 0);
		it--;
		glm::vec3 vector = glm::vec3(*it, *it, *it) * normal;
		p = p + vertexe1->position + vector;
		Ce.erase(it);
		return p;
	}
}
