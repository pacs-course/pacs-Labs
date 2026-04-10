# Exercise 4 — Neural Network Optimizers (Polymorphism)

In this exercise we work on an existent codebase:

https://github.com/pjbaioni/neural-net

---

## Setup

Clone the repository:

```bash
git clone https://github.com/pjbaioni/neural-net
cd neural-net
```

Two branches are relevant:

* `refactoring` -> complete implementation (reference solution)
* `lab` -> version with missing parts to be implemented

Switch to the lab version:

```bash
git checkout lab
```

---

## Context

The project implements a simple neural network training workflow, including:

* data generation
* model construction
* training and testing
* post-processing

In this lab, we **reuse the existing infrastructure** and focus only on selected C++ components.

---

## Objective

The goal is to explore **runtime polymorphism** in a realistic setting.

We will work on:

* the `NeuralNetwork` class
* the optimizer hierarchy in `Optimizers.hpp`

---

## Notes

* The code is based on a previous student project (GPL), slightly refactored for teaching.
* Some parts have been intentionally removed in the `lab` branch.
* You can refer to the `refactoring` branch for a complete implementation.

---

## Focus

Pay particular attention to:

* inheritance relationships (**is-a**)
* composition (**has-a**)
* use of virtual functions
* separation between interface and implementation
