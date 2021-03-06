#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <curl/curl.h>

#include "sawtooth_client.h"

//https://stackoverflow.com/a/14051107/11697589
//https://stackoverflow.com/questions/7639656/getting-a-buffer-into-a-stringstream-in-hex-representation/7639754#7639754
std::string UcharToHexStr(unsigned char *data, int len)
{
    //this was first:
    // std::stringstream ss;
    // for (int i = 0; i < data_length; ++i)
    //     ss << std::hex << (int)data[i];
    // std::string mystr = ss.str();

    //the following is better: IT FILLS WITH 0 !!!!
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < len; ++i)
        ss << std::setw(2) << static_cast<unsigned>(data[i]);
    return ss.str();
}
//http://www.cplusplus.com/forum/general/53397/
int chhex(char ch)
{
    if (isdigit(ch))
        return ch - '0';
    if (tolower(ch) >= 'a' && tolower(ch) <= 'f')
        return ch - 'a' + 10;
    return -1;
}
void HexStrToUchar(unsigned char *dest, const char *source, int bytes_n)
{
    for (bytes_n--; bytes_n >= 0; bytes_n--)
        dest[bytes_n] = 16 * chhex(source[bytes_n * 2]) + chhex(source[bytes_n * 2 + 1]);
}
void generateRandomBytes(unsigned char *key, int length)
{
    AutoSeededRandomPool rng;
    rng.GenerateBlock(key, length);
}

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

std::string sha512Data(std::string data)
{
    /////////////////////////////////////////////
    //Create a SHA-512 data Hash
    std::vector<uint8_t> message_vect(data.begin(), data.end());
    std::string digest_hex;
    HexEncoder encoder(new StringSink(digest_hex), false);
    std::string digest;

    SHA512 hash;
    hash.Update(message_vect.data(), message_vect.size());
    digest.resize(hash.DigestSize());
    hash.Final((unsigned char *)&digest[0]);
    StringSource ss(digest, true, new Redirector(encoder));
    return digest_hex;
}

void emptyBytes(unsigned char *data, int len)
{
    for (int i(0); i < len; ++i)
        data[i] = 0x00;
}
std::string ToHex(std::string s, bool upper_case = false)
{
    std::ostringstream ret;

    for (std::string::size_type i = 0; i < s.length(); ++i)
        ret << std::hex << std::setfill('0') << std::setw(2) << (upper_case ? std::uppercase : std::nouppercase) << (int)s[i];

    return ret.str();
}
void buildIntkeyAddress(std::string txnFamily, std::string entryName, unsigned char *ouput35bytes)
{
    //this address is used for intkey transaction processor
    // Example: txnFamily="intkey", entryName="name"
    //Doc: https://sawtooth.hyperledger.org/docs/core/releases/latest/app_developers_guide/address_and_namespace.html#address-components
    emptyBytes(ouput35bytes, 35);
    //build prefix namespace: first set the first 3 bytes
    std::string txnFamily_hex_str = sha512Data(txnFamily);
    unsigned char txnFamily_hex_char[6];

    hexStringToUint8_t(txnFamily_hex_char, &txnFamily_hex_str[0], 6);
     

    // HexStrToUchar(txnFamily_hex_char, txnFamily_hex_str.c_str(), 6);
    for (int i = 0; i < 3; i++)
    {
        ouput35bytes[i] = txnFamily_hex_char[i];
    }
    //now add the rest of the address: for intkey it is the 32bytes of the LSB of the sha512 of the key
    std::string entryName_hex_str = sha512Data(entryName);
    // std::cout << "First Here the extra string " << std::endl;
    std::cout << entryName_hex_str << std::endl;
    entryName_hex_str = entryName_hex_str.substr(entryName_hex_str.size() - 64, entryName_hex_str.size());
    unsigned char entryName_hex_char[32];

    hexStringToUint8_t((uint8_t *)entryName_hex_char, &entryName_hex_str[0], 32);
    // std::cout << "Here the extra string " << std::endl;
    // std::cout << entryName_hex_str << std::endl;
    // std::cout << "hexStringToUint8_t : " << std::endl; 
    // int k;
    // for(k=0; k<32; k++)
    //     std::cout << std::hex << (uint32_t) entryName_hex_char[k] << ' ';
    // std::cout << std::endl; 
    
    // HexStrToUchar(entryName_hex_char, entryName_hex_str.c_str(), 64);
    //  std::cout << "HexStrToUchar : " << std::endl;
    // for(k=0; k<64; k++)
    //     std::cout << std::hex << (uint32_t) entryName_hex_char[k] << " ";
    // std::cout << std::endl;  

    
    for (int i = 0; i < 32; i++)
    {
        ouput35bytes[3 + i] = entryName_hex_char[i];
    }
    //std::cout << "Address:" << UcharToHexStr(ouput35bytes, 35) << std::endl;
}

