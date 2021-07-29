#pragma once

#include "Vertex.h"
#include<vector>
#include<set>
#include<list>

class Edge
{
	//std::list<glm::vec3> Ce;
	std::list<float> Ce;	//ÿһ��Ce�����߶��ϵ�λ�ã���V1Ϊ��㣩

public:
	bool flippable = false;
	bool inStack = false;
	Edge* parent;
	double length;
	std::set<int> faceId;		//��ǰ���ڵ�������
	std::set<int> meshFaceId;	//���ڵ�ԭ������
	Vertex* vertexe1;
	Vertex* vertexe2;
	int edgeId;
	bool deleted = false;
	Edge();
	~Edge();
	void constructCe(double rhoV, double rhoE);
	glm::vec3 getSplitePosition(glm::vec3&, glm::vec3&);
};

