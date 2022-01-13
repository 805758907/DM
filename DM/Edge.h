#pragma once

#include "Vertex.h"
#include "hash.h"
#include<set>
#include<unordered_map>


class Edge
{
	//std::list<glm::vec3> Ce;
	std::list<float> Ce;	//ÿһ��Ce�����߶��ϵ�λ�ã���V1Ϊ��㣩Ce������¼��Щ�и�����߶��Ͼ���V1ռ�߶γ��ı���

public:
	bool flippable = false;	//�ж�һ�����Ƿ���Է�ת���������ߵ��������Ƿ���ͬһ��ƽ��
	bool inStack = false;
	bool splitted = false;
	Edge* parent;			//��ǰ������ԭģ�͵��ĸ���
	double length;
	std::set<int> faceId;		//��ǰ���ڵ�������
	std::set<int> meshFaceId;	//���ڵ�ԭ������
	Vertex* vertexe1;			//�ߵ��������㣬����1��IDӦ��С��2��ID
	Vertex* vertexe2;
	int edgeId;					//�ߵ�ID
	bool deleted = false;		//����DM�������Ƿ�ɾ��
	Edge();
	~Edge();
	void constructCe(double rhoV, double rhoE);				//����ͨ������õ���rhoV��rhoEֵ����ȡ�ڱ��ϴ���Ce���λ��
	glm::vec3 getSplitePosition(glm::vec3&, glm::vec3&);	//�����ý���DMʱ����LD�ķָ�㣨�㷨����
	glm::vec3 getSplitePosition2(glm::vec3& v1, glm::vec3& v2, double rhoV, double rhoE, std::unordered_map<glm::vec3, int, hash_point>* points);	//ֱ��ͨ�����������ҵ��ָ�㣨���俿���м�ĵ㣩
};

