#pragma once

#include<glm/glm.hpp>
#include <utility>

class hash_point {          //unordered_map��hashMap����hash���������涼�ǹ�ϣ������һ����
public:
	size_t operator()(const glm::vec3& p) const;
};

class hash_edge {
public:
	size_t operator()(const std::pair<int, int>& p) const;
};