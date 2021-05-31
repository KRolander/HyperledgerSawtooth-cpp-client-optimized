/**
 * @file app.cpp
 * @date 05/2020
 * @author Luc Gerrits <luc.gerrits@univ-cotedazur.fr>
 * @brief C++ client intkey transaction processor.
 * @version 0.1
 */

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdio.h>
#include <curl/curl.h>
#include <assert.h>
#include <string>
#include <sys/stat.h>
#include <chrono>

#include "json/single_include/nlohmann/json.hpp"
using json = nlohmann::json;

#include "sawtooth_client.h"

void showParseError(char *argv[])
{
    std::cout << "Hyperledger Sawtooth cpp client using intkey transaction processor. \n"
              << "Usage: " << argv[0] << " <option(s)>\n"
              << "Options:\n"
              << "\t-h,--help\t\tShow this help message\n"
              << "\t--push <set, inc, dec>  \t\tBuild transaction (set, inc or dec 'foo' integer) AND send it\n"
              << "\t--dry-run\t\tBuild transaction BUT don't send it\n"
              << std::endl;
}
int parseArgs(int argc, char *argv[], std::string &command, std::string &subcommand)
{
    if (argc < 2)
    {
        showParseError(argv);
        return 1;
    }
    else
    {
        if (strcmp(argv[1], "--push") == 0)
        {
            if (argc < 3)
            {
                showParseError(argv);
                return 1;
            }
            command = "push";
            if (strcmp(argv[2], "set") == 0)
            {
                subcommand = "set";
            }
            else if (strcmp(argv[2], "inc") == 0)
            {
                subcommand = "inc";
            }
            else if (strcmp(argv[2], "dec") == 0)
            {
                subcommand = "dec";
            }
            else
            {
                showParseError(argv);
                return 1;
            }
        }
        else if (strcmp(argv[1], "--dry-run") == 0)
        {
            command = "dry-run";
        }
        else
        {
            showParseError(argv);
            return 1;
        }
        return 0;
    }
}

void init_transaction(json &payload,
                      SawtoothKeys &TnxKeys,
                      SawtoothMessage &TnxMsg,
                      std::string subcommand)
{
    std::cout << "***Loading keys***" << std::endl;
    //Change in .h if you want auto generated keys


    // TODO with trezor crypto
    


    hexStringToUint8_t(TnxKeys.privateKey, &TnxKeys.privKey[0], PRIVATE_KEY_SIZE);

    //HexStrToUchar(TnxKeys.privateKey, TnxKeys.privKey.c_str(), PRIVATE_KEY_SIZE);
    
    hexStringToUint8_t(TnxKeys.publicKey_serilized, &TnxKeys.pubKey[0], PUBLIC_KEY_SERILIZED_SIZE);
    
    
    // HexStrToUchar(TnxKeys.publicKey_serilized, TnxKeys.pubKey.c_str(), PUBLIC_KEY_SERILIZED_SIZE);

    // if (LOAD_DEFAULT_KEYS)
    // {
    //     //load default keys
    //     CHECK(LoadKeys(ctx, TnxKeys) == 1);
    // }
    // else
    // {
    //     std::cout << "***Generating keys***" << std::endl;
    //     GenerateKeyPair(ctx, TnxKeys);
    //     std::cout << std::left;
    //     std::cout << std::setw(15) << "Private Key: " << TnxKeys.privKey << std::endl;
    //     std::cout << std::setw(15) << "Public Key: " << TnxKeys.pubKey << std::endl;
    // }

    std::cout << "***Build payload***" << std::endl;
    payload["Verb"] = subcommand;
    payload["Name"] = "foo";
    payload["Value"] = 1;
    std::vector<uint8_t> payload_vect = json::to_cbor(payload);
    std::string payload_str(payload_vect.begin(), payload_vect.end());
    TnxMsg.message = payload_str; //set final message
}

