#include <iostream>
#include "Mesh.h"


int main() {
    Mesh mesh;
    mesh.readSTL("../file/������.STL");
    //mesh.generateDM();
    mesh.saveSTLBinary("./test.stl");
    return 0;
}
