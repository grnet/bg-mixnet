from libcpp cimport bool

cdef extern from "Bgmix.h":
    bool generate_ciphers(const char *ciphers_file,
                          const long dim_m, const long dim_n)
    bool mix(const char *ciphers_file,
             const long dim_m, const long dim_n)