std::string here = "6e282c41be5e4254d8820772c5518a2c5a8c0c7f7eda19594a7eb539453e1ed7";

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

struct WriteThis
{
    const char *readptr;
    size_t sizeleft;
};
static size_t read_callback(void *dest, size_t size, size_t nmemb, void *userp)
{
    struct WriteThis *wt = (struct WriteThis *)userp;
    size_t buffer_size = size * nmemb;

    if (wt->sizeleft)
    {
        /* copy as much as possible from the source to the destination */
        size_t copy_this_much = wt->sizeleft;
        if (copy_this_much > buffer_size)
            copy_this_much = buffer_size;
        memcpy(dest, wt->readptr, copy_this_much);

        wt->readptr += copy_this_much;
        wt->sizeleft -= copy_this_much;
        return copy_this_much; /* we copied this many bytes */
    }

    return 0; /* no more data left to deliver */
}
int sendData(std::string data, std::string api_endpoint)
{
    CURL *curl;
    CURLcode res;

    struct WriteThis wt;

    wt.readptr = data.c_str();
    wt.sizeleft = data.length();

    //std::cout << "Length data:" << (long)wt.sizeleft << std::endl;

    struct curl_slist *headers = NULL;
    std::string readBuffer;
    curl = curl_easy_init();
    if (curl)
    {

        curl_easy_setopt(curl, CURLOPT_URL, api_endpoint.c_str());
        headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
        curl_easy_setopt(curl, CURLOPT_READDATA, &wt);

        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, (long)wt.sizeleft);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        if (VERBOSE)
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (VERBOSE)
            std::cout << "***Transaction sended***" << std::endl;
        if (VERBOSE)
            std::cout << "CURL STATUS CODE:" << res << std::endl;
        std::cout << "Response:" << std::endl;
        std::cout << readBuffer << std::endl;
    }
    
    curl_global_cleanup();
    if (res != CURLE_OK)
    {
        std::cout << "CURL STATUS CODE:" << res << std::endl;
        std::cout << curl_easy_strerror(res) << std::endl;
        return 0;
    }
    else
    {
        return 1;
    }
}

// int LoadKeys(
//     SECP256K1_API::secp256k1_context *ctx,
//      SawtoothKeys &Keys)
// {
//     if (Keys.pubKey.length() > 0 && Keys.privKey.length() > 0)
//     {
//         //std::cout << "Using default private and public keys." << std::endl;
//         /* LOAD public keys */
//         {
//             unsigned char pubkey_char[PUBLIC_KEY_SERILIZED_SIZE];
//             HexStrToUchar(pubkey_char, Keys.pubKey.c_str(), (size_t)PUBLIC_KEY_SERILIZED_SIZE);
//             if (VERBOSE)
//                 std::cout << "Parse Public key:" << UcharToHexStr(pubkey_char, PUBLIC_KEY_SERILIZED_SIZE) << std::endl;
//             CHECK(SECP256K1_API::secp256k1_ec_pubkey_parse(ctx, &Keys.publicKey, pubkey_char, PUBLIC_KEY_SERILIZED_SIZE) == 1);
//             if (VERBOSE)
//                 std::cout << "Ok." << std::endl;
//         }
//         /* LOAD private keys */
//         {
//             HexStrToUchar(Keys.privateKey, Keys.privKey.c_str(), (size_t)PRIVATE_KEY_SIZE);
//             if (VERBOSE)
//                 std::cout << "Parse private key:" << UcharToHexStr(Keys.privateKey, PRIVATE_KEY_SIZE) << std::endl;
//             CHECK(SECP256K1_API::secp256k1_ec_seckey_verify(ctx, Keys.privateKey) == 1);
//             if (VERBOSE)
//                 std::cout << "Ok." << std::endl;
//         }
//         return 1;
//     }
//     else
//     {
//         std::cerr << "ERROR: No keys loaded. Please verify that you have given keys." << std::endl;
//         return 0;
//     }
// }

