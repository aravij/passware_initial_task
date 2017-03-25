#include "check_password.h"

#include <algorithm>

#include <iostream>
#include <string>

#include "../util/gcry_exception.h"

CheckPassword::CheckPassword(const ParsedFile &cypherFile):
    cipher(),
    
    cipherTextSize(cypherFile.contentSize),
    sha256CheckSumSize(cypherFile.shaCheckSumSize),
    initialValueSize(cypherFile.initialValueSize),
    
    md5CheckSumSize(gcry_md_get_algo_dlen(GCRY_MD_MD5)),
    tripleDESKeySize(gcry_cipher_get_algo_keylen(GCRY_CIPHER_3DES)),
    
    cipherText(cypherFile.content),
    sha256CheckSum(cypherFile.shaCheckSum),
    initialValue(cypherFile.initialValue),
    
    originalText(new unsigned char[cipherTextSize]),
    md5HashResult(new unsigned char[md5CheckSumSize]),
    tripleDesKey(new unsigned char [tripleDESKeySize]),
    sha256HashResult(new unsigned char [sha256CheckSumSize])
{
    processGcryError(gcry_cipher_open(&cipher, GCRY_CIPHER_3DES, GCRY_CIPHER_MODE_CBC, 0));
}

CheckPassword::CheckPassword(const CheckPassword& otherCipher):
    cipher(),
    
    cipherTextSize(otherCipher.cipherTextSize),
    sha256CheckSumSize(otherCipher.sha256CheckSumSize),
    initialValueSize(otherCipher.initialValueSize),
    
    md5CheckSumSize(gcry_md_get_algo_dlen(GCRY_MD_MD5)),
    tripleDESKeySize(gcry_cipher_get_algo_keylen(GCRY_CIPHER_3DES)),
    
    cipherText(otherCipher.cipherText),
    sha256CheckSum(otherCipher.sha256CheckSum),
    initialValue(otherCipher.initialValue),
    
    // Even if we copy constructing, scoped_arrays have to initialize with its own buffers.
    originalText(new unsigned char[cipherTextSize]),
    md5HashResult(new unsigned char[md5CheckSumSize]),
    tripleDesKey(new unsigned char [tripleDESKeySize]),
    sha256HashResult(new unsigned char [sha256CheckSumSize])
{
    processGcryError(gcry_cipher_open(&cipher, GCRY_CIPHER_3DES, GCRY_CIPHER_MODE_CBC, 0));
}

CheckPassword::~CheckPassword()
{
    gcry_cipher_close(cipher);
}

bool CheckPassword::isPasswordAcceptable(const std::string& password)
{
    gcry_md_hash_buffer(GCRY_MD_MD5, md5HashResult.get(), password.c_str(), password.size());
    
    // MD5 algorithm output only 16 bytes, while 3DES use 24 bytes string as a key.
    // To overcome this, we uses EDE2 mode. Encrypting on phase 1 and 3 done with the same key.
    std::copy_n(md5HashResult.get(), md5CheckSumSize, tripleDesKey.get());
    std::copy_n(md5HashResult.get(), tripleDESKeySize - md5CheckSumSize, tripleDesKey.get() + md5CheckSumSize);
    
    processGcryError(gcry_cipher_setkey(cipher, tripleDesKey.get(), tripleDESKeySize));
    
    // Setting initial value is needed before each decryption, or libgcrypt will consider
    // that we decrypt one ciphertext by large blocks and will use initial last block
    // from previous decryption for next decryption.
    processGcryError(gcry_cipher_setiv(cipher, initialValue.get(), initialValueSize));
    
    processGcryError(gcry_cipher_decrypt(cipher, originalText.get(), cipherTextSize, cipherText.get(), cipherTextSize));
   
    gcry_md_hash_buffer(GCRY_MD_SHA256, sha256HashResult.get(), originalText.get(), cipherTextSize);
    
    return(std::equal(sha256HashResult.get(), sha256HashResult.get() + sha256CheckSumSize,
                      sha256CheckSum.get()));
}

std::string CheckPassword::getDecryptedText() const
{
    return(std::string(originalText.get(), originalText.get() + cipherTextSize));
}
