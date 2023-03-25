# Explanations #

This solution of the lab shows the correct use of tempalte explicit
instantiation and also a possible organization of header files.

- When dealing with templates, also the *definitions of the template go into the header file*, with the exception of 
non inline full specialisation .
However, one may still want to keep pure declaration separated from definitions. It is not required, but may help readability.
Here I decided to put only pure declarations in `sparse_matrix.hpp`, while 
`sparse_matrix_impl.hpp` contains the definitions. The latter is **included** in the former (look at one of the last lines).
So when you include `sparse_matrix.hpp`, you are effectively including also `sparse_matrix_impl.hpp`. Indeed, you could have put directly everything 
in `sparse_matrix.hpp` (and at this point write just the definitions, since definitions are also declarations!).

- I wanted to show also the explicit instantiation technique shown at the lecture on templates. 
Suppose you know that the instances with `T=double` are the one that you use most. You may want to 
compile the corresponding instances only once, saving compilation time. This has been done by adding the external explicit instantiation at the end of `sparse-matrix.hpp`, and then creating a source file where the explicit template instantiation is repeated, this time without `extern`. The corresponding object file will contain the code corresponding to the instances with `T=double` (check with `nm --demangle sparse_matrix_double.o`.

The technique of pre-compiling explicit template instantiations makes
sense only if you plan to use those instances in many translation
units. In this case you normally put the object files with the
produced code in a library, against which you will link the code that
uses those instances.
