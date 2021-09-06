#include <iostream>
#include <queue>
#include "Mesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

int main() {
    Mesh mesh;
    mesh.readOBJ("../file/simpleObj.obj");
//    mesh.readSTL("../file/test.stl");
    mesh.generateDM();
    mesh.simplification(0.5);
    mesh.saveSTLBinary("./test.stl");
    mesh.saveOBJ("./test2.obj");

    /*

    glm::mat4 m;
    m[0][0] = 1;
    m[0][1] = 2;
    m[0][2] = 3;
    m[0][3] = 2;
    m[1][0] = 5;
    m[1][1] = 7;
    m[1][2] = 7;
    m[1][3] = 9;
    m[2][0] = 9;
    m[2][1] = 10;
    m[2][2] = 11;
    m[2][3] = 11;
    m[3][0] = 0;
    m[3][1] = 0;
    m[3][2] = 0;
    m[3][3] = 1;

    glm::mat4 n = glm::inverse(m);
    glm::vec4 x(0, 0, 0, 1);
    glm::vec4 nep = x*n;
    glm::vec4 v3(1, 2, 3, 4);
    glm::vec4 res1 = m * v3;
    glm::vec4 res2= v3*m;
    glm::vec3 rrrrr = glm::vec3(nep);
    float e1 = glm::dot(v3, res1);
    float e2 = glm::dot(v3, res2);
    glm::vec4 p = glm::vec4(0.5, 0.5, 0.5, 0.5) * v3 + glm::vec4(0.5, 0.5, 0.5, 0.5) * x;
*/  
    return 0;
}
