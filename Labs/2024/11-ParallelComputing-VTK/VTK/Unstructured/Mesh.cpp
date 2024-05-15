#include "Mesh.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <regex>

// constructor that reads an OpenFOAM grid 
Mesh::Mesh(const std::string & filename) {
    readOpenFOAM(filename);
}

// method to export the mesh in a vtk format
void Mesh::exportVTK(const std::string & filename){

    // open the file
    std::ofstream vtkFile(filename);

    // check if the vtkFile was opened
    if (!vtkFile.is_open()){
        std::cerr << "Error opening vtkFile " <<  filename << std::endl; 
        return;
    }

    // Write VTK header
    vtkFile << "# vtk DataFile Version 3.0" << std::endl;
    vtkFile << "Scalar Field Data\n" << std::endl;
    vtkFile << "ASCII" << std::endl;                         // vtkFile format

    vtkFile << "DATASET UNSTRUCTURED_GRID" << std::endl;     // format of the dataset
    vtkFile << "POINTS " << nPoints << " double" << std::endl;            

    // print all the points
    for (int i = 0; i < nPoints; i++) {
        vtkFile << x[i] << " " << y[i] << " " << z[i] << std::endl;
    }

    vtkFile << std::endl;

    // value for cells connectivity
    // the format needs to provide a local number of cells to have the proper
    // dimensions the whole connectivity of the cells. 
    int local = nCells;
    for ( int i = 0; i < nCells; i++){
      local = local + cells[i].nFaces + 1;
      for (int j = 0; j < cells[i].nFaces; j++){
        local = local + faces[cells[i].faces[j]].size(); 
      }
    }

    // VTK vtkFile format:
    vtkFile <<  "CELLS ";            // keyword for cell connectivity
    vtkFile <<  nCells << " ";                   // number of cells 
    vtkFile <<  local  << std::endl;

    // loop over the cells
    for ( int i = 0; i < nCells; i++){

      // print total size of the cell: again this is 
      // employed to allocate exactly the local connectivity
      local = 1;
      local = local + cells[i].nFaces;
      for (int j = 0; j < cells[i].nFaces; j++){
        local = local + faces[cells[i].faces[j]].size(); 
      }
      vtkFile << local << " "; 

      // print number of faces
      vtkFile << cells[i].nFaces << " "; 

        // loop over the faces of elem i
        for (int j = 0; j < cells[i].nFaces; j++){

          // get the size of the face j of the elem i from the faces
          vtkFile << faces[cells[i].faces[j]].size() << " ";

            // print the vertex for each face
            for (size_t k = 0; k < faces[cells[i].faces[j]].size(); k++){
               vtkFile << faces[cells[i].faces[j]][k] << " ";
            }
        }
        vtkFile << std::endl;
    }


    vtkFile <<  "CELL_TYPES " << nCells <<  std::endl;       // keyword for cell types
    for (int i = 0; i < nCells; i++) {
        vtkFile <<  "42" << std::endl;           // 42 is the id for polyhedrons
    }
    vtkFile.close();
};


