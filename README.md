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
| Linux Ubuntu VM, 2 CPU, 4 GB RAM | non-interactive: m = 64, n = 1563  |  67         |  12          |  42         |  13           |
| Linux Ubuntu VM, 2 CPU, 4 GB RAM | Stadium: m = 64, n =  1563         |  67         |  12          |  42         |  13           |
| Linux Ubuntu VM, 2 CPU, 4 GB RAM | Stadium: m = 64, n = 15625         | 924         | 154          | 644         | 126           |
| Linux Ubuntu VM, 8 CPU, 8 GB RAM | Stadium: m = 64, n = 15625         | 405         | 89           | 263         |  53           |

In their paper (see [https://github.com/grnet/bg-mixnet/blob/master/README-stadium.md](README-stadium.md)), the authors show that in a single server installation, Stadium's efficiency improves linearly with the number of available cores in the system. The presented experiment results agree with this pattern.

### Limitations

Stadium's mixnet crashes for m > 64, e.g. 128. This is because of a vector data structure going out of range during the verification phase at round 10. Specifying larger size for the data structure fixes the crash, but causes the shuffle to fail. More on the cause of this shortly. Commit [https://github.com/grnet/bg-mixnet/commit/6e2d99aa423d720e184489056d54b233df34969f](6e2d99aa42) highlights this problem.

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
