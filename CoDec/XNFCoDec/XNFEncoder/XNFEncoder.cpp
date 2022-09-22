//
// Created by gian on 31/08/22.
//

#ifndef EVOCOM_XNFCODEC_ENCODER_CPP
#define EVOCOM_XNFCODEC_ENCODER_CPP

#include "XNFEncoder.hpp"
#include "../../../XNFEvolver/XNFEvolver.hpp"

namespace GC {

/**
         * Encodes the ConjLength and the amount of conjunctions in the XNF into the file.
         * The ConjLength is assumed to be in [1, 32], this is encoded using 5 bits (we actually write ConjLength-1 to be in [0, 31].
         * The amount of conjunctions is assumed to be in [0, 2^ConjLength) , so it's encoded using ConjLength bits
         * @tparam ConjLength the amount of digits considered in each conjunction
         * @param xnf the xnf to be encoded
         * @param writer structure used to write into a file a bit at a time.
         */
    template<size_t ConjLength>
    void XNFEncoder<ConjLength>::encodeXNFDimensions(const XNF<ConjLength> &xnf) {
        LOG_CALLER();

        //ASSERT_IN_BOUNDS(ConjLength, 1, 1ULL << XNFCoDec::bitSizeForConjLength);
        //writer.writeAmount(XNFCoDec::getEncodedConjLength(ConjLength), XNFCoDec::bitSizeForConjLength);
        //the conjunction length is always in [1, 32], so it can fit in 5 bits if we subtract 1
        std::size_t amountOfGenes = xnf.getGeneCount();
        //ASSERT_IN_BOUNDS(amountOfGenes, 0, (1ULL << ConjLength) - 1);
        //though it is possible to get more genes, any form of XNF improvement prevents the size from being more than 2^ConjLength
        writer.writeAmount(amountOfGenes, XNFCoDec::getBitSizeOfConjAmount(ConjLength));
        LOG("Encoded the dimensions ConjLength =", ConjLength, ", amountofGenes =", amountOfGenes);
    }

/**
 * Encodes a single Conjunction into the file.
 * The conjunction will be interpreted as a base 3 number, where 0=0, 1=1, *=2.
 * This integer has at most size 3^ConjLength - 1, which occupies log2(3^ConjLength -1) bits.
 * Therefore, the integer will be written using log2(3)*ConjLength bits
 * @tparam ConjLength
 * @param conj  the Conjunction to be encoded
 * @param writer
 */
    template<size_t ConjLength>
    void XNFEncoder<ConjLength>::encodeSingleConjunction(const Conjunction<ConjLength> &conj) {
        const std::size_t sizeForEncoding = XNFCoDec::getBitSizeForEncodingSingleConj(ConjLength);
        writer.writeAmount(conj.getBase3Representation(), sizeForEncoding);
        LOG("Encoded single conjunction", conj.to_string(), "as the int ", conj.getBase3Representation(), "(takes ",
            sizeForEncoding, "bits)");
    }


    template <size_t ConjLength>
    void XNFEncoder<ConjLength>::encodeSingleDelta(const Delta& delta) {
        auto mask = delta.getDifferenceMask();
        auto skipPowerIndex = Utilities::encodeSkipPower(mask);

        LOG_NONEWLINE("mask =", Utilities::asBinaryString(mask, ConjLength), ", SP =", skipPowerIndex, ", ");
        std::vector<bool> rice = Utilities::getRiceEncoded(skipPowerIndex);
        for (auto b: rice) {
            LOG_NONEWLINE(b);
        }
        LOG("");

        writer.writeRiceEncoded(skipPowerIndex);
        writer.writeVector(delta.getDifferenceDirections());
    }

/**
 * Encodes the conjunctions in the XNF, assuming that the dimensions have already been encoded.
 * This is done via delta encoding, that is we write the first conj "raw", and all of the others as their difference
 * as their difference from the previous conjunction.
 * To minimise the size of each delta, the conjs are sorted in "ternary gray".
 * NOTE: assumes that there is at least one conjunction
 * @tparam ConjLength
 * @param xnf
 * @param writer
 */
    template<size_t ConjLength>
    void XNFEncoder<ConjLength>::encodeConjunctionsInXNF(const XNF<ConjLength> &xnf) {

        std::vector<Conjunction<ConjLength>> sorted = xnf.getSortedToReduceMaskDifference();
        LOG("The sorted conjunctions are");
        Utilities::LOG_container_to_string(sorted);
        encodeSingleConjunction(sorted[0]);
        for (size_t i = 0; i < sorted.size() - 1; i++) {
            auto delta = Delta(sorted[i], sorted[i + 1]);
            encodeSingleDelta(delta);
        }
    }

/**
 * Encodes an XNF onto the given stream.
 * @tparam ConjLength
 * @param xnf
 * @param outStream
 */
    template<size_t ConjLength>
    void XNFEncoder<ConjLength>::encodeXNF(const XNF<ConjLength> &xnf) {
        encodeXNFDimensions(xnf);
        if (xnf.size() != 0)
            encodeConjunctionsInXNF(xnf);
        LOG("Wrote ", writer.getWrittenBufferAmount(), "buffers onto stream");
    }

