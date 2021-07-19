#pragma once

#include "Vertex.h"
#include<vector>
#include<set>
#include<list>

class Edge
{
	float edgeError = 0;
	glm::mat4 edgeQ;
	glm::vec3 newPos;
	//std::list<glm::vec3> Ce;
	std::list<float> Ce;	//每一个Ce点在线段上的位置（以V1为起点）

public:
	bool flippable = false;
	Edge* parent;
	double length;
	std::set<int> faceId;
	Vertex* vertexe1;
	Vertex* vertexe2;
	int edgeId;
	Edge();
	~Edge();
	void constructCe(double rhoV, double rhoE);
	glm::vec3 getSplitePosition(glm::vec3&, glm::vec3&);
};

