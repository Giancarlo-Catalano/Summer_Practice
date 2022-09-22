//
// Created by gian on 30/06/22.
//

#include <fstream>
#include "IdentityCoDec.hpp"
#include "../../Utilities/utilities.hpp"

namespace GC {
    IdentityCoDec::IdentityCoDec() {
        LOG_CALLER();
    }

    void IdentityCoDec::compressStream(std::ifstream &inputStream, std::ofstream &outputStream) {
        LOG("called IdentityCoDec::compressStream");
        int counter = 0;
        while (inputStream && outputStream) {
            counter++;
            char inputChar = inputStream.get();
            if (inputStream.eof()) {
                LOG("Wrote", counter, "character");
                return;
            }

            outputStream.put(inputChar);
        }


        LOG("Something went wrong with the inputStream or outputStream");
    }

    void IdentityCoDec::decompressStream(std::ifstream &inputStream, std::ofstream &outputStream) {
        LOG("called IdentityCoDec::decompressStream");
        int counter = 0;
        while (inputStream && outputStream) {
            counter++;
            char inputChar = inputStream.get();
            if (inputStream.eof()) {
                LOG("Wrote", counter, "character");
                return;
            }

            outputStream.put(inputChar);
        }

        LOG("Something went wrong with the inputStream or outputStream");
    }


} // GC