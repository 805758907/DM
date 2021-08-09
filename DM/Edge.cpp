#include "Edge.h"

Edge::Edge() {
	parent = nullptr;
}

Edge::~Edge() {
}
/*
void Edge::constructCe(double rhoV, double rhoE) {
	//	if (vertexe1.position - vertexe2.position != glm::vec3(0, 0, 0)) {	Ӧ�ò����������Ϊ0�����
	glm::vec3 normal = glm::normalize(vertexe1.position - vertexe2.position);
	glm::vec3 rhoVVector = glm::vec3(rhoV, rhoV, rhoV) * normal;
	glm::vec3 rhoEVector = glm::vec3(rhoE, rhoE, rhoE) * normal;
	glm::vec3 pStart = glm::vec3(0, 0, 0);
	pStart = pStart + vertexe1.position + rhoVVector;
	Ce.push_back(pStart);
	//�жϸñ��Ƿ�����̵ıߣ��Ƿ��������������
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
	double len1 = glm::distance(v1, vertexe1->position);
	double len2 = glm::distance(v2, vertexe1->position);
	double part1 = len1 / length;	//v1��v2�����������ܳ��ȵ�ռ��
	double part2 = len2 / length;
	if (part2 > 1 || part1 > 1) {
		printf("debug");
	}
	if (edgeId == 5588) {
		printf("debug");
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
/*		if (*it <= part2) {	//�ҵ�λ��v1��v2֮��ĵ�һ����
			glm::vec3 p = glm::vec3(0, 0, 0);
			glm::vec3 vector = glm::vec3(*it, *it, *it) * normal;
			p = p + vertexe1->position + vector;
			Ce.erase(it);
			return p;
		}
*/		//�ҵ�λ��v1��v2֮����м��

		if ((*it) <= part2) {
			while (it != Ce.end()&& (*it) < center) {
				it++;
			}
			if (it == Ce.end()) {
				it--;
			}
			glm::vec3 p = glm::vec3(0, 0, 0);
			glm::vec3 vector = glm::vec3(*it, *it, *it) * normal;
			p = p + vertexe1->position + vector;
			Ce.erase(it);
			return p;
		}
		//����v1��v2�м䣬������v1��v2����ĵ�
		double dis2 = *(it) - part2;
		double dis1;
		if (it != Ce.begin()) {
			dis1 = part1 - *(--it);	//v1��ߵĵ�
			if (dis1 > dis2) {		//v2�ұߵĵ�
				it++;
			}
			
		}
		//v1���û�е㣬��ȡv2�ұߵĵ�
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
