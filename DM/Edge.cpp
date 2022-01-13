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

glm::vec3 Edge::getSplitePosition2(glm::vec3& v1, glm::vec3& v2, double rhoV, double rhoE, std::unordered_map<glm::vec3, int, hash_point>* points)
{	
	if (edgeId == 4313) {
		printf("debug");
	}
	double rhoVPart = rhoV / length;
	double rhoEPart = rhoE / length;
	double rightRhoVPart = 1 - rhoVPart;	//Ce点集最右侧的点
	
	
	double len1 = glm::distance(v1, vertexe1->position);
	double len2 = glm::distance(v2, vertexe1->position);
	double part1 = len1 / length;	//v1和v2与起点距离在总长度的占比
	double part2 = len2 / length;
	if (part2 > 1 || part1 > 1) {
		printf("区间端点超过线段端点/n");
		return glm::vec3();
	}
	double center = (part1 + part2) / 2;
	glm::vec3 direction = vertexe2->position - vertexe1->position;	//线段向量

	double target = center;		//最终分割点的比例

	glm::vec3 position;
	glm::vec3 rhoEPartVection = direction * glm::vec3(rhoEPart, rhoEPart, rhoEPart);
	glm::vec3 rhoVPartVection = direction * glm::vec3(rhoVPart, rhoVPart, rhoVPart);

	bool succeed = false;					//是否找到这个分割点

	if (center <= rhoVPart) {	//中点在rhoVPart左侧，则选取rhoVPart以及右侧的最近可用点（既是候选点，也没有被用过）
		target = rhoVPart;
		position = vertexe1->position + rhoVPartVection;

		while (target <= rightRhoVPart) {
			//看看是否能在点集中找到这个点
			auto it = points->find(position);
			if (it != points->end()) {	//该点已经存在，即是被用过的，则找下一个点
				target += rhoEPart;
				position += rhoEPartVection;
			}
			else {						//不存在，则把该点作为分割点
				succeed = true;
				break;
			}
		}

	}
	else if (center >= rightRhoVPart) {	//中点在rightRhoVPart右侧，则选取rightRhoVPart以及左侧的最近可用点（既是候选点，也没有被用过）
		target = rightRhoVPart;
		position = vertexe2->position - rhoVPartVection;
		
		while (target >= rightRhoVPart) {
			//看看是否能在点集中找到这个点
			auto it = points->find(position);
			if (it != points->end()) {	//该点已经存在，即是被用过的，则找前一个点
				target -= rhoEPart;
				position -= rhoEPartVection;
			}
			else {						//不存在，则把该点作为分割点
				succeed = true;
				break;
			}
		}
	}
	else {				//中点在两个边界点中间，则找最近的
		bool succeedLeft = false;
		bool succeedRight = false;
		//先找左侧最近点
		double targetLeft = floor((center - rhoVPart) / rhoEPart) * rhoEPart + rhoVPart;//找到center左侧的最近可用点
		glm::vec3 positionLeft = vertexe1->position + direction * glm::vec3(targetLeft, targetLeft, targetLeft);

		while (targetLeft >= rhoVPart) {
			//看看是否能在点集中找到这个点
			auto itLeft = points->find(positionLeft);
			if (itLeft != points->end()) {	//该点已经存在，即是被用过的，则找前一个点
				targetLeft -= rhoEPart;
				positionLeft -= rhoEPartVection;
			}
			else {						//不存在，则把该点作为分割点
				succeedLeft = true;
				break;
			}
		}
		//再找右侧最近点
		double targetRight = ceil((center - rhoVPart) / rhoEPart) * rhoEPart + rhoVPart;//找到center右侧的最近可用点
		glm::vec3 positionRight;
		if (targetRight > rightRhoVPart) {
			targetRight = rightRhoVPart;
			positionRight = vertexe2->position - rhoVPartVection;
			auto itRight = points->find(positionRight);
			if (itRight != points->end()) {	//该点已经存在，即是被用过的，则右侧已没有可用点
				;
			}
			else {						//不存在，则把该点作为分割点
				succeedRight = true;
			}
		}
		else {
			positionRight = vertexe1->position + direction * glm::vec3(targetRight, targetRight, targetRight);
			while (targetRight <= rightRhoVPart) {
				//看看是否能在点集中找到这个点
				auto itRight = points->find(positionRight);
				if (itRight != points->end()) {	//该点已经存在，即是被用过的，则找后一个点
					targetRight += rhoEPart;
					positionRight += rhoEPartVection;
				}
				else {						//不存在，则把该点作为分割点
					succeedRight = true;
					break;
				}
			}
			if (!succeedRight && targetRight > rightRhoVPart) {
				targetRight = rightRhoVPart;
				positionRight = vertexe2->position - rhoVPartVection;
				auto itRight2 = points->find(positionRight);
				if (itRight2 != points->end()) {	//该点已经存在，即是被用过的，则右侧已没有可用点
					;
				}
				else {						//不存在，则把该点作为分割点
					succeedRight = true;
				}
			}
		}
		if (succeedLeft && succeedRight) {//左右都找到点
			if (center - targetLeft <= targetRight - center) {//左侧更近
				target = targetLeft;
				position = positionLeft;
			}
			else {
				target = targetRight;
				position = positionRight;
			}
			succeed = true;
		}
		else if (succeedLeft) {//只在左侧找到点
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
		printf("找不到可用的分割点\n");
		return glm::vec3();
	}

}