// int GenerateKeyPair(
//     SECP256K1_API::secp256k1_context *ctx,
//      SawtoothKeys &Keys)
// {
//     /* Generate a random key */
//     {
//         emptyBytes(Keys.privateKey, PRIVATE_KEY_SIZE);
//         generateRandomBytes(Keys.privateKey, PRIVATE_KEY_SIZE);
//         Keys.privKey = UcharToHexStr(Keys.privateKey, PRIVATE_KEY_SIZE);
//         if (VERBOSE)
//                 std::cout << "generatePrivateKey: " << Keys.privKey << std::endl;
//         while (SECP256K1_API::secp256k1_ec_seckey_verify(ctx, Keys.privateKey) == 0) //regenerate private key until it is valid
//         {
//             generateRandomBytes(Keys.privateKey, PRIVATE_KEY_SIZE);
//             Keys.privKey = UcharToHexStr(Keys.privateKey, PRIVATE_KEY_SIZE);
//             std::cout << "generatePrivateKey: " << Keys.privKey << std::endl;
//         }
//         CHECK(SECP256K1_API::secp256k1_ec_seckey_verify(ctx, Keys.privateKey) == 1);
//         if (VERBOSE)
//             std::cout << "Private key verified.\n->Using:" << Keys.privKey << std::endl;
//     }

//     /* Generate a public key */
//     {
//         emptyBytes(Keys.publicKey.data, PUBLIC_KEY_SIZE);
//         //FAILING:Segmentation fault
//         CHECK(SECP256K1_API::secp256k1_ec_pubkey_create(ctx, &Keys.publicKey, Keys.privateKey) == 1);
//         if (VERBOSE)
//             std::cout << "Public key verified." << std::endl;
//         if (VERBOSE)
//             std::cout << "->Using:" << UcharToHexStr(Keys.publicKey.data, PUBLIC_KEY_SIZE) << std::endl;
//     }

//     /* Serilize public key */
//     {
//         emptyBytes(Keys.publicKey_serilized, PUBLIC_KEY_SERILIZED_SIZE);
//         size_t pub_key_ser_size = PUBLIC_KEY_SERILIZED_SIZE;
//         CHECK(SECP256K1_API::secp256k1_ec_pubkey_serialize(ctx, Keys.publicKey_serilized, &pub_key_ser_size, &Keys.publicKey, SECP256K1_EC_COMPRESSED) == 1);
//         Keys.pubKey = UcharToHexStr(Keys.publicKey_serilized, PUBLIC_KEY_SERILIZED_SIZE);
//         if (VERBOSE)
//             std::cout << "Public key serilized ok." << std::endl;
//         if (VERBOSE)
//             std::cout << "->Using:" << Keys.pubKey << std::endl;
//     }
//     return 1;
// }

void SignTx(uint8_t *hash, uint8_t *sig, uint8_t * privateKey ,int *recid)
{
    const ecdsa_curve *curve = &secp256k1;
    uint8_t py;

    int ok = ecdsa_sign_digest(curve, privateKey, hash, sig, &py, NULL);

    recid[0] = py;
}

void bytes2hex(unsigned char *src, char *out, int len)
{
    while (len--)
    {
        *out++ = HexLookUp[*src >> 4];
        *out++ = HexLookUp[*src & 0x0F];
        src++;
    }
    *out = 0;
}


