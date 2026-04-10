---
marp: true
size: 16:9
style: |
    img[alt~="center"] {
      display: block;
      margin: 0 auto;
    }
---

# Laboratory 05
## Templates, Runtime Selection, and Polymorphism

### Paolo Joseph Baioni
[paolojoseph.baioni@polimi.it](mailto:paolojoseph.baioni@polimi.it)
### 10/04/2026

---
## Outline
1. Brief recap about portability
2. A small template bridge: `print_container<T>`
3. Template-based portability
4. Compile-time selection and its features
5. Runtime selection without polymorphism
6. Break

---
## Outline
7. Application: neural-network training
8. Backpropagation as reverse-mode AD
9. Optimizers and runtime polymorphism
10. Design discussion
11. Q&A
12. References

---
## Post-Easter recap: Lab 01

Lab 01 established the local working environment:

- container usage
- shell
- compilers and toolchains
- minimal C++ workflow

These are the essentials to write, compile, and run code.

---
## Post-Easter recap: Lab 02

Lab 02 moved into a real computing context:

- remote access
- cluster usage
- job submission
- reproducible environments (spack, envs)
- tool-oriented workflow

with a focus on operating correctly in a scientific-computing environment.

---
## Post-Easter recap: Lab 03

Lab 03 shifted to modern C++ programming tools:

- I/O and parameters
- containers
- timing
- header-only libraries
- lambdas and callables
- iterators, algorithms, and execution policies

---
## Post-Easter recap: Lab 04

Lab 04 on dense linear algebra gave the first scientific-computing perspective:

- same mathematical task
- many implementations
- performance consequences
- portability as an engineering concern

This is the direct starting point for today: one matrix abstraction, many backend choices underneath.

---
## Today: Lab 05

This next lab is a natural continuation.

The focus shifts from:

- "how do implementations differ across backends?"

to:

- "how do we express variability in C++ cleanly?"

---

## Today: Lab 05

We will compare four mechanisms:

- macros:  
  “same source pattern, different text, chosen at preprocessing time”

- templates:  
  “same source pattern, different types, chosen at compile time”

- runtime selection:  
  “different precompiled implementations, chosen at runtime”

- polymorphism:  
  “different behaviors behind a common interface, chosen at runtime”

---
## 1. Brief recap about portability

