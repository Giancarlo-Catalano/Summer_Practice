//
// Created by gian on 01/09/22.
//

#ifndef EVOCOM_XNFDECODER_HPP
#define EVOCOM_XNFDECODER_HPP

#include "../XNFCoDec.hpp"
#include "../Reader.hpp"

namespace GC {
    template <size_t ConjLength>
    class XNFDecoder {

        ALIAS_FROM_XNFCODEC(PopCount);
        using ArrangementIndex = size_t;
        using ArrangementRange = size_t;
        ALIAS_FROM_XNFCODEC(Direction);
        ALIAS_FROM_XNFCODEC(PopCountHuffmanCoder);
        ALIAS_FROM_XNFCODEC(DifferenceMask);
        TEMPLATED_ALIAS_FROM_XNFCODEC(ArrangementCoder);
        TEMPLATED_ALIAS_FROM_XNFCODEC(ArithmeticCoderUnit);
        using Delta = typename Conjunction<ConjLength>::ConjunctionDifference;
        TEMPLATED_ALIAS_FROM_XNFCODEC(XNFGroup);
        static const size_t XNFsPerStreamUnit = XNFCoDec::XNFsPerStreamUnit;
        ALIAS_FROM_XNFCODEC(StreamUnit);

    private:
        Reader reader;

    public:
        XNFDecoder(Reader& reader) : reader(reader) {};
        void decodeStream(std::ofstream &outStream, size_t amountOfUnits);

    private:


        std::vector<StreamUnit> unpackXNFGroupIntoVector(XNFGroup& xnfGroup, const size_t amountOfUnits);
        void unpackXNFGroupIntoStream(XNFGroup& xnfGroup, std::ofstream& outStream, const size_t amountOfUnits);

        XNF<ConjLength> decodeXNF();

        std::pair<size_t, size_t> decodeXNFDimensions();
        Conjunction<ConjLength> decodeSingleConjunction();
        XNF<ConjLength> decodeConjunctionsIntoXNF(const size_t amountOfConjs);
        std::vector<Delta> decodeDeltas(const size_t amountOfConjs);

        Delta decodeSingleDelta();

        std::vector<typename XNFDecoder<ConjLength>::PopCount> decodePopCounts(const size_t amountOfDeltas);

        std::vector<DifferenceMask> decodeDifferenceMasks(const std::vector<PopCount> &popCounts);

        std::vector<std::vector<XNFCoDec::Direction>> decodeDirections(const std::vector<PopCount> &popCounts);

        XNFGroup interpretStreamAsXNFGroup();
    };
}

#include "XNFDecoder.cpp"


#endif //EVOCOM_XNFDECODER_HPP
