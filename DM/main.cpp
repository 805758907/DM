#include <iostream>
#include <queue>
#include "Mesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

int main() {
    Mesh mesh;
    //mesh.readOBJ("../file/eiffelpart264534Obj.obj");
    //mesh.readOBJ("../file/testSuccess.obj");
    //mesh.readOBJ("../file/testObj.obj");
    //mesh.readOBJ("../file/testDM.obj");
    //mesh.readOBJ("../file/bunny.obj");
    //mesh.readOBJ("../file/bunnyDM.obj");
    //mesh.readSTL("../file/up.stl");
    //mesh.readSTL("../file/������.STL");
    //mesh.readOBJ("../file/bolizhaoObj.obj");
    //mesh.readSTL("../file/˹���ַ�����װ����STEPת.STL");
    //mesh.readOBJ("../file/T1objSuccess.obj");
    //mesh.readOBJ("../file/T1objSuccessWithNoBoundary.obj");
    //mesh.readOBJ("../file/airforce1Obj.obj");
    //mesh.readOBJ("../file/happy_vrip_res2.obj");
    mesh.printData();
    //mesh.readSTL("../file/�����ݶ�M3X5-N.STL");
    
    mesh.generateDM();
    //printf("face count: %d\n", mesh.faces.size());
    //mesh.simplification(0.1);
    mesh.printData();
    //mesh.saveSTLBinary("../file/woct.stl");
    //mesh.saveOBJ("../res/withoutCheckwithDeleteSuc.obj");
    mesh.saveOBJ("../res/res.obj");
    return 0;
}

