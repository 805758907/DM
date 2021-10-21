#pragma once

#include "Vertex.h"
#include<set>


class Edge
{
	//std::list<glm::vec3> Ce;
	std::list<float> Ce;	//每一个Ce点在线段上的位置（以V1为起点）

public:
	bool flippable = false;
	bool inStack = false;
	bool splitted = false;
	Edge* parent;
	double length;
	std::set<int> faceId;		//当前所在的三角面
	std::set<int> meshFaceId;	//所在的原三角面
	Vertex* vertexe1;
	Vertex* vertexe2;
	int edgeId;
	bool deleted = false;
	Edge();
	~Edge();
	void constructCe(double rhoV, double rhoE);
	glm::vec3 getSplitePosition(glm::vec3&, glm::vec3&);
};

