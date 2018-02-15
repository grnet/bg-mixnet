# Stadium

_A Distributed Metadata-Private Messaging System_

**SOSP Paper:**
Nirvan Tyagi, Yossi Gilad, Derek Leung, Matei Zaharia, and Nickolai Zeldovich. _Stadium: A Distributed Metadata-Private Messaging System_. SOSP 2017.

**ePrint:**
Nirvan Tyagi, Yossi Gilad, Derek Leung, Matei Zaharia, and Nickolai Zeldovich. _Stadium: A Distributed Metadata-Private Messaging System_. Cryptology ePrint Archive, Report 2016/943. http://eprint.iacr.org/2016/943. 2016.

The system consists of several main components. A participating stadium server must deploy a *shuffle server*, *shuffle client*, and the *stadium server* and *stadium coordinator*. 

This system also contains an optimized version of Stephanie Bayer and Jens Groth's verifiable shuffle.

**Bayer and Groth Verifiable Shuffles:**
Stephanie Bayer and Jens Groth. _Efficient zero-knowledge argument for correctness of a shuffle_. EUROCRYPT 2012.

## Notes on the verifiable shuffle

The original version of the verifiable shuffle is [here](https://github.com/derbear/verifiable-shuffle). Our modified version of the verified shuffle is [here](https://github.com/nirvantyagi/stadium/tree/master/groth) and mirrored [here](https://github.com/derbear/verifiable-shuffle/tree/stadium). 

We modified Bayer and Groth's verifiable shuffle, decreasing latency by more than an order of magnitude. We optimized the shuffle by applying the following improvements:

- Added OpenMP directives to optimize key operations, such as Brickell et al.'s multi-exponentiation routines.
- Replaced the use of integers with Moon and Langley's implementation of Bernstein's curve25519 group. (We avoid point compression and decompression in intermediary operations to improve speed.)
- Improved point serialization and deserialization with byte-level representations of the data.
- Taking into account different performance profile of curve25519, replaced some multi-exponentiation routines with naive version and tweaked multi-exponentiation window sizes. The bottleneck for the shuffle is currently in multi-exponentiation routines.
- Added some more small optimizations (e.g. powers of 2, reduce dynamic memory allocations, etc.)

## Setting up the Shuffle Server and Client

_This setup describes a deployment on an Ubuntu Linux machine on AWS. Adapt commands as needed for another OS_

1. Launch a EC2 instance and add security rules: all ICMP, all TCP.

| Ports   | Protocol | Source    | *Security Group* |
| ------- |:--------:| --------- | ---------------- |
| 0-65535 | tcp      | 0.0.0.0/0 | ✔                |
| 22      | tcp      | 0.0.0.0/0 | ✔                |
| -1      | icmp     | 0.0.0.0/0 | ✔                |

2. Update Repositories. `sudo apt-get update`
3. Install G++ Compiler. `sudo apt-get install g++`
4. Install Make. `sudo apt-get install make`
5. Install Boost. `sudo apt-get install libboost-all-dev`
6. Install GMP. `sudo apt-get install libgmp-dev`
7. Install JSON Spirit. `sudo apt-get install libjson-spirit-dev`
8. Install NTL.
    1. Get from source. `wget http://www.shoup.net/ntl/ntl-9.8.1.tar.gz` (replace with newer package as needed)
    2. Unpackage. `gunzip ntl-9.8.1.tar.gz; tar xf ntl-9.8.1.tar`
    3. Change Directory. `cd ntl-9.8.1/src`
    4. Compile with flags. `./configure NTL_THREADS=on NTL_GMP_LIP=on NTL_THREAD_BOOST=on SHARED=on`
    5. Make. `make` (takes some time)
    6. Make Install. `make install`
9. Build Server and Client. `cd stadium/groth_shuffle; make client server`
10. You can now run a server and client as follows,

```
./shuffle_server <PORT> <PORT> 1
```
```
./shuffle_client <SERVER_ADDRESS>:<PORT>
``` 

*You can complete steps 2-9 with this single command.*
```
sudo apt-get update; sudo apt-get install g++; sudo apt-get install make; sudo apt-get install libboost-all-dev; sudo apt-get install libgmp-dev; sudo apt-get install libjson-spirit-dev; wget http://www.shoup.net/ntl/ntl-9.8.1.tar.gz; gunzip ntl-9.8.1.tar.gz; tar xf ntl-9.8.1.tar; cd ntl-9.8.1/src; ./configure NTL_THREADS=on NTL_GMP_LIP=on NTL_THREAD_BOOST=on; make; sudo make install; cd ../..; cd stadium/groth_shuffle; make client server
```

## Starting the Stadium System

1. Set up your go workspace with src, bin, and pkg directories as outlined here https://golang.org/doc/code.html 
2. Have stadium cloned inside the src directory
3. Run `go install stadium/stadium` to install stadium package (may need to run go get github.com/Sirupsen/logrus)
4. Run `go install stadium/server` and `go install stadium/coordinator` to create binaries server and coordinator in gowork/bin

    - If you have gowork/bin in path (otherwise cd into gowork/bin) run:
 
 ```shell
server -conf config/three-server.conf -id 0
```
```shell
server -conf config/three-server.conf -id 1
 ```
 ```shell
server -conf config/three-server.conf -id 2
 ```
 ```shell
coordinator -conf config/three-server.conf
 ```

## Testing

Run the following command in any directory with tests to execute the tests. 

```shell
go test -v . | sed ''/PASS/s//$(printf "\033[32mPASS\033[0m")/'' | sed ''/FAIL/s//$(printf "\033[31mFAIL\033[0m")/''
```

## Benchmarking the shuffle

(These instructions work for a fresh Ubuntu 14.04 machine.)

Steps for benchmarking the `groth` crypto library:

1. Installing from `apt`

```shell
sudo apt-get install git, make, g++, libssl-dev, libgmp3-dev, libboost-all-dev, libtool
```

2. Installing `libtool`

```shell
wget http://www.shoup.net/ntl/ntl-9.8.1.tar.gz
gunzip ntl-9.8.1.tar.gz
tar xf ntl-9.8.1.tar
cd ntl-9.8.1/src
./configure NTL_THREADS=on NTL_GMP_LIP=on NTL_THREAD_BOOST=on SHARED=on
make
sudo make install
```

Also, add NTL's path to the linker. Open up your `bash` configuration profile (probalby `.bashrc`), and append the following line to the bottom:
```shell
export LD_LIBRARY_PATH=/usr/local/lib/
```

3. Running the benchmark

Go to `groth/` and run `make test`. This will produce the test, which you can run with `./test`.

## Benchmarking on AWS

Helper scripts you may find useful lie inside `aws/`. Note that you will need to configure your own AWS account and certificates to run the benchmarks.

### Notes

Do *not* install `libntl-dev` from `apt`! That version of the library is not thread-safe, which will cause mysterious crashes. Make sure to build it from source.
