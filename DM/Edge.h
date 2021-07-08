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
	std::list<glm::vec3> Ce;

public:
	bool flippable = false;
	double length;
	std::set<int> faceId;
	Vertex vertexe1;
	Vertex vertexe2;
	Edge();
	~Edge();
	void constructCe(double rhoV, double rhoE);
};

