#ifndef GC_UTILITIES_CPP
#define GC_UTILITIES_CPP

#include <string>
#include <fstream>
#include "utilities.hpp"
#include <cerrno>
#include <cstring>


std::string indent(const std::string& input) {
    std::string result = "\t";
    for (char c: input) {
        result += c;
        if (c =='\n') result += '\t';
    }
    return result;
}


std::string getErrorMessage() {
    //uses errno
    return std::strerror(errno);
}

std::ifstream openFileForInput(std::string filePath) {
    std::ifstream inputStream(filePath, std::ifstream::binary);

    if (!inputStream)
        LOG("There was an error opening the file (for reading)! ", getErrorMessage());
    return inputStream;
}

std::ofstream openFileForOutput(std::string filePath) {
    std::ofstream outputStream(filePath, std::ios::in);

    if (!outputStream)
        LOG("There was an error opening the file (for writing)! ", getErrorMessage());

    return outputStream;
}

std::string charToString(char c) {
    if (c >= ' ' && c <= '~')
        return std::string(1, c);
    else {
     switch (c) {
         case '\n' : return "\\n";
         case '\r' : return "\\r";
         case '\t' : return "\\t";
         default: return "[?]";
     }
    }
}


void dumpFile(std::string fileName, std::string mode) {
    LOG("called dumpFile in mode =", mode);
    auto fileStream = openFileForInput(fileName);
    if (!fileStream) return;

    bool successfulDump = true;

    /**
     * this is very weird because I made it when I was still learning C++...
     * For every mode, there is a struct that will be used to read from the file.
     * The struct needs:
     *          A constructor that takes an array of bytes and uses them to construct itself
     *          A to_string method so that the struct can be printed to the console
     *
     * When reading from the stream, it uses sizeOf the struct to detect how many bytes need to be read for the ctor
     *     */

    if (mode == "chars") {
        struct SimpleChar {
            char theChar;
            SimpleChar(unsigned char bytes[1]) { theChar = bytes[0]; }
            std::string to_string() { return charToString(theChar);}
        };
        successfulDump = dumpStream<SimpleChar>(fileStream);
    }
    else if (mode == "binary") {
        struct BinaryChar {
            char byte;
            BinaryChar(unsigned char bytes[1]) { byte = bytes[0]; }
            std::string to_string() {return "["+Utilities::asBinaryString(byte, 8)+"]";}
        };
        successfulDump = dumpStream<BinaryChar>(fileStream);
    }
    else if (mode == "RL") {
        struct RLItem {
            char letter;
            char amount;

            RLItem(unsigned char bytes[2]) { letter = bytes[0]; amount = bytes[1];}
            std::string to_string() { return "{" + charToString(letter) + ", " + std::to_string(static_cast<int>(amount)) + "}";             }
        };
        successfulDump = dumpStream<RLItem>(fileStream);
    }
    else LOG_NOSPACES("Mode \"", mode, "\" was not recognised");
    if (!successfulDump) LOG("Could not read the file completely...");
}



namespace Utilities {
    std::string doubleToString(double d, unsigned int _precision) {
        std::ostringstream oss;
        oss.precision(_precision);
        oss<<d;
        return oss.str();
    }

    uint64_t parallelBitDeposit(uint64_t src, uint64_t mask) {
        LOG_NOSPACES("Called parallelBitDeposit(src, mask)");
    }

    void repeat(const size_t amount, const std::function<void()>& operation) {
        for (size_t i=0;i<amount;i++)
            operation();
    }

    size_t log2_ceil(const size_t input) { //TODO verify this is correct
        if (input == 0)
            return 0;
        if (input == 1)
            return 0;
        return floor_log2(input-1)+1;
    }

    size_t getFileSize(const std::string& fileName) {
        std::ifstream in(fileName, std::ifstream::ate | std::ifstream::binary);
        return in.tellg();
    }


    //i !=0


    size_t getPositionOfLeftmostSetBit(const size_t i) {
        size_t pos = sizeof(decltype(i))*8-1;
        for (size_t tester = 1ULL<<pos; tester != 0; tester>>=1) {
            if (tester & i) return pos;
            pos--;
        }
    };

    size_t floor_log2(const size_t input) {  //this can definetly be optimised
        size_t pos = sizeof(decltype(input))*8-1;
        for (size_t tester = 1ULL<<pos; tester != 0; tester>>=1) {
            if (tester & input) return pos;
            pos--;
        }
    }

    size_t encodeSkipPower(const size_t input){
        //only works for values which have popcount >= 2
        return input-floor_log2(input)-2;
    };

    size_t decodeSkipPower(const size_t input) {
        return input+ floor_log2(input+2+ floor_log2(input+2))+2;
    }

    std::vector<bool> getRiceEncoded(const size_t x) {
        auto getUnaryCode = [&](const size_t n) {
            std::vector<bool> result(n);
            result.push_back(1);
            return result;
        };
        auto getFutureBitLength = [&](const size_t n) {
            auto log4 = [&](const size_t x) { return floor_log2(x)/2; };
            return log4((x+2)*3)*2;
        };

        auto getLastDigits = [&](const size_t n, const size_t amount) {
            std::vector<bool> result;
            for (int i=amount-1;i>=0;i--)
                result.push_back(getBitFromRight(n, i));
            return result;
        };

        auto getOffset = [&](const size_t bitSize) {
            //return sum of powers of 4
            auto pow4 = [&](auto n) {return 1LL<<(n*2);};
            auto sumPow4 = [&](auto n) {return (pow4(n)-1)/3;};
            return sumPow4(bitSize/2)-1;
        };

        size_t bitLength = getFutureBitLength(x);
        std::vector<bool> result = getUnaryCode((bitLength/2)-1);
        std::vector<bool> contents = getLastDigits(x-getOffset(bitLength), bitLength);
        result.insert(result.end(), contents.begin(), contents.end());

       // LOG_NONEWLINE("for x =", x, ", bitlength =", bitLength, ", offset =", getOffset(bitLength), ", ");
        return result;
    }



    size_t getRiceDecoded(const std::vector<bool> &encoded) {
        size_t pos = 0;
        size_t unaryValue = 0;
        while (!encoded[pos++])
            unaryValue++;

        size_t bitsToRead = unaryValue*2+2;
        auto getOffset = [&](const size_t bitSize) {
            //return sum of powers of 4
            auto pow4 = [&](auto n) {return 1LL<<(n*2);};
            auto sumPow4 = [&](auto n) {return (pow4(n)-1)/3;};
            return sumPow4(bitSize/2)-1;
        };

        size_t temp = 0;
        for (int i=0;i<bitsToRead;i++) {
            temp <<= 1;
            temp |= encoded[pos];
            pos++;
        }
        return getOffset(bitsToRead)+temp;
    }


};



//TODO: make utility to remove a file


#endif /* GC_UTILITIES_CPP */