    template<size_t ConjLength>
    typename XNFEncoder<ConjLength>::XNFGroup
    XNFEncoder<ConjLength>::compressStreamIntoXNFGroup(std::ifstream &inStream, const size_t amountOfUnits) {
        //ASSERT(amountOfUnits <= (1ULL<<ConjLength));
        XNFGroup xnfGroup;
        auto addConjInXNF = [&](const Conjunction<ConjLength> &conj, size_t bitPosition) {
            xnfGroup[bitPosition].activateGene(conj);
        };
        auto addIntAsConjToAppropriateXNFs = [&](const size_t positionOfUnit, const StreamUnit unit) {
            Conjunction<ConjLength> positionAsConj;
            positionAsConj.constructFromBinary(positionOfUnit);
            for (size_t whichXNF = 0; whichXNF < XNFsPerStreamUnit; whichXNF++) {
                if (Utilities::getBitFromRight(unit, whichXNF))
                    addConjInXNF(positionAsConj, whichXNF);
            }
        };

        for (int positionInStream = 0; positionInStream < amountOfUnits; positionInStream++) {
            StreamUnit unit;
            inStream.read(reinterpret_cast<char *>(&unit), sizeof(unit));
            addIntAsConjToAppropriateXNFs(positionInStream, unit);
        }

        std::for_each(xnfGroup.begin(), xnfGroup.end(), [&](XNF<ConjLength>& xnf){ improveXNF(xnf, amountOfUnits);});
        return xnfGroup;
    }

    template<size_t ConjLength>
    void XNFEncoder<ConjLength>::improveXNF(XNF<ConjLength> &xnf, const size_t fileSize) {
        if (xnf.isEmptyXNF()) {
            LOG("XNF was empty, it will be left as is");
            return;
        }
        unsigned int populationSize = 12;
        unsigned int selectionSize = populationSize / 2;
        bool isElitist = true;

        GC::XNFEvolver<ConjLength> e(xnf, populationSize, selectionSize, isElitist);
        e.evolvePopulationForGenerations(12);
        auto newXNF = e.getBestOfPopulation();
        xnf = newXNF;
        LOG("the result after normal evolution is:", xnf.to_string());
        xnf.transform_bounded_greedy(fileSize);
        LOG("after bounded transform, it is", xnf.to_string());
    }

    template<size_t ConjLength>
    void XNFEncoder<ConjLength>::encodeXNFGroup(const XNFEncoder<ConjLength>::XNFGroup &xnfGroup) {
        std::for_each(xnfGroup.begin(), xnfGroup.end(), [&](const XNF<ConjLength> &xnf) { encodeXNF(xnf); });
    }

    template<size_t ConjLength>
    void XNFEncoder<ConjLength>::encodeStream(std::ifstream &inStream, size_t amountOfUnits) {
        LOG_CALLER();
        XNFGroup xnfGroup = compressStreamIntoXNFGroup(inStream, amountOfUnits);
        LOG("The XNFs to be encoded are:");
        Utilities::LOG_container_to_string(xnfGroup);
        encodeXNFGroup(xnfGroup);
    }
}


#endif //EVOCOM_XNFCODEC_ENCODER_CPP
