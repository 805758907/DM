#include <iostream>
#include "Mesh.h"


int main() {
    Mesh mesh;
    mesh.readSTL("../file/dianchazuo.stl");
    mesh.generateDM();
    mesh.saveSTLBinary("./test.stl");
    return 0;
}
