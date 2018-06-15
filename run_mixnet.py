from mixnet import mix, generate_ciphers, encrypt_cipher
import sys
import os
import json
import multiprocessing


try:
    dim_m = long(sys.argv[1])
except IndexError:
    dim_m = 64

try:
    dim_n = long(sys.argv[2])
except IndexError:
    dim_n = 4

try:
    ciphers_file = sys.argv[3]
except IndexError:
    ciphers_file = 'ciphers.json'

CHUNKS = multiprocessing.cpu_count()
ciphers_file_exists = False

try:
    from zeus.core import ZeusCoreElection

    if not os.path.isfile(ciphers_file):
        election = ZeusCoreElection()
        crypto = election.do_get_cryptosystem()
        modulus, generator, order = crypto
        _, public_key, _, _, _ = election.create_zeus_key()

        result = encrypt_cipher.chunks([(i, modulus, generator,
            order, public_key) for i in xrange(dim_m * dim_n)], CHUNKS)
        result = result.apply_async()
        cipher_chunks = result.get()
        ciphers = [cipher for chunk in cipher_chunks
            for cipher in chunk]
        # print 'original_ciphers: %s' % str(ciphers)
        crypto_ciphers = {'modulus': modulus,
               'generator': generator,
               'order': order,
               'public': public_key,
               'proof': '',
               'original_ciphers': ciphers,
               'mixed_ciphers': ciphers}
        with open(ciphers_file, 'w') as f:
            json.dump(crypto_ciphers, f)
    else:
        ciphers_file_exists = True
except ImportError:
    result = generate_ciphers.delay(ciphers_file, dim_m, dim_n)
    result.get()

if ciphers_file_exists or result.successful():
    result = mix.delay(ciphers_file, dim_m, dim_n)
    mix_success = result.get()
    if not mix_success:
        print "Shuffle failed. Check log file."
        sys.exit(1)
