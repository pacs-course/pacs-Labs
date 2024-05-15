#ifndef WRITEVTK_HPP
#define WRITEVTK_HPP

#include <vector>
#include <string>
#include <fstream>

// generates a STRUCTURES VTK file with a scalar field
void generateVTKFile(const std::string & filename, 
                     const std::vector<std::vector<double>> & scalarField, 
                     int nx, int ny, double hx, double hy) {

    // opens the file
    std::ofstream vtkFile(filename);

    // check if the file was opened
    if (!vtkFile.is_open()) {
        std::cerr << "Error: could not open file " << filename << std::endl;
        return;
    }

    // Write VTK header
    vtkFile <<  "# vtk DataFile Version 3.0\n";
    vtkFile << "Scalar Field Data\n";
    vtkFile << "ASCII\n";                                // file format
    

    // Write grid data
    vtkFile << "DATASET STRUCTURED_POINTS\n";                             // format of the dataset
    vtkFile << "DIMENSIONS " << nx+1 << " " << ny+1 << " " << 1 << "\n";  // number of points in each direction
    vtkFile << "ORIGIN 0 0 0\n";                                          // lower-left corner of the structured grid
    vtkFile << "SPACING" << " " << hx << " " << hy << " " << 1 << "\n";   // spacing between points in each direction
    vtkFile << "POINT_DATA " << (nx+1) * (ny+1) << "\n";                  // number of points
                                                                
    
    // Write scalar field data
    vtkFile << "SCALARS scalars double\n";               // description of the scalar field
    vtkFile << "LOOKUP_TABLE default\n";                 // color table

    // Write vector field data
    for (int j = 0; j < ny+1; j++) {
        for (int i = 0; i < nx+1; i++) {
            vtkFile <<  scalarField[i][j] << "\n";
        }
    }

}

#endif // WRITEVTK_HPP
