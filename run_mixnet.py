from mixnet import mix, generate_ciphers

r = generate_ciphers.delay()
r.get()
if r.successful():
    r = mix.delay()
    r.get()