void Mesh::readOpenFOAM(const std::string & path) {

    // Opens the owner file
    std::string filename = path + "constant/polyMesh/owner";
    
    // initialize the file stream
    std::ifstream file(filename);

    // checks if the file was opened properly
    if (!file.is_open()) {
      std::cerr << "Unable to open file " << filename << std::endl;
      return;
    }

    // reads mesh information
    std::string line;

    // prepers a regex for reading mesh data
    std::regex pattern(R"(nPoints:(\d+)\s+nCells:(\d+)\s+nFaces:(\d+)\s+nInternalFaces:(\d+))");

    // openfoam header
    for (int i = 0; i < 12 && std::getline(file, line); ++i) {}

    std::getline(file, line);
    std::smatch matches;

    // regex to match the number of points, cells, faces and internal faces
    if (std::regex_search(line, matches, pattern)) {
      nPoints = std::stoi(matches[1].str());
      nCells = std::stoi(matches[2].str());
      nFaces = std::stoi(matches[3].str());
      nInternalFaces = std::stoi(matches[4].str());
    }

    for (int i = 0; i < 8 && std::getline(file, line); ++i) {}

    // The owner file contains the owner of each face
    owner.resize(nFaces); 

    for (int i = 0; i < nFaces; i++) {
      file >> owner[i];
    }

    std::cout << "Owner faces:    read!" << std::endl;
    file.close();
    
    // read neighbour file
    filename = path + "constant/polyMesh/neighbour";
    file.open(filename);

    if (!file.is_open()) {
      std::cerr << "Unable to open file " << filename << std::endl;
      return;
    }
   
    // skip header file 
    for (int i = 0; i < 21 && std::getline(file, line); ++i) {}
    neig.resize(nInternalFaces);

    // read neighbour file.
    // The number of neighbour faces is the number of internal faces
    // nFaces - nInternalFaces = nBoundaryFaces
    for (int i = 0; i < nInternalFaces; i++) {
      file >> neig[i];
    }

    std::cout << "Neig  faces:    read! " << std::endl;
    file.close();

    // read faces
    filename = path + "constant/polyMesh/faces";
    file.open(filename);
  
    // skip header file
    for (int i = 0; i < 20 && std::getline(file, line); ++i) {}
 
    // loop over the faces 
    for (int i = 0; i < nFaces; i++){

      std::getline(file, line);

      // remove ( ) 
      std::size_t start   = line.find('(');
      std::size_t finish  = line.find(')');

      // number of vertices in the face changes per face
      int nVerticesFace = std::stoi(line.substr(0, start));

      // resize the vector of vertices
      faces[i].resize(nVerticesFace);

      std::istringstream stream(line.substr(start+1, finish - start - 1));

      // maybe I should perform a for loop here
      for (int k = 0; k < nVerticesFace; k++) {
        stream >> faces[i][k];
      }
    }

    std::cout << "Faces:          read! " << std::endl;
    file.close();

    // read grid points
    filename = path + "constant/polyMesh/points";
    file.open(filename);

    // skip header file
    for (int i = 0; i < 20 && std::getline(file, line); ++i) {}
    x.resize(nPoints);
    y.resize(nPoints);
    z.resize(nPoints);

    for (int i = 0; i < nPoints; i++){
      std::getline(file, line);
      std::size_t start   = line.find('(');
      std::size_t finish  = line.find(')');
      std::istringstream stream(line.substr(start+1, finish - start - 1));
      stream >> x[i] >> y[i] >> z[i];
    }

    std::cout << "Points:         read! " << std::endl;
    file.close();
}


void Mesh::generateCells(){
    // function to reconstruct element face connectivity
  
    cells.resize(nCells);     // set the proper number of cells

    std::map<int, std::vector<int>> cellConnectivity;              // faces
    
    // loop over the faces
    for ( int i = 0; i < nFaces; i++){
      cellConnectivity[owner[i]].push_back(i);
    }
    for ( int i = 0; i < nInternalFaces; i++){
      cellConnectivity[neig[i]].push_back(i);
    }

    // build local elem conn
    for ( int i = 0; i < nCells; i++){
      std::set<int> vertexSet;
      Polyhedron poly;

      for (const int & face : cellConnectivity[i]){
        poly.faces.push_back(face);  
        for (const int & vertex : faces[face]){
          vertexSet.insert(vertex);
        }
      } 
      poly.nFaces = cellConnectivity[i].size();
      poly.nVertices = vertexSet.size();
      poly.vertices.resize(vertexSet.size());
      std::copy(vertexSet.begin(), vertexSet.end(), poly.vertices.begin());

      cells[i] = poly;
    }

}

Polyhedron Mesh::getPolyhedron(int i){
    return cells[i];
}


