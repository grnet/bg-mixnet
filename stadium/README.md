# stadium

# Stadium

This package contains the core components of the stadium system.

## Starting the Stadium System

1. Set up your goworkspace with src, bin, and pkg directories as outlined here https://golang.org/doc/code.html 
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

