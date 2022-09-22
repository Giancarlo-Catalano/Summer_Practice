//
// Created by gian on 01/09/22.
//

#ifndef EVOCOM_WRITER_HPP
#define EVOCOM_WRITER_HPP

#include <utility>
#include <fstream>
#include "XNFCoDec.hpp"

#include <utility>
#include <fstream>
#include "XNFCoDec.hpp"


namespace GC {
    struct Writer {

        using BitBufferHolder = char;
        static const std::size_t sizeOfBuffer = sizeof(BitBufferHolder) * 8;

        BitBufferHolder bitBuffer = 0;
        std::size_t occupied = 0;
        std::ofstream &outStream;

        size_t writtenBuffers = 0;

        Writer(std::ofstream &_outStream) : outStream(_outStream), occupied(0) {
        };

        void reset() {
            bitBuffer = 0;
            occupied = 0;
        }

        void pushBit_unsafe(const bool b) {
            //bits are inserted from the right
            bitBuffer <<= 1;
            bitBuffer |= b;
            occupied++;
        }

        void pushBit(const bool b) {
            if (occupied == sizeOfBuffer) {
                outStream.put(bitBuffer);
                writtenBuffers++;
                reset();
            }
            pushBit_unsafe(b);
        }

        void writeAmount(std::size_t amount, std::size_t amountOfBits) {
            if (amountOfBits > 0) {
                for (int i = amountOfBits - 1; i >= 0; i--)
                    pushBit((amount >> i) & 1);
            }
            else {
                LOG("WARNING: called writeAmount(amount=", amount, ", amountOfBits=", amountOfBits);
            }
        }

        void forceLast() {
            if (occupied != 0) {
                outStream.put(bitBuffer << (sizeOfBuffer - occupied - 1));
                writtenBuffers++;
            }

        }

        size_t getWrittenBufferAmount() {
            return writtenBuffers;
        }

        void writeVector(const std::vector<bool>& vec) {
            std::for_each(vec.begin(), vec.end(), [&](const bool b){pushBit(b);});
        }

        void writeUnary(const size_t input) {
            Utilities::repeat(input, [&](){ pushBit(0);});
            pushBit(1);
        }

        void writeRiceEncoded(size_t input) {
            auto getFutureBitLength = [&](const size_t n) {
                auto log4 = [&](const size_t x) { return Utilities::floor_log2(x)/2; };
                return log4((n+2)*3)*2;
            };

            auto getOffset = [&](const size_t bitSize) {
                //return sum of powers of 4
                auto pow4 = [&](auto n) {return 1LL<<(n*2);};
                auto sumPow4 = [&](auto n) {return (pow4(n)-1)/3;};
                return sumPow4(bitSize/2)-1;
            };

            size_t bitLength = getFutureBitLength(input);
            writeUnary((bitLength/2)-1);
            writeAmount(input-getOffset(bitLength), bitLength);
        }
    };

}

#endif //EVOCOM_WRITER_HPP
