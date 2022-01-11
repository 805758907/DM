#include <iostream>
#include <queue>
#include "Mesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

int main() {
    Mesh mesh;
    //mesh.readOBJ("../file/building.obj");
    mesh.readSTL("../file/test.STL");
    //mesh.readSTL("../file/eddgtest.STL");
    mesh.generateDM();

    mesh.simplification(0.2);
    mesh.saveSTLBinary("../file/woct.stl");
    // mesh.saveOBJ("../file/kkslptest.obj");


    return 0;
}

