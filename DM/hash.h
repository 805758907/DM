#pragma once

#include<glm/glm.hpp>
#include <utility>

class hash_point {          //unordered_map（hashMap）的hash函数，上面都是哈希函数的一部分
public:
	size_t operator()(const glm::vec3& p) const;
};

class hash_edge {
public:
	size_t operator()(const std::pair<int, int>& p) const;
};