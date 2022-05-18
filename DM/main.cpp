#include <iostream>
#include <queue>
#include "Mesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

int main() {
    Mesh mesh;
    mesh.readOBJ("../file/testSuccess.obj");
    //mesh.readOBJ("../file/testObj.obj");
    //mesh.readOBJ("../file/bunny.obj");
    //mesh.readSTL("../file/test.stl");
    //mesh.readSTL("../file/T1.stl");
    //mesh.readOBJ("../file/T1objSuccess.obj");
    //mesh.readOBJ("../file/T1objSuccessWithNoBoundary.obj");
    //mesh.readOBJ("../file/airforce1Obj.obj");
    //mesh.printData();
    //mesh.readSTL("../file/½ô¶¨ÂÝ¶¤M3X5-N.STL");
    
    //mesh.generateDM();
    //printf("face count: %d\n", mesh.faces.size());
    mesh.simplification(0.2);
    //mesh.printData();
    //mesh.saveSTLBinary("../file/woct.stl");
    //mesh.saveOBJ("../res/withoutCheckwithDeleteSuc.obj");
    mesh.saveOBJ("../res/res.obj");
    return 0;
}

