#include <iostream>
#include <queue>
#include "Mesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <time.h>

int main() {
    Mesh mesh;
    //mesh.readOBJ("../file/eiffelpart264534Obj.obj");
    //mesh.readOBJ("../file/testSuccess.obj");
    //mesh.readOBJ("../file/testObj.obj");
    //mesh.readOBJ("../file/testDM.obj");
    //mesh.readOBJ("../file/bunny.obj");
    mesh.readOBJ("../file/bunnyDM.obj");
    //mesh.readSTL("../file/teapot.stl");
    //mesh.printData();
    clock_t start, finish;
    double  duration;
    start = clock();
    //mesh.generateDM();
    //printf("face count: %d\n", mesh.faces.size());

    mesh.simplification(0.1);

    finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    printf("%f seconds\n", duration);

    //mesh.saveSTLBinary("../file/woct.stl");
    //mesh.saveOBJ("../res/withoutCheckwithDeleteSuc.obj");
    mesh.saveOBJ("../res/res.obj");
    return 0;
}

