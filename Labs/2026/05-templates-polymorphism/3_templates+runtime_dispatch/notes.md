The runtime example is a single binary that contains both implementations.

What nvcc is doing here is not “compile GPU code only”. It is acting as a compiler driver:

  - it compiles CUDA parts itself
  - it delegates ordinary host C++ code to the host compiler, typically g++
  - then it links everything together

So in current matrix.cpp:

  - the STL path is just normal host C++ code
  - the Thrust path contains CUDA-capable code
  - nvcc can build both into one executable
  - at runtime, switch chooses which one to execute

 So we get one build, both backends, choice at runtime:
```bash
  ./test_matrix_mult stl
  ./test_matrix_mult thrust
```
 But both must be available. 
