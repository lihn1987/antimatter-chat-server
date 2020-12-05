#ifndef KEYTOOL_H
#define KEYTOOL_H
#include <string>
#include <array>
extern const uint32_t PRI_KEY_SIZE;
extern const uint32_t PUB_KEY_SIZE;
extern const uint32_t HASH_SIZE;
extern const uint32_t SIGN_SIZE;
extern const uint32_t AES_IV_SIZE;
extern const uint32_t AES_KEY_SIZE;

/**
 * @brief Create a random number of length len
 * @param len The length of the random number
 * @return random number
 */
std::string CreateCustomRandom(int len);

/**
 * @brief creates a private key
 * @return private key (binary)
 */
std::string CreatePrivateKey();

/**
 * @brief calculates the public key through the private key
 * @param private_key_org private key (binary)
 * @return public key (binary)
 */
std::string GetPublicKeyByPrivateKey(const std::string private_key_org);

/**
 * @brief uses a private key to sign a piece of memory data
 * @param buf memory address
 * @param Length of memory
 * @return random number
 */
std::string GetSignByPrivateKey(const uint8_t* buf, size_t length, const std::string pri_key);

/**
 * @brief Get the shared password that only both parties know
 * @param pub_key counterparty public key (binary)
 * @param pri_key own private key (binary)
 * @return shared secret key (binary)
 */
std::string GetEcdhKey(const std::string& pub_key, const std::string& pri_key);

/**
 * @brief uses the public key to determine whether the signature is correct
 * @param buf signed data address
 * @param length The length of the signed data
 * @param pub_key The public key (binary) of the signature to be verified
 * @param sign The signature to be verified (binary)
 * @return is the signature correct
 */
bool SignIsValidate(const uint8_t* buf, size_t length, const std::string& pub_key, const std::string& sign);

/**
 * @brief uses the public key to determine whether the signature is correct
 * @param buf signed data 
 * @param pub_key The public key (binary) of the signature to be verified
 * @param sign The signature to be verified (binary)
 * @return is the signature correct
 */
bool SignIsValidate(const std::string& buf, const std::string& pub_key, const std::string& sign);

/**
 * @brief creates an AES IV vector
 * @return IV vector (binary)
 */
std::string CreateAesIVKey();

/**
 * @brief uses AES for encryption
 * @param key encryption password (binary)
 * @param iv AES IV vector (binary)
 * @param in the data to be encrypted (binary)
 * @param[out] out Encrypted data (binary)
 * @return Whether the encryption is successful
 */
bool AesEncode(const std::string& key, const std::string& iv, const std::string& in, std::string& out);

/**
 * @brief uses AES for decryption
 * @param key decrypt password (binary)
 * @param iv AES IV vector (binary)
 * @param in the data to be decrypted (binary)
 * @param[out] out The decrypted data (binary)
 * @return Whether the decryption is successful (normal use returns success, even if the password is wrong, a piece of garbled code will be solved)
 */
bool AesDecode(const std::string& key, const std::string& iv, const std::string& in, std::string& out);
uint64_t GetHash(const std::string& str);
#endif // KEYTOOL_H
