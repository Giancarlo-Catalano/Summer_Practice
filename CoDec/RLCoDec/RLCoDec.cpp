//
// Created by gian on 30/06/22.
//

#include <fstream>
#include "RLCoDec.hpp"
#include "../../Utilities/utilities.hpp"

namespace GC {
    RLCoDec::RLCoDec() {
        LOG_CALLER();
    }

    using Symbol = char;

    void writeSymbol(Symbol s, std::ofstream& outputStream) {
        outputStream << s;
    }

    Symbol readSymbol(std::ifstream& inputStream) {
        return inputStream.get();
    }

    void writeAmount(size_t amount, std::ofstream& outputStream) {
        ASSERT(amount > 0);
        outputStream << static_cast<char>(amount);
    }

    size_t readAmount(std::ifstream& inputStream) {
        char tempChar = inputStream.get();
        return static_cast<size_t>(tempChar);
    }

    void writeSymbolRepeat(Symbol s, size_t amount, std::ofstream& outputStream) {
        for (size_t i=0;i<amount;i++)
            writeSymbol(s, outputStream);
    }

    void RLCoDec::compressStream(std::ifstream &inputStream, std::ofstream &outputStream) {
        LOG("called RLCoDec::compressStream");

        Symbol prevSymbol = 0x0;
        size_t currentCount = 0;

        auto resetRL = [&](char newSymbol) {
            LOG_NOSPACES("#resetRL(", newSymbol, ")");
            prevSymbol = newSymbol;
            currentCount = 1;
        };

        auto writeRL = [&](){
            if (currentCount < 1)
                return;
            LOG_NOSPACES("writeRL, prevSymbol=", prevSymbol, ", currentCount=", currentCount);
            writeSymbol(prevSymbol, outputStream);
            writeAmount(currentCount, outputStream);
        };

        while (inputStream && outputStream) {
            Symbol inputSymbol = readSymbol(inputStream);
            if (inputStream.eof()) {
                writeRL();
                return;
            }

            if (prevSymbol == inputSymbol) {
                currentCount++;
                LOG("encountered the same character");
            }
            else {
                writeRL();
                resetRL(inputSymbol);
            }
        }

        LOG("Something went wrong with the inputStream or outputStream");
    }

    void RLCoDec::decompressStream(std::ifstream &inputStream, std::ofstream &outputStream) {
        LOG("called RLCoDec::decompressStream");
        while (inputStream && outputStream) {
            Symbol symbol = readSymbol(inputStream);
            size_t amount = readAmount(inputStream);
            if (inputStream.eof()) {
                LOG("End of file has been reached");
                return;
            }
            LOG("Received Symbol =", symbol, ", run =", amount);
            writeSymbolRepeat(symbol, amount, outputStream);
        }

        LOG("Something went wrong with the inputStream or outputStream");
    }


} // GC