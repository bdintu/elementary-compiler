# elementary-compiler

## Base from [WATlang Compiler](https://github.com/cybergartar/ce-kmitl58-compiller-elementary-compiler)

## Install
```sh
apt install gcc cmake flex bison
```

## Clone
```sh
git clone https://github.com/bdintu/elementary-compiler
cd elementary-compiler
```

## Build
```sh
mkdir build
cd build
cmake ..
make
```

## Run
```sh
./simple ../example/hello.simple
gcc -no-pie hello.s -o hello
./hello
```
