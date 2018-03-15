### Concept

This project builds on top of the Stadium software project, which is hosted at https://github.com/nirvantyagi/stadium.
Stadium is a distributed metadata-private messaging system.
The LICENSE, README, and NOTICE files of the original repo are retained in this repo also.

This project is concerned with the adapted mixnet of Bayer-Groth that is used internally by Stadium to shuffle messages.
Of particular importance are the mixnet's efficiency and limitations.

### Efficiency

The Stadium mixnet's efficiency is significantly improved over both the original version and another version that uses non-interactive Toom-Cook multiplications and Keccak SHA-3 256 hash functions. Because the Stadium mixnet parallelises computations with OpenMP directives, its efficiency scales with the power of the underlying computing infrastructure. The following table presents some experiment results.

| Computing environment            | Mixnet                     | Total (sec) | Verify (sec) | Prove (sec) | Shuffle (sec) |
|:--------------------------------:|:----------------------------------:| -----------:| ------------:| -----------:| -------------:|
| Linux Ubuntu VM, 2 CPU, 4 GB RAM | non-interactive: m = 64, n = 1563  |   67         |  12          |   42         |  13           |
| Linux Ubuntu VM, 2 CPU, 4 GB RAM | Stadium: m = 64, n =  1563         |   36         |   9          |   21         |   6           |
| Linux Ubuntu VM, 2 CPU, 4 GB RAM | Stadium: m = 64, n = 15625         |  924         | 154          |  644         | 126           |
| Linux Ubuntu VM, 8 CPU, 8 GB RAM | Stadium: m = 64, n = 15625         |  413         | 94           |  267         |  52           |
| Linux Ubuntu VM, 8 CPU, 8 GB RAM | Stadium: m = 256, n = 3907         |  419         | 89           |  277         |  53           |
| Linux Ubuntu VM, 8 CPU, 8 GB RAM | Stadium: m = 1024, n = 977         |  574         | 91          |   430         |  53           |
| Linux Ubuntu VM, 8 CPU, 8 GB RAM | Stadium: m = 4096, n = 256         | 1422         | 110          | 1253         |  59           |

In their paper (see [README-stadium.md](https://github.com/grnet/bg-mixnet/blob/master/README-stadium.md)), the authors show that in a single server installation, Stadium's efficiency improves linearly with the number of available cores in the system. The presented experiment results agree with this pattern.

### Limitations

Stadium's mixnet worked only for m = 64, m<=n where m is the ciphertext matrix rows and n is the ciphertext matrix columns. In the rest of the cases it either crashed or failed. With the present interventions, it works for m>=64, 4^x=m, x>2. Now, m and n are orthogonal as they should be. A description of the issues follows below.

For m = 16, the verification of the shuffle fails at round 8. We haven't dealt with this.

For m = 256, the implementation crashed and this is because of a vector data structure going out of range during the verification phase at round 10. Specifying larger size for the data structure fixes the crash, but causes the shuffle to fail.
Commit [6e2d99aa42](https://github.com/grnet/bg-mixnet/commit/6e2d99aa423d720e184489056d54b233df34969f) highlights this problem.
The cause of this is that additional folding of the ciphertext matrix is required in rounds 5 and 6. Once this is taken care, the mixnet works for m >= 64, where m = 4^x for x>2.
When Toom-Cook 4 optimization is used, which means that 4 convenient points are used for exponentiations, the number of rows has to be a power of 4.

Another issue in Stadium's mixnet is that the number of columns (n) should not be smaller than the number of rows (m). The implementation crashes in this case. This is because of a careless array definition that is fixed at commit [c48a5bda544](https://github.com/grnet/bg-mixnet/commit/c48a5bda544de4734afb8e52f3f336441207ca6f).

### Bayer-Groth's mixnet

Bayer Groth's mixnet implementation with Toom-Cook optimizations works for m = 16 and m = 64, but fails or crashes for all other values. For m in [16, 64] any number of columns can be specified.

### Software dependencies

- make
- gcc
- NTL library

### Build

`make test`

### Configure

Modify the `config/config` file

### Execute

`./test`
