## Concept

This project builds on top of the Stadium software project, which is hosted at https://github.com/nirvantyagi/stadium.
Stadium is a distributed metadata-private messaging system.
The LICENSE, README, and NOTICE files of the original repo are retained in this repo also.

This project is concerned with the adapted mixnet of Bayer-Groth that is used internally by Stadium to shuffle messages.
Of particular importance are the mixnet's efficiency and limitations.

## Efficiency

The Stadium mixnet's efficiency is significantly improved over both the original version and another version that uses non-interactive Toom-Cook multiplications and Keccak SHA-3 256 hash functions (see the toom-cook-non-interactive-keccak branch). Because the Stadium mixnet parallelises computations with OpenMP directives, its efficiency scales with the power of the underlying computing infrastructure. The following table presents some experiment results.

| Computing environment            | Mixnet                     | Total (sec) | Verify (sec) | Prove (sec) | Shuffle (sec) |
|:--------------------------------:|:----------------------------------:| -----------:| ------------:| -----------:| -------------:|
| Linux Ubuntu VM, 2 CPU, 4 GB RAM  | non-interactive: m = 64, n = 1563  |   67         |   12           |   42         |   13           |
| Linux Ubuntu VM, 2 CPU, 4 GB RAM  | Stadium: m = 64, n =  1563         |   36         |    9           |   21         |    6           |
| Linux Ubuntu VM, 2 CPU, 4 GB RAM  | Stadium: m = 64, n = 15625         |  924         | 154            |  644         |  126           |
| Linux Ubuntu VM, 8 CPU, 8 GB RAM  | Stadium: m = 64, n = 15625         |  413         |  94            |  267         |   52           |
| Linux Ubuntu VM, 8 CPU, 8 GB RAM  | Stadium-Zeus: m = 64, n = 1563     |  801         |  98            |  526         |  177           |
| Debian (stereo), 8 CPU, 12 GB RAM | Stadium-Zeus: m = 64, n = 15625    | 9121         | 998            | 5557         | 2566           |
| Linux Ubuntu VM, 8 CPU, 8 GB RAM  | Stadium: m = 256, n = 3907         |  419         |  89            |  277         |   53           |
| Linux Ubuntu VM, 8 CPU, 8 GB RAM  | Stadium: m = 1024, n = 977         |  574         |  91            |  430         |   53           |
| Linux Ubuntu VM, 8 CPU, 8 GB RAM  | Stadium: m = 4096, n = 256         | 1422         | 110            | 1253         |   59           |

