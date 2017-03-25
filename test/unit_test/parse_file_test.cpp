#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "io/parse_file.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include <string>
#include <random>
#include <algorithm>

struct TmpFileFixture
{
    boost::filesystem::path tmpFilePath;
    boost::filesystem::ofstream tmpFile;
    
    TmpFileFixture(const std::string &dirForTmpFile = "."):
        tmpFilePath(boost::filesystem::unique_path(boost::filesystem::path(dirForTmpFile) /
                                               "tempFile_for_unit_testing-%%%%%%")),
        tmpFile(tmpFilePath, boost::filesystem::ofstream::binary)
    {}
    
    ~TmpFileFixture()
    {
        if(tmpFile.is_open())
        {
            tmpFile.close();
        }
        
        boost::filesystem::remove(tmpFilePath);
    }
};

bool operator==(const ParsedFile &lhs, const ParsedFile &rhs)
{
    return(lhs.initialValueSize == rhs.initialValueSize &&
           std::equal(lhs.initialValue.get(), lhs.initialValue.get() + lhs.initialValueSize, rhs.initialValue.get()) &&
           lhs.contentSize == rhs.contentSize &&
           std::equal(lhs.content.get(), lhs.content.get() + lhs.contentSize, rhs.content.get()) &&
           lhs.shaCheckSumSize == rhs.shaCheckSumSize &&
           std::equal(lhs.shaCheckSum.get(), lhs.shaCheckSum.get() + lhs.shaCheckSumSize, rhs.shaCheckSum.get()));
}

std::ostream &operator<<(std::ostream &stream, const ParsedFile &parsedFile)
{
    stream << std::endl;
    
    stream << "initial value (" << parsedFile.initialValueSize << " bytes):" << std::endl;
    std::copy(parsedFile.initialValue.get(), parsedFile.initialValue.get() + parsedFile.initialValueSize,
              std::ostream_iterator<unsigned char>(stream, ""));
    stream << std::endl;
    
    stream << "content (" << parsedFile.contentSize << " bytes):" << std::endl;
    std::copy(parsedFile.content.get(), parsedFile.content.get() + parsedFile.contentSize,
              std::ostream_iterator<unsigned char>(stream, ""));
    
    stream << "SHA256 check sum (" << parsedFile.shaCheckSumSize << " bytes):" << std::endl;
    std::copy(parsedFile.shaCheckSum.get(), parsedFile.shaCheckSum.get() + parsedFile.shaCheckSumSize,
              std::ostream_iterator<unsigned char>(stream, ""));
    
    return(stream);
}

BOOST_DATA_TEST_CASE_F(TmpFileFixture, parsed_file_test,
                       boost::unit_test_framework::data::random(10, 100) ^
                       boost::unit_test_framework::data::random(10, 100) ^
                       boost::unit_test_framework::data::random(10, 100) ^
                       boost::unit_test_framework::data::xrange(5),
                       FirstFieldSize, SecondFieldSize, ThirdFieldSize, _)
{
    ParsedFile parsedFile;
    
    parsedFile.initialValueSize = FirstFieldSize;
    parsedFile.initialValue.reset(new unsigned char[parsedFile.initialValueSize]);
    tmpFile.write(reinterpret_cast<char *>(parsedFile.initialValue.get()), parsedFile.initialValueSize);
    
    parsedFile.contentSize = SecondFieldSize;
    parsedFile.content.reset(new unsigned char[parsedFile.contentSize]);
    tmpFile.write(reinterpret_cast<char *>(parsedFile.content.get()), parsedFile.contentSize);
    
    parsedFile.shaCheckSumSize = ThirdFieldSize;
    parsedFile.shaCheckSum.reset(new unsigned char[parsedFile.shaCheckSumSize]);
    tmpFile.write(reinterpret_cast<char *>(parsedFile.shaCheckSum.get()), parsedFile.shaCheckSumSize);
    
    tmpFile.close();
    
    BOOST_TEST(parsedFile == parseFile(tmpFilePath.string(), FirstFieldSize, ThirdFieldSize));
}
