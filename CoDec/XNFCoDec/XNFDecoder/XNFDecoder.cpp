//
// Created by gian on 31/08/22.
//

#ifndef EVOCOM_XNFCODEC_DECODER_XNF_CPP
#define EVOCOM_XNFCODEC_DECODER_XNF_CPP
#include <utility>
#include "XNFDecoder.hpp"
#include "../../CoDecTechniques/ArithmeticCoder/ArithmeticCoder.hpp"
#include <memory>
#include <math.h>

namespace GC {

    template <size_t ConjLength>
    std::pair<size_t, size_t> XNFDecoder<ConjLength>::decodeXNFDimensions() {
        LOG_CALLER();
        //size_t ConjLength_read = XNFCoDec::getDecodedConjLength(reader.readAmount(XNFCoDec::bitSizeForConjLength));
        size_t amountOfConjs = reader.readAmount(XNFCoDec::getBitSizeOfConjAmount(ConjLength));
        LOG_VAR(amountOfConjs);
        return {ConjLength, amountOfConjs};
    }



    template <size_t ConjLength>
    Conjunction<ConjLength> XNFDecoder<ConjLength>::decodeSingleConjunction() {
        const size_t bitSizeOfSingleConj = XNFCoDec::getBitSizeForEncodingSingleConj(ConjLength);
        Conjunction<ConjLength> conj;
        conj.setFromBase3Representation(reader.readAmount(bitSizeOfSingleConj));
        return conj;
    }

    template <size_t ConjLength>
    std::vector<typename XNFDecoder<ConjLength>::PopCount>
    XNFDecoder<ConjLength>::decodePopCounts(const size_t amountOfDeltas) {
        PopCountHuffmanCoder huffmanCoder = XNFCoDec::getPopCountHuffmanCoder(ConjLength);
        std::vector<PopCount> popCounts;
        PopCountHuffmanCoder::Decoder decoder = huffmanCoder.getDecoder([&](PopCount pc){popCounts.push_back(pc);},
                                              [&](){return reader.readBit();});
        LOG("about to decode ", amountOfDeltas, " symbols");
        decoder.decodeAmountOfSymbols(amountOfDeltas);
        return popCounts;
    }


    //TODO this is not working properly
    template <size_t ConjLength>
    std::vector<XNFCoDec::DifferenceMask>
    XNFDecoder<ConjLength>::decodeDifferenceMasks(const std::vector<PopCount>& popCounts) {
        LOG_CALLER();
        FastBitPermutationIndex<ConjLength> fbpi;
        std::vector<ArrangementRange> ranges(popCounts.size());
        std::transform(popCounts.begin(), popCounts.end(), ranges.begin(),
                       [&](const PopCount pc) {return fbpi.getEnumerationRange(ConjLength, pc);});

        auto fromArrangementIndexToMask = [&](const ArrangementIndex ai,const PopCount pc) {return fbpi.decodeFromEnumeration(ai, ConjLength, pc);};
        using AC = ArrangementCoder;
        typename AC::Decoder decoder = AC::getDecoder([&](const ArrangementIndex index){/*not used currently*/;}, //handler is to just store the mask in some way
                                             [&](){return reader.readAmount(sizeof(ArithmeticCoderUnit)*8);}); //requests new encoded units //TODO remove the handler because it's not used
        std::vector<ArrangementIndex>indexes = decoder.decodeFromRanges(ranges);
        LOG("the indexes are:");
        Utilities::LOG_container(indexes);
        LOG("\n");
        std::vector<DifferenceMask> masks(popCounts.size());
        std::transform(indexes.begin(), indexes.end(), popCounts.begin(), masks.begin(), fromArrangementIndexToMask);
        return masks;
    }


    template <size_t ConjLength>
    std::vector<std::vector<XNFCoDec::Direction>> XNFDecoder<ConjLength>::decodeDirections(const std::vector<PopCount>& popCounts) {
        std::vector<std::vector<Direction>> result;
        auto decodeForPopCount = [&](const PopCount& pc) {
            std::vector<Direction> partialResult;
            Utilities::repeat(pc, [&](){partialResult.push_back(reader.readBit());});
            result.push_back(partialResult);
        };
        std::for_each(popCounts.begin(), popCounts.end(), decodeForPopCount);
        return result;
    }


    template <size_t ConjLength>
    std::vector<typename XNFDecoder<ConjLength>::Delta> XNFDecoder<ConjLength>::decodeDeltas(const size_t amountOfDeltas) {
        return Utilities::vectorOfRepeatedApplication<Delta>([&](){return decodeSingleDelta();}, amountOfDeltas);
    }


