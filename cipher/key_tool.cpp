#include "key_tool.h"
#include <assert.h>
#include <random>
#include <string.h>
#include <iostream>
#include "string_tools.h"
#include "blake2/blake2.h"
#include "aes/aes.h"
#include "secp256k1/secp256k1.h"
#include "secp256k1/secp256k1_ecdh.h"
#include "secp256k1/secp256k1_recovery.h"
#include "../log/log.hpp"


const uint32_t PRI_KEY_SIZE = 32;
const uint32_t PUB_KEY_SIZE = PRI_KEY_SIZE*2+1;
const uint32_t HASH_SIZE = 32;
const uint32_t SIGN_SIZE = 64;
const uint32_t RECOVER_SIGN_SIZE = 65;
const uint32_t AES_IV_SIZE = AES_BLOCKSIZE;
const uint32_t AES_KEY_SIZE = AES256_KEYSIZE;

std::string CreateCustomRandom(int len)
{
    std::string rtn((size_t)(len+4), '\0');
    std::random_device rd;
    for(int i = 0; i < len; i += 4){
        if(i < len){
            *(uint32_t*)(rtn.data()+i) = rd();
        }
    }
    rtn.resize(len);
    return rtn;
}

std::string CreatePrivateKey()
{
    std::string rtn;
    std::string pub_key;
    rtn = CreateCustomRandom(32);
    return rtn;
}

std::string GetPublicKeyByPrivateKey(const std::string private_key){

    std::string rtn;
    secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY|SECP256K1_CONTEXT_SIGN);
    assert(ctx != nullptr);
    std::string vseed = CreateCustomRandom(32);
    bool ret = secp256k1_context_randomize(ctx, (uint8_t*)vseed.data());
    assert(ret);
    secp256k1_pubkey pubkey;
    ret = secp256k1_ec_pubkey_create(ctx, &pubkey, (uint8_t*)private_key.data());
    assert(ret);
    rtn.resize(PUB_KEY_SIZE, 0);
    size_t clen = PUB_KEY_SIZE;
    secp256k1_ec_pubkey_serialize(ctx, (uint8_t*)rtn.data(), &clen, &pubkey,  SECP256K1_EC_UNCOMPRESSED);
    if (ctx) {
        secp256k1_context_destroy(ctx);
    }
    return rtn;
}

bool SigHasLowR(secp256k1_context *secp256k1_context_sign, const secp256k1_ecdsa_signature* sig)
{
    unsigned char compact_sig[64];
    secp256k1_ecdsa_signature_serialize_compact(secp256k1_context_sign, compact_sig, sig);

    // In DER serialization, all values are interpreted as big-endian, signed integers. The highest bit in the integer indicates
    // its signed-ness; 0 is positive, 1 is negative. When the value is interpreted as a negative integer, it must be converted
    // to a positive value by prepending a 0x00 byte so that the highest bit is 0. We can avoid this prepending by ensuring that
    // our highest bit is always 0, and thus we must check that the first byte is less than 0x80.
    return compact_sig[0] < 0x80;
}

void static inline WriteLE32(unsigned char* ptr, uint16_t x)
{
    uint32_t v = x;
    memcpy(ptr, (char*)&v, 4);
}

std::string GetSignByPrivateKey(const uint8_t* buf, size_t length, const std::string pri_key){
    secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY|SECP256K1_CONTEXT_SIGN);
    assert(ctx != nullptr);
    std::string vseed = CreateCustomRandom(32);
    bool ret = secp256k1_context_randomize(ctx, (uint8_t*)vseed.data());
    assert(ret);

    assert(length == HASH_SIZE);

    secp256k1_ecdsa_recoverable_signature sig;
    ret = secp256k1_ecdsa_sign_recoverable(ctx, &sig, buf, (uint8_t*)pri_key.data(), secp256k1_nonce_function_rfc6979, nullptr);

    uint8_t output[RECOVER_SIGN_SIZE];
    int v;
    secp256k1_ecdsa_recoverable_signature_serialize_compact(ctx, output, &v, &sig);
    
    std::string rtn;
    rtn.resize(RECOVER_SIGN_SIZE);
    if (v == 0 || v == 27 || v == 31 || v == 35) {
        output[64] = 0x1b;
    } else if (v == 1 || v == 28 || v == 32 || v == 36) {
        output[64] = 0x1c;
    } else {
        output[64] = 0;
    }

    memcpy((void*)rtn.data(), output, RECOVER_SIGN_SIZE);
    if (ctx) {
        secp256k1_context_destroy(ctx);
    }

    return rtn;
}

