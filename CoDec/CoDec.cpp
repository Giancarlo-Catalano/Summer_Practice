//
// Created by gian on 28/06/22.
//

#include <fstream>
#include "CoDec.hpp"
#include "../Utilities/utilities.hpp"


namespace GC {

    using filePath = std::string;

    CoDec::CoDec() {
        LOG_CALLER();
    }

    filePath CoDec::compress(const filePath& fileName) {
        LOG("Called CoDec::compress(", fileName, ")");
        std::ifstream inputStream(fileName, std::ios::in);

        if (!inputStream) {
            LOG("There was an error when opening the file", fileName);
            throw std::invalid_argument("File could not be opened");
        }

        filePath compressedFileName = replaceFileExtension(fileName, getCompressedFileExtension());
        LOG("The compressed file name is", compressedFileName);
        std::ofstream outputStream(compressedFileName, std::ios::binary);

        if (!outputStream) {
            LOG("There was an error creating the file", compressedFileName);
            throw std::invalid_argument("Output compression file could not be created");
        }

        compressStream(inputStream, outputStream);
        return compressedFileName;
    }

    filePath CoDec::decompress(const filePath &fileName) {
        LOG("Called CoDec::decompress(", fileName, ")");
        std::ifstream inputStream(fileName, std::ios::in);

        if (!inputStream) {
            LOG("There was an error when opening the file", fileName);
            throw std::invalid_argument("File could not be opened");
        }

        filePath originalFileExtension = getOriginalFileExtension(inputStream);

        filePath decompressedFileName = replaceFileExtension(fileName, originalFileExtension);
        std::ofstream outputStream(decompressedFileName, std::ios::binary);

        if (!outputStream) {
            LOG("There was an error creating the file", decompressedFileName);
            throw std::invalid_argument("Output compression file could not be created");
        }

        decompressStream(inputStream, outputStream);
        return decompressedFileName;

    }

    filePath CoDec::replaceFileExtension(const filePath& originalFileName, const filePath& newExtension) {
        auto removeFileExtension = [](const filePath& original) {
            auto pos_lastPeriod = original.find_last_of(".");
            return original.substr(0, pos_lastPeriod);
        };

        return removeFileExtension(originalFileName)+"."+newExtension;
    }

    filePath CoDec::getOriginalFileExtension(const std::ifstream& inputStream) {
        //for testing purposes, this will always return txt
        return "txt";  //TODO return something more useful
    }

    void CoDec::compressStream(std::ifstream &inputStream, std::ofstream &outputStream) {
        LOG("Called CoDec::compressStream");
            //copies the original file
            while (inputStream && outputStream){
                char fromInputChar = inputStream.get();
                if (inputStream.eof())
                    return;
                outputStream.put(fromInputChar);
            }
    }

    void CoDec::decompressStream(std::ifstream &inputStream, std::ofstream &outputStream) {
        LOG("Called CoDec::decompressStream, ignore the next log line");
        compressStream(inputStream, outputStream);
    }

    filePath CoDec::getCompressedFileExtension() {
        return "defaultCodecCompressionExtension";
    }

};