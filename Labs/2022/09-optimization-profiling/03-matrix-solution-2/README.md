# Instructions

`Makefile2` contains the following targets.

> make -f Makefile2 coverage

generates a code coverage report.

> make -f Makefile2 memcheck

runs the memory check tool of `valgrind`.

> make -f Makefile2 profile

runs the `callgrind` tool of `valgrind` (profiler).
Results generated by `callgrind` can be inspected, *e.g.* with `KCacheGrind`.

**Note**: running with `valgrind` takes a lot more than a regular run!
Consider setting the matrix size equal to `100` in `test_matrix_mult.hpp` (instead of `1000`) to get results in a reasonable time.

**Note**: `Makefile` and `Makefile2` define different compilation flags (in particular the latter enables debug and coverage generation mode).
Please do a `make distclean` or `make -f Makefile2 distclean` when switching from one type of run to the other.
