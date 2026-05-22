---
marp: true
#size: 16:9
paginate: true
style: |
    img[alt~="center"] {
      display: block;
      margin: 0 auto;
    }
    section::after {
      content: attr(data-marpit-pagination) "/" attr(data-marpit-pagination-total);
      position: absolute;
      right: 30px;
      bottom: 20px;
      font-size: 18px;
      color: #666;
    }
---

# Laboratory 11
## Debugging, optimisation and C++ concurrency

### Paolo Joseph Baioni
### 22/05/2026

---
## Outline
1. Complexity
2. Profiling
3. Measuring code
4. Compiler optimisation
5. Testing with Catch2
6. `std::optional`

---
## Outline
7. Debugging with `gdb`
8. Memory checking with Valgrind
9. Exercise: `integer-list`
10. C++ multithreading
11. Locks and synchronisation
12. Futures and thread pools

---
## 1. Complexity

Complexity describes how the amount of work grows when the input size grows.
 - `O(1)`: constant time; the work does not grow with the input size
 - `O(log n)`: logarithmic time; the work grows slowly as the input size grows
 - `O(n)`: linear time; the work grows proportionally to the input size

 
---
## 1. Complexity

### `std::map<T, V>::find`: `O(log n)`

`std::map` is typically implemented as a balanced binary search tree.

Lookup uses the ordering of the keys:

```cpp
std::map<int, double> values;

auto it = values.find(key);
```

The number of steps grows logarithmically with the number of elements.

---
## 1. Complexity

### `std::unordered_map<T, V>::find`: average `O(1)`

`std::unordered_map` is a hash table.

With a good hash function and a well-sized table, lookup is constant time on average:

```cpp
std::unordered_map<int, double> values;

auto it = values.find(key);
```

The key is mapped to a bucket, and only that bucket needs to be inspected.

---
## 1. Complexity

### `std::unordered_map<T, V>::find`: worst case `O(n)`

Hash tables have a worst case.

If many keys collide into the same bucket, lookup may need to scan many elements:

```cpp
auto it = values.find(key);
```

In the worst case, this can become linear in the number of stored elements.

---
## 1. Complexity

For scientific computing, complexity often matters more than a local micro-optimisation.

Before optimising, ask:

* How does the algorithm scale?
* Which input sizes do we actually need?
* Which parts of the program are executed many times?
* Which costs are hidden by libraries or memory access?

---
## 2. Profiling

Profiling means measuring where a program spends time.

This is different from guessing.

Typical questions:

* Which function takes most of the runtime?
* Which call path reaches that function?
* Is the program compute-bound, memory-bound, or I/O-bound?
* Does an optimisation change the real runtime?

---
## 2. Profiling

`gprof` is a classic profiling tool.

Compile with instrumentation:

```bash
g++ -pg -O0 program.cpp -o program
```

Run the executable:

```bash
./program
```

This creates `gmon.out` in the current directory.

---
## 2. Profiling

Generate a readable profile:

```bash
gprof program gmon.out > profile.txt
```

The output contains two main sections:

* **flat profile**: time spent in each function
* **call graph**: which functions call which, and how time propagates

---
## 2. Profiling

`-pg` instruments the executable for profiling.

`-O0` disables optimisation.

This makes the profile easier to connect to the source code, but it is not necessarily representative of the optimised program.

> Profiling configuration is part of the experiment.

---
## 2. Profiling

Another useful workflow is Callgrind plus KCachegrind.

