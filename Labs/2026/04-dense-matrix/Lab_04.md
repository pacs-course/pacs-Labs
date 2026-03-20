---
marp: true
size: 16:9
style: |
    img[alt~="center"] {
      display: block;
      margin: 0 auto;
    }
---

# Laboratory 04
## Dense Linear Algebra Operations

### Paolo Joseph Baioni
[paolojoseph.baioni@polimi.it](mailto:paolojoseph.baioni@polimi.it)
### 20/03/2026

---
## Outline
1. Brief recap about classes
2. A dense matrix class
3. DGEMM
4. Memory layout and cache effects
5. Coverage and profiling
6. Eigen
7. OpenBLAS
8. Break
---
## Outline
9. Thrust: a "drop-in" GPU model
10. Tiling: optimising Thrust with CUDA-C
11. cuBLAS (naive)
12. STL + TBB parallelism compared with Thrust
13. A simple portability layer
14. Q&A
15. References

---
## 1. Brief recap about classes

We start from [`00-matrix`](./00-matrix/) and use it to recall the structure of a simple C++ class:

- private data for storage and indexing
- constructors, getters, and `operator()`
- method declarations in the header, definitions in the source file
- why a matrix type is a good example of encapsulation

Goal: establish one matrix abstraction that we can keep fixed while the implementation evolves during the lab.

---
## 2. A dense matrix class

In [`00-matrix`](./00-matrix/) we inspect a minimal dense matrix container based on `std::vector<double>` and column-major indexing:

- dimensions are fixed at construction
- `sub2ind(i,j)` maps 2D indices into linear memory
- `transpose()`, `factorize()`, and `solve()` define the public interface
- the same interface will be reused in all later exercises

This is the main teaching device for the whole lab: same API, different implementation choices underneath.

---
## 3. DGEMM

In [`01-DGEMM`](./01-DGEMM/) we implement dense matrix-matrix multiplication with the "textbook triple loop":

```cpp
for (i)
  for (j)
    for (k)
      C(i,j) += A(i,k) * B(k,j);
```

- this is the naive `DGEMM` baseline
- it is simple, readable, and correct
- we only focus on matrix product, not on `solve()`

From here on, every variant answers the same question: how much can we improve this baseline without changing the mathematical operation?

---
## 4. Memory layout and cache effects

In [`02-cache-alignement`](./02-cache-alignement/) we see that memory layout changes performance, even if the algebra is the same:

- the matrix stores data in column-major order
- access patterns that follow contiguous memory are cache-friendly
- transposing first can improve locality in the innermost loop
- hardware awareness already matters at class-design level

This is the first explicit performance lesson: algorithm and data layout cannot be discussed separately.

---
## 5. Coverage and profiling

In [`03-coverage+profiling`](./03-coverage+profiling/) we use tooling to inspect what the test actually exercises:

- `make coverage` shows which code paths are executed
- `make memcheck` and `make profile` expose runtime issues and hotspots
- methods that are never called, such as `solve()`, may still hide bugs

> Before optimising, we should know what we are actually running and what remains untested.

---
## 6. Eigen

In [`04-Eigen`](./04-Eigen/) we introduce a high-level, header only linear algebra library and compare it with our hand-written code:

- `Eigen::Map` wraps our existing storage without copying data
- `transposeInPlace()` replaces the explicit transpose loops
- `eigen_retval = eigen_A * eigen_B` replaces the naive `DGEMM`

> We preserve our matrix interface and delegate the heavy numerical work to a well-tested backend.

---
## 7. OpenBLAS

In [`05-OpenBLAS`](./05-OpenBLAS/) we move from a C++ library interface to a classic BLAS call:

- `dgemm_` computes the matrix product directly
- the code makes the column-major convention explicit

> BLAS stands for Basic Linear Algebra Subprograms: a standard library interface for fast low-level vector and matrix operations such as dot products, matrix-vector products, and matrix-matrix multiplication.

---
## 8. Break



---
## 9. Thrust: a "drop-in" GPU model

In [`06-Thrust`](./06-Thrust/) we express the same dense operations with Thrust functors and `thrust::for_each`:

- one functor for transpose
- one functor for matrix product
- host data copied to `thrust::device_vector`
- a familiar algorithmic style, but executed on the GPU

> "drop-in" GPU transition as the structure still resembles the `std::algorithm`s

---
## 10. Tiling: optimising Thrust with CUDA-C

In [`07-tiling`](./07-tiling/) we keep the same operation but change the optimisation strategy:

- custom CUDA kernels replace the pure Thrust approach
- shared-memory tiles reduce global-memory traffic
- computation is organised so data is moved closer to where it is used

> Earlier we improved performance by arranging data in a memory-friendly way. Here we push the same hardware-aware idea further by moving data closer to computations

---
## 11. cuBLAS (naive)

In [`08-cuBLAS`](./08-cuBLAS/) we use NVIDIA's BLAS library as the GPU counterpart of OpenBLAS:

- explicit host-to-device and device-to-host copies
- `cublasDgemm` for matrix multiplication
- `cublasDgeam` for transpose

 > As for Thrust, can be optimised further, relying on more advanced library features

---
## 12. STL + TBB parallelism compared with Thrust

In [`09-STL+tbb`](./09-STL+tbb/) we revisit the naive parallel pattern on the CPU:

- `std::for_each(std::execution::par, ...)` mirrors the Thrust style closely
- indexing logic is almost the same as in the GPU functors
- the example highlights how similar parallel patterns can look across backends

> The `stdpar` approach makes (CPU) parallelism similarity with Thrust (GPU) parallel style visible.

---
## 13. A simple portability layer

In [`10-portability`](./10-portability/) we use preprocessor macros to switch between backends:

- one source can target a Thrust-based path or an STL-based path
- the abstraction is intentionally simple and somewhat C-like, simple but fragile
- it is not the final design we would want in production, we'll see a `template`s based approach later on

> One source, different executables for different devices

---
## 14. Q&A


---
## References

 - [Eigen](https://libeigen.gitlab.io/)

- K. Goto, R. A. van de Geijn, Anatomy of High-Performance Matrix Multiplication, ACM TOMS. https://www.cs.utexas.edu/~flame/pubs/GotoTOMS_final.pdf

- [openBLAS](https://github.com/OpenMathLib/OpenBLAS)

- [CUDA Programming Guide](https://docs.nvidia.com/cuda/cuda-programming-guide/01-introduction/introduction.html)

- [Thrust](https://docs.nvidia.com/cuda/archive/11.8.0/pdf/Thrust_Quick_Start_Guide.pdf) 

- [cuBLAS](https://docs.nvidia.com/cuda/cublas/index.html)

- [C++ execution policies](https://cppreference.com/w/cpp/algorithm/execution_policy_tag.html)
