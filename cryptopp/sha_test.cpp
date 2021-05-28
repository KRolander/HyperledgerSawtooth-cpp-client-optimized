
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

#include "cryptopp/cryptlib.h"
#include "cryptopp/osrng.h"
using CryptoPP::AutoSeededRandomPool;
#include "cryptopp/hex.h"
using CryptoPP::HexEncoder;
#include "cryptopp/sha.h"
using CryptoPP::SHA256;
using CryptoPP::SHA512;
#include "cryptopp/filters.h"
using CryptoPP::Redirector;
using CryptoPP::StringSink;
using CryptoPP::StringSource;

std::string sha256Data(std::string data)
{
    /////////////////////////////////////////////
    //Create a SHA-512 data Hash
    std::vector<uint8_t> message_vect(data.begin(), data.end());
    std::string digest_hex;
    HexEncoder encoder(new StringSink(digest_hex), false);
    std::string digest;

    SHA256 hash;
    hash.Update(message_vect.data(), message_vect.size());
    digest.resize(hash.DigestSize());
    hash.Final((unsigned char *)&digest[0]);
    StringSource ss(digest, true, new Redirector(encoder));
    return digest_hex;
}
