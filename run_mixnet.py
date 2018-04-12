from mixnet import mix, generate_ciphers
import sys

try:
    dim_m = long(sys.argv[1])
except IndexError:
    dim_m = 64

try:
    dim_n = long(sys.argv[2])
except IndexError:
    dim_n = 2

r = generate_ciphers.delay(dim_m, dim_n)
r.get()
if r.successful():
    r = mix.delay(dim_m, dim_n)
    r.get()
