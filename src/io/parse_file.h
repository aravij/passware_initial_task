#ifndef PARSE_FILE_H
#define PARSE_FILE_H

#include <boost/smart_ptr/shared_array.hpp>
#include <cstddef>
#include <string>

struct ParsedFile
{
    // We use unsigned char to make it easier to use libgcrypt functions with current data.
    boost::shared_array<unsigned char> initialValue, content, shaCheckSum;
    std::size_t initialValueSize, contentSize, shaCheckSumSize;
};

/**
 * Split given file by three fields. First and third are with fixed size and second with flexible size between them.
 */
ParsedFile parseFile(const std::string &fileName, std::size_t initialValueSize, std::size_t shaCheckSumSize);

#endif
