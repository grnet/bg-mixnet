cimport pybgmix

def bg_generate_ciphers(ciphers_file, dim_m, dim_n):
    return pybgmix.generate_ciphers(ciphers_file, dim_m, dim_n)

def bg_mix(ciphers_file, dim_m, dim_n):
    return pybgmix.mix(ciphers_file, dim_m, dim_n)