void build_signature(json &payload,
                     SawtoothKeys &TnxKeys,
                     SawtoothMessage &TnxMsg,
                     SawtoothSigature &TnxSig,
                     std::string &data_to_send)
{
    std::cout << "***Start build transaction***" << std::endl;
    //init Batch
    BatchList myBatchList;
    //init batch list
    Batch *myBatch = myBatchList.add_batches(); //init the one batch that will be sent
    BatchHeader myBatchHeader;                  //init batch header
    //init Transaction
    Transaction *myTransaction = myBatch->add_transactions(); //init the one transaction that will be sent
    TransactionHeader myTransactionHeader;                    //init transaction header

    size_t nonce_size = 10;
    unsigned char Transactionnonce[nonce_size];
    generateRandomBytes(Transactionnonce, nonce_size);
    std::string TxnNonce = "7a7a7a7a7a7a7a7a7a7a7a7a7a7a7a7a7a7a7a7a";//UcharToHexStr(Transactionnonce, nonce_size);

    TnxMsg.message_hash_str = sha512Data(TnxMsg.message);
    unsigned char address[35];
    buildIntkeyAddress("intkey", payload["Name"].get<std::string>(), address);
    const std::string address_str = UcharToHexStr(address, 35);

    //first build transaction
    //& add all necessary data to protos messages
    std::cout << "Setting transaction header..." << std::endl;
    myTransactionHeader.Clear();
    myTransactionHeader.set_batcher_public_key(TnxKeys.pubKey);      //set batcher pubkey
    myTransactionHeader.set_signer_public_key(TnxKeys.pubKey);       //set signer pubkey
    myTransactionHeader.set_family_name("intkey");                   //the transaction familly to use
    myTransactionHeader.set_family_version("1.0");                   //familly version
    myTransactionHeader.set_payload_sha512(TnxMsg.message_hash_str); //set a hash sha512 of the payload
    myTransactionHeader.add_inputs(address_str);                     //1cf126cc488cca4cc3565a876f6040f8b73a7b92475be1d0b1bc453f6140fba7183b9a
    myTransactionHeader.add_outputs(address_str);
    myTransactionHeader.set_nonce(TxnNonce); //set nonce of the transaction
    //done transaction header
    myTransaction->Clear();
    std::cout << "Setting transaction..." << std::endl;
    myTransaction->set_payload(TnxMsg.message);
    myTransaction->set_header(myTransactionHeader.SerializePartialAsString());               //build a string of the transaction header
    std::string myTransactionHeader_string = myTransactionHeader.SerializePartialAsString(); //serialize batch header to string

    std::cout << "Signing transaction header..." << std::endl;
    TnxMsg.message_hash_str = sha256Data(myTransactionHeader_string);
    if (VERBOSE)
        std::cout << "message_hash_str: " << TnxMsg.message_hash_str << std::endl;


    hexStringToUint8_t(TnxMsg.message_hash_char, &TnxMsg.message_hash_str[0], HASH_SHA256_SIZE);



    // HexStrToUchar(TnxMsg.message_hash_char, TnxMsg.message_hash_str.c_str(), (size_t)HASH_SHA256_SIZE);
    
    int recid[1] = {0};
    SignTresor((uint8_t *) TnxMsg.message_hash_char, TnxSig.signature_serilized, (uint8_t*) TnxKeys.privateKey , recid);
    int i;
    std::cout << "Trezor Crypto signature Transaction: " << std::endl;
    std::cout << "r : " << std::endl;
    for(i=0; i<32; i++)
    {
        std::cout << std::hex << (uint32_t) TnxSig.signature_serilized[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "s : " << std::endl;
    for(i=32; i<64; i++)
    {
        std::cout << std::hex << (uint32_t) TnxSig.signature_serilized[i] << " ";
    }
    std::cout << std::endl;

    // CHECK(SECP256K1_API::secp256k1_ecdsa_sign(ctx, &TnxSig.signature, TnxMsg.message_hash_char, TnxKeys.privateKey, NULL, NULL) == 1); //make signature
    // CHECK(SECP256K1_API::secp256k1_ecdsa_signature_serialize_compact(ctx, TnxSig.signature_serilized, &TnxSig.signature) == 1);

    // std::cout << "Secp256k1 signature Trnasaction: " << std::endl;
    // std::cout << "r : " << std::endl;
    // for(i=0; i<32; i++)
    // {
    //     std::cout << std::hex << (uint32_t) TnxSig.signature_serilized[i] << " ";
    // }
    // std::cout << std::endl;
    // std::cout << "s : " << std::endl;
    // for(i=32; i<64; i++)
    // {
    //     std::cout << std::hex << (uint32_t) TnxSig.signature_serilized[i] << " ";
    // }
    // std::cout << std::endl;
 
    // std::string serializedSignatureTrezorFirst = UcharToHexStr(signatureTrezor, SIGNATURE_SERILIZED_SIZE);
    
    // std::cout << "Trezor serialized : " << serializedSignatureTrezorFirst << std::endl;

    TnxSig.signature_serilized_str = UcharToHexStr(TnxSig.signature_serilized, SIGNATURE_SERILIZED_SIZE);
    std::cout << "Trezor serialized : " << TnxSig.signature_serilized_str << std::endl;
   

    myTransaction->set_header_signature(TnxSig.signature_serilized_str); //set header signature

    //done transaction

    //add transaction to batch header
    std::cout << "Setting batch header..." << std::endl;
    myBatchHeader.add_transaction_ids(TnxSig.signature_serilized_str); //add transaction to batch

    //build batch
    myBatchHeader.set_signer_public_key(TnxKeys.pubKey); //set batch public key
    //myBatchHeader.SerializeToOstream()
    std::string myBatchHeader_string = myBatchHeader.SerializePartialAsString(); //serialize batch header to string
    std::cout << "Setting batch..." << std::endl;
    myBatch->set_header(myBatchHeader_string); //set header

    std::cout << "Signing batch header..." << std::endl;
    TnxMsg.message_hash_str = sha256Data(myBatchHeader_string);


    hexStringToUint8_t(TnxMsg.message_hash_char, &TnxMsg.message_hash_str[0], HASH_SHA256_SIZE);

    // HexStrToUchar(TnxMsg.message_hash_char, TnxMsg.message_hash_str.c_str(), (size_t)HASH_SHA256_SIZE);


    SignTresor((uint8_t *) TnxMsg.message_hash_char, TnxSig.signature_serilized, (uint8_t*) TnxKeys.privateKey , recid);


    // CHECK(SECP256K1_API::secp256k1_ecdsa_sign(ctx, &TnxSig.signature, TnxMsg.message_hash_char, TnxKeys.privateKey, NULL, NULL) == 1); //make signature
    // CHECK(SECP256K1_API::secp256k1_ecdsa_signature_serialize_compact(ctx, TnxSig.signature_serilized, &TnxSig.signature) == 1);

    // std::cout << "Secp256k1 signature BatchHeader: " << std::endl;
    // std::cout << "r : " << std::endl;
    // for(i=0; i<32; i++)
    // {
    //     std::cout << std::hex << (uint32_t) TnxSig.signature_serilized[i] << " ";
    // }
    // std::cout << std::endl;
    // std::cout << "s : " << std::endl;
    // for(i=32; i<64; i++)
    // {
    //     std::cout << std::hex << (uint32_t) TnxSig.signature_serilized[i] << " ";
    // }
    // std::cout << std::endl;

    // std::string serializedSigTrezor = UcharToHexStr(signatureTrezor, SIGNATURE_SERILIZED_SIZE);
    // std::cout << "Trezor serialized : " << serializedSigTrezor << std::endl;

    TnxSig.signature_serilized_str = UcharToHexStr(TnxSig.signature_serilized, SIGNATURE_SERILIZED_SIZE);
    std::cout << "Trezor serialized : " << TnxSig.signature_serilized_str << std::endl;

    myBatch->set_header_signature(TnxSig.signature_serilized_str);

    data_to_send = myBatchList.SerializePartialAsString();
    // voiddata_to_send_full_serialized
    // std::string data_to_send_full_serialized


    size_t dataSize = myBatchList.ByteSizeLong(); 
    uint32_t tabData[dataSize];

    myBatchList.SerializeToArray(tabData, dataSize);
    std::cout << "***Done build transaction***" << std::endl;
    std::cout << " Transaction to send : " << std::endl;
    // std::cout << data_to_send_full_serialized << std::endl;
    for(i=0; i<dataSize; i++)
    {
        std::cout << std::hex << (uint32_t) tabData[i] << " ";
    }
    std::cout << std::endl;

}

void send_transaction(std::string data_to_send, std::string command)
{
    //send transaction
    if (command == "push")
    {
        sendData(data_to_send, std::string(SAWTOOTH_REST_API) + "/batches");
    }
    else
    {
        std::cout << "Mode: " << command << std::endl;
    }
}

// void clear_program(SECP256K1_API::secp256k1_context *ctx)
// {
//     SECP256K1_API::secp256k1_context_destroy(ctx);
// }
//////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    // std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    // std::chrono::steady_clock::time_point end;
    std::string command = "", subcommand = "";
    if (int exit = parseArgs(argc, argv, command, subcommand))
        return exit;

    std::cout << "***Initialization***" << std::endl;
    // static SECP256K1_API::secp256k1_context *ctx;
    json payload;

    SawtoothMessage TnxMsg;
    SawtoothKeys TnxKeys;
    SawtoothSigature TnxSig;

    init_transaction(payload, TnxKeys, TnxMsg, subcommand);

    std::string data_to_send;
    build_signature(payload, TnxKeys, TnxMsg, TnxSig, data_to_send);

    //send_transaction(data_to_send, command);

    // end = std::chrono::steady_clock::now();
    // std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
    //           << "[µs]" << std::endl;
    // clear_program(ctx);

    return 0;
}