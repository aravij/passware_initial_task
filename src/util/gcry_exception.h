#ifndef GCRY_EXCEPTION_H
#define GCRY_EXCEPTION_H

#include <exception>
#include <sstream>
#include <string>

#include <gcrypt.h>

/**
 * Class GcryException is intended to handle errors from libgcrypt library.
 * By taking as constructor parameter gcry_error_t it creates a error message translating
 * given error code into a error description and error source.
 */
class GcryException: public std::exception
{
private:
    std::string message;
public:
    GcryException(const gcry_error_t &gcryError)
    {
        std::stringstream errorMessage;
        
        // Following function are not thread safe.
        // At the same time the following code does not intended to be executed very often.
        // So usage of critical section shouldn't greatly impact on speed degradation.
        #pragma omp critical
        {
            errorMessage << "REASON: " << gcry_strerror(gcryError) << std::endl;
            errorMessage << "SOURCE: " << gcry_strsource(gcryError) << std::endl;
        }
        message = errorMessage.str();
    }
    
    virtual const char *what() const noexcept override
    {
        return(message.c_str());
    }
};

/**
 * processGcryError is taking a gcry_error_t as a parameter and if it is not ok 
 * throws an GcryException.
 * Such behavior is useful, because each time we get an error code from gcry functions
 * we have to check is there was actually an error, and if it was, throw an exception.
 */
inline void processGcryError(const gcry_error_t &gcryError)
{
    if(gcryError)
    {
        throw GcryException(gcryError);
    }
}

#endif
