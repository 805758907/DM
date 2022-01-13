#include "Edge.h"
#include "hash.h"
#include <math.h>


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
}


glm::vec3 Edge::getSplitePosition(glm::vec3& v1, glm::vec3& v2) {//v1��v2Ϊ����������˵�
	double len1 = glm::distance(v1, vertexe1->position);
	double len2 = glm::distance(v2, vertexe1->position);
	double part1 = len1 / length;	//v1��v2�����������ܳ��ȵ�ռ��
	double part2 = len2 / length;
	if (part2 > 1 || part1 > 1) {
		printf("����˵㳬���߶ζ˵�");
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
/*		if ((*it) < part2) {	//�ҵ�λ��v1��v2֮������һ����
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
		if ((*it) <= part2) {	//�ҵ�λ��v1��v2֮��ĵ�һ����
			glm::vec3 p = glm::vec3(0, 0, 0);
			glm::vec3 vector = glm::vec3(*it, *it, *it) * normal;
			p = p + vertexe1->position + vector;
			Ce.erase(it);
			return p;
		}*/
	//�ҵ�λ��v1��v2֮����м��

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
/**/		//����v1��v2�м䣬������v1��v2����ĵ�
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
	else {//����β����δ�ҵ��������ڵĵ㣬��ѡȡ���һ����
		glm::vec3 p = glm::vec3(0, 0, 0);
		it--;
		glm::vec3 vector = glm::vec3(*it, *it, *it) * normal;
		p = p + vertexe1->position + vector;
		Ce.erase(it);
		return p;
	}
}

glm::vec3 Edge::getSplitePosition2(glm::vec3& v1, glm::vec3& v2, double rhoV, double rhoE, std::unordered_map<glm::vec3, int, hash_point>* points)
{	
	if (edgeId == 4313) {
		printf("debug");
	}
	double rhoVPart = rhoV / length;
	double rhoEPart = rhoE / length;
	double rightRhoVPart = 1 - rhoVPart;	//Ce�㼯���Ҳ�ĵ�
	
	
	double len1 = glm::distance(v1, vertexe1->position);
	double len2 = glm::distance(v2, vertexe1->position);
	double part1 = len1 / length;	//v1��v2�����������ܳ��ȵ�ռ��
	double part2 = len2 / length;
	if (part2 > 1 || part1 > 1) {
		printf("����˵㳬���߶ζ˵�/n");
		return glm::vec3();
	}
	double center = (part1 + part2) / 2;
	glm::vec3 direction = vertexe2->position - vertexe1->position;	//�߶�����

	double target = center;		//���շָ��ı���

	glm::vec3 position;
	glm::vec3 rhoEPartVection = direction * glm::vec3(rhoEPart, rhoEPart, rhoEPart);
	glm::vec3 rhoVPartVection = direction * glm::vec3(rhoVPart, rhoVPart, rhoVPart);

	bool succeed = false;					//�Ƿ��ҵ�����ָ��

	if (center <= rhoVPart) {	//�е���rhoVPart��࣬��ѡȡrhoVPart�Լ��Ҳ��������õ㣨���Ǻ�ѡ�㣬Ҳû�б��ù���
		target = rhoVPart;
		position = vertexe1->position + rhoVPartVection;

		while (target <= rightRhoVPart) {
			//�����Ƿ����ڵ㼯���ҵ������
			auto it = points->find(position);
			if (it != points->end()) {	//�õ��Ѿ����ڣ����Ǳ��ù��ģ�������һ����
				target += rhoEPart;
				position += rhoEPartVection;
			}
			else {						//�����ڣ���Ѹõ���Ϊ�ָ��
				succeed = true;
				break;
			}
		}

	}
	else if (center >= rightRhoVPart) {	//�е���rightRhoVPart�Ҳ࣬��ѡȡrightRhoVPart�Լ�����������õ㣨���Ǻ�ѡ�㣬Ҳû�б��ù���
		target = rightRhoVPart;
		position = vertexe2->position - rhoVPartVection;
		
		while (target >= rightRhoVPart) {
			//�����Ƿ����ڵ㼯���ҵ������
			auto it = points->find(position);
			if (it != points->end()) {	//�õ��Ѿ����ڣ����Ǳ��ù��ģ�����ǰһ����
				target -= rhoEPart;
				position -= rhoEPartVection;
			}
			else {						//�����ڣ���Ѹõ���Ϊ�ָ��
				succeed = true;
				break;
			}
		}
	}
	else {				//�е��������߽���м䣬���������
		bool succeedLeft = false;
		bool succeedRight = false;
		//������������
		double targetLeft = floor((center - rhoVPart) / rhoEPart) * rhoEPart + rhoVPart;//�ҵ�center����������õ�
		glm::vec3 positionLeft = vertexe1->position + direction * glm::vec3(targetLeft, targetLeft, targetLeft);

		while (targetLeft >= rhoVPart) {
			//�����Ƿ����ڵ㼯���ҵ������
			auto itLeft = points->find(positionLeft);
			if (itLeft != points->end()) {	//�õ��Ѿ����ڣ����Ǳ��ù��ģ�����ǰһ����
				targetLeft -= rhoEPart;
				positionLeft -= rhoEPartVection;
			}
			else {						//�����ڣ���Ѹõ���Ϊ�ָ��
				succeedLeft = true;
				break;
			}
		}
		//�����Ҳ������
		double targetRight = ceil((center - rhoVPart) / rhoEPart) * rhoEPart + rhoVPart;//�ҵ�center�Ҳ��������õ�
		glm::vec3 positionRight;
		if (targetRight > rightRhoVPart) {
			targetRight = rightRhoVPart;
			positionRight = vertexe2->position - rhoVPartVection;
			auto itRight = points->find(positionRight);
			if (itRight != points->end()) {	//�õ��Ѿ����ڣ����Ǳ��ù��ģ����Ҳ���û�п��õ�
				;
			}
			else {						//�����ڣ���Ѹõ���Ϊ�ָ��
				succeedRight = true;
			}
		}
		else {
			positionRight = vertexe1->position + direction * glm::vec3(targetRight, targetRight, targetRight);
			while (targetRight <= rightRhoVPart) {
				//�����Ƿ����ڵ㼯���ҵ������
				auto itRight = points->find(positionRight);
				if (itRight != points->end()) {	//�õ��Ѿ����ڣ����Ǳ��ù��ģ����Һ�һ����
					targetRight += rhoEPart;
					positionRight += rhoEPartVection;
				}
				else {						//�����ڣ���Ѹõ���Ϊ�ָ��
					succeedRight = true;
					break;
				}
			}
			if (!succeedRight && targetRight > rightRhoVPart) {
				targetRight = rightRhoVPart;
				positionRight = vertexe2->position - rhoVPartVection;
				auto itRight2 = points->find(positionRight);
				if (itRight2 != points->end()) {	//�õ��Ѿ����ڣ����Ǳ��ù��ģ����Ҳ���û�п��õ�
					;
				}
				else {						//�����ڣ���Ѹõ���Ϊ�ָ��
					succeedRight = true;
				}
			}
		}
		if (succeedLeft && succeedRight) {//���Ҷ��ҵ���
			if (center - targetLeft <= targetRight - center) {//������
				target = targetLeft;
				position = positionLeft;
			}
			else {
				target = targetRight;
				position = positionRight;
			}
			succeed = true;
		}
		else if (succeedLeft) {//ֻ������ҵ���
			target = targetLeft;
			position = positionLeft;
			succeed = true;
		}
		else  if (succeedRight) {
			target = targetRight;
			position = positionRight;
			succeed = true;
		}
		else {
			succeed = false;
		}
		
		
	}
	if (succeed) {
		return position;

	}
	else {
		printf("�Ҳ������õķָ��\n");
		return glm::vec3();
	}

}
