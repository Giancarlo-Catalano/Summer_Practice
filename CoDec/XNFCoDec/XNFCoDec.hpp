//
// Created by gian on 01/09/22.
//

#ifndef EVOCOM_XNFCODEC_HPP
#define EVOCOM_XNFCODEC_HPP

#include <utility>
#include "../../Utilities/utilities.hpp"
#include <fstream>
#include <math.h>
#include <vector>
#include <array>
#include "../../Utilities/FastBitPermutationIndex.hpp"
#include "../CoDecTechniques/CoDecUtilities/CoDecUtilities.hpp"
#include "../CoDecTechniques/HuffmanCoder/HuffmanCoder.hpp"
#include "../CoDecTechniques/ArithmeticCoder/ArithmeticCoder.hpp"
#include "../../XNF/XNF.hpp"




namespace GC {

    class XNFCoDec {

#define ALIAS_FROM_XNFCODEC(alias) using alias = XNFCoDec::alias
#define TEMPLATED_ALIAS_FROM_XNFCODEC(alias) using alias = XNFCoDec::alias<ConjLength>

    public:
        using PopCount = unsigned short int;  //TODO this could be aliases from Conjunction<ConjLength>::ConjunctionDifference, but the templates make it extremely messy
        using ArrangementIndex = std::size_t;  //for simplicity
        using ArrangementRange = std::size_t;  //for simplicity
        using Direction = bool;
        using DifferenceMask = uint64_t;

        using PopCountHuffmanCoder = HuffmanCoder<PopCount, double>;


        using StreamUnit = uint8_t;
        static const size_t XNFsPerStreamUnit = sizeof(StreamUnit)*8;
        template <size_t ConjLength>
        using XNFGroup = std::array<XNF<ConjLength>, XNFsPerStreamUnit>;

        template <size_t ConjLength>
        using ArithmeticCoderUnit = T_IF(ConjLength <= 5, uint8_t,
                                 T_IF(ConjLength <= 10, uint16_t, T_IF(ConjLength <= 18, uint64_t, uint64_t)));

        template <size_t ConjLength>
        using ArrangementCoder = ArithmeticCoder<ArithmeticCoderUnit<ConjLength>>;


        static const size_t bitSizeOfAmountOfUnitsInFile = 64;
        static const size_t bitSizeForConjLength = 5;
        static const size_t getBitSizeForEncodingSingleConj(size_t ConjLength) {
            const double log2of3 = 1.5849625; //TODO: should this be increased slightly in case of precision errors?
            return ceil(static_cast<double>(ConjLength) * log2of3);
        }

        static const size_t getEncodedConjLength(const size_t ConjLength){return ConjLength-1;}
        static const size_t getDecodedConjLength(const size_t encodedCL){return encodedCL+1;}

        static const size_t getBitSizeOfConjAmount(size_t ConjLength) {return ConjLength;}

        static const std::vector<std::pair<PopCount, double>> getProbabilitiesOfPopCounts(const size_t ConjLength) {
            std::vector<std::pair<PopCount, double>> result;
            double currentProb = Utilities::pow_constexpr(2, ConjLength);
            for (std::size_t i = 2; i <= ConjLength; i++) {
                result.push_back(
                        {i, ConjLength - i});//result.push_back({i, currentProb}); //TODO decide the distribution
                currentProb /= 1.5;
            }
            return result;
        };

        static const PopCountHuffmanCoder getPopCountHuffmanCoder(const size_t ConjLength) {
            return PopCountHuffmanCoder(getProbabilitiesOfPopCounts(ConjLength));
        }

        static const size_t bitSizeForAmountOfUnitsInFile = 32;

        static void compressFile(const std::string& originalFileName, const std::string& compressedFileName);

        static void decompressFile(const std::string& compressedFileName, const std::string& decompressedFileName);



    };
}

#endif //EVOCOM_XNFCODEC_HPP
