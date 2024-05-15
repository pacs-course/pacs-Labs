#include <iostream>
#include <vector>
#include "writeVTK.hpp"

int main(){

  int NX, NY;
  NX = 50;
  NY = 30;

  double hx, hy;
  hx = 1.0/ (NX+1);
  hy = 1.0/ (NY+1);

  std::vector<std::vector<double>> exacSol;

  double x,y;
  auto  fun = [](const double & x, const double & y){ return x*y;};

  for (int i = 0; i < NX+1; i++) {
    std::vector<double> row;
    x = i*hx;
    for (int j = 0; j < NY+1; j++) {
      y = j*hy;
      row.push_back(fun(x, y));
    }
    exacSol.push_back(row);
  }

  generateVTKFile("mesh/out.vtk", exacSol, NX,NY, hx, hy);

  return 0;
}
