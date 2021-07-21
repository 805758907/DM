#include <iostream>
#include "Mesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

glm::vec3 getAnotherPoint1(glm::vec3& v3, glm::vec3& v1, glm::vec3& center) {
    float x31 = v3.x - v1.x;
    float x21 = center.x - v1.x;
    float y31 = v3.y - v1.y;
    float y21 = center.y - v1.y;
    float z31 = v3.z - v1.z;
    float z21 = center.z - v1.z;


    float t = 2 * (x31 * x21 + y31 * y21 + z31 * z21) / (x31 * x31 + y31 * y31 + z31 * z31);

    return glm::vec3(v1.x + t * x31, v1.y + t * y31, v1.z + t * z31);
    //return t;
}

int main() {
    Mesh mesh;
    mesh.readSTL("../file/test.stl");
    mesh.generateDM();
    mesh.saveSTLBinary("./test.stl");
//    mesh.saveSTLASCII("./test.stl");
/*    float angle = 1.57;
    glm::vec3 ve (1,1,0); //该边的向量
    glm::mat4 trans = glm::mat4(1.0f);//创建单位矩阵
    trans = glm::rotate(trans, -angle, ve);
    glm::mat4 moveTo = glm::mat4(1.0f);
    moveTo = glm::translate(moveTo, glm::vec3(0,-1,0));

    glm::mat4 moveBack = glm::mat4(1.0f);
    moveBack = glm::translate(moveBack, glm::vec3(0, 1, 0));


    glm::vec4 newVertex3Position = moveBack * trans * moveTo * glm::vec4(0,2,0,1);


    glm::vec3 v1(1, 1, 3);
    glm::vec3 v2(1, 3, 1);
    glm::vec3 v3(2.414, 2, 2);

    std::vector<glm::vec3> vector;
    vector.push_back(v1);
    vector.push_back(v2);
    vector.push_back(v3);

    glm::vec3 res = solveCenterPointOfCircle(vector);
    res = getAnotherPoint(glm::vec3(1, 4, 0), glm::vec3(1, 1, 3), res);

    glm::vec3 v1(3, 1, 0);
    glm::vec3 v2(2, 2, 0);
    glm::vec3 v3(5, 1, 0);
    glm::vec3 res = getAnotherPoint1(v3, v1, v2);
*/    return 0;
}