bool SignIsValidate(const uint8_t* buf, size_t length, const std::string& pub_key, const std::string& sign){
    if (length != HASH_SIZE) {
        LogWarning("invalid hash len:"<<length);
        return false;
    }

    if (sign.size() != RECOVER_SIGN_SIZE) {
        LogWarning("invalid sign len:"<<sign.size());
        return false;
    }

    secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY);
    assert(ctx != nullptr);
    std::string vseed = CreateCustomRandom(32);
    bool ret = secp256k1_context_randomize(ctx, (uint8_t*)vseed.data());
    assert(ret);

    uint8_t v = sign[64];
    if (v >= 27 && v <= 30) {
        v -= 27;
    } else if (v >= 31 && v <= 34) {
        v -= 31;
    } else if (v >= 35 && v <= 38) {
        v -= 35;
    }

    secp256k1_ecdsa_recoverable_signature sig;
    int succ = secp256k1_ecdsa_recoverable_signature_parse_compact(ctx, &sig, (uint8_t*)sign.data(), v);
    if (!succ) {
        LogWarning("SignIsValidate parse sign error:"<<Byte2HexAsc(sign));
        return false;
    }
    
    secp256k1_pubkey recover_pub_key;
    int res = secp256k1_ecdsa_recover(ctx, &recover_pub_key, &sig, buf);

    uint8_t output[65];
    size_t output_len = 65;
    secp256k1_ec_pubkey_serialize(ctx, output, &output_len, &recover_pub_key, SECP256K1_EC_UNCOMPRESSED);
     
    if (ctx) {
        secp256k1_context_destroy(ctx);
    }

    bool result = false;
    do {
        if (pub_key.size() != output_len) {
            result = false;
            break;
        }
        result = (memcmp(pub_key.data(), output, output_len) == 0);
    } while (0);

    if (!result) {
        LogWarning("SignIsValidate failed, from pub key "<<Byte2HexAsc(pub_key)<<", recover pub key");
    }
    return result;
}

std::string CreateAesIVKey()
{
    return CreateCustomRandom(AES_BLOCKSIZE);
}

bool AesEncode(const std::string& key, const std::string& iv, const std::string& in, std::string &out)
{
    std::string key_use = key;
    if(iv.size() != AES_BLOCKSIZE){
        return false;
    }
    if(key_use.size() != AES256_KEYSIZE){
        key_use.resize(AES256_KEYSIZE, 0);
    }
    out.resize((in.size()+AES_BLOCKSIZE) - (in.size()+AES_BLOCKSIZE)%AES_BLOCKSIZE, 0);
    AES256CBCEncrypt enc((uint8_t*)key_use.data(), (uint8_t*)iv.data(), true);
    int len = enc.Encrypt((uint8_t*)in.data(), in.size(), (uint8_t*)out.data());
    return len == out.size();
}

bool AesDecode(const std::string &key, const std::string &iv, const std::string &in, std::string &out)
{
    out.resize((in.size()+AES_BLOCKSIZE) - (in.size()+AES_BLOCKSIZE)%AES_BLOCKSIZE, 0);
    std::string key_use = key;
    if(iv.size() != AES_BLOCKSIZE){
        return false;
    }
    if(key_use.size() != AES256_KEYSIZE){
        key_use.resize(AES256_KEYSIZE, 0);
    }
    
    AES256CBCDecrypt enc((uint8_t*)key_use.data(), (uint8_t*)iv.data(), true);
    int len = enc.Decrypt((uint8_t*)in.data(), in.size(), (uint8_t*)out.data());
    out.resize(len);
    return true;
}

std::string GetEcdhKey(const std::string &pub_key, const std::string &pri_key)
{

    if(pub_key.size() != PUB_KEY_SIZE || pri_key.size() != PRI_KEY_SIZE){
        return std::string();
    }

    secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY|SECP256K1_CONTEXT_SIGN);
    assert(ctx != nullptr);
    std::string vseed = CreateCustomRandom(32);
    bool ret = secp256k1_context_randomize(ctx, (uint8_t*)vseed.data());
    assert(ret);

    std::string rtn((size_t)(AES_KEY_SIZE), '\0');
    secp256k1_pubkey pubkey;
    secp256k1_ec_pubkey_parse(ctx, &pubkey, (uint8_t*)pub_key.data(), PUB_KEY_SIZE);
    secp256k1_ecdh(ctx, (uint8_t*)rtn.data(), &pubkey, (uint8_t*)pri_key.data(),NULL, NULL);

    if (ctx) {
        secp256k1_context_destroy(ctx);
    }
    return rtn;
}


uint64_t GetHash(const std::string &str)
{
    std::string rtn(8, 0);
    blake2b_state hash_state;
    blake2b_init(&hash_state, 8);
    blake2b_update(&hash_state, str.data(), str.size());
    blake2b_final(&hash_state, (char*)rtn.data(), rtn.size());
    return *(uint64_t*)rtn.data();
}