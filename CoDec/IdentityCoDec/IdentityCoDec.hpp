//
// Created by gian on 30/06/22.
//

#include "../CoDec.hpp"

#ifndef TREE_TESTCODEC_HPP
#define TREE_TESTCODEC_HPP

namespace GC {

    class IdentityCoDec : public CoDec {

    private:

    public:
        IdentityCoDec();

        virtual std::string getCompressedFileExtension() override {return "indentityCompression";};

        virtual void compressStream(std::ifstream &inputStream, std::ofstream &outputStream) override;

        virtual void decompressStream(std::ifstream &inputStream, std::ofstream &outputStream) override;

    }; // GC
}
#endif //TREE_TESTCODEC_HPP