    /**
     * Reads all of the encoded Conjs, and returns an XNF containing all of them
     * @tparam ConjLength
     * @param xnf
     * @param reader
     */
    template <size_t ConjLength>
    XNF<ConjLength> XNFDecoder<ConjLength>::decodeConjunctionsIntoXNF(const size_t amountOfConjs) {
        XNF<ConjLength> xnf;
        if (amountOfConjs == 0)
            return xnf;
        Conjunction<ConjLength> firstConj = decodeSingleConjunction();
        LOG("Decoded the first conjunction: ", firstConj.to_string());
        const size_t amountOfDeltas = amountOfConjs-1; //TODO check that in the encoder the amount of deltas is calculated properly
        std::vector<Delta> deltas = decodeDeltas(amountOfDeltas);


        Conjunction<ConjLength> currentConj = firstConj;

        xnf.activateGene(currentConj);
        auto calculateNewConjAndAppend = [&](const Delta& delta) {
            currentConj = delta.apply(currentConj);
            xnf.activateGene(currentConj);
        };
        std::for_each(deltas.begin(), deltas.end(), calculateNewConjAndAppend);

        LOG("the decoded conjunctions are:", xnf.to_string());
        return xnf;
    }

    template <size_t ConjLength>
    XNF<ConjLength> XNFDecoder<ConjLength>::decodeXNF() {
        LOG_CALLER();
        std::pair<size_t, size_t> xnfDimensions = decodeXNFDimensions();
        size_t ConjLength_read = xnfDimensions.first;
        size_t amountOfConjs = xnfDimensions.second;
        LOG_VAR(ConjLength_read);
        LOG_VAR(amountOfConjs);
        return decodeConjunctionsIntoXNF(amountOfConjs);
    }

    /**
     * NOTE: this consumes the XNFGroup
     * @tparam ConjLength
     * @param xnfGroup
     * @param amountOfUnits
     * @return
     */
    template <size_t ConjLength>
    std::vector<typename XNFDecoder<ConjLength>::StreamUnit>
    XNFDecoder<ConjLength>::unpackXNFGroupIntoVector(XNFGroup& xnfGroup, const size_t amountOfUnits) {
        LOG_CALLER();
        std::vector<StreamUnit> result(amountOfUnits); //should be all zeros
        for (auto item: result)
            ASSERT_EQUALS(item, 0);

        for (int i=0;i<xnfGroup.size();i++)
            xnfGroup[i].unpackIntoVector(result, i);
        return result;
    }

    template<size_t ConjLength>
    void XNFDecoder<ConjLength>::decodeStream(std::ofstream &outStream, size_t amountOfUnits) {
        LOG_CALLER();
        XNFGroup extractedXNFGroup = interpretStreamAsXNFGroup();
        LOG("Extracted the XNFGroups, they are:");
        Utilities::LOG_container_to_string(extractedXNFGroup);
        unpackXNFGroupIntoStream(extractedXNFGroup, outStream, amountOfUnits);
    }

    template<size_t ConjLength>
    typename XNFDecoder<ConjLength>::XNFGroup
    XNFDecoder<ConjLength>::interpretStreamAsXNFGroup() {
        XNFGroup xnfGroup;
        size_t nextXNFIndex = 0;
        auto extractNextXNF = [&]() {
            xnfGroup[nextXNFIndex++] = decodeXNF();
        };

        Utilities::repeat(XNFsPerStreamUnit, extractNextXNF);
        return xnfGroup;
    }

    template<size_t ConjLength>
    void
    XNFDecoder<ConjLength>::unpackXNFGroupIntoStream(XNFDecoder::XNFGroup &xnfGroup, std::ofstream &outStream,
                                                     const size_t amountOfUnits) {

        LOG_CALLER();

        std::for_each(xnfGroup.begin(), xnfGroup.end(), [](XNF<ConjLength>& xnf){xnf.transform_remove_X();});
        for (size_t i = 0;i<amountOfUnits;i++) {
            Conjunction<ConjLength> iAsConj; iAsConj.constructFromBinary(i);
            StreamUnit buffer = 0;
            for (int pos = 0;pos<xnfGroup.size();pos++) {
                buffer |= (xnfGroup[pos].contains(iAsConj))<<pos;
            }
            outStream.put(buffer);
        }
    }

    template<size_t ConjLength>
    typename XNFDecoder<ConjLength>::Delta XNFDecoder<ConjLength>::decodeSingleDelta() {
        size_t skipPowerIndex = reader.readRiceEncoded();
        size_t mask = Utilities::decodeSkipPower(skipPowerIndex);
        size_t popCount = Utilities::getPopCount(mask);
        std::vector<bool> directions = reader.readVector(popCount);
        return Delta(mask, directions);
    }

}

#endif //EVOCOM_XNFCODEC_DECODER_XNF_CPP
