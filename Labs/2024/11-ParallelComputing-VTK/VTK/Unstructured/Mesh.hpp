#ifndef MESH_HPP
#define MESH_HPP

#include <map>
#include <vector>
#include <string>

// this struct collects the element local connectivity
// no mesh information is stored here
struct Polyhedron {
  int nVertices;
  int nFaces;
  std::vector<int> faces;
  std::vector<int> vertices;
};

class Mesh {
  private:
    int nPoints;           // number of points
    int nFaces;            // number of faces
    int nCells;            // number of elements
    int nInternalFaces;    // number of internal faces
                    
    std::vector<double> x; // x coordinates
    std::vector<double> y; // y coordinates
    std::vector<double> z; // z coordinates
                           
    std::vector<unsigned int> owner;      // face owner
    std::vector<unsigned int> neig;       // face neighbour
                           
    std::map<int, std::vector<int>> faces;              // faces
    std::map<int, std::vector<int>> elemConnectivity;   // elements
                                                        
    std::vector<Polyhedron> cells;

  public:
    // default constructor
    Mesh() {};

    Mesh(const std::string & filename);
    
    // reads an OpenFOAM mesh file
    void readOpenFOAM(const std::string & filename);

    // exports a mesh into a VTK
    void exportVTK(const std::string & filename);

    // constructor that directly reads OpenFOAM mesh
    Polyhedron getPolyhedron(int i);

    // get the cell connectivity
    void generateCells();
};

#endif // MESH_HPP
