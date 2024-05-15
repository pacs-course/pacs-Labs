#include <iostream>
#include <string>
#include "Mesh.hpp"

int main(){

  std::string path = "input/";

  // read OpenFOAM mesh
  Mesh voro(path);
  voro.generateCells();

  
  // export mesh to VTK format
  voro.exportVTK("output/poly.vtk");

  return 0;
}
