//
// Created by gian on 15/08/22.
//

#ifndef EVOCOM_XNFENCODER_HPP
#define EVOCOM_XNFENCODER_HPP


#include "../XNFCoDec.hpp"
#include <vector>
#include "../../../Utilities/utilities.hpp"
#include "../../../XNF/XNF.hpp"
#include "../Writer.hpp"
#include <fstream>
#include <memory>



namespace GC {

    template <size_t ConjLength>
    class XNFEncoder {

        ALIAS_FROM_XNFCODEC(PopCount);
        ALIAS_FROM_XNFCODEC(ArrangementIndex);
        ALIAS_FROM_XNFCODEC(ArrangementRange);
        ALIAS_FROM_XNFCODEC(Direction);
        ALIAS_FROM_XNFCODEC(PopCountHuffmanCoder);
        TEMPLATED_ALIAS_FROM_XNFCODEC(ArrangementCoder);
        TEMPLATED_ALIAS_FROM_XNFCODEC(ArithmeticCoderUnit);
        static const size_t XNFsPerStreamUnit = XNFCoDec::XNFsPerStreamUnit;
        TEMPLATED_ALIAS_FROM_XNFCODEC(XNFGroup);
        using Delta = typename Conjunction<ConjLength>::ConjunctionDifference;
        ALIAS_FROM_XNFCODEC(StreamUnit);


    public:
        XNFEncoder(Writer& writer) : writer(writer) {
            LOG("Called constructor for XNFEncoder<", ConjLength, ">");};

        void encodeStream(std::ifstream& inStream, size_t amountOfUnits);


    private:

        Writer writer;


        void encodeXNF(const XNF<ConjLength>& xnf);
        void encodeSingleDelta(const Delta& delta);

        void encodeXNFGroup(const XNFGroup& xnfGroup);

        void encodeXNFDimensions(const XNF<ConjLength> &xnf);

        static XNFGroup compressStreamIntoXNFGroup(std::ifstream& inStream, const size_t amountOfUnits);

        static void improveXNF(XNF<ConjLength>& xnf, const size_t fileSize);

        void encodeSingleConjunction(const Conjunction<ConjLength> &conj);

        void encodeDeltas(const std::vector<Delta> &deltas);

        void encodeDeltaPopCounts(const std::vector<PopCount> &popCounts);

        void encodeDeltaArrangements(const std::vector<std::pair<ArrangementRange, ArrangementIndex>> &rangesAndArrangements);

        void encodeDeltaDirections(const std::vector<Direction> &directions);

        void encodeConjunctionsInXNF(const XNF<ConjLength> &xnf);
    };




}
#include "XNFEncoder.cpp"





#endif //EVOCOM_XNFENCODER_HPP