void hexStringToUint8_t(uint8_t *dest, const char *source, int bytes_n)
{

    int i;
    int j = 0;
    for (i = 0; i < bytes_n; i++)
    {
        if (source[j] == '0')
        {
            dest[i] = 0; //0x00
        }
        else if (source[j] == '1')
        {
            dest[i] = 16; // 0x10
        }
        else if (source[j] == '2')
        {
            dest[i] = 32; // 0x20
        }
        else if (source[j] == '3')
        {
            dest[i] = 48; // 0x30
        }
        else if (source[j] == '4')
        {
            dest[i] = 64; // 0x40
        }
        else if (source[j] == '5')
        {
            dest[i] = 80; // 0x50
        }
        else if (source[j] == '6')
        {
            dest[i] = 96; // 0x60
        }
        else if (source[j] == '7')
        {
            dest[i] = 112; // 0x70
        }
        else if (source[j] == '8')
        {
            dest[i] = 128; // 0x80
        }
        else if (source[j] == '9')
        {
            dest[i] = 144; // 0x90
        }
        else if (source[j] == 'a')
        {
            dest[i] = 160; // 0xa0
        }
        else if (source[j] == 'b')
        {
            dest[i] = 176; // 0xb0
        }
        else if (source[j] == 'c')
        {
            dest[i] = 192; // 0xc0
        }
        else if (source[j] == 'd')
        {
            dest[i] = 208; // 0xd0
        }
        else if (source[j] == 'e')
        {
            dest[i] = 224; // 0xe0
        }
        else if (source[j] == 'f')
        {
            dest[i] = 240; // 0xf0
        }
        else if (source[j] == 'A')
        {
            dest[i] = 160; // 0xa0
        }
        else if (source[j] == 'B')
        {
            dest[i] = 176; // 0xb0
        }
        else if (source[j] == 'C')
        {
            dest[i] = 192; // 0xc0
        }
        else if (source[j] == 'D')
        {
            dest[i] = 208; // 0xd0
        }
        else if (source[j] == 'E')
        {
            dest[i] = 224; // 0xe0
        }
        else if (source[j] == 'F')
        {
            dest[i] = 240; // 0xf0
        }

        j++;

        if (source[j] == '0')
        {
            dest[i] = (dest[i] | 0x00);
        }
        else if (source[j] == '1')
        {
            dest[i] = (dest[i] | 0x01);
        }
        else if (source[j] == '2')
        {
            dest[i] = (dest[i] | 0x02);
        }
        else if (source[j] == '3')
        {
            dest[i] = (dest[i] | 0x03);
        }
        else if (source[j] == '4')
        {
            dest[i] = (dest[i] | 0x04);
        }
        else if (source[j] == '5')
        {
            dest[i] = (dest[i] | 0x05);
        }
        else if (source[j] == '6')
        {
            dest[i] = (dest[i] | 0x06);
        }
        else if (source[j] == '7')
        {
            dest[i] = (dest[i] | 0x07);
        }
        else if (source[j] == '8')
        {
            dest[i] = (dest[i] | 0x08);
        }
        else if (source[j] == '9')
        {
            dest[i] = (dest[i] | 0x09);
        }
        else if (source[j] == 'a')
        {
            dest[i] = (dest[i] | 0x0a);
        }
        else if (source[j] == 'b')
        {
            dest[i] = (dest[i] | 0x0b);
        }
        else if (source[j] == 'c')
        {
            dest[i] = (dest[i] | 0x0c);
        }
        else if (source[j] == 'd')
        {
            dest[i] = (dest[i] | 0x0d);
        }
        else if (source[j] == 'e')
        {
            dest[i] = (dest[i] | 0x0e);
        }
        else if (source[j] == 'f')
        {
            dest[i] = (dest[i] | 0x0f);
        }
        else if (source[j] == 'A')
        {
            dest[i] = (dest[i] | 0x0a);
        }
        else if (source[j] == 'B')
        {
            dest[i] = (dest[i] | 0x0b);
        }
        else if (source[j] == 'C')
        {
            dest[i] = (dest[i] | 0x0c);
        }
        else if (source[j] == 'D')
        {
            dest[i] = (dest[i] | 0x0d);
        }
        else if (source[j] == 'E')
        {
            dest[i] = (dest[i] | 0x0e);
        }
        else if (source[j] == 'F')
        {
            dest[i] = (dest[i] | 0x0f);
        }

        j++;
    }
}