Last time we ended with [`10-portability`](https://github.com/pacs-course/pacs-Labs/tree/main/Labs/2026/04-dense-matrix/10-portability):

- one source can target different implementations
- the mechanism was intentionally simple, and macros-based / C-like
- this was useful as a baseline, but not yet as an idiomatic C++ design

First goal for today: revisit the same portability idea with stronger abstraction mechanisms.

---
## 1. Brief recap about portability

The conceptual question remains the same:

> how do we keep the high-level operation fixed while allowing the implementation to vary?

We examine three C++ answers:

- templates, which address compile-time variability
- runtime dispatch with `switch`
- runtime polymorphism through a common interface

---
## 2. A small template bridge: `print_container<T>`

We start from a simple template:

```cpp
template <typename T>
std::ostream & print_container(const T& c, std::ostream & os = std::cout)
{
    for (const auto & r : c)
        os << r << " ";
    return os;
}
```

---
## 2. A small template bridge: `print_container<T>`

That can be called 

```cpp
int main ()
{
	std::vector <int> v {0,1,2,3,4,5,6,7,8,9};
	print_container (v) << std::endl;
	return 0;
}
```

Abstraction over the type allows compile-time check that macros, being plain text substitution performed by the pre-processor, avoid

---
## 2. A small template bridge: `print_container<T>`

The function does not care whether `c` is:

- `std::vector<int>`
- `std::list<double>`
- `std::array<float, N>`

as long as:

- it can be iterated
- its elements can be written to the stream

 > templates as abstraction mechanism
 > can we assume constraints are met? can we control it?

---
## 2. A small template bridge: `print_container<T>`

Indeed,

- the function assumes the elements are printable
- that assumption is not made explicit yet

We'll see during the labs a first example of how to use

- a concept
- or another kind of compile-time constraint

that checks whether `os << x` is valid.

---
## 3. Template-based portability

In [`2_templates-based-portability`](https://github.com/pacs-course/pacs-Labs/tree/main/Labs/2026/05-templates-polymorphism/2_templates-based-portability) we use templates to solve the Lab 4 portability problem:

- same mathematical operation
- different implementation backends
- backend-specific code expressed statically

---
## 3. Template-based portability

Templates are especially attractive in scientific computing because they support:

- specialization
- inlining
- zero-overhead abstractions
- optimization opportunities for the compiler

So they often fit kernels, data layouts, numeric types, and backend policies very well.

---
## 3. Template-based portability

In [`2_templates-based-portability`](https://github.com/pacs-course/pacs-Labs/tree/main/Labs/2026/05-templates-polymorphism/2_templates-based-portability) we have compile-time selection

> same source pattern, different types or policies, chosen at compile time

Model:

- one build
- one chosen backend

> Typical of high-performance kernels

---
## 4. Compile-time selection and its features

Compile-time selection is powerful, but it has a limit:

- the choice must be known before the executable is built

So if the user wants to decide later, at execution time, we have a different problem.

This is the transition to the next mechanism.

---
## 4. Compile-time selection and its features

Templates:

- decision made at compile time: compile-time templates define the backend-specific code
- type checking
- no runtime overhead (compile time can increase though)

---
## 5. Runtime selection without polymorphism

In [`3_templates+runtime_dispatch`](https://github.com/pacs-course/pacs-Labs/tree/main/Labs/2026/05-templates-polymorphism/3_templates+runtime_dispatch) we keep backend-specific implementations, but we change how the choice is made:

- compile all required versions into one binary
- select the backend at runtime
- dispatch with a small `switch`

So the choice is dynamic, even though the implementations are still precompiled.

---
## 5. Runtime selection without polymorphism

This gives a useful hybrid model in which:

- compile-time templates define backend-specific code
- runtime logic chooses which compiled version to execute

This avoids virtual functions and class hierarchies, while still giving runtime choice.

---
## 5. Runtime selection without polymorphism

Key distinction: runtime dispatch with `switch` implies

- the caller knows all alternatives explicitly
- the branch is visible in the code
- there is no common virtual interface hiding behavior

So this is runtime selection, but not yet polymorphism.

---
## 5. Runtime selection without polymorphism

To wrap-up,
 - runtime switch selects which already-compiled version to execute, 
 - t his avoids virtual functions, but all selected backends must still be available in the build . 
 
In the example, `nvcc` is required for compilation, even though we then choose to run on CPU

Having distinguished between pre-processor time, compile-time and runtime selection, in the second part of the lab, we add one more ingredient, 

- a common runtime interface

That will make the next step polymorphic.

---
## 6. Break



---
## 7. A real application: neural-network training

We'll be introducing polymorphism in a real example (taken from old [PACS project](https://github.com/pjbaioni/neural-net.git)), in which it is used as an instrument to build a hierarchy of optimizers inside a simple deep neural network architecture, with the goal of selecting a solver family at runtime. 

Note that the goal is not to learn deep learning, but to show a scientific lab codebase where polymorphism cleanly separates optimizer behavior from the training loop, as well as to provide an example of `is a` and `has a` paradigms in object oriented programming.

For the final step we use the project in [`4_polymorphism_neural-net`](https://github.com/pacs-course/pacs-Labs/tree/main/Labs/2026/05-templates-polymorphism/4_polymorphism_neural-net).

---
## 7. A real application: neural-network training

Before delving into implementation details, let's have a tour of the workflow:

1. generate or load datasets
2. read parameters and architecture
3. construct the network
4. train
5. test
6. save predictions
7. plot results


---
## 7. A real application: neural-network training

The repository is organised as a small application:

- `data/`: parameters, architectures, train/test sets, predictions
- `include/`: public headers
- `src/`: `main.cpp` and `NeuralNetwork.cpp`
- `src/write_set/`: dataset generator
- `doc/`: report and figures

So now we are looking at a compact scientific code base, focussing on how to implement user-selected strategies.

---
## 7. A real application: neural-network training

The top-level `Makefile` is also worth a brief comment:

- it delegates to `src/write_set`
- it delegates to `src`

So recursive Makefiles play a modular role at build level:

- top-level orchestration
- specialised local components

---
## 7. A real application: neural-network training

`src/main.cpp` is the orchestrator:

- parse command-line options
- read the `.pot` parameter file
- load training and test data
- load the architecture
- construct `NeuralNetwork`
- train and test
- save predictions and plot

---
## 8. Backpropagation as reverse-mode AD

In this applications, it is important to conceptually separate two tasks:

- compute gradients
- use gradients to update parameters

In this project:

- backpropagation computes the gradients (reverse-mode automatic differentiation)
- the optimizer hierarchy decides how the update is performed

---
## 8. Backpropagation as reverse-mode AD

Inside `NeuralNetwork`, the stored quantities are:

- `L`: pre-activation values
- `A`: activations
- `B`: biases
- `W`: weights 
- `dW`, `db`: gradients

This is the computational graph made explicit in arrays and vectors.

The reverse pass is hand-written, but conceptually it follows the same chain-rule structure as autodiff frameworks.

---
## 9. Optimizers and runtime polymorphism

In `NeuralNetwork.hpp`, the optimizer objects are stored through smart pointers to a base class:

- `std::unique_ptr<Optimizers::GradientDescent<Eigen::MatrixXd>>`
- `std::unique_ptr<Optimizers::GradientDescent<Eigen::VectorXd>>`

So the network owns:

- one common optimizer interface
- many possible concrete behaviors

---
## 9. Optimizers and runtime polymorphism

In `Optimizers.hpp`, the hierarchy is:

```
                    GradientDescent<T>
                             ^
                             |
                    GDwithMomentum<T>
                             ^
              +--------------+--------------+
              |                             |
          RMSprop<T>                 KingmaBa<T>
                                              ^
                              +---------------+---------------+
                              |                               |
                          Adam<T>                       AdaMax<T>
```

---
## 9. Optimizers and runtime polymorphism

All the arrows represent inheritance, so:

```
GDwithMomentum<T> is a GradientDescent<T>
RMSprop<T> is a GDwithMomentum<T>
KingmaBa<T> is a GDwithMomentum<T>
Adam<T> is a KingmaBa<T>
AdaMax<T> is a KingmaBa<T>
```

So the diagram is primarily an `is-a` hierarchy, the typical OOP public inheritance tree.

---
## 9. Optimizers and runtime polymorphism

The base class exposes a common callable interface:

- virtual destructor
- virtual `operator()`

So every optimizer can be used through the same syntax:

`optimizer(theta, gradient, alpha, t)`

The behavior varies, but the call site stays fixed.

---
## 9. Optimizers and runtime polymorphism

```
   +------------------------------+
   |  GradientDescent<T>  |
   +------------------------------+
   | # virtual update_objective() |
   | + GradientDescent()          |
   | + virtual ~GradientDescent() |
   | + virtual operator()         |
   +------------------------------+
 
```

GD defines the common interface, while providing a usable optimizer

---
## 9. Optimizers and runtime polymorphism

```
             +----------------------+
             |   GDwithMomentum<T>  |
             +----------------------+
             | # T mt               |
             | # double beta        |
             |----------------------|
             | # compute_momentum() |
             | + GDwithMomentum()   |
             | + set_beta()         |
             | + operator() override|
             +----------------------+
```
- `GDwithMomentum` is-a `GradientDescent`  
- Introduces state

---
## 9. Optimizers and runtime polymorphism

```
+----------------------+          +--------------------------+
|      RMSprop<T>      |          |      KingmaBa<T>         |
+----------------------+          +--------------------------+
| - double epsilon     |          | # double beta2           |
|----------------------|          |--------------------------|
| + RMSprop()          |          | + KingmaBa()             |
| + virtual set_eps()  |          | + virtual set_beta2()    |
| + operator() override|          | + operator() override = 0|
+----------------------+          +--------------------------+

```
Both are is-a GDwithMomentum, but w/ diverging strategies:  
- RMSprop → adaptive scaling  
- KingmaBa → virtual foundation for Adam  

---
## 9. Optimizers and runtime polymorphism

```
   +----------------------+           +----------------------+
   |       Adam<T>        |           |      AdaMax<T>       |
   +----------------------+           +----------------------+
   | - RMSprop<T> rms     |           | - T ut               |
   |----------------------|           | - double epsilon     |
   | - T correction_step()|           | - cwiseMax()         |
   | + Adam()             |           |----------------------|
   | + operator() override|           | + AdaMax()           |
   +----------------------+           | + set_epsilon()      |
                                      | + operator() override|
                                      +----------------------+
```
Both are is-a KingmaBa, and Adam has-a RMSprop (composition example)

---
## 9. Optimizers and runtime polymorphism
Design pattern:
- inheritance → interface + polymorphism  
- composition → reuse + flexibility  
(other choices available)

---
## 9. Optimizers and runtime polymorphism

In `NeuralNetwork` class we can see a runtime `switch`:

- explicit branch among known implementations

Polymorphism:

- a common interface
- behavior hidden behind base-class handles
- runtime choice with a stable call site

---
## 9. Optimizers and runtime polymorphism

In `NeuralNetwork::train`:

- a `switch` selects which optimizer objects are created
- later, all updates use the same interface

For example:

- `(*W_optimizer[l])(W[l], dW[l], alpha, t);`
- `(*b_optimizer[l])(b[l], db[l], alpha, t);`

So the training loop does not need optimizer-specific update code.

---
## 9. Optimizers and runtime polymorphism

The practical value of polymorphism here is:

- the forward pass remains unchanged
- the backward pass remains unchanged
- the update sites remain unchanged
- only the optimizer behavior varies

---
## 10. Design discussion

- templates are used for data representation (`MatrixXd` vs `VectorXd`)
- concepts are used to introduce requirements on template types (`CallableMatrix`)
- polymorphism is used for behavioral variation
- the application remains readable from `main.cpp`

---
## 10. Design discussion

Some off-topic improvements:
- optimizer selection is encoded with integer codes from the parameter file
> Prefer an `enum class OptimizerKind { ... };`, and handle properly the relation w/ GetPot

- old-style, manual string management for files path
> prefer `#include <filesystem>` for robustness

And examples:
- GetPot for CLI and file parameters
- gnuplot and gnuplot iostream

---
## 11. Q&A


---
## References

- [cppreference: templates](https://en.cppreference.com/w/cpp/language/templates)

- [cppreference: constraints and concepts](https://en.cppreference.com/w/cpp/language/constraints)

- [cppreference: virtual functions](https://en.cppreference.com/w/cpp/language/virtual)

- [Eigen](https://libeigen.tuxfamily.org/)

- [gnuplot: A GNU graphing utility](http://www.gnuplot.info/)

- [gnuplot-iostream: A C++ interface to gnuplot](https://github.com/dstahlke/gnuplot-iostream)

- D. P. Kingma, J. Ba, Adam: A Method for Stochastic Optimization, 2015. https://arxiv.org/abs/1412.6980
