//
// Created by gian on 15/08/22.
//

#include "XNFCoDec.hpp"
#include "XNFEncoder/XNFEncoder.hpp"
#include "XNFDecoder/XNFDecoder.hpp"
#include "Writer.hpp"
#include "Reader.hpp"

namespace GC {

    void XNFCoDec::compressFile(const std::string& originalFileName, const std::string& compressedFileName) {
        LOG("called XNFCoDec.compressFile(", originalFileName, ",", compressedFileName);
        size_t originalFileSize = Utilities::getFileSize(originalFileName);
        size_t amountOfUnitsInFile = originalFileSize / sizeof(StreamUnit);
        size_t ConjLengthForFile = Utilities::log2_ceil(amountOfUnitsInFile);
        LOG("the file size is", originalFileSize);
        LOG("the calculated ConjLength is ", ConjLengthForFile);

        std::ifstream inStream(originalFileName, std::ios_base::binary);
        std::ofstream outStream(compressedFileName, std::ios_base::binary);

        Writer writer(outStream);
        writer.writeAmount(amountOfUnitsInFile, bitSizeOfAmountOfUnitsInFile);

#define HANDLE_COMPRESSION_CONJLENGTH(M_ConjLength) \
            case M_ConjLength : XNFEncoder<M_ConjLength>(writer).encodeStream(inStream, amountOfUnitsInFile); break;

        switch (ConjLengthForFile) {
            GC_MAP_FROM_1_TO_32(HANDLE_COMPRESSION_CONJLENGTH);
            default: {LOG("The file size exceeds 4GB, cannot be compressed!"); ERROR_NOT_IMPLEMENTED();}
        }

        writer.forceLast();

        LOG("closing the streams");
        inStream.close();
        outStream.close();
    }

    void XNFCoDec::decompressFile(const std::string &compressedFileName, const std::string &decompressedFileName) {
        LOG("called XNFCoDec.decompressFile(", compressedFileName, ",", decompressedFileName);
        std::ifstream inStream(compressedFileName, std::ios_base::binary);
        std::ofstream outStream(decompressedFileName, std::ios_base::binary);

        Reader reader(inStream);
        size_t amountOfUnitsInFile = reader.readAmount(bitSizeOfAmountOfUnitsInFile);
        LOG("The original file contained", amountOfUnitsInFile, "units");
        size_t ConjLengthForFile = Utilities::log2_ceil(amountOfUnitsInFile);
        LOG("This corresponds to ConjLength =", ConjLengthForFile);

#define HANDLE_DECOMPRESSION_CONJLENGTH(M_ConjLength) \
        case M_ConjLength : XNFDecoder<M_ConjLength>(reader).decodeStream(outStream, amountOfUnitsInFile); break;

        switch (ConjLengthForFile) {
            GC_MAP_FROM_1_TO_32(HANDLE_DECOMPRESSION_CONJLENGTH);
            default: {
                LOG("The compressed file size is not supported, it would be bigger than 4GB!");
                ERROR_NOT_IMPLEMENTED();
            }
        }

        LOG("Closing the streams");
        inStream.close();
        outStream.close();
    }


}
