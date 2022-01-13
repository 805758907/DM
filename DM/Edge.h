#pragma once

#include "Vertex.h"
#include "hash.h"
#include<set>
#include<unordered_map>


class Edge
{
	//std::list<glm::vec3> Ce;
	std::list<float> Ce;	//每一个Ce点在线段上的位置（以V1为起点）Ce仅仅记录这些切割点在线段上距离V1占线段长的比例

public:
	bool flippable = false;	//判断一个边是否可以翻转，即边两边的三角形是否在同一个平面
	bool inStack = false;
	bool splitted = false;
	Edge* parent;			//当前边属于原模型的哪个边
	double length;
	std::set<int> faceId;		//当前所在的三角面
	std::set<int> meshFaceId;	//所在的原三角面
	Vertex* vertexe1;			//边的两个顶点，其中1的ID应当小于2的ID
	Vertex* vertexe2;
	int edgeId;					//边的ID
	bool deleted = false;		//生成DM过程中是否删除
	Edge();
	~Edge();
	void constructCe(double rhoV, double rhoE);				//用于通过计算得到的rhoV和rhoE值来获取在边上创建Ce点的位置
	glm::vec3 getSplitePosition(glm::vec3&, glm::vec3&);	//计算获得建立DM时满足LD的分割点（算法二）
	glm::vec3 getSplitePosition2(glm::vec3& v1, glm::vec3& v2, double rhoV, double rhoE, std::unordered_map<glm::vec3, int, hash_point>* points);	//直接通过两个参数找到分割点（区间靠近中间的点）
};

