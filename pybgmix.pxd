from libcpp cimport bool

cdef extern from "Bgmix.h":
    bool generate_ciphers(char *ciphers_file)
    bool mix(char *ciphers_file)
