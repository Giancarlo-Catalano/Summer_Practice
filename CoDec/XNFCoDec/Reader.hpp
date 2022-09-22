//
// Created by gian on 01/09/22.
//

#ifndef EVOCOM_READER_HPP
#define EVOCOM_READER_HPP

#include <utility>
#include <fstream>
#include "XNFCoDec.hpp"

namespace GC {
    struct Reader {

        using BitBufferHolder = char;
        static const std::size_t sizeOfBuffer = sizeof(BitBufferHolder) * 8;


        BitBufferHolder bitBuffer = 0;
        std::size_t readSoFar = 0;
        std::ifstream &inStream;
        size_t bitSizeOfBuffer = sizeof(BitBufferHolder) * 8;

        Reader(std::ifstream &_inStream) : inStream(_inStream) { requestNew(); };

        void requestNew() {
            inStream.read(&bitBuffer, sizeof(bitBuffer));
            readSoFar = 0;
        }

        bool readBit_unsafe() {
            bool result = (bitBuffer >> (bitSizeOfBuffer - 1 - readSoFar)) & 1;
            readSoFar++;
            return result;
        }

        bool readBit() {
            if (readSoFar >= bitSizeOfBuffer)
                requestNew();
            return readBit_unsafe();
        }

        size_t readAmount(const size_t amountOfBits) {
            size_t result = 0;
            for (size_t i = 0; i < amountOfBits; i++) {
                result <<= 1;
                result |= readBit();
            }
            return result;
        }

        std::vector<bool> readVector(const size_t amount) {
            std::vector<bool> result;
            Utilities::repeat(amount, [&]() {result.push_back(readBit());});
            return result;
        }

        size_t readUnary() {
            size_t result = 0;
            while (!readBit())
                result++;
            return result;
        }


        size_t readRiceEncoded() {
            size_t bitSizeRead = readUnary();
            size_t bitSize = (bitSizeRead+1)*2;
            auto getOffsetOfBitSize = [&](const size_t bs) {
                auto pow4 = [&](auto n) {return 1LL<<(n*2);};
                auto sumPow4 = [&](auto n) {return (pow4(n)-1)/3;};
                return sumPow4(bitSize/2)-1;   //this is effectively bitSizeRead..
            };
            size_t contents = readAmount(bitSize);
            return contents+getOffsetOfBitSize(bitSize);
        }
    };
} //end of namespace GC

#endif //EVOCOM_READER_HPP
