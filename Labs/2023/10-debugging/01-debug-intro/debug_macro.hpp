// Use screaming letters to define macro!
#ifndef NDEBUG
#  define BUGPRINT(x)                                     \
    do                                                    \
      {                                                   \
        std::cerr << __FILE__ << ":" << __LINE__ << ": "; \
        std::cerr << #x << " -> " << (x) << std::endl;    \
      }                                                   \
    while (0)
#else
#  define BUGPRINT(x)
#endif

#define TWO_X(x) x * 2
