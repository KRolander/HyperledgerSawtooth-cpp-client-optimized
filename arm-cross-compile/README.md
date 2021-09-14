# Cross-compile for ARM

This file contains the Makefile for ARM cross-compile, in the Makefile spcify to the variable `CXX` the path of arm-linux-gnueabi-g++ cross-compile tool. 

This file also helps how to cross-compile the CryptoPP, Secp256k1, Trezor-Crypto, Curl and Google Protocol buffer libraries. 

It must be noted that all of the cross-compiled librarires (libx.a) must be copied next to the Makefile which is intended to compile the API. 

## Compile Curl for ARM architecture

Compile to obtain a static library (libcurl.a)


```bash
cd curl
./configure --prefix=${WORK_SPACE}/build CC=arm-linux-gnueabi-gcc-7 --host=arm-linux-gnueabi --disable-shared --enable-static
```



# Compile protobufs for ARM architecture

```bash
cd protobuf
./configure --prefix=$(pwd)/.libs/ --disable-shared  --host=arm-linux CC=arm-linux-gnueabi-gcc CXX=arm-linux-gnueabi-g++ --with-protoc=/src/protoc
```

It is possible that the --prefix does not work fine, in this case dependencies are in protobuf/src, and libprotobuf.a is in protobuf/src/.libs



# Compile CryptoPP for ARM architecture

In setenv-embedded.sh you should set the PATH to the arm-linux-gnueabi tools.

```bash
cd cryptopp
source ./setenv-embedded.sh 
make -f GNUmakefile-cross
```

If you have this error:

```bash
GNUmakefile-cross:774 : la recette pour la cible « cryptest.exe » a échouée
```
There is no worry the library licryptopp.a would exist, only cryptest.exe would not exist, but it doesn't necessary in this context.

You can find all of the instructions [Here](https://www.cryptopp.com/wiki/ARM_Embedded_(Command_Line))




# Compile secp256k1 for ARM architecture

```bash
cd secp256k1
./configure CC=arm-linux-gnueabi-gcc-5 --host=arm --enable-exhaustive-tests=no
```

# Compile Trezor-Crypto for ARM architecture

```bash
cd trezorCrypto
```
In the Makefile modify `CC` = to the path to arm-linux-gnueabi-gcc