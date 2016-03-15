# Notes on Bayer and Groth's verifiable shuffle

**Bayer and Groth Verifiable Shuffles:**
Stephanie Bayer and Jens Groth. _Efficient zero-knowledge argument for correctness of a shuffle_. EUROCRYPT 2012.

The original version of the verifiable shuffle is [here](https://github.com/derbear/verifiable-shuffle). Our modified version of the verified shuffle is [here](https://github.com/nirvantyagi/stadium/tree/master/groth) and mirrored [here](https://github.com/derbear/verifiable-shuffle/tree/stadium). 

We modified Bayer and Groth's verifiable shuffle, decreasing latency by more than an order of magnitude. We optimized the shuffle by applying the following improvements:

- Added OpenMP directives to optimize key operations, such as Brickell et al.'s multi-exponentiation routines.
- Replaced the use of integers with Moon and Langley's implementation of Bernstein's curve25519 group. (We avoid point compression and decompression in intermediary operations to improve speed.)
- Improved point serialization and deserialization with byte-level representations of the data.
- Taking into account different performance profile of curve25519, replaced some multi-exponentiation routines with naive version and tweaked multi-exponentiation window sizes. The bottleneck for the shuffle is currently in multi-exponentiation routines.
- Added some more small optimizations (e.g. powers of 2, reduce dynamic memory allocations, etc.)

## Stadium

**SOSP Paper:**
Nirvan Tyagi, Yossi Gilad, Derek Leung, Matei Zaharia, and Nickolai Zeldovich. _Stadium: A Distributed Metadata-Private Messaging System_. SOSP 2017.

**ePrint:**
Nirvan Tyagi, Yossi Gilad, Derek Leung, Matei Zaharia, and Nickolai Zeldovich. _Stadium: A Distributed Metadata-Private Messaging System_. Cryptology ePrint Archive, Report 2016/943. http://eprint.iacr.org/2016/943. 2016.

This version of the shuffle library was used to implement verifiable shuffles in [Stadium](https://github.com/nirvantyagi/stadium).
