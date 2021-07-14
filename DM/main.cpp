#include <iostream>
#include "Mesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


int main() {
    Mesh mesh;
//    mesh.readSTL("../file/dianchazuo.stl");
//    mesh.generateDM();
//    mesh.saveSTLBinary("./test.stl");
//    mesh.saveSTLASCII("./test.stl");
    float angle = 1.57;
    glm::vec3 ve (1,1,0); //该边的向量
    glm::mat4 trans = glm::mat4(1.0f);//创建单位矩阵
    trans = glm::rotate(trans, -angle, ve);
    glm::mat4 moveTo = glm::mat4(1.0f);
    moveTo = glm::translate(moveTo, glm::vec3(0,-1,0));

    glm::mat4 moveBack = glm::mat4(1.0f);
    moveBack = glm::translate(moveBack, glm::vec3(0, 1, 0));


    glm::vec4 newVertex3Position = moveBack * trans * moveTo * glm::vec4(0,2,0,1);
    return 0;
}
