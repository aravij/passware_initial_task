#include <boost/program_options.hpp>
#include <boost/range/irange.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include "io/parse_file.h"

#include "cryptography/check_password.h"

#include "util/variadic_iter_join.h"
#include "util/cartesian_range_power.h"
#include "util/gcry_exception.h"

#include <iostream>
#include <string>
#include <cstdlib>
#include <exception>

#include <gcrypt.h>

#include <omp.h>

constexpr char nextChar(char letter)
{
    return(letter + 1);
}

int main(int argc, char **argv)
{
/*----------start of libgcrypt initialization section----------*/

    // Version check should be the very first call because it
    // makes sure that important subsystems are initialized.
    if(!gcry_check_version (GCRYPT_VERSION))
    {
        std::cerr << "ERROR: Can't initialize libgcrypt. Version mismatch." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Disable secure memory.
    try
    {
        processGcryError(gcry_control(GCRYCTL_DISABLE_SECMEM, 0));
    }
    catch(const GcryException &gcryException)
    {
        std::cerr << "WARNING: Disabling secure memory failed." << std::endl;
    }

    // Tell Libgcrypt that initialization has completed.
    try
    {
        processGcryError(gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0));
    }
    catch(const GcryException &gcryException)
    {
        std::cerr << "ERROR: Can't initialize libgcrypt." << std::endl;
        std::cerr << gcryException.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
/*----------end of libgcrypt initialization section----------*/

/*----------start of command line options parsing section----------*/
    
    // Variables to store command line options.
    std::string cipherFileName;
    bool printDecryptedText;
    
    try{
        // We separate help options and main options to allow users to specify help options 
        // without specifying REQUIRED main options.
        // This is achieved by parsing command line options two times. First time just for help options
        // and second time for all others.
        boost::program_options::options_description helpOptions("Help options");
        helpOptions.add_options()("help,h", "Prints this help message and exit.");
        
        // A main option is a filename with ciphertext.
        // Its structure described in below in option description field.
        // Another option is a bool flag, whether or not a decrypted text should be printed.
        boost::program_options::options_description mainOptions("Main options");
        mainOptions.add_options()
            ("print-decrypted,p", boost::program_options::bool_switch(&printDecryptedText)->default_value(false),
             "Prints for all acceptable password decrypted text.")
            ("CIPHERFILE", boost::program_options::value<std::string>(&cipherFileName)->required(),
             "Can be passed a first positional argument.\n"
             "A binary file in the following format:\n"
             "  1. \t8 bytes field with initial value for CBC mode.\n"
             "  2. \tCiphertext, encrypted by 3DES(EDE2) algorithm with keys got from MD5 from the password.\n"
             "  3. \t32 bytes of SHA256 of original text.");
        
        // We compile all types of options into a single one for easy printing help message.
        boost::program_options::options_description allOptions(
            "Usage: test_problem [-h|--help] | [-p|--print-decrypted] CIPHERFILE\n"
            "Guess the password of CIPHERFILE. The password guessed is in the form [a-zA-Z0-9]{3}.\n\n"
            "All options");
        allOptions.add(mainOptions);
        allOptions.add(helpOptions);
        
        // Option CIPHERFILE can be (and even should be) used as positional.
        boost::program_options::positional_options_description positionalMainOptions;
        positionalMainOptions.add("CIPHERFILE", 1);
        
        // First time parsing, we look only for help options, allowing unregistered options.
        boost::program_options::variables_map parsedOptions;
        boost::program_options::store(boost::program_options::command_line_parser(argc, argv)
                                      .options(helpOptions).allow_unregistered().run(),
                                      parsedOptions);
        boost::program_options::notify(parsedOptions);
        
        // Printing help message and exiting.
        if(parsedOptions.count("help"))
        {
            std::cout << allOptions << std::endl;
            std::exit(EXIT_SUCCESS);
        }
        
        // Second time we look only for main options
        // (if help options have been found we should already exited the program).
        boost::program_options::store(boost::program_options::command_line_parser(argc, argv)
                                      .options(mainOptions).positional(positionalMainOptions).run(),
                                      parsedOptions);
        boost::program_options::notify(parsedOptions);
    }
    catch(const boost::program_options::error &parsingProgramOptionsError)
    {
        std::cerr << "ERROR: Failed parsing command line arguments." << std::endl;
        std::cerr << parsingProgramOptionsError.what() << "." << std::endl;
        std::exit(EXIT_FAILURE);
    }
/*----------end of command line options parsing section----------*/

    // Reading and parsing provided CIPHERFILE.
    const std::size_t initialValueSize = 8, shaCheckSumSize = 32;
    ParsedFile parsedFile;
    try
    {
        parsedFile = parseFile(cipherFileName, initialValueSize, shaCheckSumSize);
    }
    catch(const std::exception &error)
    {
        std::cerr << "ERROR: Failed parsing given " << cipherFileName << " file." << std::endl;
        std::cerr << error.what() << "." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    
    // Constructing allowed chars by joining following three ranges, we get [a-zA-Z0-9].
    // NOTICE: We use nextChar function to have end including range.
    //         This function increase char by 1, so applying it to the last char '\xf7' in ASCII chart will result
    //         -128, because of the overflow, and it will probably break boost::irange,
    //         as first argument will be greater than the second one.
    auto allowedCharsForPasswordRange = join(boost::irange('a', nextChar('z')),
                                             boost::irange('A', nextChar('Z')),
                                             boost::irange('0', nextChar('9')));
    
    // Entering parallel section. Here we go with a single thread through foreach loop.
    // We cant use omp for directive, because we iterate through forward traversal range.
    // To exploit multithreading, we use tasks, for evaluating loop body.
    #pragma omp parallel
    #pragma omp single nowait
    {
        // Setting up cryptography algorithms. In the following for loop we will create tasks, to be evaluated by
        // threads. Each thread need to use object of class CheckPassword, but such objects are not thread safe.
        // To overcome such difficulty we create an collection with number of objects equaled to number of threads.
        // Each thread use its identity as an index to get its object and work with it exclusively.
        // NOTICE: Objects of CheckPassword class are not assignable, due to their inner structure.
        //         They all store scoped_arrays and some constants. To allow to use multiple objects of that class
        //         we have to store it in a collection, but it is problematic to store not assignable objects.
        //         To overpass such difficulty we allocate all needed objects in a heap and store pointers to them in 
        //         a special vector: boost::ptr_vector.
        boost::ptr_vector<CheckPassword> ciphersForMultithread;
        try
        {
            for(int threadIndex = 0; threadIndex < omp_get_max_threads(); ++threadIndex)
            {
                ciphersForMultithread.push_back(new CheckPassword(parsedFile));
            }
        }
        catch(const GcryException &gcryException)
        {
            std::cerr << "ERROR: Failed in initializing libgcrypt." << std::endl;
            std::cerr << gcryException.what() << std::endl;
            std::exit(EXIT_FAILURE);
        }
        
        // Iterating through all possible passwords of fixed size.
        const int passwordLength = 3;
       
        for(auto passwordInVector: CartesianPowerRange(allowedCharsForPasswordRange, passwordLength))
        {
            #pragma omp task
            {
                // Transform our picked password into a string and, if it acceptable, print it.
                std::string password(passwordInVector.begin(), passwordInVector.end());
                // Using thread identity to get a reference to CheckPassword object
                // and use it exclusively (each thread works with different objects)
                auto &checkPassword = ciphersForMultithread[omp_get_thread_num()];
                bool isPasswordAcceptable;
                try
                {
                    isPasswordAcceptable = checkPassword.isPasswordAcceptable(password);
                }
                catch(const GcryException &gcryException)
                {
                    // If something went wring in cryptography algorithms, we just skip this picked password
                    // and print this warning.
                    std::cerr << "WARNING: Processing password \"" << password 
                              << "\" some exceptions appeared." << std::endl;
                    std::cerr << "         Skipping current password!" << std::endl;
                    std::cerr << gcryException.what() << std::endl;
                    isPasswordAcceptable = false;
                }
                
                // We print all acceptable passwords, even if there would be multiple of them.
                // Such may happened, if different decrypted messages have SHA256 collision.
                if(isPasswordAcceptable)
                {
                    // Putting printing into critical section will make output clear, and wouldn't mess up
                    // output of different threads.
                    #pragma omp critical
                    {
                        std::cout << password << std::endl;
                        
                        // Print decrypted text if needed
                        if(printDecryptedText)
                        {
                            std::cout << checkPassword.getDecryptedText() << std::endl;
                        }
                    }
                }
            }
        }
        
        #pragma omp taskwait
    }

    return 0;
}
