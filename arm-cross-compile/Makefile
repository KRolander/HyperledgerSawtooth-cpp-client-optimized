
CXX = /home/kromes/Documents/Kromes/Qbox_SystemC/Linux_Kernel/gcc-arm-8.2-2018.11-x86_64-arm-linux-gnueabi/bin/arm-linux-gnueabi-g++


libs = -L. -lcurl 
libs += -Lprotobuf/src/.libs
libs += -lprotobuf
# libs += -lprotoc
libs += #-lprofiler

includes = -I . 
includes += -I secp256k1/includes
includes += -I protos_pb_h/ -Iprotobuf/src
# includes += -I protobuf/.libs/include
#######objects for main prog:
app_objects = app.cpp
app_objects += sawtooth_client.o
app_objects += ./protos_pb_h/transaction.pb.cc ./protos_pb_h/batch.pb.cc ./protos_pb_h/seth.pb.cc

app_static_lib = -L/cryptopp -lcryptopp
# app_static_lib = /media/kromes/Elements/Xilinx-SystemC-QEMU/demo/zynq-demo/Zynq_7000_PwClkARCH/devices/Power_Manager_Device/IP_test_build/v2/test_with_cryptopp/cryptopp/libcryptopp.a
app_static_lib +=  -L. -lsecp256k1
# app_static_lib += secp256k1/.libs/libsecp256k1.a

client_static_lib = -l:protobuf/src/.libs/libprotobuf.a
client_static_lib += -l:protobuf/src/.libs/libprotoc.a

#some flags
flag_global = -O2 -std=c++11  -g #eosio is on standart c++17  -pg 
flag_main = -Wall -pthread


SRCS   = trezorCrypto/bignum.c trezorCrypto/ecdsa.c trezorCrypto/curves.c trezorCrypto/secp256k1.c trezorCrypto/nist256p1.c trezorCrypto/rand.c trezorCrypto/hmac.c trezorCrypto/bip32.c trezorCrypto/bip39.c trezorCrypto/pbkdf2.c trezorCrypto/base58.c trezorCrypto/base32.c
SRCS  += trezorCrypto/address.c
SRCS  += trezorCrypto/script.c
SRCS  += trezorCrypto/ripemd160.c
SRCS  += trezorCrypto/sha2.c
SRCS  += trezorCrypto/sha3.c
SRCS  += trezorCrypto/hasher.c
SRCS  += trezorCrypto/aes/aescrypt.c trezorCrypto/aes/aeskey.c trezorCrypto/aes/aestab.c trezorCrypto/aes/aes_modes.c
SRCS  += trezorCrypto/ed25519-donna/curve25519-donna-32bit.c trezorCrypto/ed25519-donna/curve25519-donna-helpers.c trezorCrypto/ed25519-donna/modm-donna-32bit.c
SRCS  += trezorCrypto/ed25519-donna/ed25519-donna-basepoint-table.c trezorCrypto/ed25519-donna/ed25519-donna-32bit-tables.c trezorCrypto/ed25519-donna/ed25519-donna-impl-base.c
SRCS  += trezorCrypto/ed25519-donna/ed25519.c trezorCrypto/ed25519-donna/curve25519-donna-scalarmult-base.c trezorCrypto/ed25519-donna/ed25519-sha3.c trezorCrypto/ed25519-donna/ed25519-keccak.c
SRCS  += trezorCrypto/monero/base58.c
SRCS  += trezorCrypto/monero/serialize.c
SRCS  += trezorCrypto/monero/xmr.c
SRCS  += trezorCrypto/monero/range_proof.c
SRCS  += trezorCrypto/blake256.c
SRCS  += trezorCrypto/blake2b.c trezorCrypto/blake2s.c
SRCS  += trezorCrypto/chacha_drbg.c
SRCS  += trezorCrypto/groestl.c
SRCS  += trezorCrypto/chacha20poly1305/chacha20poly1305.c trezorCrypto/chacha20poly1305/chacha_merged.c trezorCrypto/chacha20poly1305/poly1305-donna.c trezorCrypto/chacha20poly1305/rfc7539.c
SRCS  += trezorCrypto/rc4.c
SRCS  += trezorCrypto/nem.c
SRCS  += trezorCrypto/segwit_addr.c trezorCrypto/cash_addr.c
SRCS  += trezorCrypto/memzero.c
SRCS  += trezorCrypto/shamir.c
SRCS  += trezorCrypto/hmac_drbg.c
SRCS  += trezorCrypto/rfc6979.c
SRCS  += trezorCrypto/slip39.c

OBJS   = $(SRCS:.c=.o)

all: app

app: $(app_objects)
	$(CXX) $(flag_global) $(flag_main) -DNDEBUG $(app_objects) $(OBJS) $(libs) $(includes) -o app $(app_static_lib) -static

sawtooth_client.o: sawtooth_client.cpp sawtooth_client.h protos_pb_h/transaction.pb.h
	$(CXX) $(flag_global) $(libs) $(includes) -c sawtooth_client.cpp $(client_static_lib)  -o sawtooth_client.o 

protos_pb_h/transaction.pb.h: protos/transaction.proto
	mkdir -p protos_pb_h &&  ./protobuf/src/protoc --proto_path=protos --cpp_out=protos_pb_h/ protos/*

clean: 
	rm -fr app
	rm -fr *.o *.out.* *.out *.stats *.a protos_pb_h