In their paper (see [README-stadium.md](https://github.com/grnet/bg-mixnet/blob/master/README-stadium.md)), the authors show that in a single server installation, Stadium's efficiency improves linearly with the number of available cores in the system. The presented experiment results agree with this pattern.

Stadium-Zeus regards the software setting where the cryptosystem and the ciphers are provided by GRNET's Zeus e-voting system (see [run_mixnet.py](https://github.com/grnet/bg-mixnet/blob/master/run_mixnet.py#L30)). In this setting, more computational resources are required to make ends meet both in terms of CPU time and memory space. Regading memory space, for 1M ciphers (64 * 15625) the resident set size (RSS) peaked at 8.5 GB RAM.

Zeus uses ElGamal over big integers. The Stadium mixnet can use ElGamal over:
- big integers
- points of the elliptic curve [25519](https://en.wikipedia.org/wiki/Curve25519)

A compile macro `USE_REAL_POINTS` decides which flavor is used.

The measurements of Stadium-Zeus are carried out using Zeus cryptosystem. All other measurements with the Stadium mixnet are carried out using ElGamal over the elliptic curve.


## Limitations

Stadium's mixnet worked only for m = 64, m<=n where m is the ciphertext matrix rows and n is the ciphertext matrix columns. In the rest of the cases it either crashed or failed. With the present interventions, it works for m>=64, 4^x=m, x>2 and n>=4. Now, m and n are orthogonal as they should be. A description of the issues follows below.

For m = 16, the verification of the shuffle fails at round 8. We haven't dealt with this.

For m = 256, the implementation crashed and this is because of a vector data structure going out of range during the verification phase at round 10. Specifying larger size for the data structure fixes the crash, but causes the shuffle to fail.
Commit [6e2d99aa42](https://github.com/grnet/bg-mixnet/commit/6e2d99aa423d720e184489056d54b233df34969f) highlights this problem.
The cause of this is that additional folding of the ciphertext matrix is required in rounds 5 and 6. Once this is taken care, the mixnet works for m >= 64, where m = 4^x for x>2.
When Toom-Cook 4 optimization is used, which means that 4 convenient points are used for exponentiations, the number of rows has to be a power of 4.

Another issue in Stadium's mixnet is that the number of columns (n) should not be smaller than the number of rows (m). The implementation crashes in this case. This is because of a careless array definition that is fixed at commit [c48a5bda544](https://github.com/grnet/bg-mixnet/commit/c48a5bda544de4734afb8e52f3f336441207ca6f).

If n<4 was supplied, the mixnet generated ciphers up to n = 4.
To cover for this, we don't allow n < 4.

## Bayer-Groth's mixnet implementation in toom-cook-non-interactive-keccak branch

Bayer Groth's mixnet implementation with Toom-Cook optimizations works for m = 16 and m = 64, but fails or crashes for all other values. For m in [16, 64] any number of columns can be specified.

## Software dependencies

- Make
- GCC
- NTL library (>=10.5.0)
- GMP
- Boost
- OpenMP (comes with GCC >=4.2)

Remember to set `LD_LIBRARY_PATH` to the install location of the shared libraries.
For convenience, you can export the variable in your favorite shell profile, say `~/.bashrc`, e.g.:

`export LD_LIBRARY_PATH=/usr/local/lib`

## Configure

Modify the `config/config` file

## Build test executable and shared library

`make`

By default the bgmix shared library (`libbgmix.so`) is installed in the local directory.
Again, for convenience, you can add the path to `LD_LIBRARY_PATH` as before to avoid specifying it when invoking executables that link to it.

## Execute

`./bgmix`

## Logging

By default the mixnet (library) logs messages in /var/log/celery/bg\_mixnet.log.
This behavior can be changed at compile time with:

`make LOG_CRYPTO_OUTPUT=log_file_with_absolute_path`

## Zeus integration

After cloning the current repository, clone [GRNET Zeus](https://github.com/grnet/zeus) and export PYTHONPATH to point to Zeus's root directory, e.g.

`export PYTHONPATH="/home/user/zeus"`

### Wrap the mixnet as a Python module with Cython

`python setup.py build_ext -i`

### IO interface between Zeus and mixnet

A JSON file with the following schema:
```lang=python
{
 'modulus': BigInt,
 'generator': BigInt,
 'order': BigInt,
 'public': BigInt,
 'proof': String,
 'original_ciphers': Array,
 'mixed_ciphers': Array
}
```

The mixnet's JSON (de)serialization functions are the following two:

- [serialization function](https://github.com/grnet/bg-mixnet/blob/master/src/Functions.cpp#L122)
- [deserialization function](https://github.com/grnet/bg-mixnet/blob/master/src/Functions.cpp#L295)

### Programming interface extension

The mixnet's programming interface currently includes:
- [the generation of ciphertexts](https://github.com/grnet/bg-mixnet/blob/master/src/Bgmix.cpp#L361)
- [mixing](https://github.com/grnet/bg-mixnet/blob/master/src/Bgmix.cpp#L449)

To extend the programming interface:

1. the C/C++ function to be added has to be declared in [Bgmix.h](https://github.com/grnet/bg-mixnet/blob/master/src/Bgmix.h) and [pybgmix.pxd](https://github.com/grnet/bg-mixnet/blob/master/pybgmix.pxd)
2. a new Python function has to be implemented in [pybgmix.pyx](https://github.com/grnet/bg-mixnet/blob/master/pybgmix.pyx) that calls the C/C++ one
3. the new Python function can be called from a Python context by importing the Python module, that is, `import pybgmix` as in [mixnet.py](https://github.com/grnet/bg-mixnet/blob/master/mixnet.py)

### Run as Celery application with tasks

```lang=bash
sudo mkdir /var/log/celery /var/run/celery
sudo chown user:user /var/log/celery /var/run/celery # Ubuntu
sudo cp default_celeryd /etc/default/
sudo cp initd_celeryd /etc/init.d/celeryd
sudo chmod +x /etc/init.d/celeryd
sudo service celeryd start
```

### Trigger task execution

`python run_mixnet.py <cipher matrix rows> <cipher matrix columns>`
