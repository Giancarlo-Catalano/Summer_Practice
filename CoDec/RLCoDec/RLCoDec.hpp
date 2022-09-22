//
// Created by gian on 30/06/22.
//

#include "../CoDec.hpp"

#ifndef TREE_RLCODEC_HPP
#define TREE_RLCODEC_HPP

namespace GC {

    class RLCoDec : public CoDec {

    private:

    public:
        RLCoDec();

        virtual std::string getCompressedFileExtension() override {return "RepetitionLengthCompression";};

        virtual void compressStream(std::ifstream &inputStream, std::ofstream &outputStream) override;

        virtual void decompressStream(std::ifstream &inputStream, std::ofstream &outputStream) override;

    }; // GC
}
#endif //TREE_RLCODEC_HPP
