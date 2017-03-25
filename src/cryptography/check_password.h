#ifndef CHECK_PASSWORD_H
#define CHECK_PASSWORD_H

#include <gcrypt.h>

#include <string>
#include <cstddef>

#include "../io/parse_file.h"

#include <boost/smart_ptr/shared_array.hpp>
#include <boost/smart_ptr/scoped_array.hpp>

/**
 * Class CheckPassword is a wrapper for C-style functions from libgcrypt.
 */
class CheckPassword
{
private:
    gcry_cipher_hd_t cipher;
   
    // In this class we store several buffers and their sizes.
    // Buffers from parsed file we store as shared, because we don't own them.
    // Internal buffers stored in scoped array and own them exclusively.
    // Those internal buffers are used in isPasswordAcceptable function only and could be
    // local variables, but allocating memory once will speed up its execution.
    // NOTICE: cipherText and originalText has same size cipherTextSize and
    //         sha256CheckSum and sha256HashResult has same size sha256CheckSumSize.
    const std::size_t cipherTextSize, sha256CheckSumSize, initialValueSize, md5CheckSumSize, tripleDESKeySize;
    const boost::shared_array<unsigned char> cipherText, sha256CheckSum, initialValue;
    boost::scoped_array<unsigned char> originalText, md5HashResult, tripleDesKey, sha256HashResult;
public:
    explicit CheckPassword(const ParsedFile &cypherFile);
    CheckPassword(const CheckPassword &otherCipher);
    ~CheckPassword();
    
    CheckPassword operator=(const CheckPassword &checkPassword) = delete;

    /**
     * Checking password is done in following way:
     * 1) Applying MD5 to password.
     * 2) Decrypting cipherText with 3DES (EDE2/DED2) algorithm in CBC mode using MD5 result as key 
     *    and initial value for CBC from parsedFile, passed in construction.
     * 3) Applying SHA256 to original text and comparing it with sha256CheckSum from parsedFile.
     */
    bool isPasswordAcceptable(const std::string &password);
    
    /**
     * Returns content from internal buffer with decrypted text.
     */
    std::string getDecryptedText() const;
};

#endif
