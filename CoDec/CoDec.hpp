//
// Created by gian on 28/06/22.
//

#ifndef TREE_CODEC_HPP
#define TREE_CODEC_HPP


#include <string>
#include <ostream>
#include <istream>




namespace GC {
    class CoDec {

        using filePath = std::string;
        
    public:
        CoDec();
        virtual filePath compress(const filePath &fileName);
        virtual filePath decompress(const filePath &fileName);

    protected:

        virtual filePath getCompressedFileExtension();

        virtual void compressStream(std::ifstream &inputStream, std::ofstream &outputStream);
        virtual void decompressStream(std::ifstream &inputStream, std::ofstream &outputStream);


    private:
        virtual filePath replaceFileExtension(const filePath& originalFileName, const filePath& newExtension);
        virtual filePath getOriginalFileExtension(const std::ifstream& inputStream);
    };


};

#endif //TREE_CODEC_HPP