Example: [pacs-Labs/04-dense-matrix/03-coverage+profiling](https://github.com/pacs-course/pacs-Labs/tree/main/Labs/2026/04-dense-matrix/03-coverage%2Bprofiling)

```bash
make profile
valgrind --tool=callgrind ./test_matrix_mult
kcachegrind callgrind.out.<pid> &
```

Callgrind records call graph and instruction-count information.

---
## 2. Profiling

Example output:

```text
==32920== Events    : Ir
==32920== Collected : 436997955
==32920==
==32920== I   refs:      436,997,955
```

`Ir` means instruction reads.

This is not wall-clock time, but it is often useful to compare code paths.

---
## 2. Profiling

Profiling and coverage often live close to each other in the workflow.

Example compile lines:

```bash
g++ -std=c++17 -O0 -g --coverage -Wall -pedantic -I. -c -o matrix.o matrix.cpp
g++ -std=c++17 -O0 -g --coverage -Wall -pedantic -I. test_matrix_mult.o matrix.o -o test_matrix_mult
```

Coverage asks: which code was executed?

Profiling asks: where did execution time go?

---
## 3. Example

We can exploit the C++ techniques we have seen to measure code effectively.

Let's say we want to benchmark a function `f()`

```cpp
template <typename Func>
auto timer(Func f) {
    auto start = chrono::high_resolution_clock::now();
    f();
    auto end = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(end - start);
}
```
This works if `f()` takes no parameters, or if the parameters are fixed elsewhere.

---
## 3. Example

We can solve the problem with variadic templates, to accommodate a varying number of parameters,
and perfect forwarding, to preserve value categories and cv/ref qualifiers.


```cpp
template <typename Func, typename... Args>
auto timer(Func f, Args&&... args) {
    auto start = chrono::high_resolution_clock::now();
    f(std::forward<Args>(args)...);
    auto end = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(end - start);
}
```

---
## 3. Measuring Code

When measuring code, be careful:

* run the same experiment more than once
* choose input sizes large enough to measure
* avoid measuring unrelated setup work
* know whether you are measuring debug or optimised code
* compare with a baseline


---
## 4. Compiler Optimisation

Without optimisation flags, the compiler tries to keep compilation fast and debugging predictable.

With optimisation flags, the compiler tries to improve performance and/or code size.

This can affect:

* runtime
* compilation time
* binary size
* debugging experience

---
## 4. Compiler Optimisation

Common GCC levels:

* `-O0`: default; reduce compilation time; best source-level debugging
* `-O1`: basic optimisation
* `-O2`: stronger optimisation; common release default
* `-O3`: even more optimisation; may increase code size
* `-Os`: optimise for size
* `-Og`: optimise while preserving a good debugging experience
* `-Ofast`: aggressive, may break strict standard compliance

---
## 4. Compiler Optimisation

At `-O0`, many optimisation passes are disabled.

At `-O2`, GCC enables many transformations, including:

* inlining
* vectorisation
* dead-code elimination
* loop optimisations
* register and control-flow optimisations

The generated program may no longer map cleanly to source statements.

---
## 4. Compiler Optimisation

`-Og` is useful during the edit-compile-debug cycle.

It keeps a reasonable debugging experience while enabling some compiler passes.

Example:

```bash
g++ -std=c++17 -Og -g -Wall -pedantic program.cpp -o program
```

For final performance measurements, test the release configuration too.

---
## 4. Compiler Optimisation

`-Ofast` enables `-O3` plus optimisations that are not valid for all standard-compliant programs.

In particular, it may affect floating-point semantics through options such as `-ffast-math`.

For scientific computing, this matters.

> Faster is not automatically correct.

---
## 5. Testing with Catch2

Catch2 is a modern C++ testing framework.

A basic single-file setup:

```cpp
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

// Test cases follow...
```

`CATCH_CONFIG_MAIN` asks Catch2 to generate `main()`.

It should appear in exactly one source file.

---
## 5. Testing with Catch2

Example class under test:

```cpp
class BlockTransfer {
    std::size_t block_size;
    std::size_t max_block_size;

public:
    BlockTransfer(std::size_t requested, std::size_t max_size)
        : block_size(requested), max_block_size(max_size) {}

    std::size_t size() const { return block_size; }
    std::size_t maxSize() const { return max_block_size; }

    void setBlockSize(std::size_t requested) {
        block_size = requested;
    }
};
```

---
## 5. Testing with Catch2

Suppose the block size represents the amount of data transferred at once.

For example, we may want each block to fit a cache-aware limit.

The bug is simple:

* `setBlockSize` accepts values larger than `max_block_size`

Tests should express the intended boundary.

---
## 5. Testing with Catch2

Catch2 tests use `TEST_CASE` and assertions such as `REQUIRE`.

```cpp
TEST_CASE("BlockTransfer enforces the maximum block size", "[block]") {
    BlockTransfer transfer(256, 1024);

    SECTION("accepted block size is stored") {
        transfer.setBlockSize(512);
        REQUIRE(transfer.size() == 512);
    }

    SECTION("oversized block is capped") {
        transfer.setBlockSize(2048);
        REQUIRE(transfer.size() == transfer.maxSize());
    }
}
```

---
## 5. Testing with Catch2

Each `SECTION` runs independently from a fresh setup.

In the previous example:

* `BlockTransfer transfer(256, 1024);` is shared setup
* each section starts from a new `transfer`
* failures identify different behaviours

This keeps related tests compact without making them depend on each other.

---
## 5. Testing with Catch2

Compile and run:

```bash
g++ -std=c++17 -I/path/to/catch2 tests.cpp -o tests
./tests
```

Possible failures:

```text
FAILED:
  REQUIRE( transfer.size() == transfer.maxSize() )
with expansion:
  2048 == 1024
```

The failing test tells us the boundary condition is not enforced.

---
## 5. Testing with Catch2

Fix `setBlockSize` by checking the upper bound:

```cpp
void setBlockSize(std::size_t requested) {
    if (requested > max_block_size) {
        block_size = max_block_size;
    } else {
        block_size = requested;
    }
}
```

---
## 5. Testing with Catch2

Then rerun the tests:

```text
All tests passed (2 assertions in 1 test case)
```

> Turn expected behaviour into executable checks.

---
## 6. `std::optional`
A very common bug source is division by zero. Also in this case, modern C++ can help.

`std::optional<T>` represents either:

* a value of type `T`
* no value

It is useful when absence is expected and should be represented explicitly.

Example:

```cpp
#include <optional>

std::optional<double> safe_divide(double numerator, double denominator);
```

---
## 6. `std::optional`

Example: avoid division by zero by making failure part of the type.

```cpp
std::optional<double> safe_divide(double numerator, double denominator) {
    if (denominator == 0.0) {
        return std::nullopt;
    }

    return numerator / denominator;
}
```

This is not an exception and not a sentinel value.

---
## 6. `std::optional`

Typical use:

```cpp
std::optional<double> result = safe_divide(a, b);

if (result) {
    std::cout << "result = " << *result << '\n';
} else {
    std::cout << "cannot divide by zero\n";
}
```

The caller is forced to decide what to do when no value is available.

---
## 6. `std::optional`

Common operations:

```cpp
result.has_value();
result.value();
result.value_or(0.0);
*result;
```

Be careful with `value()`:

```cpp
double x = result.value(); // throws if no value is present
```

Check first, or use `value_or` when a default makes sense.

---
## 7. Debugging with `gdb`

`gdb` lets us inspect a running program.

Typical workflow:

```bash
g++ -std=c++17 -Og -ggdb -Wall -pedantic program.cpp -o program
gdb ./program
```

Inside `gdb`:

```gdb
run
bt
break main
next
step
print variable
continue
```

---
## 7. Debugging with `gdb`

Useful commands:

* `run`: start the program
* `bt`: print the backtrace
* `break <location>`: set a breakpoint
* `next`: execute the next source line
* `step`: step into a function
* `print <expr>`: inspect an expression
* `continue`: resume execution
* `quit`: exit

---
## 7. Debugging with `gdb`

Example debugging target:

```cpp
#include <map>
#include <iostream>
#include <string>

int main() 
{
    std::map<int, std::string> m;
    m[1]="1";
    std::cout << m.at (2) << std::endl;
    return 0;
}
```

`std::map::at` throws if the key does not exist: you can use `gdb` + `up` to spot the error.

---
## 7. Debugging with `gdb`

Without debug symbols, `gdb` can run the program, but it cannot point back to our source lines.

```bash
g++ bug.cpp -o bug
gdb ./bug
```

```
Reading symbols from ./bug...
(No debugging symbols found in ./bug)
```

---
## 7. Debugging with `gdb`

Compile again with debug information and run inside `gdb`:

```bash
g++ -O0 -ggdb bug.cpp -o bug
gdb ./bug
```

Inside `gdb`:

```
(gdb) run
Starting program: ./bug 
terminate called after throwing an instance of 'std::out_of_range'
  what():  map::at
Program received signal SIGABRT, Aborted.
```

---
## 7. Debugging with `gdb`

Move up the stack until the failing user code appears:

```gdb
(gdb) up
(gdb) up
(gdb) up
...
```

Representative output:

```text
#9  0x00005555555562c6 in main () at bug.cpp:9
9       std::cout << m.at(2) << std::endl;
```

---
## 8. Memory Checking with Valgrind

Valgrind `memcheck` detects memory errors and leaks.

Example: [pacs-Labs/04-dense-matrix/03-coverage+profiling](https://github.com/pacs-course/pacs-Labs/tree/main/Labs/2026/04-dense-matrix/03-coverage%2Bprofiling)

```bash
apptainer shell ~/containers/pacs_2025-2026.sif
make memcheck
```

The Makefile target may run:

```bash
valgrind --tool=memcheck ./test_matrix_mult
```

---
## 8. Memory Checking with Valgrind

Example clean output:

```text
HEAP SUMMARY:
    in use at exit: 0 bytes in 0 blocks
  total heap usage: 13 allocs, 13 frees, 484,132 bytes allocated

All heap blocks were freed -- no leaks are possible

ERROR SUMMARY: 0 errors from 0 contexts
```

This is the target shape, not a guarantee that the program is correct.

---
## 9. Exercise: `integer-list`

The program `integer-list` in directory `02-bug` has:

* a compile error
* a runtime error
* a memory leak
* bonus: a potential memory leak not captured by `main`

Find the issues and fix them.

Use `gdb` and Valgrind to help.

---
## 9. Exercise: `integer-list`

Suggested workflow:

1. Build the program and read the compiler error.
2. Fix the compile error.
3. Run the program normally.
4. Use `gdb` to inspect the runtime error.
5. Use Valgrind to inspect memory behaviour.
6. Compare with `02-bug-solution` only after trying.

---
## Break

---
## 10. C++ Multithreading

A thread is a unit of execution inside a process.

In C++, `std::thread` can run a function or callable object concurrently with the main thread.

Threads can help with:

* responsiveness
* CPU-intensive work on multiple cores
* parallel processing

They also introduce new failure modes.

---
## 10. C++ Multithreading

Basic example:

```cpp
#include <thread>

void sampleFunction() {
    // Task to execute
}

int main() {
    std::thread t1(sampleFunction);
    t1.join();
}
```

`join()` waits for the thread to complete.

---
## 10. C++ Multithreading

Joining:

```cpp
t1.join();
```

The main thread waits for `t1`.

Detaching:

```cpp
t1.detach();
```

The thread runs independently.

Detached threads cannot be joined later, so lifetime and shared state become harder to manage.

---
## 10. C++ Multithreading

Thread lifecycle management matters because threads own resources.

Good practices:

* keep the number of threads controlled
* join threads before program exit
* avoid detached threads unless the lifetime is very clear
* handle exceptions carefully
* use RAII to release resources

---
## 10. C++ Multithreading

Example RAII guard:

```cpp
class ThreadGuard {
public:
    explicit ThreadGuard(std::thread& t) : t_(t) {}

    ~ThreadGuard() {
        if (t_.joinable()) {
            t_.join();
        }
    }

private:
    std::thread& t_;
};
```

---
## 10. C++ Multithreading

Common threading pitfalls:

* race conditions
* deadlocks
* inconsistent shared state
* resource leaks
* exceptions crossing thread boundaries

The main design question is not only "can this run in parallel?", but "what state is shared?"

---
## 11. Locks and Synchronisation

A race condition occurs when multiple threads access shared data and at least one writes to it.

Basic mutex pattern:

```cpp
std::mutex myMutex;

myMutex.lock();
// Critical section
myMutex.unlock();
```

Manual lock/unlock is fragile.

---
## 11. Locks and Synchronisation

Manual mutex management creates exception-safety problems.

Do not use this pattern:

```cpp
std::mutex mtx;

try {
    mtx.lock();
    throw std::runtime_error("Error occurred");
    mtx.unlock();
} catch (const std::exception& e) {
    mtx.lock();
    mtx.unlock();
}
```

If the exception is thrown, the first `unlock()` is never reached.

---
## 11. Locks and Synchronisation

Use RAII:

```cpp
std::mutex mtx;

try {
    std::lock_guard<std::mutex> lock(mtx);
    throw std::runtime_error("Error occurred");
} catch (const std::exception& e) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Error handled: " << e.what();
}
```

`lock_guard` releases the mutex when it goes out of scope.

---
## 11. Locks and Synchronisation

Self-deadlock example:
```cpp
class BankAccount {
    std::mutex mtx;
    int balance = 0;
public:
    void withdraw(int amount) {
        std::lock_guard<std::mutex> lock(mtx);
        balance -= amount;
    }
    void transfer(int amount) {
        std::lock_guard<std::mutex> lock(mtx);
        withdraw(amount);
    }
};
```
`transfer` holds the mutex and then calls a method that tries to lock it again.

---
## 11. Locks and Synchronisation

One possible solution is `std::recursive_mutex`.
```cpp
class BankAccount {
    std::recursive_mutex mtx;
    int balance = 0;
public:
    void withdraw(int amount) {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        balance -= amount;
    }
    void transfer(int amount) {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        withdraw(amount);
    }
};
```
Use it intentionally; it may also hide design problems.

---
## 11. Locks and Synchronisation

Classic deadlock:

```cpp
std::mutex mutex_a;
std::mutex mutex_b;

void thread1() {
    std::lock_guard<std::mutex> lock_a(mutex_a);
    std::lock_guard<std::mutex> lock_b(mutex_b);
}

void thread2() {
    std::lock_guard<std::mutex> lock_b(mutex_b);
    std::lock_guard<std::mutex> lock_a(mutex_a);
}
```

Each thread may hold the resource the other one needs.

---
## 11. Locks and Synchronisation

Solution: consistent lock ordering.

```cpp
void thread1() {
    std::lock_guard<std::mutex> lock_a(mutex_a);
    std::lock_guard<std::mutex> lock_b(mutex_b);
}

void thread2() {
    std::lock_guard<std::mutex> lock_a(mutex_a);
    std::lock_guard<std::mutex> lock_b(mutex_b);
}
```

Always acquire multiple locks in the same predefined order.

---
## 11. Locks and Synchronisation

Better solution for multiple locks: `std::lock`.

```cpp
void safeOperation() {
    std::unique_lock<std::mutex> lock_a(mutex_a, std::defer_lock);
    std::unique_lock<std::mutex> lock_b(mutex_b, std::defer_lock);

    std::lock(lock_a, lock_b);

    // Critical section
}
```

`std::lock` acquires multiple locks in a deadlock-free way.

---
## 11. Locks and Synchronisation

Alternative with `std::adopt_lock`:

```cpp
void safeOperation() {
    std::lock(mutex_a, mutex_b);

    std::lock_guard<std::mutex> lock_a(mutex_a, std::adopt_lock);
    std::lock_guard<std::mutex> lock_b(mutex_b, std::adopt_lock);

    // Critical section
}
```

The lock guards take ownership of locks that were already acquired.

---
## 11. Locks and Synchronisation

Reader-writer locks allow:

* multiple simultaneous readers
* exclusive writers

Example:

```cpp
#include <shared_mutex>

class Counter {
    std::shared_mutex mtx;
    int value = 0;
};
```

This is useful when reads are frequent and writes are rare.

---
## 11. Locks and Synchronisation

Reader-writer example:

```cpp
int read() {
    std::shared_lock<std::shared_mutex> lock(mtx);
    return value;
}

void write(int newValue) {
    std::unique_lock<std::shared_mutex> lock(mtx);
    value = newValue;
}
```

`shared_lock` allows other readers.

`unique_lock` excludes readers and writers.

---
## 11. Locks and Synchronisation

Mechanism reference:

* `mutex`: basic mutual exclusion
* `recursive_mutex`: same thread can lock multiple times
* `shared_mutex`: shared readers, exclusive writers
* `lock_guard`: simple RAII lock wrapper
* `unique_lock`: flexible RAII lock wrapper
* `shared_lock`: RAII wrapper for shared access
* `std::lock`: acquire multiple locks safely

---
## 12. Futures and Thread Pools

Futures and promises provide asynchronous communication.

Basic idea:

* `promise<T>` sends a value
* `future<T>` receives a value

The future blocks only when the value is requested.

---
## 12. Futures and Thread Pools

Basic future/promise pattern:

```cpp
#include <future>

std::promise<int> prom;
std::future<int> fut = prom.get_future();

// Producer thread:
prom.set_value(42);

// Consumer thread:
int result = fut.get();
```

`get()` blocks until the value is available.

---
## 12. Futures and Thread Pools

Asynchronous communication example:
```cpp
void apiCall(std::promise<std::string>& prom) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    prom.set_value("API Response");
}
int main() {
    std::promise<std::string> prom;
    std::future<std::string> fut = prom.get_future();
    std::thread t(apiCall, std::ref(prom));
    t.detach();
    std::string result = fut.get();
}
```
This demonstrates the mechanism; prefer clear thread ownership in production code.

---
## 12. Futures and Thread Pools

`std::async` is simpler for many asynchronous tasks.

```cpp
#include <future>

std::string apiCall(int parameter) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return "Response for " + std::to_string(parameter);
}

int main() {
    std::future<std::string> fut = std::async(apiCall, 6);
    std::string result = fut.get();
}
```

The result is returned through the future.

---
## 12. Futures and Thread Pools

A thread pool maintains worker threads that process tasks from a queue.

Why use a pool?

* avoid creating a new thread for every small task
* control the number of worker threads
* reuse resources
* express work as tasks

This is useful when many independent jobs must be processed.

---
## 12. Futures and Thread Pools

Thread pool data structures:

```cpp
class ThreadPool {
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> shutdown{false};
};
```

The queue is shared state, so it needs synchronisation.

---
## 12. Futures and Thread Pools

Worker loop:

```cpp
while (true) {
    std::function<void()> task;
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        condition.wait(lock, [this]() {
            return shutdown || !tasks.empty();
        });
        if (shutdown && tasks.empty()) 
            return;
        task = std::move(tasks.front());
        tasks.pop();
    }
    task();
}
```
The lock is released before executing the task.

---
## 12. Futures and Thread Pools

Adding tasks:

```cpp
void enqueue(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        tasks.push(std::move(task));
    }
    condition.notify_one();
}
```

The condition variable wakes one worker.

The queue lock protects only queue manipulation.

---
## 12. Futures and Thread Pools

Destructor:

```cpp
~ThreadPool() {
    shutdown = true;
    condition.notify_all();

    for (std::thread& worker : workers) {
        worker.join();
    }
}
```

The destructor asks workers to stop, wakes them, and joins them.

This gives the pool RAII-style lifetime management.

---
## 12. Futures and Thread Pools

Thread pools should not be copied:

```cpp
ThreadPool(const ThreadPool&) = delete;
ThreadPool& operator=(const ThreadPool&) = delete;
```

The pool owns threads and shared state.

Accidental copies would duplicate ownership semantics that do not make sense.

---
## 12. Futures and Thread Pools

Using the pool:

```cpp
int main() {
    ThreadPool pool(4);
    std::mutex print_mutex;

    for (int i = 0; i < 100; ++i) {
        pool.enqueue([i, &print_mutex]() {
            std::lock_guard<std::mutex> lock(print_mutex);
            std::cout << "Task " << i << " completed\n";
        });
    }
}
```

Task execution order is not deterministic.

---
## 12. Futures and Thread Pools

Pattern comparison:

* `promise`/`future`: explicit one-time value transfer
* `async`: simpler asynchronous execution with a returned future
* thread pool: reuse threads for many small tasks

Choose the pattern that matches the ownership and lifetime of the work.

---
## References
 - [https://en.cppreference.com/cpp/thread](https://en.cppreference.com/cpp/thread)
 - [https://learn.microsoft.com/en-us/cpp/](https://learn.microsoft.com/en-us/cpp/)
 - [https://catch2.org/](https://catch2.org/)
 - [https://valgrind.org/docs/manual/quick-start.html](https://valgrind.org/docs/manual/quick-start.html)
 - [https://github.com/KDE/kcachegrind](https://github.com/KDE/kcachegrind)
 - [https://www.gnu.org/savannah-checkouts/gnu/gdb/index.html]
 - [https://lldb.llvm.org/](https://lldb.llvm.org/)
