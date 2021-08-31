#include <iostream>
#include "Mesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


int main() {
    Mesh mesh;
    mesh.readOBJ("../file/testObj.obj");
//    mesh.readSTL("../file/test.stl");
    mesh.generateDM();
    mesh.simplification(0.5);
    mesh.saveSTLBinary("./test.stl");
    mesh.saveOBJ("./test2.obj");


    
    
    return 0;
}
