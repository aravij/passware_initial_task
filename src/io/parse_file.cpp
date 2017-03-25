#include "parse_file.h"

#include <fstream>
#include <sstream>

#include <boost/filesystem.hpp>

#include "../util/make_shared_array.h"

ParsedFile parseFile(const std::string& fileName, std::size_t initialValueSize, std::size_t shaCheckSumSize)
{
    // Boost file_size function automatically check if provided file name points to a real existing file
    std::size_t fileSize = boost::filesystem::file_size(fileName);
    
    // Provided file consist of three fields, while two of them have fixed size, third occupied the rest of the file.
    // We have to be sure, that the size of that third field (content) has size greater than zero.
    if(fileSize <= initialValueSize + shaCheckSumSize)
    {
        std::stringstream errorMessage;
        errorMessage << "The given file " << fileName << " must have size greater than "
                     << initialValueSize + shaCheckSumSize  << " bytes"
                     << " to be able to store initial value (" << initialValueSize << " bytes) and SHA256 check sum ("
                     << shaCheckSumSize << " bytes) fields in addition to ciphertext";
        throw std::runtime_error(errorMessage.str());
    }
    
    std::ifstream fileToParse;
    // Setting up exception to be thrown.
    fileToParse.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fileToParse.open(fileName, std::ios_base::binary);
    
    ParsedFile parsedFile;
    
    // Reading each field one by one and assigning it to parsedFile structure.
    
    parsedFile.initialValueSize = initialValueSize;
    parsedFile.initialValue = make_shared_array<unsigned char>(parsedFile.initialValueSize);
    fileToParse.read(reinterpret_cast<char *>(parsedFile.initialValue.get()), parsedFile.initialValueSize);
    
    parsedFile.contentSize = fileSize - initialValueSize - shaCheckSumSize;
    parsedFile.content = make_shared_array<unsigned char>(parsedFile.contentSize);
    fileToParse.read(reinterpret_cast<char *>(parsedFile.content.get()), parsedFile.contentSize);
    
    parsedFile.shaCheckSumSize = shaCheckSumSize;
    parsedFile.shaCheckSum = make_shared_array<unsigned char>(parsedFile.shaCheckSumSize);
    fileToParse.read(reinterpret_cast<char *>(parsedFile.shaCheckSum.get()), parsedFile.shaCheckSumSize);
    
    return(parsedFile);
}


